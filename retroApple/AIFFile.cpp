/*\
 * AIFFile.cpp
 * Copyright (C) 2004-2008, MMSGURU - written by Till Toenshoff
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
#include "../retroBase/Basics.h"
#include <map>
#ifndef WIN32
#include <netinet/in.h>
#else
#include "Winsock2.h"
#endif
#include "../include/Resource.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/Endian.h"
#include "../retroBase/Packer.h"
#include "AIFFile.h"
#include "AIFProperty.h"

DYNIMPPROPERTY(CAIFFile,CAIFProperty)

CAIFFile::CAIFFile(void)
{
	TRACEIT2("aif constructor\n");
	//file magic bytes definition; 4 bytes, reading "FORM"
	m_nMagicSize=4;
	m_pcMagic="FORM";
	//file format name
	m_sFormatName="Audio Interchange File Format (AIFF)";
	//default extension for encoding
	m_sDefaultExtension=_T("aif");
	//long format description
	m_sFormatDescription.Load(IDS_FORMDESC_AIF);
	//encoding compatible sample parameters
	m_encodingCaps.freqs[	CSampleCompatibility::supports8Bit|
							CSampleCompatibility::supports16Bit|
							CSampleCompatibility::supportsMono|
							CSampleCompatibility::supportsStereo].setRange(1,192000);
	//format specific commandline parameters
	m_encodingPara.addPara(cmdParaString,paraStrTitle);
	m_encodingPara.addPara(cmdParaString,paraStrNote);
	m_encodingPara.addPara(cmdParaString,paraStrWriter);
	m_encodingPara.addPara(cmdParaString,paraStrCopyright);

	ZeroMemory(&m_Header,sizeof(m_Header));
	TRACEIT2("aif constructed\n");
}

 CAIFFile::~CAIFFile(void)
{
}

/*\
 * <---------- CAIFFile :: nExtAppleFloatToLong ----------> 
 * @m convert an 80 bit IEEE Standard 754 floating point number to an unsigned long.
 * --> I N <-- @p
 * unsigned char *buffer - pointer to input buffer containing the IEEE 754 float
 * <-- OUT --> @r
 * uint32_t- converted long value
\*/
uint32_t CAIFFile::nExtAppleFloatToLong(unsigned char *buffer)
{
	uint32_t mantissa;
	uint32_t last = 0;
	uint8_t exp;

	memcpy(&mantissa,buffer+2,4);
	mantissa=ntohl(mantissa);
	exp = 30 - *(buffer+1);
	while (exp--)
	{
		last = mantissa;
		mantissa >>= 1;
	};
	if (last & 0x00000001) 
		mantissa++;
	return(mantissa);
}

/*\
 * <---------- CAIFFile :: *pcExtLongToAppleFloat ----------> 
 * @m convert a long value into an 80 bit IEEE standard 754 floating point number 
 * --> I N <-- @p
 * unsigned char *buffer - pointer to destination buffer, 80bits size
 * uint32_tvalue   - uint32_tvalue
 * <-- OUT --> @r
 * destination buffer pointer
\*/
uint8_t *CAIFFile::pcExtLongToAppleFloat(uint8_t *buffer,uint32_t value)
{
	uint32_t exp;
	uint8_t i;

	memset(buffer, 0, 10);
	*buffer=0x40;

	exp = value;
	exp >>= 1;
	for (i=0; i<32; i++)
	{
		exp >>= 1;
		if (!exp) 
			break;
	}
	*(buffer+1) = i;
	for (i=32; i; i--)
	{
		if (value & 0x80000000) 
			break;
		value <<= 1;
	}
	//*(uint32_t*)(buffer+2)=htonl(value);
	value=htonl(value);
	memcpy(buffer+2,&value,4);

	return buffer;
}

/*\
 * <---------- CAIFFile :: nReadSubchunk ----------> 
 * @m read a complete chunk
 * --> I N <-- @p
 * istream &ar       - input stream reference
 * AIFFHEADER *pAiff - pointer to aiff header
 * <-- OUT --> @r
 * complete chunk-size
\*/
uint32_t CAIFFile::nReadSubchunk(istream &ar, AIFFHEADER *pAiff) 
{
	uint32_t nRet=0;
	int8_t pcChunk[5];
	uint32_t nSize,nRead;
	map<tstring,int> :: const_iterator iterChunk;
	map<tstring,int> mapChunkId;

	Unpacker up(ar,false);

	mapChunkId[tstring(_T("COMM"))]=cidCOMM;
	mapChunkId[tstring(_T("INST"))]=cidINST;
	mapChunkId[tstring(_T("MIDI"))]=cidMIDI;
	mapChunkId[tstring(_T("NAME"))]=cidNAME;
	mapChunkId[tstring(_T("AUTH"))]=cidAUTH;
	mapChunkId[tstring(_T("(c) "))]=cidCOPYRIGHT;
	mapChunkId[tstring(_T("ANNO"))]=cidANNO;
	mapChunkId[tstring(_T("MARK"))]=cidMARK;
	mapChunkId[tstring(_T("SSND"))]=cidSSND;
	mapChunkId[tstring(_T("COMT"))]=cidCOMT;

	//Read the first 4 bytes, which will hold 'FORM' = 	63 6D 69 64 
	ar.read((char *)pcChunk,4); 
	pcChunk[4]=0;
	nRet+=4;
	up.read("l",&nSize);
	nRet+=nSize+4;
	if (nSize+(uint32_t)ar.tellg() > (uint32_t)m_nFileSize)
	{
		TRACEIT2("invalid chunk size (chunk: \"%s\", size: %d - filesize: %d)\n",pcChunk,nSize,m_nFileSize);
		throw new CFormatException(CFormatException::formaterrInvalid,"invalid chunk size");
	}
	Log2(verbLevDebug1,"decoded chunk: %s, size: %d\n",pcChunk,nSize);
	if ((iterChunk=mapChunkId.find((const char *)pcChunk)) == mapChunkId.end())
	{
		TRACEIT2("unknown chunk\n");
	}
	else
	{
		switch(iterChunk->second)
		{
			case cidCOMT:
			break;
			case cidMARK:
				up.read("s",&pAiff->wNumMarkers);
				nSize-=2;
				TRACEIT2("markers: %d\n",pAiff->wNumMarkers);
			break;
			case cidMIDI:	TRACEIT2("MIDI data found\n");		break;
			case cidSSND:	
				TRACEIT2("sample data found\n");
				up.read("l",&pAiff->nOffset);
				nSize-=4;
				up.read("l",&pAiff->nBlockSize);
				nSize-=4;
				TRACEIT2("sample offset: %d, block size: %d\n",pAiff->nOffset,pAiff->nBlockSize);
				if (pAiff->nOffset)
				{
					ar.seekg((int)ar.tellg()+(int)pAiff->nOffset);
					nSize-=pAiff->nOffset;
				}
				if (nSize)
				{
					m_nCSSize=nSize;
					m_pcCSBuffer=Alloc(m_nCSSize);
					ASSERT(m_pcCSBuffer);
					if(m_pcCSBuffer)
					{
						Log2(verbLevDebug3,"trying to read %d bytes...\n",m_nCSSize);
						if (m_nCSBitsPerSample == 16)
						{
							unsigned short int *p=(unsigned short int *)m_pcCSBuffer;
							for (uint32_t i=0;i < m_nCSSize/2;i++)
								up.read("s",p++);
						}
						else
							ar.read((char *)m_pcCSBuffer,m_nCSSize);
						hexdump("decoded aiff pcm: ",(unsigned char *)m_pcCSBuffer,40);
					}
				}
				nSize=0;
			break;
			case cidINST:
				up.read("b",&pAiff->cBaseNote);
				nSize-=sizeof(pAiff->cBaseNote);
				up.read("b",&pAiff->cDetune);
				nSize-=sizeof(pAiff->cDetune);
				up.read("b",&pAiff->cLowNote);
				nSize-=sizeof(pAiff->cLowNote);
				up.read("b",&pAiff->cHighNote);
				nSize-=sizeof(pAiff->cHighNote);
				up.read("b",&pAiff->cLowVelocity);
				nSize-=sizeof(pAiff->cLowVelocity);
				up.read("b",&pAiff->cHighVelocity);
				nSize-=sizeof(pAiff->cHighVelocity);
				up.read("s",&pAiff->wGain);
				nSize-=sizeof(pAiff->wGain);
				Log2(verbLevDebug1,"base: %d, detune: %d, LowNote: %d, HighNote: %d, LowVelo: %d, HighVelo: %d\n",
						pAiff->cBaseNote,
						pAiff->cDetune,
						pAiff->cLowNote,
						pAiff->cHighNote,
						pAiff->cLowVelocity,
						pAiff->cHighVelocity);
			break;
			case cidCOMM:
				up.read("s",(char*)&pAiff->wChannelCount);
				nSize-=2;
				up.read("l",&pAiff->nSampleFrames);
				nSize-=4;
				up.read("s",&pAiff->wSampleSize);
				nSize-=2;
				ar.read((char*)pAiff->pcExtFloat_SampleRate,sizeof(pAiff->pcExtFloat_SampleRate));
				pAiff->nSampleRate=nExtAppleFloatToLong(pAiff->pcExtFloat_SampleRate);
				nSize-=sizeof(pAiff->pcExtFloat_SampleRate);
				m_nCSBitsPerSample=pAiff->wSampleSize;
				m_nCSSamplesPerSecond=pAiff->nSampleRate;
				m_nCSChannels=pAiff->wChannelCount;
			break;
			case cidNAME:
				nRead=min(255,nSize);
				ar.read((char*)pAiff->pcName,nRead);
				pAiff->pcName[nRead]=0;
				nSize-=nRead;
			break;
			case cidAUTH:
				nRead=min(255,nSize);
				ar.read((char*)pAiff->pcAuthor,nRead);
				pAiff->pcAuthor[nRead]=0;
				nSize-=nRead;
			break;
			case cidCOPYRIGHT:
				nRead=min(255,nSize);
				ar.read((char*)pAiff->pcCopyright,nRead);
				pAiff->pcCopyright[nRead]=0;
				nSize-=nRead;
			break;
			case cidANNO:
				nRead=min(255,nSize);
				ar.read((char*)pAiff->pcAnnotation,nRead);
				pAiff->pcAnnotation[nRead]=0;
				nSize-=nRead;
			break;
			default:
				TRACEIT2("known but unhandled chunk\n");
		}
	}
	//skip unwanted data
	if (nSize)
	{
		TRACEIT2("skipping %d bytes\n",nSize);
		ar.seekg((uint32_t)ar.tellg()+(uint32_t)nSize);
	}
	return nRet;
}

/*\
 * <---------- CAIFFile :: read ----------> 
 * @m read, parse and decode a complete AIFF file
 * --> I N <-- @p
 * istream &ar - input stream object reference
\*/
void CAIFFile::Read(istream &ar) 
{ 
	uint32_t nSize;
	Unpacker up(ar,false);
	//Read the first 4 bytes, which will hold 'FORM' = 	63 6D 69 64 
	ar.read(m_Header.form,4); 
	//Now, read the size (a.aif = 00 0B E9 B0)
	up.read("l",&m_Header.size); 
	if (m_Header.size > m_nFileSize)
	{
		TRACEIT2("invalid FORM size\n");
		throw new CFormatException(CFormatException::formaterrInvalid,"invalid FORM size");
	}
	//read the AIFF tag
	ar.read((char*)&m_Header.aiff,4);
	if (memcmp(m_Header.aiff,"AIFF",4))
	{
		TRACEIT2("not an AIFF file\n");
		throw new CFormatException(CFormatException::formaterrInvalid,"not an AIFF file");
	}
	if (m_Header.size <= 4)
	{
		TRACEIT2("AIFF file truncated\n");
		throw new CFormatException(CFormatException::formaterrInvalid,"AIFF file truncated");
	}
	//now read all subchunks until we reached the end of the file
	nSize=m_Header.size-4;
	while (nSize)
	{
		TRACEIT2("nSize: %d\n",nSize);
		nSize-=nReadSubchunk(ar,&m_Header);
	};
}

/*\
 * <---------- CAIFFile :: nGetPlaytime ----------> 
 * @m get the total playback time
 * <-- OUT --> @r
 * int - duration of the sample in milliseconds
\*/
uint32_t CAIFFile::nGetPlaytime()
{
	uint32_t nRet=0;
	uint32_t nNom,nDiv;
	nNom=nGetSamples()*1000;
	TRACEIT2("samples: %d\n",nNom);
	nDiv=nGetSamplesPerSecond();
	ASSERT(nDiv);
	if (nDiv)
		nRet=round(nNom,nDiv);
	return nRet;
}

/*\
 * <---------- CAIFFile :: RenderDestination ----------> 
 * @m create an AIF
 * --> I N <-- @p
 * ostream &out - 
\*/
void CAIFFile::Write(ostream &out)
{
	uint32_t nLen;

	Packer pk(out,false);

	ASSERT(m_pCSSource);
 
	m_nFileSize=16+18+12+m_pCSSource->m_nCSSize;		//FORM, SIZE, AIFF, COMM + COMMSize + SND,Size + Sample
	if(!m_pParameters->m_bSwitch[paraSwitchNoMeta])
	{
		if ((nLen=(uint32_t)m_pParameters->m_strParameter[paraStrTitle].length()) > 0)
			m_nFileSize+=nLen+8;
		if ((nLen=(uint32_t)m_pParameters->m_strParameter[paraStrWriter].length()) > 0)
			m_nFileSize+=nLen+8;
		if ((nLen=(uint32_t)m_pParameters->m_strParameter[paraStrCopyright].length()) > 0)
			m_nFileSize+=nLen+8;
		if ((nLen=(uint32_t)m_pParameters->m_strParameter[paraStrNote].length()) > 0)
			m_nFileSize+=nLen+8;
	}

	out.write("FORM",4);
	//output size (a.aif = 00 0B E9 B0)
	pk.write("l",&m_nFileSize);
	//write the AIFF header tag
	out.write("AIFF",4);
	//sub-tags
	out.write("COMM",4);
	pk.write((uint32_t)0x0012);
	pk.write((uint16_t)m_pCSSource->m_nCSChannels);
	pk.write((uint32_t)(m_pCSSource->m_nCSSize / (m_pCSSource->m_nCSBitsPerSample >> 3)) / m_pCSSource->m_nCSChannels);
	pk.write((uint16_t)m_pCSSource->m_nCSBitsPerSample);
	unsigned char pcExtFloat_SampleRate[10]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	pcExtLongToAppleFloat(pcExtFloat_SampleRate,m_pCSSource->m_nCSSamplesPerSecond);
	out.write((const char *)pcExtFloat_SampleRate,10);

	out.write("SSND",4);
	pk.write((uint32_t)m_pCSSource->m_nCSSize+8);
	pk.write((uint32_t)0x0000);					//nOffset
	pk.write((uint32_t)0x0000);					//nBlockSize
	TRACEIT2("writing sample data...\n");	
	if (m_pCSSource->m_nCSBitsPerSample == 16)
	{
		uint32_t i;
		uint16_t *p=(uint16_t *)m_pCSSource->m_pcCSBuffer;
		for(i=0;i < m_pCSSource->m_nCSSize/2;i++)
			pk.write("s",p++);
	}
	else
		out.write((const char *)m_pCSSource->m_pcCSBuffer,m_pCSSource->m_nCSSize);

	TRACEIT2("writing metadata etc...\n");	
	if (!m_pParameters->m_bSwitch[paraSwitchNoMeta] && (nLen=(uint32_t)m_pParameters->m_strParameter[paraStrTitle].length()) > 0)
	{
		out.write("NAME",4);
		pk.write("l",&nLen);
		out.write(m_pParameters->m_strParameter[paraStrTitle].c_str(),nLen);
	}
	if (!m_pParameters->m_bSwitch[paraSwitchNoMeta] && (nLen=(uint32_t)m_pParameters->m_strParameter[paraStrWriter].length()) > 0)
	{
		out.write("AUTH",4);
		pk.write("l",&nLen);
		out.write(m_pParameters->m_strParameter[paraStrWriter].c_str(),nLen);
	}
	if (!m_pParameters->m_bSwitch[paraSwitchNoMeta] && (nLen=(uint32_t)m_pParameters->m_strParameter[paraStrCopyright].length()) > 0)
	{
		out.write("(c) ",4);
		pk.write("l",&nLen);
		out.write(m_pParameters->m_strParameter[paraStrCopyright].c_str(),nLen);
	}
	if (!m_pParameters->m_bSwitch[paraSwitchNoMeta] && (nLen=(uint32_t)m_pParameters->m_strParameter[paraStrNote].length()) > 0)
	{
		out.write("ANNO",4);
		pk.write("l",&nLen);
		out.write(m_pParameters->m_strParameter[paraStrNote].c_str(),nLen);
	}

	m_nCSChannels=m_pCSSource->m_nCSChannels;
	m_nCSSamplesPerSecond=m_pCSSource->m_nCSSamplesPerSecond;
	m_nCSBitsPerSample=0;
	m_nCSSize=m_pCSSource->m_nCSSize;
	m_nFileSize=out.tellp();
}
