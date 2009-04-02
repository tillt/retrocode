/*\
 * WaveFile.cpp
 * Copyright (C) 2004-2009, MMSGURU - written by Till Toenshoff
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
\*/
#include "stdafx.h"
#include <iostream>
#include <map>
#include <math.h>
#include <strstream>
#include <fstream>
#include "../include/Resource.h"
#include "../retroBase/Basics.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MyString.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/Adpcm.h"
#include "../retroBase/Endian.h"
#include "../retroBase/Packer.h"
#include "WaveFile.h"
#include "WaveProperty.h"

DYNIMPPROPERTY(CWaveFile,CWaveProperty)

CWaveFile::CWaveFile(void)
{
	TRACEIT2("constructing WAVE object\n");
	ZeroMemory(&m_Header,sizeof(m_Header));
	m_pcMagic="RIFF";
	m_nMagicSize=4;
	m_sFormatName=_T("Wave");
	m_sDefaultExtension=_T("wav");
	m_sFormatDescription.Load(IDS_FORMDESC_WAVE);
	m_encodingCaps.freqs[	CSampleCompatibility::supports8Bit|
							CSampleCompatibility::supports16Bit|
							CSampleCompatibility::supportsMono|
							CSampleCompatibility::supportsStereo].setRange(1,192000);
	m_encodingPara.addPara(cmdParaSwitch,paraSwitchReverseOrder);
	m_encodingPara.addPara(cmdParaSwitch,paraSwitchGoldWave);
	m_encodingPara.addPara(cmdParaSwitch,paraSwitchFact);
	m_encodingPara.addPara(cmdParaNumber,paraNumCompression);
	m_encodingPara.addPara(cmdParaString,paraStrTitle);
	m_encodingPara.addPara(cmdParaNumber,paraNumBlockSize);
	m_encodingPara.addPara(cmdParaString,paraStrArtist);
	m_encodingPara.addPara(cmdParaString,paraStrNote);
	m_encodingPara.addPara(cmdParaString,paraStrCategory);
	m_encodingPara.addPara(cmdParaString,paraStrCopyright);
	m_encodingPara.addPara(cmdParaString,paraStrDateCreated);
}

CWaveFile::~CWaveFile(void)
{
	TRACEIT2("deleting WAVE object\n");
}


/*\ 
 * <---------- CWaveFile::nReadLIST ----------> 
 * @m get the metadata from a wave file
 * --> I N <-- @p
 * istream &ar - input stream reference
 * uint32_tnListSize - size of list tag data
 * <-- OUT --> @r
 * uint32_t- number of bytes read and parsed
 \*/ 
uint32_t CWaveFile::nReadLIST(istream &ar,uint32_t nListSize)
{
	char *pcBuffer;
	uint32_t nBufferSize=1024;
	uint32_t nTag;
	uint32_t nSize;
	uint32_t nOffset=0;
	char pcDummy[5]={0,0,0,0,0};

	Unpacker up(ar,true);

	map<uint32_t,int> :: const_iterator iterChunk;
	map<uint32_t,int> mapChunkId;

	mapChunkId[nMakeID4("INAM")]=infoTitle;
	mapChunkId[nMakeID4("IART")]=infoArtist;
	mapChunkId[nMakeID4("IARL")]=infoArchiveLocation;
	mapChunkId[nMakeID4("ICMS")]=infoCommissioned;
	mapChunkId[nMakeID4("ICMT")]=infoComments;
	mapChunkId[nMakeID4("ICOP")]=infoCopyright;
	mapChunkId[nMakeID4("ICRD")]=infoDateCreated;
	mapChunkId[nMakeID4("ICRP")]=infoCropped;
	mapChunkId[nMakeID4("IDIM")]=infoDimensions;
	mapChunkId[nMakeID4("IDPI")]=infoDotsPerInch;
	mapChunkId[nMakeID4("IENG")]=infoEngineer;
	mapChunkId[nMakeID4("IGNR")]=infoCategory;
	mapChunkId[nMakeID4("IKEY")]=infoKeywords;
	mapChunkId[nMakeID4("ILGT")]=infoLightness;
	mapChunkId[nMakeID4("IMED")]=infoMedium;
	mapChunkId[nMakeID4("IPLT")]=infoPalette;
	mapChunkId[nMakeID4("IPRD")]=infoProduct;
	mapChunkId[nMakeID4("ISBJ")]=infoSubject;
	mapChunkId[nMakeID4("ISFT")]=infoSoftware;
	mapChunkId[nMakeID4("ISHP")]=infoSharpness;
	mapChunkId[nMakeID4("ISRC")]=infoSource;
	mapChunkId[nMakeID4("ISRF")]=infoSourceForm;
	mapChunkId[nMakeID4("ITCH")]=infoTechnician;
	mapChunkId[nMakeID4("DISP")]=infoSoundScheme;

	pcBuffer=(char *)calloc(nBufferSize,1);
	ZeroMemory(pcBuffer,nBufferSize);

	while(nListSize)
	{
		Log2(verbLevDebug3,"info list left (outer loop): %d\n",nListSize);
		ar.read((char *)&nTag,4);
		nOffset+=4;
		nListSize-=4;
		switch (nTag)
		{
			case MAKEID4('I','N','F','O'):
				while(nListSize)
				{
					Log2(verbLevDebug3,"info list left (inner loop): %d\n",nListSize);
					ar.read((char *)&nTag,4);
					up.read("l",&nSize);
					nOffset+=8;
					nListSize-=8;
					if (nSize > nBufferSize)
					{
						while (nSize > nBufferSize)
							nBufferSize+=nBufferSize;
						pcBuffer=(char *)realloc(pcBuffer,nBufferSize);
					}
					if ((iterChunk=mapChunkId.find(nTag)) == mapChunkId.end())
					{
						Log2(verbLevWarnings,"unknown song subtag \"%s\"\n",pcSplitID4(nTag,pcDummy));
						ar.seekg(nSize,ios_base::cur);
						nOffset+=nSize;
						nListSize-=nSize;
					}
					else
					{
						ar.read(pcBuffer,nSize);
						pcBuffer[nSize]=0;
						Log2(verbLevDebug3,"read chunk %s, data: %s\n",pcSplitID4(nTag,pcDummy),pcBuffer);
						m_strInfo[iterChunk->second]=pcBuffer;
						nOffset+=nSize;
						nListSize-=nSize;
					}
					if (nOffset & 1)
					{
						ar.seekg(1,ios_base::cur);
						nOffset++;
						nListSize--;
					}
				};
			break;
			default:
				Log2(verbLevWarnings,"expected an info tag within this list\n");
		}
	};
	free (pcBuffer);
	//delete [] pcBuffer;
	if (!m_strInfo[infoSubject].empty() && m_strInfo[infoArtist].empty())
		m_strInfo[infoArtist]=m_strInfo[infoSubject];
	return nOffset;
}

/*\ 
 * <---------- CWaveFile::ReadHeader ----------> 
 * @m read the wave file header and all other non sample information
 * --> I N <-- @p
 * istream &ar - input stream
 * WAVEHEADER *pWave - pointer to header data structure to be filled
\*/ 
void CWaveFile::ReadHeader(istream &ar) 
{ 
 	uint32_t nOffset=36;
 	uint32_t nHeaderSize=nOffset;
	WAVEHEADER *pWave=&m_Header;
	uint16_t wValue=0;
	uint32_t nValue=0;

    Unpacker up(ar, true);

	ZeroMemory(pWave,sizeof(WAVEHEADER));
 	
 	m_Adpcm.wave=NULL;
 	m_nCSChannels=0;
 	//Read the first 4 bytes, which will hold 'RIFF' 
    up.read("4b", pWave->RIFF);
 	//Read the size 
    up.read("l", &pWave->nSize);
 	//Read 'WAVE' and 'fmt' in a single procedure 
    up.read("8b", pWave->WAVEfmt);
 	//Read wFormatLength 
    up.read("l", &pWave->nFormatLength);
 	Log2(verbLevDebug3," format length %d\n",pWave->nFormatLength);
 	//Read the wFormatTag (which tells whether it is PCM wave file) 
    up.read("s", &pWave->wFormatTag);
 	Log2(verbLevDebug3," format tag %d\n",pWave->wFormatTag);
 	//Read the number of channels (1-mono, 2-stereo) 
    up.read("s", &pWave->wChannels);
 	Log2(verbLevDebug3," channels %d\n",pWave->wChannels);
 	//Read the sample rate - 11025,22050 or 44100 (if adhering to standard)
    up.read("l", &pWave->nSamplesPerSec);
	Log2(verbLevDebug3," rate %d\n",pWave->nSamplesPerSec);
 	//The average number of bytes per second 
    up.read("l", &pWave->nAvgBytesPerSec);
 	Log2(verbLevDebug3," avg byte per sec %d\n",pWave->nAvgBytesPerSec);
 	//1 is for 8bit data, 2 is for 16bit data (usually multiplied with the channel count) 
    up.read("s", &pWave->wBlockAlign);
 	Log2(verbLevDebug3," block align %d\n",pWave->wBlockAlign);
    up.read("s", &pWave->wBitsPerSample);
 	Log2(verbLevDebug3," bits per sample %d\n",pWave->wBitsPerSample);
 
	//correct invalid bit per sample value
	if (pWave->wBitsPerSample == 0x00)
	{
		if (pWave->wBlockAlign == 1)
			pWave->wBitsPerSample=8;
		if (pWave->wBlockAlign == 2)
			pWave->wBitsPerSample=16;
	}

	switch(pWave->wFormatTag)
	{
		case 0x0002:
		{
			Log2(verbLevDebug1,"sample is MS ADPCM compressed\n");
			m_Adpcm.wave=pWave;
			uint16_t wExtraSize;
			//Read the number of bytes of extra data
			up.read("s",&wExtraSize);
			Log2(verbLevDebug1,"size of extra wave data is %d\n",wExtraSize);
			//Read the number of samples per block
			up.read("s",&m_Adpcm.wSamplesPerBlock);
			Log2(verbLevDebug1,"%d samples per ADPCM block\n",m_Adpcm.wSamplesPerBlock);
			//Read the number of coefficients
			up.read("s",&m_Adpcm.wNumCoef);
			Log2(verbLevDebug1,"%d adpcm coefficients stored\n",m_Adpcm.wNumCoef);
			if (!m_Adpcm.wNumCoef)
			{
				Log2(verbLevErrors,"no adpcm coefficients stored at all\n");
				throw new CFormatException(CFormatException::formaterrInvalid,"no adpcm coefficients stored at all");
			}
			if (m_Adpcm.wNumCoef < 7)
			{
				Log2(verbLevErrors,"invalid number of adpcm coefficients stored (%d)\n",m_Adpcm.wNumCoef);
			}
			if (m_Adpcm.wNumCoef < 4096)
			{
				//alloc coefficient table memory (make sure its a valid size)
				m_Adpcm.paCoeff=new ADPCMCOEFSET[max(m_Adpcm.wNumCoef,7)];
				//preinit adpcm coefficients
				CopyMemory(m_Adpcm.paCoeff,m_codec.pnGetMSCoef(),7 * 4);
				//read stored number of coefficients
				for (unsigned int i=0;i < (unsigned int)m_Adpcm.wNumCoef*2;i+=2)
					up.read("2s",(short *)(m_Adpcm.paCoeff)+i);
			}
			else
			{
				Log2(verbLevErrors,"too many adpcm coefficients stored (%d)\n",m_Adpcm.wNumCoef);
				throw new CFormatException(CFormatException::formaterrInvalid,"too many adpcm coefficients stored");
			}
			nHeaderSize+=6 + (4 * m_Adpcm.wNumCoef);
		}
		break;
		case 0x0011:
		{
			Log2(verbLevDebug1,"sample is IMA ADPCM compressed\n");
			m_Adpcm.wave=pWave;
			uint16_t wExtraSize;
			//Read the number of bytes of extra data
			up.read("s",&wExtraSize);
			Log2(verbLevDebug1,"size of extra wave data is %d\n",wExtraSize);
			//Read the number of samples per block
			up.read("s",&m_Adpcm.wSamplesPerBlock);
			Log2(verbLevDebug1,"%d samples per ADPCM block\n",m_Adpcm.wSamplesPerBlock);
			nHeaderSize+=2 + wExtraSize;
		}
		break;
		case 0x0101:
		{
			Log2(verbLevDebug1,"sample is sagem ADPCM compressed\n");
			m_Adpcm.wave=pWave;
			uint16_t wExtraSize;
			//Read the number of bytes of extra data
			up.read("s",&wExtraSize);
			Log2(verbLevDebug1,"size of extra wave data is %d\n",wExtraSize);
			//Read the number of samples per block
			up.read("s",&m_Adpcm.wSamplesPerBlock);
			Log2(verbLevDebug1,"%d samples per ADPCM block\n",m_Adpcm.wSamplesPerBlock);
			up.read("l",&nValue);
			Log2(verbLevDebug1,"copyrighted: %08X\n",nValue);
			up.read("l",&nValue);
			nValue=0;
			Log2(verbLevDebug1,"extra 2: %08X\n",nValue);
			up.read("s",&wValue);
			Log2(verbLevDebug1,"extra 3: %04X\n",wValue);
			nHeaderSize+=2 + wExtraSize;
		}
		break;
		case 0x0000:
			Log2(verbLevDebug1,"sample is possibly encoded in an unknown format (0000h), assuming PCM\n");
		break;
		case 0x0001:
			Log2(verbLevDebug1,"sample is PCM uncompressed\n");
		break;
		default:
			/*
			if (pWave->wBitsPerSample <= 0)
			{
				Log2(verbLevErrors,"unsupported WAVE fomat (wFormatTag %04Xh)\n",pWave->wFormatTag);
				throw new CFormatException(CFormatException::formaterrIncompatibleSampleFormat,"unsupported WAVE codec");
			}
			*/
			Log2(verbLevWarnings,"unsupported WAVE fomat (wFormatTag %04Xh) - any following actions will most likely operate on false data and the result might very well be unplayable.\n",pWave->wFormatTag);
	}
	if ((pWave->nFormatLength+20) > nHeaderSize)							//unexpected header size?
	{
		Log2(verbLevWarnings,"header is bigger than expected (expected %ld bytes, got %ld bytes), skipping %ld bytes\n",pWave->nFormatLength+20,nHeaderSize,(pWave->nFormatLength+20)-nHeaderSize);
		ar.seekg((pWave->nFormatLength+20)-nHeaderSize,ios_base::cur);		//skip unwanted header data...
	}
	nOffset=ar.tellg();
	Log2(verbLevDebug2,"offset after header is %ld\n",nOffset);

	uint32_t nTag=0;
	uint32_t nSize=0;
	uint32_t nDataOffset=nOffset;
	//Now, read remaining tags (mostly "data") 
	while (nOffset+8 < m_nFileSize)
	{
		nTag=nSize=0;
		up.read("l",&nTag);
		up.read("l",&nSize);
		nOffset+=8;
		if (nSize > m_nFileSize-nOffset)
		{
			nSize=m_nFileSize-nOffset;
			Log2(verbLevWarnings,"capped illegal tag length (tag: %Xh, given size: %Xh, size left: %Xh, at offset: %Xh)\n",nTag,nSize,m_nFileSize-nOffset,nOffset);
		}
		switch (nTag)
		{
			//sample data following?
			case MAKEID4('d','a','t','a'):
				Log2(verbLevDebug3,"data tag located...\n");
				nDataOffset=(uint32_t)ar.tellg();
				m_nCSChannels=pWave->wChannels;
				m_nCSBitsPerSample=pWave->wBitsPerSample;
				m_nCSSamplesPerSecond=pWave->nSamplesPerSec;
				if (nDataOffset+nSize >= m_nFileSize)
					m_nCSSize=m_nFileSize-nDataOffset;
				else
					m_nCSSize=nSize;
				Log2(verbLevDebug3,"data size %d,filesize %d,limit %d\n",m_nCSSize,m_nFileSize,m_nCSSize+nDataOffset);
				nOffset+=m_nCSSize;
				ar.seekg(m_nCSSize,ios_base::cur);
			break;
			//metadata following?
			case MAKEID4('L','I','S','T'):
				nOffset+=nReadLIST(ar,nSize);
			break;
			//unknown tag?
			default:
				//skip it!
				ar.seekg(nSize,ios_base::cur);
				nOffset+=nSize;
		}
	};
	ar.seekg(nDataOffset,ios_base::beg);
	if (m_nCSChannels != 1 && m_nCSChannels != 2)
	{
		Log2(verbLevErrors,"invalid wave file - channel count (%d) invalid\n",m_nCSChannels);
		throw new CFormatException(CFormatException::formaterrInvalid,"channel count invalid");
	}
	/*
	if (m_nCSSize == 0)
	{
		Log2(verbLevErrors,"invalid wave file - channel count (%d) invalid\n",m_nCSChannels);
		throw new CFormatException(CFormatException::formaterrInvalid,"channel count invalid");
	}
	*/
} 

/*\ 
 * <---------- CWaveFile::sGetFormatName ----------> 
 * @m get a format name for a wave format value
 * --> I N <-- @p
 * int nFormat - format value
 * <-- OUT --> @r
 * tstring - readable string
\*/ 
tstring CWaveFile::sGetFormatName(int nFormat)
{
	map<int,tstring> mapFormatName;
	mapFormatName[0x0000]=tstring(_T("Microsoft Corporation : Unknown"));
	mapFormatName[0x0001]=tstring(_T("Microsoft Corporation : PCM"));
	mapFormatName[0x0002]=tstring(_T("Microsoft Corporation : ADPCM"));
	mapFormatName[0x0003]=tstring(_T("Microsoft Corporation : IEEE Float"));
	mapFormatName[0x0004]=tstring(_T("Compaq Computer Corp. : Vselp"));
	mapFormatName[0x0005]=tstring(_T("IBM Corporation : IBM Cvsd"));
	mapFormatName[0x0006]=tstring(_T("Microsoft Corporation : aLAW"));
	mapFormatName[0x0007]=tstring(_T("Microsoft Corporation : uLAW"));
	mapFormatName[0x0008]=tstring(_T("Microsoft Corporation : DTS"));
	mapFormatName[0x0009]=tstring(_T("Microsoft Corporation : DRM"));
	mapFormatName[0x0010]=tstring(_T("OKI : OKI ADPCM"));
	mapFormatName[0x0011]=tstring(_T("Intel Corporation : DVI ADPCM / IMA ADPCM"));
	mapFormatName[0x0012]=tstring(_T("Videologic : Mediaspace ADPCM"));
	mapFormatName[0x0013]=tstring(_T("Sierra Semiconductor Corp : Sierra ADPCM"));
	mapFormatName[0x0014]=tstring(_T("Antex Electronics Corporation : G723 ADPCM"));
	mapFormatName[0x0015]=tstring(_T("DSP Solutions, Inc. : Digistd"));
	mapFormatName[0x0016]=tstring(_T("DSP Solutions, Inc. : Digifix"));
	mapFormatName[0x0017]=tstring(_T("Dialogic Corporation : Dialogic OKI ADPCM"));
	mapFormatName[0x0018]=tstring(_T("Media Vision, Inc. : Mediavision_ADPCM"));
	mapFormatName[0x0019]=tstring(_T("Hewlett-Packard Company : Cu CoDec"));
	mapFormatName[0x0020]=tstring(_T("Yamaha Corporation of America : Yamaha ADPCM"));
	mapFormatName[0x0021]=tstring(_T("Speech Compression : Sonarc"));
	mapFormatName[0x0022]=tstring(_T("DSP Group, Inc : DSPgroup_Truespeech"));
	mapFormatName[0x0023]=tstring(_T("Echo Speech Corporation : Echosc1"));
	mapFormatName[0x0024]=tstring(_T("Virtual Music, Inc. : Audiofile AF36"));
	mapFormatName[0x0025]=tstring(_T("Audio Processing Technology : Aptx"));
	mapFormatName[0x0026]=tstring(_T("Virtual Music, Inc. : Audiofile AF10"));
	mapFormatName[0x0027]=tstring(_T("Aculab plc : Prosody_1612"));
	mapFormatName[0x0028]=tstring(_T("Merging Technologies S.A. : Lrc"));
	mapFormatName[0x0030]=tstring(_T("Dolby Laboratories : Dolby AC2"));
	mapFormatName[0x0031]=tstring(_T("Microsoft Corporation : GSM610"));
	mapFormatName[0x0032]=tstring(_T("Microsoft Corporation : Msnaudio"));
	mapFormatName[0x0033]=tstring(_T("Antex Electronics Corporation : Antex ADPCMe"));
	mapFormatName[0x0034]=tstring(_T("Control Resources Limited : Control Res Vqlpc"));
	mapFormatName[0x0035]=tstring(_T("DSP Solutions, Inc. : Digireal"));
	mapFormatName[0x0036]=tstring(_T("DSP Solutions, Inc. : DigiADPCM"));
	mapFormatName[0x0037]=tstring(_T("Control Resources Limited : Control Res Cr10"));
	mapFormatName[0x0038]=tstring(_T("Natural MicroSystems : Nms_VbxADPCM"));
	mapFormatName[0x0039]=tstring(_T("Crystal Semiconductor IMA ADPCM : Cs ImaADPCM"));
	mapFormatName[0x003A]=tstring(_T("Echo Speech Corporation : Echosc3"));
	mapFormatName[0x003B]=tstring(_T("Rockwell International : Rockwell ADPCM"));
	mapFormatName[0x003C]=tstring(_T("Rockwell International : Rockwell Digitalk"));
	mapFormatName[0x003D]=tstring(_T("Xebec Multimedia Solutions Limited : Xebec"));
	mapFormatName[0x0040]=tstring(_T("Antex Electronics Corporation : G721 ADPCM"));
	mapFormatName[0x0041]=tstring(_T("Antex Electronics Corporation : G728 CELP"));
	mapFormatName[0x0042]=tstring(_T("Microsoft Corporation : Msg723"));
	mapFormatName[0x0050]=tstring(_T("Microsoft Corporation : MPEG"));
	mapFormatName[0x0052]=tstring(_T("InSoft, Inc. : RT24"));
	mapFormatName[0x0053]=tstring(_T("InSoft, Inc. : PAC"));
	mapFormatName[0x0055]=tstring(_T("ISO/MPEG : MPEG layer 3"));
	mapFormatName[0x0059]=tstring(_T("Lucent Technologies : Lucent G723"));
	mapFormatName[0x0060]=tstring(_T("Cirrus Logic : Cirrus"));
	mapFormatName[0x0061]=tstring(_T("ESS Technology : ESPCM"));
	mapFormatName[0x0062]=tstring(_T("Voxware Inc : Voxware"));
	mapFormatName[0x0063]=tstring(_T("Canopus, co., Ltd. : Canopus Atrac"));
	mapFormatName[0x0064]=tstring(_T("APICOM : G726_ADPCM"));
	mapFormatName[0x0065]=tstring(_T("APICOM : G722_ADPCM"));
	mapFormatName[0x0067]=tstring(_T("Microsoft Corporation : DSAT Display"));
	mapFormatName[0x0069]=tstring(_T("Voxware Inc : Voxware (byte aligned)"));
	mapFormatName[0x0070]=tstring(_T("Voxware Inc : Voxware AC8"));
	mapFormatName[0x0071]=tstring(_T("Voxware Inc : Voxware AC10"));
	mapFormatName[0x0072]=tstring(_T("Voxware Inc : Voxware AC16"));
	mapFormatName[0x0073]=tstring(_T("Voxware Inc : Voxware AC20"));
	mapFormatName[0x0074]=tstring(_T("Voxware Inc : Voxware RT24"));
	mapFormatName[0x0075]=tstring(_T("Voxware Inc : Voxware RT29"));
	mapFormatName[0x0076]=tstring(_T("Voxware Inc : Voxware RT29HW"));
	mapFormatName[0x0077]=tstring(_T("Voxware Inc : Voxware VR12"));
	mapFormatName[0x0078]=tstring(_T("Voxware Inc : Voxware VR18"));
	mapFormatName[0x0079]=tstring(_T("Voxware Inc : Voxware TQ40"));
	mapFormatName[0x0080]=tstring(_T("Softsound, Ltd. : Softsound"));
	mapFormatName[0x0081]=tstring(_T("Voxware Inc : Voxware TQ60"));
	mapFormatName[0x0082]=tstring(_T("Microsoft Corporation : Msrt24"));
	mapFormatName[0x0083]=tstring(_T("AT&T Labs, Inc. : G729A"));
	mapFormatName[0x0084]=tstring(_T("Motion Pixels : Mvi Mvi2"));
	mapFormatName[0x0085]=tstring(_T("DataFusion Systems (Pty) (Ltd) : Df G726"));
	mapFormatName[0x0086]=tstring(_T("DataFusion Systems (Pty) (Ltd) : Df Gsm610"));
	mapFormatName[0x0088]=tstring(_T("Iterated Systems, Inc. : ISIaudio"));
	mapFormatName[0x0089]=tstring(_T("OnLive! Technologies, Inc. : OnLive"));
	mapFormatName[0x0091]=tstring(_T("Siemens Business Communications Sys : SBC24"));
	mapFormatName[0x0092]=tstring(_T("Sonic Foundry : Dolby AC3 SpDif"));
	mapFormatName[0x0093]=tstring(_T("MediaSonic : Mediasonic G723"));
	mapFormatName[0x0094]=tstring(_T("Aculab plc : Prosody 8Kbps"));
	mapFormatName[0x0097]=tstring(_T("ZyXEL Communications, Inc. : Zyxel ADPCM"));
	mapFormatName[0x0098]=tstring(_T("Philips Speech Processing : Philips Lpcbb"));
	mapFormatName[0x0099]=tstring(_T("Studer Professional Audio AG : Packed"));
	mapFormatName[0x00A0]=tstring(_T("Malden Electronics Ltd. : Malden Phonytalk"));
	mapFormatName[0x0100]=tstring(_T("Rhetorex Inc. : Rhetorex ADPCM"));
	mapFormatName[0x0101]=tstring(_T("BeCubed Software Inc. : Irat / Sagem Mobile : IMA ADPCM"));
	mapFormatName[0x0111]=tstring(_T("Vivo Software : Vivo G723"));
	mapFormatName[0x0112]=tstring(_T("Vivo Software : Vivo Siren"));
	mapFormatName[0x0123]=tstring(_T("Digital Equipment Corporation : Digital G723"));
	mapFormatName[0x0125]=tstring(_T("Sanyo Electric Co., Ltd. : Sanyo Ld ADPCM"));
	mapFormatName[0x0130]=tstring(_T("Sipro Lab Telecom Inc. : Siprolab Aceplnet"));
	mapFormatName[0x0131]=tstring(_T("Sipro Lab Telecom Inc. : Siprolab Acelp4800"));
	mapFormatName[0x0132]=tstring(_T("Sipro Lab Telecom Inc. : Siprolab Acelp8V3"));
	mapFormatName[0x0133]=tstring(_T("Sipro Lab Telecom Inc. : Siprolab G729"));
	mapFormatName[0x0134]=tstring(_T("Sipro Lab Telecom Inc. : Siprolab G729A"));
	mapFormatName[0x0135]=tstring(_T("Sipro Lab Telecom Inc. : Siprolab Kelvin"));
	mapFormatName[0x0140]=tstring(_T("Dictaphone Corporation : G726ADPCM"));
	mapFormatName[0x0150]=tstring(_T("Qualcomm, Inc. : Qualcomm Purevoice"));
	mapFormatName[0x0151]=tstring(_T("Qualcomm, Inc. : Qualcomm Halfrate"));
	mapFormatName[0x0155]=tstring(_T("Ring Zero Systems, Inc. : Tubgsm"));
	mapFormatName[0x0160]=tstring(_T("Microsoft Corporation : WMA 1"));
	mapFormatName[0x0161]=tstring(_T("Microsoft Corporation : WMA 2"));
	mapFormatName[0x0170]=tstring(_T("Unisys Corp. : Unisys Nap ADPCM"));
	mapFormatName[0x0171]=tstring(_T("Unisys Corp. : Unisys Nap Ulaw"));
	mapFormatName[0x0172]=tstring(_T("Unisys Corp. : Unisys Nap Alaw"));
	mapFormatName[0x0173]=tstring(_T("Unisys Corp. : Unisys Nap 16K"));
	mapFormatName[0x0200]=tstring(_T("Creative Labs, Inc : Creative ADPCM"));
	mapFormatName[0x0202]=tstring(_T("Creative Labs, Inc : Creative FastSpeech8"));
	mapFormatName[0x0203]=tstring(_T("Creative Labs, Inc : Creative FastSpeech10"));
	mapFormatName[0x0210]=tstring(_T("UHER informatic GmbH : Uher ADPCM"));
	mapFormatName[0x0220]=tstring(_T("Quarterdeck Corporation : Quarterdeck"));
	mapFormatName[0x0230]=tstring(_T("I-link Worldwide : Ilink_VC"));
	mapFormatName[0x0240]=tstring(_T("Aureal Semiconductor : Raw Sport"));
	mapFormatName[0x0241]=tstring(_T("ESS Technology, Inc. : ESST AC3"));
	mapFormatName[0x0250]=tstring(_T("Interactive Products, Inc. : IPI HSX"));
	mapFormatName[0x0251]=tstring(_T("Interactive Products, Inc. : IPI RQELP"));
	mapFormatName[0x0260]=tstring(_T("Consistent Software : CS2"));
	mapFormatName[0x0270]=tstring(_T("Sony Corp. : Sony SCX"));
	mapFormatName[0x0300]=tstring(_T("Fujitsu Corp. : FM Towns Snd"));
	mapFormatName[0x0400]=tstring(_T("Brooktree Corporation : Btv Digital"));
	mapFormatName[0x0450]=tstring(_T("QDesign Corporation : Qdesign_Music"));
	mapFormatName[0x0680]=tstring(_T("AT&T Labs, Inc. : Vme_VmPCM"));
	mapFormatName[0x0681]=tstring(_T("AT&T Labs, Inc. : Tpc"));
	mapFormatName[0x1000]=tstring(_T("Ing C. Olivetti & C., S.p.A. : OliGSM"));
	mapFormatName[0x1001]=tstring(_T("Ing C. Olivetti & C., S.p.A. : OliADPCM"));
	mapFormatName[0x1002]=tstring(_T("Ing C. Olivetti & C., S.p.A. : OliCELP"));
	mapFormatName[0x1003]=tstring(_T("Ing C. Olivetti & C., S.p.A. : OliSBC"));
	mapFormatName[0x1004]=tstring(_T("Ing C. Olivetti & C., S.p.A. : OliOPR"));
	mapFormatName[0x1100]=tstring(_T("Lernout & Hauspie : LH CoDec"));
	mapFormatName[0x1400]=tstring(_T("Norris Communications, Inc. : Norris"));
	mapFormatName[0x1500]=tstring(_T("AT&T Labs, Inc. : Soundspace Musicompress"));
	mapFormatName[0x181C]=tstring(_T("VoxWare RT24 speech codec"));
	mapFormatName[0x181E]=tstring(_T("Lucent elemedia AX24000P Music codec"));
	mapFormatName[0x1971]=tstring(_T("Sonic Foundry LOSSLESS"));
	mapFormatName[0x1979]=tstring(_T("Innings Telecom Inc. ADPCM"));
	mapFormatName[0x1C07]=tstring(_T("Lucent SX8300P speech codec"));
	mapFormatName[0x1C0C]=tstring(_T("Lucent SX5363S G.723 compliant codec"));
	mapFormatName[0x1F03]=tstring(_T("CUseeMe DigiTalk (ex-Rocwell)"));
	mapFormatName[0x1FC4]=tstring(_T("NCT Soft ALF2CD ACM"));
	mapFormatName[0x2000]=tstring(_T("Dolby AC3 / FAST Multimedia AG DVM"));
	mapFormatName[0x2001]=tstring(_T("Dolby DTS (Digital Theater System)"));
	mapFormatName[0x2002]=tstring(_T("RealAudio 1 / 2 14.4"));
	mapFormatName[0x2003]=tstring(_T("RealAudio 1 / 2 28.8"));
	mapFormatName[0x2004]=tstring(_T("RealAudio G2 / 8 Cook (low bitrate)"));
	mapFormatName[0x2005]=tstring(_T("RealAudio 3 / 4 / 5 Music (DNET)"));
	mapFormatName[0x2006]=tstring(_T("RealAudio 10 AAC (RAAC)"));
	mapFormatName[0x2007]=tstring(_T("RealAudio 10 AAC+ (RACP)"));
	mapFormatName[0x2500]=tstring(_T("Reserved range to 0x2600 Microsoft"));
	mapFormatName[0x3313]=tstring(_T("makeAVIS (ffvfw fake AVI sound from AviSynth scripts)"));
	mapFormatName[0x4143]=tstring(_T("Divio MPEG-4 AAC audio"));
	mapFormatName[0x4201]=tstring(_T("Nokia adaptive multirate Nokia Mobile Phones"));
	mapFormatName[0x4243]=tstring(_T("Divio's G726 Divio, Inc."));
	mapFormatName[0x434C]=tstring(_T("LEAD Speech"));
	mapFormatName[0x564C]=tstring(_T("LEAD Vorbis"));
	mapFormatName[0x5756]=tstring(_T("WavPack Audio"));
	mapFormatName[0x674f]=tstring(_T("Ogg Vorbis (mode 1)"));
	mapFormatName[0x6750]=tstring(_T("Ogg Vorbis (mode 2)"));
	mapFormatName[0x6751]=tstring(_T("Ogg Vorbis (mode 3)"));
	mapFormatName[0x676f]=tstring(_T("Ogg Vorbis (mode 1+)"));
	mapFormatName[0x6770]=tstring(_T("Ogg Vorbis (mode 2+)"));
	mapFormatName[0x6771]=tstring(_T("Ogg Vorbis (mode 3+)"));
	mapFormatName[0x7000]=tstring(_T("3COM NBX 3Com Corporation"));
	mapFormatName[0x706D]=tstring(_T("FAAD AAC"));
	mapFormatName[0x77A1]=tstring(_T("The True Audio"));
	mapFormatName[0x7A21]=tstring(_T("GSM-AMR (CBR, no SID)"));
	mapFormatName[0x7A22]=tstring(_T("GSM-AMR (VBR, including SID)"));
	mapFormatName[0xA100]=tstring(_T("Comverse Infosys Ltd. G723 1"));
	mapFormatName[0xA101]=tstring(_T("Comverse Infosys Ltd. AVQSBC"));
	mapFormatName[0xA102]=tstring(_T("Comverse Infosys Ltd. OLDSBC"));
	mapFormatName[0xA103]=tstring(_T("Symbol Technology's G729A Symbol Technologies Canada"));
	mapFormatName[0xA104]=tstring(_T("VoiceAge AMR WB VoiceAge Corporation"));
	mapFormatName[0xA105]=tstring(_T("Ingenient Technologies Inc. G726"));
	mapFormatName[0xA106]=tstring(_T("ISO/MPEG-4 advanced audio Coding"));
	mapFormatName[0xA107]=tstring(_T("Encore Software Ltd's G726"));
	mapFormatName[0xA109]=tstring(_T("Speex ACM Codec xiph.org"));
	mapFormatName[0xC0CC]=tstring(_T("GigaLink Audio Codec"));
	mapFormatName[0xDFAC]=tstring(_T("DebugMode SonicFoundry Vegas FrameServer ACM Codec"));
	mapFormatName[0xE708]=tstring(_T("Unknown"));
	mapFormatName[0xF1AC]=tstring(_T("Free Lossless Audio Codec FLAC"));
	mapFormatName[0xFFFC]=tstring(_T("VDOwave Audio"));
	mapFormatName[0xFFFE]=tstring(_T("Extensible wave format"));
	mapFormatName[0xFFFF]=tstring(_T("In Development / Unregistered"));
	return mapFormatName[nFormat];
}

/*\ 
 * <---------- CWaveFile::bMagicHead ----------> 
 * @m check quickly if this might be a WAVE file
 * --> I N <-- @p
 * std::istream &ar - input stream object reference
 * uint32_tnSize - input file size
 * <-- OUT --> @r
 * bool - true=wave file, false=something else
\*/ 
bool CWaveFile::bMagicHead(std::istream &ar,uint32_t nSize)
{
	uint32_t i;
	unsigned char cCompare;

	if (m_nMagicSize == 0 || (int)nSize < 13)
		return false;
	for (i=0;i < m_nMagicSize;i++)
	{
		ar.read((char *)&cCompare,1);
		if (cCompare != *(m_pcMagic+i))
			return false;
	}
	ar.seekg(8,ios_base::beg);
	const char *pcSubTag={"WAVE"};
	for (i=0;i < 4;i++)
	{
		ar.read((char *)&cCompare,1);
		if (cCompare != pcSubTag[i])
			return false;
	}
	return true;
}

/*\ 
 * <---------- CWaveFile::nRenderMetadata ----------> 
 * @m write encoded metadata
 * --> I N <-- @p
 * ostream &out - output stream
 * CMobileSampleContent *pSource - source content
 * bool bRender - false=simulate and calc only the resulting size in byte
 * <-- OUT --> @r
 * uint32_t- number of bytes (potentially) used for encoded metadata
\*/ 
uint32_t CWaveFile::nRenderMetadata(ostream &out,CMobileSampleContent *pSource,bool bRender)
{
	map<int,uint32_t> :: const_iterator iterChunk;
	map<int,uint32_t> mapChunkId;
	tstring para[infoLast];

	TRACEIT2("rendering metadata\n");

	Packer pk(out,true);

	mapChunkId[infoTitle]=nMakeID4("INAM");
	mapChunkId[infoArtist]=nMakeID4("IART");
	mapChunkId[infoArchiveLocation]=nMakeID4("IARL");
	mapChunkId[infoCommissioned]=nMakeID4("ICMS");
	mapChunkId[infoComments]=nMakeID4("ICMT");
	mapChunkId[infoCopyright]=nMakeID4("ICOP");
	mapChunkId[infoDateCreated]=nMakeID4("ICRD");
	mapChunkId[infoCropped]=nMakeID4("ICRP");
	mapChunkId[infoDimensions]=nMakeID4("IDIM");
	mapChunkId[infoDotsPerInch]=nMakeID4("IDPI");
	mapChunkId[infoEngineer]=nMakeID4("IENG");
	mapChunkId[infoCategory]=nMakeID4("IGNR");
	mapChunkId[infoKeywords]=nMakeID4("IKEY");
	mapChunkId[infoLightness]=nMakeID4("ILGT");
	mapChunkId[infoMedium]=nMakeID4("IMED");
	mapChunkId[infoPalette]=nMakeID4("IPLT");
	mapChunkId[infoProduct]=nMakeID4("IPRD");
	mapChunkId[infoSubject]=nMakeID4("ISBJ");
	mapChunkId[infoSoftware]=nMakeID4("ISFT");
	mapChunkId[infoSharpness]=nMakeID4("ISHP");
	mapChunkId[infoSource]=nMakeID4("ISRC");
	mapChunkId[infoSourceForm]=nMakeID4("ISRF");
	mapChunkId[infoTechnician]=nMakeID4("ITCH");
	mapChunkId[infoSoundScheme]=nMakeID4("DISP");
	uint32_t i=0;
	uint32_t nEntrySize;
	uint32_t nLongValue;
	uint32_t nOffset=0;

	uint32_t nToBeStored=0;
	for (i=0;i < infoLast;i++)
	{
		if (!pSource->m_strInfo[i].empty() && (iterChunk=mapChunkId.find(i)) != mapChunkId.end())
			nToBeStored++;
	}
	if (nToBeStored)
	{
		if (bRender)
			out.write("LIST",4);
		nOffset+=4;
		if (bRender)
		{
			nEntrySize=nRenderMetadata(out,pSource,false)-8;	//calculate meta-data LIST size
			pk.write("l",&nEntrySize);							//write it to the file
		}		
		nOffset+=4;
		if (bRender)
			out.write("INFO",4);
		nOffset+=4;
		for (i=0;i < infoLast;i++)
		{
			if (!pSource->m_strInfo[i].empty() && (iterChunk=mapChunkId.find(i)) != mapChunkId.end())
			{
				nEntrySize=(uint32_t)pSource->m_strInfo[i].length()+1;				
				if (bRender)
				{
					nLongValue=iterChunk->second;
					pk.write("l",&nLongValue);
				}
				nOffset+=4;
				if (bRender)
					pk.write("l",&nEntrySize);
				nOffset+=4;
				if (bRender)
					out.write(pSource->m_strInfo[i].c_str(),nEntrySize);
				nOffset+=nEntrySize;
				if (nOffset & 1)
				{
					if (bRender)
						out.write("\000",1);
					++nOffset;
				}
			}
		}
	}
	TRACEIT2("metadata takes %d bytes\n",nOffset);
	return nOffset;
}

/*\ 
 * <---------- CWaveFile::Read ----------> 
 * @m read and parse a wave file
 * --> I N <-- @p
 * istream &ar - reference to the input stream
\*/ 
void CWaveFile::Read(istream &ar)
{
	Endian endian;
	endian.init();

	ReadHeader(ar);
	if (m_nCSSize)
	{
		m_pcCSBuffer=Alloc(m_nCSSize);

		ASSERT(m_pcCSBuffer);
		if(m_pcCSBuffer)
		{
			Log2(verbLevDebug1,"trying to read %d bytes...\n",m_nCSSize);
			ar.read((char *)m_pcCSBuffer,m_nCSSize);
			hexdump("input wave: ",(unsigned char *)m_pcCSBuffer,40);

			switch (m_Header.wFormatTag)
			{
				case 0x02:			//ms adpcm
				{
					signed short int *pwOut=NULL;
					uint32_t nOutSize=0;
					Log2(verbLevDebug1,"format is ms adpcm (%03Xh), decompressing...\n",m_Header.wFormatTag);
					pwOut=pwMSAdpcmDecompress((unsigned char *)m_pcCSBuffer,m_nCSSize,&nOutSize);
					m_nCSBitsPerSample=16;
					ASSERT(pwOut);
					ASSERT(nOutSize);
					if (pwOut && nOutSize)
					{
						free(m_pcCSBuffer);
						m_pcCSBuffer=(void *)pwOut;
						m_nCSSize=nOutSize;
						Log2(verbLevDebug1,"MSADPCM decoding done\n");
					}
					else
					{
						Log2(verbLevErrors,"MSADPCM decoding failed\n");
						throw new CFormatException(CFormatException::formaterrSource,"MSADPCM decoding failed");
					}
				}
				break;
				case 0x11:			//ima adpcm
				{
					signed short int *pwOut=NULL;
					uint32_t nOutSize=0;
					Log2(verbLevDebug1,"format is ima adpcm (%03Xh), decompressing...\n",m_Header.wFormatTag);
					m_codec.init(false,0x00);
					pwOut=(int16_t *)Alloc(m_nCSSize*5);
					m_codec.pwIMAAdpcmDecompress(	pwOut,
													(unsigned char *)m_pcCSBuffer,
													m_nCSSize,
													&nOutSize,
													m_Adpcm.wave->wBlockAlign,
													m_Adpcm.wSamplesPerBlock,
													m_Header.wChannels	);
					m_nCSBitsPerSample=16;
					ASSERT(pwOut);
					ASSERT(nOutSize);
					if (pwOut && nOutSize)
					{
						Reset((void *)pwOut,nOutSize);
						Log2(verbLevDebug1,"IMAADPCM decoding done\n");
					}
					else
					{
						Log2(verbLevErrors,"IMAADPCM decoding failed\n");
						throw new CFormatException(CFormatException::formaterrSource,"IMAADPCM decoding failed");
					}
				}
				break;
				case 0x101:			//sagem adpcm
				{
					int16_t *pwOut=NULL;
					uint32_t nOutSize=0;
					Log2(verbLevDebug1,"format is sagem adpcm (%03Xh), decompressing...\n",m_Header.wFormatTag);
					m_codec.init(true,0x01);
					pwOut=(int16_t *)Alloc(m_nCSSize*5);
					m_codec.pwIMAAdpcmDecompress(pwOut,(unsigned char *)m_pcCSBuffer,m_nCSSize,&nOutSize,m_Adpcm.wave->wBlockAlign,m_Adpcm.wSamplesPerBlock,m_Header.wChannels);
					m_nCSBitsPerSample=16;
					ASSERT(pwOut);
					ASSERT(nOutSize);
					if (pwOut && nOutSize)
					{
						Reset((void *)pwOut,nOutSize);
						Log2(verbLevDebug1,"sagem decoding done\n");
					}
					else
					{
						Log2(verbLevErrors,"sagem decoding failed\n");
						throw new CFormatException(CFormatException::formaterrSource,"sagem decoding failed");
					}
				}
				break;
				default:			//pcm
					Log2(verbLevDebug1,"format is pcm (%02Xh)\n",m_Header.wFormatTag);
					if (m_Header.wBitsPerSample == 16)
						endian.HostFromLittleShortArray((uint16_t *)m_pcCSBuffer,(uint16_t *)m_pcCSBuffer,m_nCSSize);
			}
		}
	}
}

/*\ 
 * <---------- CWaveFile::Serialize ----------> 
 * @m create destination wave format from raw source sample data
 * --> I N <-- @p
 * ostream &out - reference to the binary output stream
\*/ 
void CWaveFile::Write(ostream &out)
{
	uint16_t wFormatValue;
	uint32_t nFormatLength;
	uint32_t nFileSize;
	uint32_t nMetaSize=0;
	uint16_t wExtraHeaderDataSize=0;
	uint32_t nSamplesPerBlock=0;
	Endian endian;

	endian.init();
	Packer pk(out,true);

	m_Header.wBlockAlign=0;

	if (m_pParameters->m_nParameter[paraNumBlockSize])
		m_Header.wBlockAlign=m_pParameters->m_nParameter[paraNumBlockSize];

	if (!m_pParameters->m_bSwitch[paraSwitchNoMeta])
		nMetaSize=nRenderMetadata(out,m_pCSSource,false);

	Log2(verbLevDebug2,"rendering wave data...\n");

	m_nCSBitsPerSample=16;
	m_nCSChannels=m_pCSSource->m_nCSChannels;
	m_nCSSamplesPerSecond=m_pCSSource->m_nCSSamplesPerSecond;

	m_Adpcm.wave=&m_Header;
	m_Adpcm.wNumCoef=0;
	wFormatValue=0x00;
	nFormatLength=sizeof(WAVEHEADER)-20;

	switch(m_pParameters->m_nParameter[paraNumCompression])
	{
		case compIMAADPCM:													//IMA ADPCM
			wFormatValue=0x11;												//wave format ID=0x11
			if (!m_Header.wBlockAlign)
				m_Header.wBlockAlign=256 * m_nCSChannels * max(2 * (m_nCSSamplesPerSecond/22050),1);
			m_Adpcm.wSamplesPerBlock = ((m_Header.wBlockAlign - 4*m_nCSChannels)/(4*m_nCSChannels)) * 8 + 1;
			m_Header.nAvgBytesPerSec=(m_nCSSamplesPerSecond/2)*m_nCSChannels;
			m_nCSBitsPerSample=4;
			wExtraHeaderDataSize=2;
		break;				
		case compMSADPCM:													//MS ADPCM
			wFormatValue=0x02;												//wave format ID=0x02
			if (!m_Header.wBlockAlign)
				m_Header.wBlockAlign=256 * m_nCSChannels * max(2 * (m_nCSSamplesPerSecond/22050),1);
            m_Adpcm.wSamplesPerBlock =  m_Header.wBlockAlign - (7*m_nCSChannels);       // bytes beyond block-header
            m_Adpcm.wSamplesPerBlock = (2*m_Adpcm.wSamplesPerBlock)/m_nCSChannels + 2;	// nibbles/chans + 2 in header 
			m_Adpcm.wNumCoef=7;												//number of coefficients
			m_nCSBitsPerSample=4;
			m_Header.nAvgBytesPerSec=(m_nCSSamplesPerSecond/2)*m_nCSChannels;
			wExtraHeaderDataSize=(uint16_t)4+(m_Adpcm.wNumCoef*4);
		break;				
		case compQCELP:														//QCELP - currently not supported in WAVEs
			Log2(verbLevErrors,"failed to compress data into QCELP format\n");
			throw new CFormatException(CFormatException::formaterrUnknown,"QCELP encoded WAVE files are currently not supported");
		break;				
		case compSAGEM:														//Sagem IMA ADPCM
			wFormatValue=0x101;												//wave format ID=0x101
			m_Adpcm.wSamplesPerBlock=81*m_nCSChannels;						//81 samples per block
			m_Header.wBlockAlign=(m_Adpcm.wSamplesPerBlock/2)+4;
			m_Header.nAvgBytesPerSec=(m_nCSSamplesPerSecond/2)*m_nCSChannels;
			m_nCSBitsPerSample=4;
			if (!m_pParameters->m_bSwitch[paraSwitchFact])
			{
				Log2(verbLevWarnings,"sagem wave-files need the fact-chunk to play correctly, enabling fact-chunk\n");
				m_pParameters->m_bSwitch[paraSwitchFact]=true;
			}
			//wExtraHeaderDataSize=24;
			wExtraHeaderDataSize=12;
		break;				
		default:															//PCM
			wFormatValue=0x01;												//wave format ID=0x01
			m_Adpcm.wSamplesPerBlock=1;										//1 sample per block
			m_Header.wBlockAlign=m_nCSChannels*(m_nCSBitsPerSample/8);		//
			wExtraHeaderDataSize=0;
	}	
	
	TRACEIT2("block align: %d\n",m_Header.wBlockAlign);
	TRACEIT2("samples per block: %d\n",m_Adpcm.wSamplesPerBlock);

	if(wExtraHeaderDataSize)
		nFormatLength+=wExtraHeaderDataSize+2;
	m_Header.wBitsPerSample=(uint16_t)m_nCSBitsPerSample;
	m_Header.wChannels=(uint16_t)m_nCSChannels;
	m_Header.nSamplesPerSec=m_nCSSamplesPerSecond;
	
	nFileSize=20+nFormatLength;
	if(wExtraHeaderDataSize && m_pParameters->m_bSwitch[paraSwitchFact])
		nFileSize+=12;

	uint32_t nOutSize=m_pCSSource->m_nCSSize;

	switch(m_pParameters->m_nParameter[paraNumCompression])
	{
		case compIMAADPCM:									//IMAADPCM
		{
			unsigned char *pcOut=NULL;
			m_codec.init(false,0x00);
			pcOut=m_codec.pcIMAAdpcmCompress(	(signed short *)m_pCSSource->m_pcCSBuffer,
												m_pCSSource->m_nCSSize,
												&nOutSize,
												m_Adpcm.wave->wBlockAlign,
												m_Adpcm.wSamplesPerBlock,
												m_Header.wChannels	);
			if (pcOut == NULL)
			{
				Log2(verbLevErrors,"failed to compress IMAADPCM sample data\n");
				throw new CFormatException(CFormatException::formaterrUnknown,"failed to compress IMAADPCM sample data");
			}
			if (m_pcCSBuffer)
				CMobileSampleContent::Free(m_pcCSBuffer);
			m_pcCSBuffer=pcOut;
			m_nCSBitsPerSample=4;
			m_Header.nAvgBytesPerSec=(m_Header.nSamplesPerSec/2)*m_Header.wChannels;
		}
		break;
		case compMSADPCM:									//MSADPCM
		{
			unsigned char *pcOut=NULL;
			pcOut=pcMSAdpcmCompress((signed short *)m_pCSSource->m_pcCSBuffer,m_pCSSource->m_nCSSize,&nOutSize);
			if (pcOut == NULL)
			{
				Log2(verbLevErrors,"failed to compress MSADPCM sample data\n");
				throw new CFormatException(CFormatException::formaterrUnknown,"failed to compress MSADPCM sample data");
			}
			if (m_pcCSBuffer)
				CMobileSampleContent::Free(m_pcCSBuffer);
			m_pcCSBuffer=pcOut;
			m_nCSBitsPerSample=4;
			m_Header.nAvgBytesPerSec=(m_Header.nSamplesPerSec/2)*m_Header.wChannels;
		}
		break;
		case compSAGEM:										//SAGEM ADPCM
		{
			unsigned char *pcOut=NULL;
			m_codec.init(true,0x01);
			pcOut=m_codec.pcIMAAdpcmCompress(	(signed short *)m_pCSSource->m_pcCSBuffer,
												m_pCSSource->m_nCSSize,
												&nOutSize,
												m_Adpcm.wave->wBlockAlign,
												m_Adpcm.wSamplesPerBlock,
												m_Header.wChannels	);
			if (pcOut == NULL)
			{
				Log2(verbLevErrors,"failed to compress IMAADPCM sample data\n");
				throw new CFormatException(CFormatException::formaterrUnknown,"failed to compress IMAADPCM sample data");
			}
			if (m_pcCSBuffer)
				CMobileSampleContent::Free(m_pcCSBuffer);
			m_pcCSBuffer=pcOut;
			m_nCSBitsPerSample=4;
			m_Header.nAvgBytesPerSec=(m_Header.nSamplesPerSec/2) * m_Header.wChannels;
			m_Header.nAvgBytesPerSec+=345;
		}
		break;
		default:
			m_Header.nAvgBytesPerSec=(m_Header.nSamplesPerSec*(m_Header.wBitsPerSample>>3))*m_Header.wChannels;
			m_pcCSBuffer=CMobileSampleContent::Alloc(nOutSize);
			if (m_pCSSource->m_nCSBitsPerSample == 16)
				endian.HostFromLittleShortArray((unsigned short *)m_pcCSBuffer,(unsigned short *)m_pCSSource->m_pcCSBuffer,nOutSize);
			else
				CopyMemory(m_pcCSBuffer,m_pCSSource->m_pcCSBuffer,nOutSize);
	}
	nFileSize+=nOutSize+8;
	if (!m_pParameters->m_bSwitch[paraSwitchNoMeta])
		nFileSize+=nMetaSize;
	m_nCSSize=nOutSize;
	out.write("RIFF",4);
	pk.write("l",&nFileSize);
	out.write("WAVEfmt ",8);
	pk.write("l",&nFormatLength);							//
	pk.write("s",&wFormatValue);							//format value
	pk.write((uint16_t)m_nCSChannels);						//channel count
	pk.write("l",&m_nCSSamplesPerSecond);					//sample rate
	pk.write("l",&m_Header.nAvgBytesPerSec);				//bandwidth
	pk.write("s",&m_Header.wBlockAlign);					//byte per block
	pk.write((uint16_t)m_nCSBitsPerSample);					//write a single short

	switch(m_pParameters->m_nParameter[paraNumCompression])
	{
		case compIMAADPCM:								//IMAADPCM
			pk.write((uint16_t)wExtraHeaderDataSize);	//extra header size
			pk.write("s",&m_Adpcm.wSamplesPerBlock);	//number of samples per block
			Log2(verbLevDebug1,"ima adpcm samples per block: %d\n",m_Adpcm.wSamplesPerBlock);
		break;
		case compMSADPCM:										//MSADPCM
		{
			pk.write((uint16_t)wExtraHeaderDataSize);	//extra header size
			pk.write("s",&m_Adpcm.wSamplesPerBlock);	//number of samples per block
			Log2(verbLevDebug1,"ms adpcm samples per block: %d\n",m_Adpcm.wSamplesPerBlock);
			pk.write("s",&m_Adpcm.wNumCoef);			//number of coefficients
			Log2(verbLevDebug1,"storing %d adpcm coefficients\n",m_Adpcm.wNumCoef);
			
			m_Adpcm.paCoeff=new ADPCMCOEFSET[max(m_Adpcm.wNumCoef,7)];
			CopyMemory(m_Adpcm.paCoeff,m_codec.pnGetMSCoef(),7*4);
			out.write((char *)m_Adpcm.paCoeff,m_Adpcm.wNumCoef*4);
		}
		break;
		case compSAGEM:									//SAGEM IMAADPCM
			pk.write(wExtraHeaderDataSize);				//extra header size
			pk.write(m_Adpcm.wSamplesPerBlock);			//number of samples per block
			Log2(verbLevDebug1,"sagem adpcm samples per block: %d\n",m_Adpcm.wSamplesPerBlock);
			pk.write((uint32_t)0x00000001);
			pk.write((uint32_t)0x00000000);
			pk.write((uint16_t)0x0002);
		break;
	}

	if (m_pParameters->m_bSwitch[paraSwitchFact])
	{
		out.write("fact",4);						//fact-chunk
		pk.write((uint32_t)0x00000004);
		pk.write(m_pCSSource->m_nCSSize / (m_pCSSource->m_nCSBitsPerSample >> 3));
	} 

	//are we allowed to write metadata in general and are we supposed to put it in front of the sample data?
	if (!m_pParameters->m_bSwitch[paraSwitchNoMeta] && m_pParameters->m_bSwitch[paraSwitchReverseOrder])	
		nRenderMetadata(out,m_pCSSource);					//yes->generate the metadata block
	out.write("data",4);									//sample data now following
	pk.write("l",&m_nCSSize);								//sample data size
	out.write((char *)m_pcCSBuffer,m_nCSSize);
	//are we allowed to write metadata in general and are we supposed to put it behind the sample data?
	if (!m_pParameters->m_bSwitch[paraSwitchNoMeta] && !m_pParameters->m_bSwitch[paraSwitchReverseOrder])	
		nRenderMetadata(out,m_pCSSource);					//yes->generate the metadata block
	m_nFileSize=nFileSize;
	/*
	m_nFileSize=out.tellp();
	TRACEIT2("nFileSize %d\n",nFileSize);
	TRACEIT2("m_nFileSize %d\n",m_nFileSize);
	ASSERT(nFileSize == m_nFileSize);
	*/
}
