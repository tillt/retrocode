/*\
 * SWFFile.cpp
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
#include <map>
#include <math.h>
#include <iostream>
#include <strstream>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include "../include/Resource.h"
#include "../retroBase/Basics.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MyString.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/Endian.h"
#include "../retroBase/Packer.h"
#include "../retroBase/CompressedIO.h"
#include "../retroBase/PacketCollection.h"
#include "MP3File.h"
#include "SWFFile.h"
#include "MP3Property.h"
#include "SWFProperty.h"

DYNIMPPROPERTY(CSWFFile,CSWFProperty)

CSWFFile::CSWFFile(void)
{
	m_nMagicSize=3;
	m_pcMagic="FWS";
	m_bContainsSound=false;
	m_bContainsPicture=false;
	m_bCompressed=false;
	m_sFormatName="MacroMedia Flash (SWF)";
	m_sDefaultExtension=_T("swf");
	m_sFormatDescription.Load(IDS_FORMDESC_SWF);
	m_sFormatCredits=_T("The SWF codec is entirely based on: \"Macromedia Flash (SWF) and Flash Video (FLV) File Format Specification Version 8\", Copyright (c) 2006 by Adobe Inc." );

	m_bContainsMP3=false;
	m_bUsesStreaming=false;
	m_nLoops=0;
	m_nFrameRate=2560;
	/*
	m_encodingCaps.freqs[	CSampleCompatibility::supports8Bit|
							CSampleCompatibility::supports16Bit|
							CSampleCompatibility::supportsMono|
							CSampleCompatibility::supportsStereo].addDigit(5512);
							*/
	/*
	m_encodingCaps.freqs[	CSampleCompatibility::supports8Bit|
							CSampleCompatibility::supports16Bit|
							CSampleCompatibility::supportsMono|
							CSampleCompatibility::supportsStereo].addDigit(11025);
	m_encodingCaps.freqs[	CSampleCompatibility::supports8Bit|
							CSampleCompatibility::supports16Bit|
							CSampleCompatibility::supportsMono|
							CSampleCompatibility::supportsStereo].addDigit(22050);
	m_encodingCaps.freqs[	CSampleCompatibility::supports8Bit|
							CSampleCompatibility::supports16Bit|
							CSampleCompatibility::supportsMono|
							CSampleCompatibility::supportsStereo].addDigit(44100);
	*/
//	m_encodingPara.addPara(cmdParaNumber,paraNumBitrate);
//	m_encodingPara.addPara(cmdParaBool,paraBoolAllowVBR);
//	m_encodingPara.addPara(cmdParaBool,paraBoolAllowCRC);
//	m_encodingPara.addPara(cmdParaBool,paraBoolAllowJointStereo);
	
	m_encodingPara.addPara(cmdParaNumber,paraNumFrameWidth);
	m_encodingPara.addPara(cmdParaNumber,paraNumFrameHeight);
	m_encodingPara.addPara(cmdParaNumber,paraNumBitrate);
	m_encodingPara.addPara(cmdParaNumber,paraNumBackgroundRGB);

	endian.init();
}

CSWFFile::~CSWFFile(void)
{
}

/*\
 * <---------- bMagicHead ---------->
 * @m 
 * --> I N <-- @p
 * std::istream &ar - 
 * uint32_tnSize - 
 * <-- OUT --> @r
 * bool - 
\*/
bool CSWFFile::bMagicHead(std::istream &ar,uint32_t nSize)
{
	char sCompare[4];
	bool bRet=false;

	if (m_nMagicSize == 0 || (int)nSize < m_nMagicSize)
		return false;
	try 
	{
		ar.read(sCompare,m_nMagicSize);
		sCompare[m_nMagicSize]=0;
		if (!strcmp(sCompare,"FWS"))
			bRet=true;
		else if (!strcmp(sCompare,"CWS"))
			bRet=true;
	}
	catch(istream::failure const &e)
	{
		Log2(verbLevErrors,"read error on bit-stream magic head (%s)\n",e.what());
		TRACEIT("catching exception in bMagicHead\n");
		bRet=false;
	}
	return bRet;
}

void CSWFFile::CompressMP3(CMobileSampleContent *pSource,unsigned char **ppDest,uint32_t*pnSize)
{
	CMP3File mpeg;
	stringstream memfile;

	mpeg.AttachParameter(m_pParameters);

	ASSERT(pnSize);
	ASSERT(ppDest);
	if (pnSize && ppDest)
	{
		Log2(verbLevDebug3,"rendering embedded MP3...\n");
		memfile.exceptions(ios_base::badbit|ios_base::eofbit|ios_base::failbit);
		mpeg.AttachSourceSample(pSource);
		mpeg.Write(memfile);
		if ((*pnSize=memfile.tellp()) > 0)
		{
			*ppDest=new unsigned char[*pnSize];
			memfile.seekg(0,ios_base::beg);
			memfile.read((char *)*ppDest,*pnSize);
			Log2(verbLevDebug3,"rendered %d byte of MP3 data\n",*pnSize);
		}
		else
		{
			Log2(verbLevErrors,"failed to encode mp3 data\n");
			throw new CFormatException(CFormatException::formaterrIncompatibleSampleFormat);
		}
	}
}

/*\
 * <---------- DecompressMP3 ---------->
 * @m decompress an mp3 from a memory block
 * --> I N <-- @p
 * void *pMpeg - pointer to mp3 data
 * uint32_tnSize - size of the block
\*/
void CSWFFile::DecompressMP3(void *pMpeg,uint32_t nSize)
{
	CMP3File mpeg;
	stringstream memfile;
	Log2(verbLevDebug3,"parsing embedded MP3...\n");
	memfile.exceptions(ios_base::badbit|ios_base::eofbit|ios_base::failbit);
	memfile.write((const char *)pMpeg,nSize);
	memfile.seekg(0,ios_base::beg);
	mpeg.Write(memfile);
	m_nCSSize=mpeg.m_nCSSize;
	m_nCSBitsPerSample=16;
	m_bContainsMP3=true;
	if (mpeg.nGetChannels() != m_nCSChannels)
	{
		Log2(verbLevWarnings,"embedded mp3 differs from wrapper description in channel count\n");
		m_nCSChannels=mpeg.nGetChannels();
	}
	if ((unsigned int)mpeg.nGetSamplesPerSecond() != (unsigned int)m_nCSSamplesPerSecond)
	{
		Log2(verbLevWarnings,"embedded mp3 differs from wrapper description in sampling frequency\n");
		m_nCSSamplesPerSecond=mpeg.nGetSamplesPerSecond();
	}
	m_nBitRate=mpeg.nGetBitRate();
	m_nMinBitRate=mpeg.nGetMinBitRate();
	m_nMaxBitRate=mpeg.nGetMaxBitRate();
	m_nPlayTime=mpeg.nGetPlaytime();
	m_bCRC=mpeg.bGetCRC();
	m_bVBR=mpeg.bGetVBR();
	m_bJointStereo=mpeg.bGetJointStereo();
	m_bOriginalBit=mpeg.bGetOriginalBit();
	m_bCopyrightBit=mpeg.bGetCopyrightBit();
	m_bPrivateBit=mpeg.bGetPrivateBit();
	if ((m_pcCSBuffer=CMobileSampleContent::Alloc(m_nCSSize)) != NULL)
		memcpy(m_pcCSBuffer,mpeg.m_pcCSBuffer,m_nCSSize);
}

/*\
 * <---------- bParseStreamBlock ---------->
 * @m 
 * --> I N <-- @p
 * swf_tag *pTag - 
 * <-- OUT --> @r
 * bool - 
\*/
bool CSWFFile::bParseStreamBlock(swf_tag *pTag)
{
	bool bRet=false;
	if (pTag->pcData && pTag->nDataSize >= 4)
	{
		if (m_pcCSBuffer == NULL)
		{
			unsigned short int nSampleCount;
			unsigned char *pcRead=pTag->pcData;
			nSampleCount=*(unsigned short int *)pcRead;
			pcRead+=2;
			switch(m_nSoundCompression)
			{
				case swfcodecMP3:		//mp3
				{
					signed short int nSeekSamples;
					nSeekSamples=endian.wFromLittle(*(signed short int *)pcRead);
					pcRead+=2;
					//collect mp3 frame
					m_bc.CreateCopyPacket((unsigned char *)pcRead,pTag->nDataSize-4);
				}
				break;
				default:
					m_bc.CreateCopyPacket((unsigned char *)pcRead,pTag->nDataSize-2);
			}
		}
	}
	return bRet;
}

/*\
 * <---------- bParseStreamHead ---------->
 * @m 
 * --> I N <-- @p
 * swf_tag *pTag - 
 * <-- OUT --> @r
 * bool - 
\*/
bool CSWFFile::bParseStreamHead(swf_tag *pTag)
{
	bool bRet=false;
	const unsigned int nRates[4]={5500,11025,22050,44100};
	const unsigned int nComp[15]={1,2,3,1,0,0,4,0,0,0,0,0,0,0,0};
	if (pTag->nDataSize >= 4)
	{
		if (m_pcCSBuffer == NULL)
		{
			unsigned char *pcRead=pTag->pcData;
			ASSERT(pcRead);
			m_bUsesStreaming=true;
			m_bContainsSound=true;
			m_nCSSamplesPerSecond=nRates[(*pcRead>>2)&0x03];
			m_nCSBitsPerSample=*pcRead&2 ? 16 : 8;
			m_nCSChannels=*pcRead&1 ? 2 : 1;
			TRACEIT2("channels %d\n",m_nCSChannels);
			TRACEIT2("sample rate %d\n",m_nCSSamplesPerSecond);
			TRACEIT2("bits per sample %d\n",m_nCSBitsPerSample);
			++pcRead;
			m_nSoundCompression=nComp[*pcRead>>4];
			TRACEIT2("compression %d\n",m_nSoundCompression);
			bRet=true;
			switch(m_nSoundCompression)
			{
				case swfcodecMP3:			m_bContainsMP3=true;																				break;
				case swfcodecAdpcm:			throw new CFormatException(CFormatException::formaterrIncompatibleSampleFormat,"adpcm");			break;
				case swfcodecNellyMoser:	throw new CFormatException(CFormatException::formaterrIncompatibleSampleFormat,"nelly moser");		break;
				default:
					Log2(verbLevErrors,"unsupported streaming sound format\n");
					throw new CFormatException(CFormatException::formaterrIncompatibleSampleFormat,"unsupported streaming sound format");
			}
		}
	}
	return bRet;
}

/*\
 * <---------- bParseDefineSound ---------->
 * @m DEFINE_SOUND tag parsing
 * --> I N <-- @p
 * swf_tag *pTag - tag data
 * <-- OUT --> @r
 * bool - true=ok
\*/
bool CSWFFile::bParseDefineSound(swf_tag *pTag)
{
	bool bRet=false;
	unsigned short int nID;
	unsigned char cFlags;
	unsigned char *pcRead=pTag->pcData;
	const unsigned int nRates[4]={5500,11025,22050,44100};
	const unsigned int nComp[15]={1,2,3,1,0,0,4,0,0,0,0,0,0,0,0};
	
	if (pTag->nDataSize >= 3)
	{
		nID=endian.wFromLittle(*(unsigned short int *)pcRead);		
		pcRead+=2;
		cFlags=*pcRead;
		pcRead+=1;
		//any sound found yet?
		if (m_pcCSBuffer == NULL)
		{	//nope->...
			TRACEIT2("number of samples %d\n",endian.lFromLittle(*(uint32_t*)pcRead));
			pcRead+=4;
			m_nCSSamplesPerSecond=nRates[(cFlags>>2)&3];
			TRACEIT2("samples per second in swf wrapper %d\n",m_nCSSamplesPerSecond);
			m_nCSBitsPerSample=cFlags&2 ? 16 : 8;
			m_nCSChannels=cFlags&1 ? 2 : 1;
			//map compression identifier for simplification
			m_nSoundCompression=nComp[cFlags>>4];		
			switch (m_nSoundCompression)
			{
				case swfcodecNellyMoser:		//nellymoser
				{
					Log2(verbLevErrors,"Nellymoser ASAO is currently not supported\n");
					throw new CFormatException(CFormatException::formaterrIncompatibleSampleFormat,"Nellymoser ASAO is currently not supported");
				}
				break;
				case swfcodecAdpcm:				//adpcm
				{
					Log2(verbLevErrors,"Flash ADPCM is currently not supported\n");
					throw new CFormatException(CFormatException::formaterrIncompatibleSampleFormat,"Flash ADPCM is currently not supported");
				}
				break;
				case swfcodecRaw:				//raw
				{
					m_nCSSize=pTag->nDataSize-7;
					if ((m_pcCSBuffer=CMobileSampleContent::Alloc(m_nCSSize)) != NULL)
						memcpy(m_pcCSBuffer,pcRead,m_nCSSize);
					m_nPlayTime=nGetSamplePlaytime();
				}
				break;
				case swfcodecMP3:				//mp3
					TRACEIT2("mp3 prefix: %04X\n",*(unsigned short *)pcRead);
					DecompressMP3(pcRead+2,pTag->nDataSize-9);
				break;
				default:
					Log2(verbLevErrors,"unknown codec\n");
					throw new CFormatException(CFormatException::formaterrIncompatibleSampleFormat,"unknown codec");
			}			
			bRet=true;
			m_bContainsSound=true;
			TRACEIT2("id: %d, sound compression: %d, samplerate: %d, bit per sample %d, channels %d\n",nID,m_nSoundCompression,m_nCSSamplesPerSecond,m_nCSBitsPerSample,m_nCSChannels);
		}
	}
	return bRet;
}

/*\
 * <---------- bParseStartSound ---------->
 * @m 
 * --> I N <-- @p
 * swf_tag *pTag - 
 * <-- OUT --> @r
 * bool - 
\*/
bool CSWFFile::bParseStartSound(swf_tag *pTag)
{
	bool bRet=false;
	unsigned short int nID;
	unsigned char cFlags;
	unsigned char *pcRead=pTag->pcData;
	if (pTag->nDataSize >= 3)
	{
		nID=endian.wFromLittle(*(unsigned short int *)pcRead);
		pcRead+=2;
	
		cFlags=*(unsigned char *)pcRead;
		pcRead+=1;

		bRet=true;

		if(cFlags & SI_HASINPOINT)
		{
			m_nCueIn=endian.lFromLittle(*(uint32_t *)pcRead);
			pcRead+=4;
			Log2(verbLevDebug1,"sound info: inpoint %d\n",m_nCueIn);
		}
		if(cFlags & SI_HASOUTPOINT)
		{
			m_nCueOut=endian.lFromLittle(*(uint32_t *)pcRead);
			pcRead+=4;
			Log2(verbLevDebug1,"sound info: outpoint %d\n",m_nCueOut);
		}
		if(cFlags & SI_HASLOOPS)
		{
			m_nLoops=endian.wFromLittle(*(unsigned short int *)pcRead);
			pcRead+=2;
			Log2(verbLevDebug1,"sound info: loops %d\n",m_nLoops);
		}
		if(cFlags & SI_STOP)
		{
			Log2(verbLevDebug1,"sound info: stop\n");
		}
		if(cFlags & SI_NOMULTIPLE)
		{
			Log2(verbLevDebug1,"sound info: no multiple\n");
		}
		if(cFlags & SI_HASENVELOPE)
 		{
			Log2(verbLevDebug1,"sound info: envelope - not implemented\n");
			throw new CFormatException(CFormatException::formaterrParameters,"sound info: envelope - not implemented");
		}
	}
	return bRet;
}

/*\
 * <---------- read ---------->
 * @m 
 * --> I N <-- @p
 * istream &ar - 
\*/
void CSWFFile::Serialize(istream &ar)
{
	bool bFailed=false;
	uint32_t nFileSize;
	unsigned char theByte;
	Unpacker up(ar,false);

	ar.seekg(0,ios_base::beg);
	ar.read((char *)&theByte,1);
	switch(theByte)
	{
		case 'F':
		{
			TRACEIT2("its an uncompressed file\n");
			m_bCompressed=false;
		}
		break;
		case 'C':
		{
			TRACEIT2("its a compressed file - needs decompression first...\n");
			m_bCompressed=true;
		}
		break;
		default:
			Log2(verbLevErrors,"not a valid Flash header\n");
			throw new CFormatException(CFormatException::formaterrSource,"not a valid Flash header");
	}
	ar.seekg(2,ios_base::cur);
	ar.read((char *)&theByte,1);
	m_nFormatVersion=theByte;
	TRACEIT2("format version %d\n",m_nFormatVersion);	
	ar.read((char *)&nFileSize,4);
	TRACEIT2("file size %d\n",nFileSize);
	if (nFileSize)
	{
		char *pcRaw;
		if (m_bCompressed)
		{
			char *pcCompressed;
			pcCompressed=new char[m_nFileSize-8];
			ar.read(pcCompressed,m_nFileSize-8);
			pcRaw=new char [nFileSize];
			nDecompressZLIB((unsigned char *)pcCompressed,m_nFileSize-8,(unsigned char *)pcRaw,nFileSize);
			delete [] pcCompressed;
		}
		else
		{
			nFileSize=m_nFileSize-8;
			pcRaw=new char [nFileSize];
			ar.read(pcRaw,nFileSize);
		}
		hexdump("",(unsigned char *)pcRaw,10);
		//init bitstream read from memory
		readInit(pcRaw,nFileSize);

		//read movie size
		readRect(&m_rcMovieSize);
		TRACEIT2("display size %d x %d\n",(m_rcMovieSize.right-m_rcMovieSize.left)/20,(m_rcMovieSize.bottom-m_rcMovieSize.top)/20);

		//frame rate
		if (!bFailed && !readShort(&m_nFrameRate))
			bFailed=true;
		else
		{
			TRACEIT2("frame rate %f\n",(float)m_nFrameRate/256.0);
		}

		//frame count
		if (!bFailed && !readShort(&m_nFrameCount))
			bFailed=true;
		else
		{
			TRACEIT2("frame count %d\n",m_nFrameCount);
		}

		//read through all available tags
		swf_tag *pTag=NULL;
		while ((pTag=readTag()) != NULL)
		{
			Log2(verbLevDebug3,"%08X - tag: %02Xh, len: %d byte\n",m_pc-m_pcStart,pTag->nID,pTag->nLen);
			//if (pTag->nDataSize && pTag->pcData)
				//hexdump("",pTag->pcData,min(pTag->nDataSize,16));
			switch (pTag->nID)
			{
				case ST_DEFINESOUND:		bParseDefineSound(pTag);	break;
				case ST_SOUNDSTREAMHEAD:	bParseStreamHead(pTag);		break;
				case ST_SOUNDSTREAMBLOCK:	bParseStreamBlock(pTag);	break;
				case ST_STARTSOUND:			bParseStartSound(pTag);		break;
			}
			if (pTag->pcData != NULL)
				delete [] pTag->pcData;
			delete pTag;
			pTag=NULL;
		};
		delete [] pcRaw;
		if (m_bUsesStreaming && m_bc.nGetSize())
		{
			if (m_bContainsMP3)
			{
				unsigned char *pBuffer;
				uint32_t nSize;
				nSize=m_bc.nGetSize();
				pBuffer=new unsigned char [nSize];
				m_bc.nCopyLinear(pBuffer,nSize);
				DecompressMP3(pBuffer,nSize);
				delete [] pBuffer;
			}
			else
			{
				m_nCSSize=m_bc.nGetSize();
				if ((m_pcCSBuffer=CMobileSampleContent::Alloc(m_nCSSize)) != NULL)
					m_bc.nCopyLinear((unsigned char *)m_pcCSBuffer,m_nCSSize);
			}
		}
	}
	else
	{
		Log2(verbLevErrors,"not a valid Flash header\n");
		throw new CFormatException(CFormatException::formaterrSource,"not a valid Flash header");
	}
}

/*\
 * <---------- readTag ---------->
 * @m read one tag from input memory
 * <-- OUT --> @r
 * CSWFFile::swf_tag * - pointer to tag package or NULL
\*/
CSWFFile::swf_tag *CSWFFile::readTag(void)
{
	unsigned short int raw;
	uint32_t len;
	int id;
	swf_tag *pTag=NULL;

	bool bFailed=!readShort(&raw);

	if (!bFailed)
	{
		len = raw&0x3f;
		id  = raw>>6;
		if (len == 0x3f)
			bFailed=!readLong(&len);
	}
	if (!bFailed)
	{
		if (id == ST_DEFINESPRITE) 
			len = 4;
		pTag = new swf_tag;
		memset(pTag,0,sizeof(swf_tag));
		pTag->nID=id;
		pTag->nLen=len;
		if (len)
		{ 
			pTag->pcData = new unsigned char [len];
			pTag->nDataSize = len;
			bFailed=!readBytes(pTag->pcData,len);
		}
	}
	if (bFailed)
	{
		if (pTag != NULL)
		{
			if (pTag->pcData != NULL)
				delete [] pTag->pcData;
			delete pTag;
		}
		pTag=NULL;
	}
	return pTag;
}

/*\
 * <---------- readBit ---------->
 * @m read one bit from input memory
 * <-- OUT --> @r
 * unsigned char - return bit value
\*/
unsigned char CSWFFile::readBit(void)
{
	if(m_nBitpos == 8) 
	{
		m_nBitpos=0;
		m_cBitBuffer=*(m_pc++);
	}
	return (m_cBitBuffer >> (7-m_nBitpos++))&1;
}

/*\
 * <---------- readBits ---------->
 * @m read up to 32 bit from input memory
 * --> I N <-- @p
 * int nSize - bit count
 * <-- OUT --> @r
 * int - accumulated value
\*/
int CSWFFile::readBits(int nSize)
{
	int i;
	int val = 0;
	for(i=0;i < nSize;i++)
	{
		val<<=1;
		val|=readBit();
	}
	return val;
}

/*\
 * <---------- readRect ---------->
 * @m read a rectangle specifier from input memory
 * --> I N <-- @p
 * swf_rectangle *rc - pointer to rectangle structure
\*/
void CSWFFile::readRect(swf_rectangle *rc)
{
	unsigned char nSize;
	nSize=readBits(5);
	TRACEIT2("number of bits for storage: %d\n",nSize);
	rc->left=readBits(nSize);
	rc->right=readBits(nSize);
	rc->top=readBits(nSize);
	rc->bottom=readBits(nSize);
	TRACEIT2("rect: %d,%d - %d,%d\n",rc->left,rc->top,rc->right,rc->bottom);
}

/*\
 * <---------- writeInit ---------->
 * @m 
 * --> I N <-- @p
 * void *pDest - 
 * uint32_tnSize - 
\*/
void CSWFFile::writeInit(void *pDest,uint32_t nSize)
{
	m_pc=(unsigned char *)pDest;
	m_nBitpos=8;
	m_pcStart=m_pc;
	m_cBitBuffer=0;
	m_nBitCount=0;
	if (pDest)
		m_pcEnd=(unsigned char *)pDest+nSize;
	else
		m_pcEnd=NULL;
}

/*\
 * <---------- readInit ---------->
 * @m 
 * --> I N <-- @p
 * void *pSource - 
 * uint32_tnSize - 
\*/
void CSWFFile::readInit(void *pSource,uint32_t nSize)
{
	m_pc=(unsigned char *)pSource;
	m_pcStart=m_pc;
	m_nBitpos=8;
	m_nBitCount=0;
	if (pSource)
		m_pcEnd=(unsigned char *)pSource+nSize;
	else
		m_pcEnd=NULL;
}

/*\
 * <---------- writeBit ---------->
 * @m 
 * --> I N <-- @p
 * bool cBit - 
\*/
void CSWFFile::writeBit(bool cBit)
{
	m_nBitCount++;
	m_cBitBuffer<<=1;
	m_cBitBuffer|=(unsigned char)cBit;
 	if(--m_nBitpos == 0) 
	{
		if (m_pc)
			*(m_pc++)=m_cBitBuffer;
		m_nBitpos=8;
		m_cBitBuffer=0;
	}
}

/*\
 * <---------- writeBits ---------->
 * @m 
 * --> I N <-- @p
 * int cBits - 
 * unsigned int nSize - 
\*/
void CSWFFile::writeBits(int cBits,unsigned int nSize)
{
	unsigned int i;
	static const unsigned int nMask[32]=
	{
		0x00000001,	0x00000002,	0x00000004,	0x00000008,
		0x00000010,	0x00000020,	0x00000040,	0x00000080,
		0x00000100,	0x00000200, 0x00000400,	0x00000800,
		0x00001000,	0x00002000, 0x00004000,	0x00008000,
		0x00010000,	0x00020000, 0x00040000,	0x00080000,
		0x00100000,	0x00200000, 0x00400000,	0x00800000,
		0x01000000,	0x02000000, 0x04000000,	0x08000000,
		0x10000000,	0x20000000, 0x40000000,	0x80000000
	};
	for (i=0;i < nSize;i++)
	{
		unsigned int iMask=nSize-(i+1);
		writeBit((cBits&nMask[iMask]) == nMask[iMask]);
	}
}

/*\
 * <---------- writeFlush ---------->
 * @m 
\*/
void CSWFFile::writeFlush(void)
{
	while(m_nBitpos != 8) 
		writeBit(false);
}

/*\
 * <---------- writeRect ---------->
 * @m 
 * --> I N <-- @p
 * swf_rectangle *pRect - 
 * <-- OUT --> @r
 * unsigned int - 
\*/
unsigned int CSWFFile::writeRect(swf_rectangle *pRect)
{
	unsigned int nMax=0;
	unsigned int nOffset=0;
	unsigned char nBit;
	if (pRect)
	{
		nMax=max(unsignedvalue(pRect->left),nMax);
		nMax=max(unsignedvalue(pRect->right),nMax);
		nMax=max(unsignedvalue(pRect->top),nMax);
		nMax=max(unsignedvalue(pRect->bottom),nMax);
		nBit=1;
		TRACEIT("highest rect value %d\n",nMax);
		if (nMax > 0)
		{
			while (nBit < 32)
			{
				nMax = nMax >> 1;
				if (nMax > 0)
					nBit++;
				else
					break;
			};
		}
		TRACEIT("needs %d bit for storage\n",++nBit);
		//write bitcount to stream
		writeBits(nBit,5);
		//write frame values
		writeBits(pRect->left,nBit);
		writeBits(pRect->right,nBit);
		writeBits(pRect->top,nBit);
		writeBits(pRect->bottom,nBit);
		//flush
		writeFlush();
		nOffset=(m_nBitCount/8)+1;
	}
	return nOffset;
}

/*\
 * <---------- readShort ---------->
 * @m read a short int from input memory
 * --> I N <-- @p
 * unsigned short int *pn - pointer to shrot value
 * <-- OUT --> @r
 * bool - true=ok
\*/
bool CSWFFile::readShort(unsigned short int *pn)
{
	bool bRet=false;
	if (m_pc+2 < m_pcEnd && pn)
	{
		*pn=endian.wFromLittle(*(unsigned short *)m_pc);
		m_pc+=2;
		m_nBitpos=8;
		bRet=true;
	}
	else
	{
		TRACEIT2("end of file reached\n");
	}
	return bRet;
}

/*\
 * <---------- writeShort ---------->
 * @m 
 * --> I N <-- @p
 * short int nValue - 
 * <-- OUT --> @r
 * unsigned int - 
\*/
unsigned int CSWFFile::writeShort(short int nValue)
{
	unsigned int nRet=2;
	if (m_pc)
	{
		if (m_pc+nRet < m_pcEnd)
		{
			*(unsigned short *)m_pc=endian.wToLittle(nValue);
			m_pc+=nRet;
			m_nBitpos=8;
		}
		else
		{
			nRet=0;
			TRACEIT2("end of file reached\n");
		}
	}
	return nRet;
}

/*\
 * <---------- writeShort ---------->
 * @m 
 * --> I N <-- @p
 * short int nValue - 
 * <-- OUT --> @r
 * unsigned int - 
\*/
unsigned int CSWFFile::writeLong(unsigned int nValue)
{
	unsigned int nRet=4;
	if (m_pc)
	{
		if (m_pc+nRet < m_pcEnd)
		{
			*(unsigned int *)m_pc=endian.lToLittle(nValue);
			m_pc+=nRet;
			m_nBitpos=8;
		}
		else
		{
			nRet=0;
			TRACEIT2("end of file reached\n");
		}
	}
	return nRet;
}

/*\
 * <---------- readLong ---------->
 * @m read a long int from the input stream
 * --> I N <-- @p
 * uint32_tint *pn - pointer to long value
 * <-- OUT --> @r
 * bool - true=ok
\*/
bool CSWFFile::readLong(uint32_t *pn)
{
	bool bRet=false;
	if (m_pc+4 < m_pcEnd && pn)
	{
		*pn=endian.lFromLittle(*(uint32_t*)m_pc);
		m_pc+=4;
		m_nBitpos=8;
		bRet=true;
	}
	else
	{
		TRACEIT2("end of file reached\n");	
	}
	return bRet;
}

/*\
 * <---------- readBytes ---------->
 * @m 
 * --> I N <-- @p
 * void *pDest - 
 * unsigned int nSize - 
 * <-- OUT --> @r
 * bool - 
\*/
bool CSWFFile::readBytes(void *pDest,unsigned int nSize)
{
	bool bRet=false;
	if (m_pc+nSize < m_pcEnd)
	{
		memcpy(pDest,m_pc,nSize);
		m_pc+=nSize;
		m_nBitpos=8;
		bRet=true;
	}
	else
	{
		TRACEIT2("end of file reached\n");	
	}
	return bRet;
}

/*\
 * <---------- writeBytes ---------->
 * @m 
 * --> I N <-- @p
 * void *pSource - 
 * unsigned int nSize - 
 * <-- OUT --> @r
 * unsigned int - 
\*/
unsigned int CSWFFile::writeBytes(const void *pSource,unsigned int nSize)
{
	unsigned int nRet=nSize;
	if (m_pc)
	{
		if (m_pc+nRet < m_pcEnd)
		{
			memcpy(m_pc,pSource,nRet);
			m_pc+=nRet;
			m_nBitpos=8;
		}
		else
		{
			nRet=0;
			TRACEIT2("end of file reached\n");
		}
	}
	return nRet;
}

/*\
 * <---------- sGetFormatName ---------->
 * @m 
 * --> I N <-- @p
 * int nFormat - 
 * <-- OUT --> @r
 * tstring - 
\*/
tstring CSWFFile::sGetFormatName(int nFormat)
{
	//SCCCZVVVV
	bool bStreaming=(nFormat&0x100) == 0x100;
	int nSoundComp=(nFormat>>5)&0x03;
	bool bZlib=(nFormat&0x10) == 0x10;
	int nVers=nFormat&0x0F;

	tstring sOut="Unknown Flash Version";
	char szBuffer[255]={0};
	const char *szCompressionNames[4]=
	{
		"Uncompressed",
		"ADPCM",
		"MP3",
		"ASAO"
	};
	if (nVers)
	{
		sprintf(szBuffer,"Version %d",nVers);
		sOut=szBuffer;
	}
	if (nSoundComp)
	{
		sOut+=" - ";
		if (bZlib)
			sOut+="Compressed Wrapper";
		if (bStreaming)
		{
			if (bZlib)
				sOut+=", ";
			sOut+="Streaming";
		}
		if (nSoundComp)
		{
			if (bZlib || bStreaming)
				sOut+=", ";
			sOut+=szCompressionNames[nSoundComp-1];
			sOut+=" Sound";
		}
	}
	return sOut;
}

/*\
 * <---------- nGetFormat ---------->
 * @m 
 * <-- OUT --> @r
 * int - 
\*/
int CSWFFile::nGetFormat(void)
{
	int nRet=0;
	if (m_nFormatVersion)
		nRet=m_nFormatVersion;
	if (m_bContainsSound || m_bContainsPicture)
	{
		if (m_bCompressed)
			nRet|=0x10;
		if (m_bUsesStreaming)
			nRet|=0x100;
		if (m_bContainsSound)
			nRet|=m_nSoundCompression<<5; 
	}
	return nRet;
}

/*\
 * <---------- nRenderTag ---------->
 * @m 
 * --> I N <-- @p
 * unsigned int nId - 
 * unsigned int nSize - 
 * void *pData - 
 * <-- OUT --> @r
 * unsigned int - 
\*/
unsigned int CSWFFile::nRenderTag(unsigned int nId, unsigned int nSize, void *pData)
{
	unsigned int nOffset=0;
	unsigned int nLen;
	unsigned short int raw;

	nLen=min(0x3F,nSize);
	raw=(nId<<6) | nLen;
	//
	nOffset+=writeShort(raw);
	//
	if (nLen == 0x3F)
		nOffset+=writeLong(nSize);
	//
	if (nSize)
		nOffset+=writeBytes(pData,nSize);
	return nOffset;
}

/*\
 * <---------- nRenderBackgroundColor ---------->
 * @m 
 * --> I N <-- @p
 * unsigned char cRed - 
 * unsigned char cGreen - 
 * unsigned char cBlue - 
 * <-- OUT --> @r
 * unsigned int - 
\*/
unsigned int CSWFFile::nRenderBackgroundColor(unsigned char cRed,unsigned char cGreen,unsigned char cBlue)
{
	unsigned char color[3]={cRed,cGreen,cBlue};	
	return nRenderTag(ST_SETBACKGROUNDCOLOR,sizeof(color),(void *)color);
}

/*\
 * <---------- nRenderDefineSound ---------->
 * @m 
 * --> I N <-- @p
 * CMobileSampleContent *pSource - 
 * <-- OUT --> @r
 * unsigned int - 
\*/
unsigned int CSWFFile::nRenderDefineSound(unsigned int nSoundId,uint32_t nSampleCount,CMobileSampleContent *pSource,unsigned char *pMpeg,uint32_t nMpegSize)
{
//	static const unsigned int nRates[4]={5500,11025,22050,44100};
	map<unsigned int,unsigned int> :: const_iterator iterRate;
	map<unsigned int,unsigned int> mapRate;
	unsigned char cFlags=0;
	unsigned char *pData=NULL;
	unsigned char *pWrite;
	uint32_t nWritten=0;
	uint32_t nDataSize=0;

	mapRate[5512]=0;
	mapRate[11025]=1;
	mapRate[22050]=2;
	mapRate[44100]=3;

	if ((iterRate=mapRate.find(pSource->m_nCSSamplesPerSecond)) == mapRate.end())
	{
		Log2(verbLevErrors,IDS_ERR_INVALIDSAMPLE);
		throw new CFormatException(CFormatException::formaterrIncompatibleSampleFormat,"invalid sample rate");
	}
	//add sample rate index to flags
	cFlags|=(unsigned char)(iterRate->second<<2);
	//add sample width (16bit) to flags
	cFlags|=0x02;
	//add channel count to flags
	cFlags|=pSource->m_nCSChannels == 2 ? 0x01 : 0x00;
	//add compression identifier to flags
	cFlags|=32;
	//CompressMP3(pSource,&pMpeg,&nMpegSize);
	nDataSize=nMpegSize+2+7;
	pData=new unsigned char[nDataSize];
	pWrite=pData;
	//sound id
	*(unsigned short *)pWrite=endian.wFromLittle(nSoundId);
	pWrite+=2;
	//flags
	*pWrite=cFlags;
	pWrite++;
	//number of samples
	*(uint32_t*)pWrite=endian.lFromLittle(nSampleCount);
	pWrite+=4;
	//2 bytes for MP3 prefix
	*(unsigned short *)pWrite=0;
	pWrite+=2;	
	memcpy(pWrite,pMpeg,nMpegSize);
	nWritten=nRenderTag(ST_DEFINESOUND,nDataSize,pData);
	delete [] pData;
	return nWritten;
}

/*\
 * <---------- nRenderStartSound ---------->
 * @m 
 * --> I N <-- @p
 * unsigned short int nLoopCount - 
 * <-- OUT --> @r
 * unsigned int - 
\*/
unsigned int CSWFFile::nRenderStartSound(unsigned int nSoundId,unsigned short int nLoopCount)
{
	unsigned char cFlags=0;
	unsigned int nWritten=0;
	unsigned int nSize=3;
	if (nLoopCount)
	{
		nSize+=2;
		cFlags|=SI_HASLOOPS;
	}
	unsigned char *pData,*pWrite;
	pData=new unsigned char[nSize];
	pWrite=pData;
	*(unsigned short int*)pWrite=endian.wToLittle(nSoundId);
	pWrite+=2;
	*(unsigned char *)pWrite=cFlags;
	pWrite+=1;
	if (nLoopCount)
	{
		*(unsigned short int*)pWrite=endian.wToLittle(nLoopCount);
		pWrite+=2;
	}

	nWritten=nRenderTag(ST_STARTSOUND,nSize,pData);
	delete [] pData;
	return nWritten;
}

unsigned int CSWFFile::nRenderShowFrame(void)
{
	return nRenderTag(ST_SHOWFRAME);
}

unsigned int CSWFFile::nRenderEnd(void)
{
	return nRenderTag(ST_END);
}

/*\
 * <---------- nRenderBody ---------->
 * @m 
 * --> I N <-- @p
 * CMobileSampleContent *pSource - 
 * unsigned char *pDest - 
 * <-- OUT --> @r
 * unsigned int - 
\*/
unsigned int CSWFFile::nRenderBody(CMobileSampleContent *pSource,uint32_t nSampleCount,uint32_t nBackgroundRGB,unsigned char *pMpeg,uint32_t nMpegSize)
{
	unsigned int nSoundId=24;
	unsigned int nOffset=0;
	//background color
	nOffset+=nRenderBackgroundColor((unsigned char)((nBackgroundRGB>>16)&0xFF),(unsigned char)((nBackgroundRGB>>8)&0xFF),(unsigned char)(nBackgroundRGB&0xFF));
	//define sound
	nOffset+=nRenderDefineSound(nSoundId,nSampleCount,pSource,pMpeg,nMpegSize);
	//start sound
	nOffset+=nRenderStartSound(nSoundId,m_pParameters->m_nParameter[paraNumLoopcount]);
	//show frame
	nOffset+=nRenderShowFrame();
	//end
	nOffset+=nRenderEnd();
	return nOffset;
}

/*\
 * <---------- nRenderHeader ---------->
 * @m 
 * --> I N <-- @p
 * CMobileSampleContent *pSource - source data
 * swf_rectangle *pRect - display geometry
 * unsigned int nFileSize - file size
 * unsigned char *pDest - pointer to destination buffer
 * <-- OUT --> @r
 * unsigned int - 
\*/
unsigned int CSWFFile::nRenderHeader(CMobileSampleContent *pSource,swf_rectangle *pRect,unsigned int nFileSize)
{
	unsigned int nOffset=0;
	unsigned char nVersion=5;
	//file magic
	nOffset+=writeBytes("FWS",3);
	//file format version
	nOffset+=writeBytes(&nVersion,1);
	//total file size
	nOffset+=writeLong(nFileSize);
	//display geometry
	nOffset+=writeRect(pRect);
	//frame rate
	nOffset+=writeShort(m_nFrameRate);
	//frame count
	nOffset+=writeShort(m_nFrameCount);
	return nOffset;
}

void CSWFFile::Read(istream &ar)
{
	Serialize(ar);
}

void CSWFFile::Write(ostream &out)
{
	Serialize(out);
}

/*\
 * <---------- RenderDestination ---------->
 * @m 
 * --> I N <-- @p
 * ostream &out - 
\*/
void CSWFFile::Serialize(ostream &out)
{
	unsigned char *pMpeg=NULL;
	uint32_t nMpegSize=0;
	unsigned int nSize=0;
	uint32_t nSampleCount;
	float fFramesPerBlock;
	float fSamplesPerFrame;
	float fSamplesPerBlock;
	unsigned int nBlockSize=m_pCSSource->m_nCSSamplesPerSecond > 22050 ? 1152 : 576;
	float fBlocksPerSecond=(float)m_pCSSource->m_nCSSamplesPerSecond / nBlockSize;
	float fFramesPerSecond=fBlocksPerSecond;

	//default display frame size is 20units = 1pixel
	swf_rectangle rect={0,0,m_pParameters->m_nParameter[paraNumFrameWidth]*20,m_pParameters->m_nParameter[paraNumFrameHeight]*20};
	//verify parameter validaty
	if (m_pParameters->m_nParameter[paraNumBitrate] == 0)
	{
		m_pParameters->m_nParameter[paraNumBitrate]=32000*m_pCSSource->m_nCSChannels;
		Log2(verbLevWarnings,"bitrate not set, using default : %d\n",m_pParameters->m_nParameter[paraNumBitrate]);
	}
	if (m_pCSSource->m_nCSBitsPerSample != 16)
	{
		Log2(verbLevErrors,"sample width incompatible\n");
		throw new CFormatException(CFormatException::formaterrSource,"sample width incompatible");
	}
	if (m_pCSSource->m_nCSSamplesPerSecond != 5500 &&
		m_pCSSource->m_nCSSamplesPerSecond != 11025 &&
		m_pCSSource->m_nCSSamplesPerSecond != 22050 &&
		m_pCSSource->m_nCSSamplesPerSecond != 44100)
	{
		Log2(verbLevErrors,"sample rate incompatible\n");
		throw new CFormatException(CFormatException::formaterrSource,"sample rate incompatible");
	}
	//use default framerate fitting the sample rate
	m_nFrameRate=0;
	m_nFrameCount=0;

	if (m_nFrameRate)
		fFramesPerSecond = (float)(m_nFrameRate / 256.0);
	fFramesPerBlock = fFramesPerSecond / fBlocksPerSecond;
	fSamplesPerFrame = (nBlockSize * fBlocksPerSecond) / fFramesPerSecond;
	fSamplesPerBlock = fSamplesPerFrame * fFramesPerBlock;
	nSampleCount=m_pCSSource->m_nCSSize / (m_pCSSource->m_nCSChannels * (m_pCSSource->m_nCSBitsPerSample/8));

	m_nFrameRate=(unsigned short)(fFramesPerSecond*256);
	m_nFrameCount=1;

	CompressMP3(m_pCSSource,&pMpeg,&nMpegSize);

	writeInit(NULL,0);
	nSize+=nRenderHeader(m_pCSSource,&rect);
	nSize+=nRenderBody(m_pCSSource,nSampleCount,m_pParameters->m_nParameter[paraNumBackgroundRGB],pMpeg,nMpegSize);
	if (nSize)
	{
		unsigned char *pDest=new unsigned char [nSize];
		writeInit(pDest,nSize);
		nRenderHeader(m_pCSSource,&rect,nSize);
		nRenderBody(m_pCSSource,nSampleCount,m_pParameters->m_nParameter[paraNumBackgroundRGB],pMpeg,nMpegSize);
		out.write((char *)pDest,nSize);
		delete [] pDest;
		m_nCSChannels=m_pCSSource->m_nCSChannels;
		m_nCSSamplesPerSecond=m_pCSSource->m_nCSSamplesPerSecond;
		m_nCSBitsPerSample=0;
		m_nFileSize=out.tellp();
		ASSERT(m_nFileSize == nSize);
	}
}
