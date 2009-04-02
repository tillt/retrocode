/*\
 * RMFFile.cpp
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
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#ifndef WIN32
#include <netinet/in.h>
#else
#include <winsock2.h>
#endif 
#include "../include/Resource.h"
#include "../retroBase/Basics.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MyString.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/Endian.h"
#include "../retroBase/Packer.h"
#include "../retroBase/Adpcm.h"
#include "RMFBasics.h"
#include "RMFFile.h"
#include "../retroBase/MIDIFile.h"
#include "RMFInstrument.h"
#include "RMFSound.h"
#include "RMFSequence.h"
#include "RMFProperty.h"
#include "Version.h"

DYNIMPPROPERTY(CRMFFile,CRMFProperty)

/*\ 
 * <---------- CRMFFile::CRMFFile ----------> 
 * @m default constructor
\*/ 
 CRMFFile::CRMFFile(void) : m_pSequence(NULL)
{
	m_nMagicSize=4;
	m_pcMagic="IREZ";
	m_sFormatName=_T("Beatnik Rich Music Format (RMF)");
	m_sDefaultExtension=_T("rmf");
	m_sFormatDescription.Load(IDS_FORMDESC_RMF);
	m_sFormatCredits=_T("Retro's RMF codec is NOT licensed or in any way approved by Beatnik Software.");
	m_encodingCaps.freqs[	CSampleCompatibility::supports16Bit|
							CSampleCompatibility::supportsMono].setRange(1,44100);
	/*
	m_encodingCaps.freqs[	CSampleCompatibility::supports16Bit|
							CSampleCompatibility::supportsMono|
							CSampleCompatibility::supportsStereo].setRange(1,44100);
	*/
	m_encodingPara.addPara(cmdParaNumber,	paraNumVolume);
	m_encodingPara.addPara(cmdParaNumber,	paraNumTempo);
	m_encodingPara.addPara(cmdParaNumber,	paraNumPlaytime);
	m_encodingPara.addPara(cmdParaNumber,	paraNumFadetime);
	m_encodingPara.addPara(cmdParaString,	paraStrTitle);
	m_encodingPara.addPara(cmdParaString,	paraStrArtist);
	m_encodingPara.addPara(cmdParaString,	paraStrComposer);
	m_encodingPara.addPara(cmdParaString,	paraStrNote);
	m_encodingPara.addPara(cmdParaString,	paraStrCategory);
	m_encodingPara.addPara(cmdParaString,	paraStrSubcategory);
	m_encodingPara.addPara(cmdParaString,	paraStrCopyright);
	m_encodingPara.addPara(cmdParaString,	paraStrPublisher);
	m_encodingPara.addPara(cmdParaString,	paraStrLicenseUse);
	m_encodingPara.addPara(cmdParaString,	paraStrLicenseTerm);
	m_encodingPara.addPara(cmdParaString,	paraStrLicenseUrl);
	m_encodingPara.addPara(cmdParaString,	paraStrLicenseExp);
	m_encodingPara.addPara(cmdParaString,	paraStrSource);
	m_encodingPara.addPara(cmdParaString,	paraStrTempo);
	m_encodingPara.addPara(cmdParaString,	paraStrIndex);
}

/*\ 
 * <---------- CRMFFile::~CRMFFile ----------> 
 * @m destructor
\*/ 
CRMFFile::~CRMFFile(void)
{
	if (m_pSequence != NULL)
	{
		delete m_pSequence;
		m_pSequence=NULL;
	}

	unsigned int i;
	for (i=0;i < (unsigned )m_Sounds.size();i++)
	{
		ASSERT (m_Sounds[i]);
		if (m_Sounds[i] != NULL)
			delete m_Sounds[i];
	}
	if (m_Sounds.size())
		m_Sounds.erase(m_Sounds.begin(), m_Sounds.end());
	m_pcCSBuffer=NULL;

	for (i=0;i < (unsigned )m_Instruments.size();i++)
	{
		ASSERT (m_Instruments[i]);
		if (m_Instruments[i] != NULL)
			delete m_Instruments[i];
	}
	if (m_Instruments.size())
		m_Instruments.erase(m_Instruments.begin(), m_Instruments.end());
}

/*\ 
 * <---------- CRMFFile::read ----------> 
 * @m read and validate a complete rmf-file
 * --> I N <-- @p
 * istream &ar - input stream
\*/ 
void CRMFFile::Read(istream &ar)
{
	char pcTagSpace[5]={0,0,0,0,0};
	Unpacker up(ar,false);
	//read header
	if (bValidateHeader(ar))
	{
		char pcBuffer[5];
		uint32_t nTag,nTagSize,nNextTagOffset;
		unsigned int nOffset=0;
		unsigned int nTags;
		uint32_t nFileSize=m_nFileSize;
		uint32_t nSize=m_nFileSize;
		uint32_t nLastTagOffset=0;
		nSize-=12;
		pcBuffer[4]=0;

		up.read("l",&nTags);
		Log2(5,"expecting %d subtags\n",nTags);
		while (nTags && nSize > 0)
		{
			if (CRMFBasics::bReadTag(ar,&nTag,&nNextTagOffset))
			{
				if (nNextTagOffset > 0 && nNextTagOffset > nLastTagOffset)
				{
					nTagSize=nNextTagOffset-ar.tellg();
					switch (TOFOURCC(nTag))
					{
						case MAKEFOURCC('S','O','N','G'):
							Log2(verbLevDebug3,"decoding meta \"SONG\" data with %d bytes...\n",nTagSize);
							ParseSONG(ar);
						break;
						case MAKEFOURCC('I','N','S','T'):
							Log2(verbLevDebug3,"decoding instrument \"INST\" data with %d data bytes....\n",nTagSize);
							ParseINST(ar);
						break;
						case MAKEFOURCC('e','s','n','d'):
							Log2(verbLevDebug3,"decoding encrypted sound \"esnd\" data with %d data bytes....\n",nTagSize);
							ParseESND(ar,true);
						break;
						case MAKEFOURCC('s','n','d',' '):
							Log2(verbLevDebug3,"decoding unencrypted sound \"snd \" data with %d data bytes....\n",nTagSize);
							ParseESND(ar,false);
						break;
						case MAKEFOURCC('e','c','m','i'):
							Log2(verbLevDebug3,"decoding compressed encrypted MIDI sequence \"ecmi\" data with %d data bytes....\n",nTagSize);
							ParseECMI(ar);
						break;
						default:
							Log2(verbLevDebug3,"skipping tag \"%s\" with %d data bytes...\n",pcSplitID4(nTag,pcTagSpace),nTagSize);
					}
					ar.seekg(nNextTagOffset,ios_base::beg);
					nLastTagOffset=nNextTagOffset;
					nSize=nFileSize-nNextTagOffset;
				}
				else
				{
					nSize=0;						
					Log2(verbLevErrors,"invalid next tag offset %08Xh after %08Xh\n",nNextTagOffset,nLastTagOffset);
					throw new CFormatException(CFormatException::formaterrTruncated,"invalid next tag offset");
				}
			}
			else
			{
				nSize=0;
				Log2(verbLevErrors,"failed to read tag at %08Xh\n",nOffset);
				throw new CFormatException(CFormatException::formaterrTruncated,"failed to read tag");
			}
			--nTags;
		};
		if (nSize != 0)
		{
			Log2(verbLevWarnings,"file has trailing garbage\n");
		}
		if (nTags != 0)
		{
			Log2(verbLevErrors,"file is truncated\n");
			throw new CFormatException(CFormatException::formaterrTruncated,"file is truncated");
		}
	}
	else
	{
		Log2(verbLevErrors,IDS_ERR_INVFILEHEADER);
		throw new CFormatException(CFormatException::formaterrInvalid,"invalid file header");
	}
	Log2(verbLevDebug3,"RMF parsed\n");
	if(m_pSequence != NULL)
	{
		CMIDIFileLoader midi;
		stringstream memfile;

		Log2(verbLevDebug3,"parsing embedded MIDI...\n");
		memfile.exceptions(ios_base::badbit|ios_base::eofbit|ios_base::failbit);

		memfile.write((const char *)m_pSequence->pcGetSequence(),m_pSequence->nGetSequenceSize());
		memfile.seekg(0,ios_base::beg);
		midi.Load(memfile);

		Log2(verbLevDebug3,"midi playtime: %d\n",midi.GetPlaytime());
		m_nPlaytime=midi.GetPlaytime();
	}
	if (m_Sounds.size())
	{
		unsigned int nLargestSize=0;
		CRMFSound *pSound=NULL;
		for (unsigned int i=0;i < m_Sounds.size();i++)
		{
			if (m_Sounds[0]->nGetSize() > nLargestSize)
			{
				pSound=m_Sounds[i];
				nLargestSize=pSound->nGetSize();
			}
		}
		if (pSound)
		{
			m_nCSBitsPerSample=pSound->nGetBitsPerSample();
			m_nCSChannels=pSound->nGetChannels();
			m_nCSSamplesPerSecond=pSound->nGetSamplesPerSecond();
			m_nCSSize=pSound->nGetSize();
			m_pcCSBuffer=pSound->pcGetSample();
		}
	}
}

/*\ 
 * <---------- CRMFFile::bValidateHeader ----------> 
 * @m make sure the header has right magic and version info
 * --> I N <-- @p
 * istream &ar - reference to input stream
 * <-- OUT --> @r
 * bool - TRUE=ok
\*/ 
bool CRMFFile::bValidateHeader(istream &ar)
{
	char pcTagSpace[5];
	char acHeader[12];
	uint32_t nTag;
	ar.read(acHeader,8);
	memcpy(&nTag,&acHeader[0],4);
	if (nTag != nMakeID4("IREZ"))
	{
		Log2(verbLevErrors,"tag is not expected (\"%s\")\n",pcSplitID4(nTag,pcTagSpace));
		throw new CFormatException(CFormatException::formaterrInvalid,"tag is not expected");
	}
	uint32_t nVersionCode;
	memcpy(&nVersionCode,&acHeader[0]+4,4);
	int nVersion=ntohl(nVersionCode);
	if (nVersion != 0x01)
	{
		Log2(verbLevErrors,"version is invalid (%08X)\n",nVersion);
		throw new CFormatException(CFormatException::formaterrInvalid,"version is invalid");
	}
	Log2(verbLevDebug1,"RMF Version %d\n",nVersion);
	return true;
}

/*\ 
 * <---------- *CRMFFile::pcReadString ----------> 
 * @m read a pascal formated string while allocating enough destination buffer data
 * --> I N <-- @p
 * istream &ar - reference to input stream
 * <-- OUT --> @r
 * char * - pointer to read string (has to be freed manually using free())
 \*/ 
char *CRMFFile::pcReadString(istream &ar)
{
	char *pcString=NULL;
	unsigned char nLen;
	
	ar.read((char *)&nLen,1);
	TRACEIT("pcReadString  - string size %d byte\n",nLen);
	if (nLen > 0)
	{
		//pcString=(char *)malloc(nLen+1);
		pcString=new char[nLen+1];
		ar.read(pcString,nLen);
		pcString[nLen]=0;
	}
	return pcString;
}

/*\ 
 * <---------- CRMFFile::bParseINST ----------> 
 * @m parse instrument data
 * --> I N <-- @p
 * istream &ar - reference to input stream
\*/ 
void CRMFFile::ParseINST(istream &ar)
{
	CRMFInstrument *pInstrument=new CRMFInstrument ();
	pInstrument->Serialize(ar);
	m_Instruments.push_back(pInstrument);
}

/*\ 
 * <---------- CRMFFile::ParseESND ----------> 
 * @m parse sample data
 * --> I N <-- @p
 * istream &ar - reference to input stream
\*/ 
void CRMFFile::ParseESND(istream &ar,bool bEncrypted)
{
	CRMFSound *pSound=new CRMFSound;
	pSound->Serialize(ar,bEncrypted);
	pSound->Decompress();
	m_Sounds.push_back(pSound);
}

/*\ 
 * <---------- CRMFFile::ParseECMI ----------> 
 * @m parse compressed midi
 * --> I N <-- @p
 * istream &ar - reference to the input stream
\*/ 
void CRMFFile::ParseECMI(istream &ar)
{
	ASSERT(m_pSequence == NULL);
	m_pSequence=new CRMFSequence(CRMFSequence::seqEcmi);
	m_pSequence->Serialize(ar);
}

/*\ 
 * <---------- CRMFFile::ParseSONG ----------> 
 * @m parse sequence
 * --> I N <-- @p
 * istream &ar - reference to input stream
\*/ 
void CRMFFile::ParseSONG(istream &ar)
{
	bool bRet=false;
	int nTextLen;
	unsigned char sEntry[1024];
	char *pcTitle;
	uint32_t nVersion;
	unsigned char *pc;
	uint32_t nTag,nTagCount;
	uint32_t nBlockSize;
	unsigned char *pcSong=NULL;
	unsigned char pcBuffer[5]={0x00,0x00,0x00,0x00,0x00};

	map<unsigned long,int> :: const_iterator iterChunk;
	map<unsigned long,int> mapChunkId;

	mapChunkId[nMakeID4("TITL")]=infoTitle;
	mapChunkId[nMakeID4("COMP")]=infoComposer;
	mapChunkId[nMakeID4("COPD")]=infoCopyright;
	mapChunkId[nMakeID4("LICC")]=infoPublisher;
	mapChunkId[nMakeID4("LDOM")]=infoLicenseURL;
	mapChunkId[nMakeID4("LTRM")]=infoLicenseTerm;
	mapChunkId[nMakeID4("EXPD")]=infoExpirationDate;
	mapChunkId[nMakeID4("NOTE")]=infoComposerNote;
	mapChunkId[nMakeID4("INDX")]=infoProduct;
	mapChunkId[nMakeID4("PERF")]=infoArtist;
	mapChunkId[nMakeID4("GENR")]=infoCategory;
	mapChunkId[nMakeID4("SUBG")]=infoSubCategory;
	mapChunkId[nMakeID4("TMPO")]=infoTempoDescription;
	mapChunkId[nMakeID4("OSRC")]=infoOriginalSource;
	mapChunkId[nMakeID4("LUSE")]=infoUseLicense;

	ar.read((char *)&nVersion,4);
	nVersion=ntohl(nVersion);
	Log2(5,"block version: %08X\n",nVersion);
		
	if ((pcTitle=pcReadString(ar)) != NULL)
	{
		Log2(verbLevDebug3,"song name: \"%s\"\n",pcTitle);
		ar.read((char *)&nBlockSize,4);
		nBlockSize=ntohl(nBlockSize);
		Log2(verbLevDebug3,"song block size: %d\n",nBlockSize);

		pcSong=new unsigned char[nBlockSize];
		ar.read((char *)pcSong,nBlockSize);

		hexdump("SONG: ",pcSong,49);

		nTagCount=(unsigned long)*(pcSong+49);
		Log2(verbLevDebug3,"expecting %d subtags\n",nTagCount);

		pc=pcSong+50;
		nBlockSize-=50;
		while (nBlockSize && nTagCount)
		{
			//nTag=*(uint32_t*)pc;
			memcpy(&nTag,pc,4);
			pc+=4;
			//TRACEIT("decrypting tag %s data...\n",pcSplitID4(nTag,(char *)pcBuffer));
			nTextLen=CRMFBasics::nDecryptText(pc,sEntry,1023);
			//TRACEIT("result: \"%s\"\n",sEntry);
			Log2(verbLevDebug3,"\"%s\" = \"%s\"\n",pcSplitID4(nTag,(char *)pcBuffer),sEntry);
			if ((iterChunk=mapChunkId.find(nTag)) == mapChunkId.end())
			{
				Log2(verbLevErrors,IDS_ERR_UNKSUBCHNK);
			}
			else
			{
				m_strInfo[iterChunk->second]=tstring((TCHAR *)sEntry);
			}
			nBlockSize-=4+nTextLen;
			pc+=nTextLen;
			--nTagCount;
		};
		if (nBlockSize)
		{
			Log2(verbLevWarnings,"trailing garbage in SONG tag data\n");
		}
		if (nTagCount)
		{
			Log2(verbLevErrors,"file truncated\n");
			throw new CFormatException(CFormatException::formaterrTruncated,"file truncated");
		}
		bRet=true;
		delete [] pcSong;
		delete [] pcTitle;
	}
	else
	{
		Log2(verbLevErrors,"invalid title\n");
		throw new CFormatException(CFormatException::formaterrInvalid,"invalid title");
	}
}

/*\ 
 * <---------- CRMFFile::nGetSamplePlaytime ----------> 
 * @m get the maximum sample playtime of an RMF
 * <-- OUT --> @r
 * int - playtime in microseconds
\*/ 
uint32_t CRMFFile::nGetSamplePlaytime(void)
{
	unsigned int iSound;
	uint32_t nMaxPlaytime=0;
	for (iSound=0;iSound < m_Sounds.size();iSound++)
		nMaxPlaytime=max(m_Sounds[iSound]->nGetPlaytime(),nMaxPlaytime);
	return nMaxPlaytime;
}

/*\ 
 * <---------- CRMFFile::nGetBitRate ----------> 
 * @m get the highest bitrate of a 
 * <-- OUT --> @r
 * int - 
\*/ 
uint32_t CRMFFile::nGetBitRate(void)
{
	unsigned int iSound;
	unsigned int nMax=0;
	for (iSound=0;iSound < m_Sounds.size();iSound++)
		nMax=max(m_Sounds[iSound]->nGetBitRate(),nMax);
	return nMax;
}

/*\ 
 * <---------- CRMFFile::nGetBitsPerSample ----------> 
 * @m get the maximum bits per sample
 * <-- OUT --> @r
 * int -  
\*/ 
uint32_t CRMFFile::nGetBitsPerSample(void)
{
	unsigned int iSound;
	uint32_t nMax=0;
	for (iSound=0;iSound < m_Sounds.size();iSound++)
		nMax=max(m_Sounds[iSound]->nGetStoredBitsPerSample(),nMax);
	return nMax;
}

/*\ 
 * <---------- CRMFFile::nGetFormat ----------> 
 * @m get the sample codec identifier used
 * <-- OUT --> @r
 * int - rmfformatAdpcm, rmfformatAlaw, rmfformatUlaw, rmfformatUncompressed 
\*/ 
uint32_t CRMFFile::nGetFormat(void)
{
	uint32_t nFormat=rmfformatUnknown;
	uint32_t nSaved=0;
	unsigned int iSound;
	for (iSound=0;iSound < m_Sounds.size();iSound++)
	{
		int nValue=m_Sounds[iSound]->nGetType();
		TRACEIT2("value=%d\n",nValue);
		if (nFormat == rmfformatUnknown || (nSaved == nValue && nFormat != rmfformatUnknown))
		{
			switch(nValue)
			{
				case CRMFSound::compAdpcm:	nFormat=rmfformatAdpcm;			break;
				case CRMFSound::compAlaw:	nFormat=rmfformatAlaw;			break;
				case CRMFSound::compUlaw:	nFormat=rmfformatUlaw;			break;
				case CRMFSound::compNone:	nFormat=rmfformatUncompressed;	break;
			}
			nSaved=nValue;
		}
		else
			nFormat=rmfformatMultiple;
	}
	return nFormat;
}

#ifdef RESOURCES_INCLUDED
/*\
 * <---------- CRMFFile :: sGetFormatName ----------> 
 * @m get the file format name
 * --> I N <-- @p
 * int nFormat - format identifier;
 * rmfformatSequence
 * rmfformatUncompressed
 * rmfformatAdpcm
 * rmfformatUlaw
 * rmfformatAlaw
 * rmfformatMpeg
 * rmfformatMultiple
 * <-- OUT --> @r
 * tstring - readable format name
\*/
tstring CRMFFile::sGetFormatName(int nFormat)
{
	CMyString strText;
	strText.Load(IDS_FORMATNAME_RMFSEQUENCE+(nFormat-rmfformatSequence));
	return strText;
}
#endif

/*\ 
 * <---------- CRMFFile::nGetSamplesPerSecond ----------> 
 * @m get the maximum sample frequency used
 * <-- OUT --> @r
 * uint32_t - highest sample frequency stored  
 \*/ 
uint32_t CRMFFile::nGetSamplesPerSecond(void)
{
	unsigned int iSound;
	uint32_t nMax=0;
	for (iSound=0;iSound < m_Sounds.size();iSound++)
		nMax=max(m_Sounds[iSound]->nGetSamplesPerSecond(),nMax);
	return nMax;
}

/*\ 
 * <---------- CRMFFile::nGetChannels ----------> 
 * @m get the number of channels used
 * <-- OUT --> @r
 * int - 1=mono, 2=stereo
\*/ 
uint32_t CRMFFile::nGetChannels(void)
{
	unsigned int iSound;
	uint32_t nMaxChannels=0;
	for (iSound=0;iSound < m_Sounds.size();iSound++)
		nMaxChannels=max(m_Sounds[iSound]->nGetChannels(),nMaxChannels);
	return nMaxChannels;
}

/*\ 
 * <---------- CRMFFile::nRenderSONGSubTags ----------> 
 * @m create metadata sub-tags including their encrypted data
 * --> I N <-- @p
 * CMobileSampleContent *pSource - source file data
 * unsigned char *pDest - destination buffer
 * <-- OUT --> @r
 * uint32_t - number of bytes used for this block
\*/ 
uint32_t CRMFFile::nRenderSONGSubTags(CMobileSampleContent *pSource,unsigned char *pDest)
{
	unsigned char *pStart=pDest;

	typedef struct 
	{
		int nStringIndex;
		const char *pszTag;
		int nInfoIndex;
	}SONGMap;

	SONGMap mmap[]=
	{
		{	paraStrTitle,		"TITL",	infoTitle		},
		{	paraStrComposer,	"COMP", infoComposer	},
		{	paraStrArtist,		"PERF", infoArtist		},
		{	paraStrCopyright,	"COPD", infoCopyright	},
		{	paraStrCategory,	"GENR",	infoCategory	},
		{	paraStrSubcategory,	"SUBG",	infoSubCategory	},
		{	paraStrPublisher,	"LICC",	infoPublisher	},
		{	paraStrNote,		"NOTE", infoComposerNote	},
		{	paraStrLicenseUrl,	"LDOM", infoLicenseURL	},
		{	paraStrLicenseUse,	"LUSE",	infoUseLicense},
		{	paraStrLicenseTerm,	"LTRM",	infoLicenseTerm},
		{	paraStrLicenseExp,	"EXPD",	infoExpirationDate},
		{	paraStrSource,		"OSRC",	infoOriginalSource},
		{	paraStrTempo,		"TMPO",	infoTempoDescription},
		{	0,					NULL	}
	};

	int i,nLen,nUsed=0;
	int nSubTagCount=0;
	tstring sParameter;
	for (i=0;mmap[i].pszTag != NULL;i++)
	{
		sParameter=pSource->m_strInfo[mmap[i].nInfoIndex];
		if (!sParameter.empty())
			++nSubTagCount;
	}
	if(pDest)
		pDest=CRMFBasics::pRenderInteger(pDest,nSubTagCount);

	nUsed+=4;
	for (i=0;mmap[i].pszTag != NULL;i++)
	{
		sParameter=pSource->m_strInfo[mmap[i].nInfoIndex];
		if (!sParameter.empty())
		{
			nLen=(unsigned long)sParameter.length()+1;
			if (pDest)
			{
				//copy tag
				memcpy(pDest,mmap[i].pszTag,4);
				pDest+=4;
				//encrypt string
				CRMFBasics::nEncryptText((unsigned char *)sParameter.c_str(),pDest,nLen);
				pDest+=nLen;
			}
			nUsed+=nLen+4;
		}
	}
	if(pDest)
	{
		ASSERT((unsigned long)(pDest-pStart) == (unsigned long)nUsed);
	}
	return nUsed;
}

/*\ 
 * <---------- CRMFFile::nRenderSONG ----------> 
 * @m create the master song segment
 * --> I N <-- @p
 * CMobileSampleContent *pSource - source sample data
 * rmfCACH *pCache - pointer to destination cache object
 * unsigned char *pDest - pointer to destination buffer (or NULL for size calculation)
 * <-- OUT --> @r
 * uint32_t - bytes used for encoding
\*/ 
uint32_t CRMFFile::nRenderSONG(CMobileSampleContent *pSource,rmfCACH *pCache,unsigned char *pDest)
{
	int nLen,nUsed=0;
	uint32_t nRMFValue;
	uint32_t nSubTagSize=nRenderSONGSubTags(pSource,NULL);
	//unpatched song property data
	unsigned char cSONGData[46]=
	{
		0xFF,0xFF,0x00,0x01, 0xFF,0xFF,0x01,0x01, 0x00,0x00,0x00,0x04, 0x00,0x38,0x00,0x08,
		0x00,0xFF,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00
	};
	cSONGData[0]=((CRMFSequence::sequenceID)>>8)&0xFF;
	cSONGData[1]=(CRMFSequence::sequenceID)&0xFF;
	if (m_pParameters->m_nParameter[paraNumTempo] != 100)
	{
		nRMFValue=m_pParameters->m_nParameter[paraNumTempo]*500;
		if (nRMFValue%3)
			nRMFValue=(nRMFValue/3)+1;
		else
			nRMFValue=nRMFValue/3;
	}
	else
		nRMFValue=0;
	cSONGData[4]=(unsigned char)(((nRMFValue)>>8)&0xFF);
	cSONGData[5]=(unsigned char)((nRMFValue)&0xFF);
	cSONGData[17]=(unsigned char)(((unsigned long)m_pParameters->m_nParameter[paraNumVolume]*127)/100);
	if (pDest)
	{
		ASSERT(pCache);
		memcpy(pDest,"SONG",4);
		memcpy(pCache->sTag,"SONG",4);
		pDest+=4;
	}
	nUsed+=4;
	if (pDest)
	{
		pCache->nFirstValue=0x0000;
		pDest=CRMFBasics::pRenderInteger(pDest,pCache->nFirstValue);
	}
	nUsed+=4;
	nLen=(int)m_pParameters->m_strParameter[paraStrTitle].length();
	if (pDest)
	{
		pCache->nNameOffset=nUsed;
		*(pDest++)=(unsigned char)nLen;
		memcpy(pDest,(unsigned char *)m_pParameters->m_strParameter[paraStrTitle].c_str(),nLen);
		pDest+=nLen;
	}
	nUsed+=nLen+1;
	if (pDest)
	{
		pCache->nDataSize=nSubTagSize+46;
		pDest=CRMFBasics::pRenderInteger(pDest,pCache->nDataSize);
	}
	nUsed+=4;
	if (pDest)
	{
		pCache->nDataOffset=nUsed;
		memcpy(pDest,cSONGData,46);
		pDest+=46;
	}
	nUsed+=46;
	nUsed+=nRenderSONGSubTags(pSource,pDest);
	return nUsed;
}

/*\ 
 * <---------- CRMFFile::nRenderINST ----------> 
 * @m render the instrument data
 * --> I N <-- @p
 * rmfCACH *pCache - pointer to the cache entry destination
 * unsigned char *pDest - pointer to destination buffer
 * <-- OUT --> @r
 * uint32_t - number of bytes used for encoding
\*/ 
uint32_t CRMFFile::nRenderINST(rmfCACH *pCache,unsigned char *pDest)
{
	CRMFInstrument inst(m_pParameters->m_strParameter[paraStrTitle].c_str());
	return inst.nRender(pCache,pDest);
}

/*\ 
 * <---------- CRMFFile::nRenderESND ----------> 
 * @m render the sound data containing our sample
 * --> I N <-- @p
 * rmfCACH *pCache - pointer to the cache entry destination
 * int nType - identifier describing the sample encoding
 * compNone - zse no compression
 * compAdpcm - use Apple specific IMA ADPCM
 * int nChannels - number of sample channels
 * int nSampleRate - sample frequency
 * uint32_tnSourceSize - raw source sample data size
 * bool bLoop - loop sample continously
 * unsigned char *pcSource - source sample data buffer
 * unsigned char *pcDest - destination data buffer
 * <-- OUT --> @r
 * uint32_t - number of bytes used for encoding
\*/ 
uint32_t CRMFFile::nRenderESND(rmfCACH *pCache,int nType,int nChannels,int nSampleRate,uint32_t nSourceSize,bool bLoop,unsigned char *pcSource,unsigned char *pcDest)
{
	CRMFSound esnd(m_pParameters->m_strParameter[paraStrTitle].c_str());
	return esnd.nRender(pCache,nType,nChannels,nSampleRate,nSourceSize,bLoop,pcSource,pcDest);
}

/*\ 
 * <---------- CRMFFile::nRenderEMID ----------> 
 * @m render an RMF sequence triggering our sample data
 * --> I N <-- @p
 * rmfCACH *pCache - pointer to the cache entry destination
 * int nType - identifier describing the sequence data encoding;
 * seqMidi=plain MIDI
 * seqEcmi=encrypted compressed MIDI 
 * seqCmid=compressed MIDI
 * seqEmid=encrypted MIDI
 * int nChannels - number of sample channels
 * int nSampleRate - sample frequency
 * uint32_t nSourceSize - sample size
 * uint32_t nPlaytime - sample duration
 * uint32_t nFadetime - sample fade out duration
 * unsigned char *pcDest - output buffer (or NULL for size calculation)
 * <-- OUT --> @r
 * uint32_t - number of bytes rendered
\*/ 
uint32_t CRMFFile::nRenderEMID(rmfCACH *pCache,int nType,int nChannels,int nSampleRate,uint32_t nSourceSize,uint32_t nPlaytime,uint32_t nFadetime,unsigned char *pcDest)
{
	CRMFSequence emid(nType,m_pParameters->m_strParameter[paraStrTitle].c_str());
	return emid.nRender(pCache,pcDest,nChannels,nSampleRate,nSourceSize,nPlaytime,nFadetime);
}

/*\ 
 * <---------- CRMFFile::nRenderCACH ----------> 
 * @m create cached index of some important file sections
 * --> I N <-- @p
 * int nTags - number of cache data packets to store
 * rmfCACH *pCach - cache data packets
 * unsigned char *pcDest - destination data buffer (or NULL for size calculation)
 * <-- OUT --> @r
 * uint32_t- number of bytes written
 \*/ 
uint32_t CRMFFile::nRenderCACH(int nTags,rmfCACH *pCach,unsigned char *pcDest)
{
	uint32_t nUsed=0;

	const unsigned char cCACH[13]=
	{
		0x43, 0x41, 0x43, 0x48,  0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00,  0x90
	};
	if (pcDest)
	{
		memcpy(pcDest,cCACH,13);
		pcDest+=13;
	}
	nUsed+=13;
	if (pcDest)
		pcDest=CRMFBasics::pRenderInteger(pcDest,nTags);
	nUsed+=4;
	for (int i=0;i < nTags;i++)
	{
		if (pcDest)
		{
			memcpy(pcDest,&pCach->sTag,4);
			pcDest+=4;
		}
		nUsed+=4;
		if (pcDest)
			pcDest=CRMFBasics::pRenderInteger(pcDest,pCach->nFirstValue);
		nUsed+=4;
		if (pcDest)
			pcDest=CRMFBasics::pRenderInteger(pcDest,pCach->nDataSize);
		nUsed+=4;
		if (pcDest)
			pcDest=CRMFBasics::pRenderInteger(pcDest,pCach->nNameOffset+4);
		nUsed+=4;
		if (pcDest)
			pcDest=CRMFBasics::pRenderInteger(pcDest,pCach->nDataOffset+4);
		nUsed+=4;
		pCach++;
	}
	nUsed+=20;
	return nUsed;
}

/*\ 
 * <---------- CRMFFile::RenderDestination ----------> 
 * @m render an RMF file out of the given sample source data (+ attributes and metadata)
 * --> I N <-- @p
 * ostream &out - reference to the output stream
 \*/ 
void CRMFFile::Write(ostream &out)
{
	const unsigned char cVERS[2][19] = 
	{
		{
			0x56, 0x45, 0x52, 0x53, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x02, 0x00, 
			0x00, 0x00, 0x00
		},
		{
			0x56, 0x45, 0x52, 0x53, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x01, 0x00, 
			0x06, 0x00, 0x00
		}
	};
	Packer pk(out,false);

	rmfCACH cache[15];

	unsigned nSONGSize;
	unsigned nINSTSize;
	unsigned nCACHSize;
	unsigned nEMIDSize;
	unsigned nESNDSize;
	uint32_t nRMFValue;

	int iTag=0;
	int nTagCount=0x07;
	uint32_t nOffset=0;

	Log2(verbLevMessages,IDS_PRG_SAMPLEATTRIBS,m_pCSSource->m_nCSBitsPerSample,m_pCSSource->m_nCSSamplesPerSecond,m_pCSSource->m_nCSChannels,m_pCSSource->m_nFileSize,m_pCSSource->m_nCSSize);

	nSONGSize=nRenderSONG(m_pCSSource,NULL);
	Log2(verbLevDebug3,"nSONGSize = %d\n",nSONGSize);

	nINSTSize=nRenderINST(NULL,NULL);
	Log2(verbLevDebug3,"nINSTSize = %d\n",nINSTSize);

	nESNDSize=nRenderESND(	NULL,
							CRMFSound::compAdpcm,
							m_pCSSource->m_nCSChannels,
							m_pCSSource->m_nCSSamplesPerSecond,
							m_pCSSource->m_nCSSize,
							m_pParameters->m_nParameter[paraNumPlaytime] > 0,
							NULL,
							NULL);
	Log2(verbLevDebug3,"nESNDSize = %d\n",nESNDSize);

	nEMIDSize=nRenderEMID(	NULL,
							CRMFSequence::seqEmid,
							m_pCSSource->m_nCSChannels,
							m_pCSSource->m_nCSSamplesPerSecond,
							m_pCSSource->m_nCSSize,
							m_pParameters->m_nParameter[paraNumPlaytime],
							m_pParameters->m_nParameter[paraNumFadetime],
							NULL);
	Log2(verbLevDebug3,"nEMIDSize = %d\n",nEMIDSize);

	out.write("IREZ",4);									//IREZ
	nRMFValue=htonl(0x01);					
	out.write((char *)&nRMFValue,4);						//00 00 00 01
	nRMFValue=htonl(nTagCount);
	out.write((char *)&nRMFValue,4);						//subtag count
	nOffset=12;

	nRMFValue=htonl(nOffset+nINSTSize+4);
	out.write((char *)&nRMFValue,4);						//skip mark
	unsigned char *pINST=new unsigned char [nINSTSize];
	nRenderINST(&cache[iTag],pINST);						//INST
	cache[iTag].nNameOffset+=nOffset;
	cache[iTag].nDataOffset+=nOffset;
	out.write((const char *)pINST,nINSTSize);
	hexdump("inst: ",pINST,nINSTSize);
	nOffset+=nINSTSize+4;
	++iTag;


	nRMFValue=htonl(nOffset+nESNDSize+4);
	out.write((char *)&nRMFValue,4);						//skip mark
	unsigned char *pESND=new unsigned char [nESNDSize];
	memset(pESND,0,nESNDSize);

	nRenderESND(	&cache[iTag],							//esnd
					CRMFSound::compAdpcm,
					m_pCSSource->m_nCSChannels,
					m_pCSSource->m_nCSSamplesPerSecond,
					m_pCSSource->m_nCSSize,
					m_pParameters->m_nParameter[paraNumPlaytime] > 0,
					(unsigned char *)m_pCSSource->m_pcCSBuffer,
					pESND);
	cache[iTag].nNameOffset+=nOffset;
	cache[iTag].nDataOffset+=nOffset;
	out.write((const char *)pESND,nESNDSize);
	hexdump("esnd: ",pESND,80);
	nOffset+=nESNDSize+4;
	++iTag;


	nRMFValue=htonl(nOffset+nEMIDSize+4);
	out.write((char *)&nRMFValue,4);						//skip mark
	unsigned char *pEMID=new unsigned char [nEMIDSize];
	memset(pEMID,0,nEMIDSize);
	nRenderEMID(&cache[iTag],
				CRMFSequence::seqEmid,
				m_pCSSource->m_nCSChannels,
				m_pCSSource->m_nCSSamplesPerSecond,
				m_pCSSource->m_nCSSize,
				m_pParameters->m_nParameter[paraNumPlaytime],
				m_pParameters->m_nParameter[paraNumFadetime],
				pEMID);
	cache[iTag].nNameOffset+=nOffset;
	cache[iTag].nDataOffset+=nOffset;
	out.write((const char *)pEMID,nEMIDSize);
	hexdump("emid: ",pEMID,nEMIDSize);
	nOffset+=nEMIDSize+4;
	++iTag;

	nRMFValue=htonl(nOffset+nSONGSize+4);
	out.write((char *)&nRMFValue,4);						//skip mark
	unsigned char *pSONG=new unsigned char [nSONGSize];
	nRenderSONG(m_pCSSource,&cache[iTag],pSONG);				//SONG
	cache[iTag].nNameOffset+=nOffset;
	cache[iTag].nDataOffset+=nOffset;
	out.write((const char *)pSONG,nSONGSize);
	hexdump("song: ",pSONG,nSONGSize);
	nOffset+=nSONGSize+4;
	++iTag;
	
	nRMFValue=htonl(nOffset+19+4);
	out.write((char *)&nRMFValue,4);						//skip mark
	memcpy(cache[iTag].sTag,"VERS",4);
	cache[iTag].nDataSize=6;
	cache[iTag].nFirstValue=0x0000;
	cache[iTag].nNameOffset=nOffset+8;
	cache[iTag].nDataOffset=nOffset+8+5;
    out.write((const char *)cVERS[0],19);
	nOffset+=19+4;
	++iTag;

	nRMFValue=htonl(nOffset+19+4);
	out.write((char *)&nRMFValue,4);						//skip mark
	memcpy(cache[iTag].sTag,"VERS",4);
	cache[iTag].nFirstValue=0x0001;
	cache[iTag].nDataSize=6;
	cache[iTag].nNameOffset=nOffset+8;
	cache[iTag].nDataOffset=nOffset+8+5;
    out.write((const char *)cVERS[1],19);
	nOffset+=19+4;
	++iTag;

	nCACHSize=nRenderCACH(iTag,&cache[0],NULL);
	Log2(5,"nCACHSize = %d\n",nCACHSize);
	nRMFValue=htonl(nOffset+nCACHSize+4);
	out.write((char *)&nRMFValue,4);						//skip mark
	unsigned char *pCACH=new unsigned char [nCACHSize];
	memset(pCACH,0,nCACHSize);
	nRenderCACH(iTag,&cache[0],pCACH);						//CACH
	out.write((const char *)pCACH,nCACHSize);
	hexdump("cach: ",pCACH,nCACHSize);

	m_nCSBitsPerSample=4;
	m_nCSChannels=m_pCSSource->m_nCSChannels;
	m_nCSSamplesPerSecond=m_pCSSource->m_nCSSamplesPerSecond;
	m_nCSSize=nESNDSize;
	m_nFileSize=out.tellp();
}
