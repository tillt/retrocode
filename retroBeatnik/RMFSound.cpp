/*\
 * RMFSound.cpp
 * Copyright (C) 2004-2007, MMSGURU - written by Till Toenshoff
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
#include "../retroBase/Adpcm.h"
#include "RMFBasics.h"
#include "RMFSound.h"

using namespace std;

CRMFSound::CRMFSound(const TCHAR *pszName) : m_sName(_T("unknown"))
{
	if (pszName != NULL)
		m_sName=pszName;
	m_nChannels=0;
	m_nBitRate=0;
	m_nSampleRate=0;
	m_nCompressionType=compNone;
	m_nBitsPerSample=0;
	m_pcSampleBuffer=NULL;
	m_nSampleSize=0;
	m_nStoredBitsPerSample=0;
}

CRMFSound::~CRMFSound()
{
	if (m_pcSampleBuffer != NULL)
		CMobileSampleContent::Free(m_pcSampleBuffer);
}

/*\ 
 * <---------- CRMFSound::nDecompressedSize ----------> 
 * @m calculate the number of bytes needed for decompressed sample data
 * --> I N <-- @p
 * int nType - identifier describing the sample encoding
 * compNone - no compression used
 * compAdpcm - Apple specific IMA ADPCM used
 * uint32_tnSampleSize - compressed sample data size
 * <-- OUT --> @r
 * uint32_t - size in bytes for decompressed data
\*/ 
uint32_t CRMFSound::nDecompressedSize(int nType,uint32_t nSampleSize)
{
	uint32_t nDeflatedSize=0;
	switch(nType)
	{
		case compNone:
			nDeflatedSize=nSampleSize;
		break;
		case compAdpcm:
		{
			int nPackets=0;
			//nDeflatedSize=(nSampleSize*4);
			nPackets=(nSampleSize/m_nChannels)/0x22;
			if ((nSampleSize/m_nChannels)%0x22)
				++nPackets;
			nDeflatedSize=((nPackets*m_nChannels)*0x40)*2;
		}
		break;
		default:
			Log2(verbLevErrors,IDS_ERR_NODEC);
			throw new CFormatException(CFormatException::formaterrInvalid,"could not decode sample");
	}
	return nDeflatedSize;
}

/*\ 
 * <---------- CRMFSound::Decompress ----------> 
 * @m decode current encoded sample data
\*/ 
void CRMFSound::Decompress(void)
{
	uint32_t nDeflatedSize=nDecompressedSize(m_nCompressionType,m_nSampleSize);
	unsigned char *pcRaw=(unsigned char *)CMobileSampleContent::Alloc(nDeflatedSize);

	switch(m_nCompressionType)
	{
		case compNone:
			if (m_nBitsPerSample == 16)
			{
				for (unsigned int i=0;i < nDeflatedSize/2;i++)
				{
					//*((short *)pcRaw+i)=ntohs(*(short *)m_pcSampleBuffer+i);
					signed short int wBuffer;
					memcpy(&wBuffer,(short *)m_pcSampleBuffer+i,2);
					wBuffer=ntohs(wBuffer);
					memcpy((short *)pcRaw+i,&wBuffer,2);
				}
			}
			else
			{
				Log2(verbLevErrors,IDS_ERR_INVALIDSAMPLE);
				throw new CFormatException(CFormatException::formaterrInvalid,"invalid sample width");
			}
		break;
		case compAdpcm:
			CAdpcm::RMFAdpcmDecompress(pcRaw,(unsigned char *)m_pcSampleBuffer,m_nSampleSize,m_nChannels);
		break;
	}
	CMobileSampleContent::Free(m_pcSampleBuffer);
	m_pcSampleBuffer=(char *)pcRaw;
	m_nSampleSize=nDeflatedSize;
	m_nBitsPerSample=16;
}

/*\ 
 * <---------- CRMFSound::nCompressedSize ----------> 
 * @m calculate the number of bytes needed for the compressed sample data
 * --> I N <-- @p
 * int nType - identifier describing the sample encoding
 * compNone - use no compression
 * compAdpcm - use Apple specific IMA ADPCM (should be default)
 * uint32_tnSampleSize - size of the source sample
 * <-- OUT --> @r
 * uint32_t - size in bytes for compressed data
\*/ 
uint32_t CRMFSound::nCompressedSize(int nType,uint32_t nSampleSize)
{
	uint32_t nInflatedSize=0;
	uint32_t nMultiplier;
	switch(nType)
	{
		case compNone:
			nInflatedSize=nSampleSize;
		break;
		case compAdpcm:
			nMultiplier=(nSampleSize/2)/64;
			if ((nSampleSize/2)%64)
				nMultiplier++;
			//two samples per byte - 64 sample -> 32 byte
			//two extra bytes per block - 32 byte -> 34 byte
			nInflatedSize=(nMultiplier*34)+1;
		break;
		default:
			Log2(verbLevErrors,IDS_ERR_NOCOD);
			throw new CFormatException(CFormatException::formaterrInvalid,"invalid compression chosen");
	}
	return nInflatedSize;
}

/*\
 * <---------- CRMFSound :: nCompress ---------->
 * @m encode the current raw sample data
 * --> I N <-- @p
 * int nType - identifier describing the sample encoding
 * compNone - use no compression
 * compAdpcm - use Apple specific IMA ADPCM (should be default)
 * int nChannels - number of sample channels used
 * uint32_tnSourceSize - source data size in bytes
 * unsigned char *pcSource - raw source sample data buffer
 * unsigned char *pcDest - pointer to buffer for the storage 
 * uint32_tnDestSize - size of the destination buffer in bytes
 * <-- OUT --> @r
 * uint32_t- number of bytes used for encoding
\*/
uint32_t CRMFSound::nCompress(int nType,int nChannels,uint32_t nSourceSize,unsigned char *pcSource, unsigned char *pcDest, uint32_t nDestSize)
{
	uint32_t nInflatedSize=nCompressedSize(nType,nSourceSize);
	if(pcDest && nDestSize && pcSource)
	{
		ASSERT(nInflatedSize == nDestSize);
		switch(nType)
		{
			case compNone:
				if (m_nBitsPerSample == 16)
				{
					for (unsigned int i=0;i < nInflatedSize;i++)
					{
						*((short *)pcDest+i) = htons(*((short *)pcSource+i));
					}
				}
				else
				{
					Log2(verbLevErrors,IDS_ERR_INVALIDSAMPLE);
					throw new CFormatException(CFormatException::formaterrParameters,"invalid sample width");
				}
			break;
			case compAdpcm:
				RMFAdpcmCompress(pcDest,(unsigned char *)pcSource,nSourceSize,nChannels,nDestSize);
			break;
		}
	}
	return nInflatedSize;
}

/*\
 * <---------- CRMFSound :: ExportRaw ----------> 
 * @m internal debugging function
\*/
void CRMFSound::ExportRaw(void)
{
	FILE *fp;
	char cName[255]="sound.raw";

	Log2(verbLevMessages,"exporting sound data as \"%s\"\n",cName);

	if ((fp=fopen(cName,"wb")) != NULL)
	{
		fwrite(m_pcSampleBuffer,m_nSampleSize,1,fp);
		fclose(fp);
	}
	else
	{
		Log2(verbLevErrors,IDS_ERR_COULDNTCREATE,cName);
		throw new CFormatException(CFormatException::formaterrUnknown,"could not create export file");
	}
}

/*\
 * <---------- CRMFSound :: Serialize ----------> 
 * @m currently pretty single sided serializing of input streams
 * --> I N <-- @p
 * istream &ar     - input stream object reference
 * bool bEncrypted - use "encryption"
\*/
void CRMFSound::Serialize(istream &ar,bool bEncrypted)
{
	uint32_t nTag;
	unsigned char nNameLen;
	TCHAR sBuffer[256];
	char pcBuffer[5];
	pcBuffer[4]=0;
	tstring s;
	unsigned int nPatch;

	map<unsigned long,int> :: const_iterator iterChunk;
	map<unsigned long,int> mapChunkId;

	mapChunkId[nMakeID4("mpgi")]=160;
	mapChunkId[nMakeID4("mpgh")]=128;
	mapChunkId[nMakeID4("mpgg")]=112;
	mapChunkId[nMakeID4("mpgf")]=96;
	mapChunkId[nMakeID4("mpge")]=80;
	mapChunkId[nMakeID4("mpgd")]=64;
	mapChunkId[nMakeID4("mpgc")]=56;
	mapChunkId[nMakeID4("mpgb")]=48;
	mapChunkId[nMakeID4("mpga")]=40;
	mapChunkId[nMakeID4("mpgn")]=32;

	ar.read((char *)&nPatch,4);
	nPatch=ntohl(nPatch);
	TRACEIT2("??? %Xh\n",nPatch);

	ar.read((char *)&nNameLen,1);
	if (nNameLen > 0)
	{
		ar.read(sBuffer,nNameLen);
		sBuffer[nNameLen]=0;
		m_sName.assign((const TCHAR *)sBuffer);
	}
	Log2(verbLevDebug2,"name: \"%s\"\n",m_sName.c_str());

	uint32_t nBlockSize;
	unsigned char *pcSound,*pcDecrypted;

	ar.read((char *)&nBlockSize,4);
	nBlockSize=ntohl(nBlockSize);
	Log2(verbLevDebug2,"esnd block size: %d\n",nBlockSize);

	pcSound=new unsigned char[nBlockSize];
	ar.read((char *)pcSound,nBlockSize);
	if (bEncrypted)
	{
		pcDecrypted=new unsigned char[nBlockSize];
		CRMFBasics::DecryptBinary(pcSound,pcDecrypted,nBlockSize);
		delete [] pcSound;
		hexdump("decrypted esnd headr: ",pcDecrypted,100);
	}
	else
	{
		pcDecrypted=pcSound;
	}

	uint32_t nDataOffset=0;
	int nSoundVersion=(int)*(pcDecrypted+1);
	Log2(verbLevDebug2,"sound block version %d\n",nSoundVersion);
	switch(nSoundVersion)
	{
		case 0x01:
			//nTag=*(uint32_t*)(pcDecrypted+60);
			memcpy(&nTag,pcDecrypted+60,4);
			m_nChannels=(unsigned char)*(pcDecrypted+27);
			//m_nSampleRate=htons(*(unsigned short *)(pcDecrypted+28));
			unsigned short int wRateCode;
			memcpy(&wRateCode,pcDecrypted+28,2);
			m_nSampleRate=htons(wRateCode);
			switch (TOFOURCC(nTag))
			{
				case 0:
					Log2(verbLevDebug1,IDS_PRG_SMPUNCOMP);
					m_nCompressionType=compNone;
					m_nBitsPerSample=*(pcDecrypted+69);
					nDataOffset=84;
				break;
				case MAKEFOURCC('i','m','a','4'):
					Log2(verbLevDebug1,IDS_PRG_SMPIMA);
					m_nCompressionType=compAdpcm;
					m_nBitsPerSample=4;
					nDataOffset=84;
				break;
				case MAKEFOURCC('u','l','a','w'):
					Log2(verbLevDebug1,IDS_PRG_SMPULAW);
					m_nCompressionType=compUlaw;
					m_nBitsPerSample=8;
					nDataOffset=84;
				break;
				case MAKEFOURCC('a','l','a','w'):
					Log2(verbLevDebug1,IDS_PRG_SMPALAW);
					m_nCompressionType=compAlaw;
					m_nBitsPerSample=8;
					nDataOffset=84;
				break;
				default:
					Log2(verbLevErrors,IDS_ERR_UNKNSMPENCN,pcSplitID4(nTag,pcBuffer));
					throw new CFormatException(CFormatException::formaterrInvalid,"unknown sample encoding");
			}
			Log2(verbLevDebug2,IDS_PRG_CHANNELS,m_nChannels);
			Log2(verbLevDebug2,IDS_PRG_RATE,m_nSampleRate);
			Log2(verbLevDebug2,"sample width: %dbit\n",m_nBitsPerSample);
		break;
		case 0x03:
			//nTag=*(uint32_t*)(pcDecrypted+2);
			memcpy(&nTag,pcDecrypted+2,4);
			if ((iterChunk=mapChunkId.find(nTag)) == mapChunkId.end())
			{
				Log2(verbLevErrors,IDS_ERR_UNKNSMPENCS,pcSplitID4(nTag,pcBuffer));
				throw new CFormatException(CFormatException::formaterrInvalid,"unknown sample encoding");
			}
			else
			{
				m_nCompressionType=compMpeg;
				m_nBitRate=iterChunk->second*1000;
				m_nBitsPerSample=16;
				//m_nSampleRate=htons(*(unsigned short *)(pcDecrypted+6));
				unsigned short int nRateCode;
				memcpy(&nRateCode,pcDecrypted+6,2);
				m_nSampleRate=htons(nRateCode);
				m_nChannels=*(pcDecrypted+87);
				Log2(verbLevMessages,IDS_PRG_MPEGBITRATE,iterChunk->second);
				Log2(verbLevMessages,IDS_PRG_CHANNELS,m_nChannels);
				Log2(verbLevMessages,IDS_PRG_RATE,m_nSampleRate);
				nDataOffset=126;
			}
		break;
		default:
			Log2(verbLevMessages,IDS_ERR_INVSOUNDVERS);
			throw new CFormatException(CFormatException::formaterrInvalid,"invalid sound block version");
	}
	hexdump("decrypted sound header: ",pcDecrypted,nDataOffset);
	hexdump("decrypted sound data: ",pcDecrypted+nDataOffset,64);
	m_nStoredBitsPerSample=m_nBitsPerSample;
	m_nSampleSize=nBlockSize-nDataOffset;
	m_pcSampleBuffer=(char *)CMobileSampleContent::Alloc(m_nSampleSize);
	memcpy(m_pcSampleBuffer,pcDecrypted+nDataOffset,m_nSampleSize);
	delete [] pcDecrypted;
}

/*\
 * <---------- CRMFSound :: nGetPlaytime ----------> 
 * @m get playback duration
 * <-- OUT --> @r
 * uint32_t- duration in miliseconds
\*/
uint32_t CRMFSound::nGetPlaytime(void)
{
	uint32_t nRet=0;
	uint32_t nNom,nDiv;
	nNom=(m_nSampleSize*1000)*8;
	nDiv=(m_nSampleRate*m_nChannels)*m_nBitsPerSample;
	Log2(verbLevDebug3,"nNom %d\n",nNom);
	Log2(verbLevDebug3,"nDiv %d\n",nDiv);
	ASSERT(nDiv);
	if (nDiv)
		nRet=round(nNom,nDiv);
	Log2(verbLevDebug3,"value=%d\n",nRet);
	return nRet;
}

/*\
 * <---------- CRMFSound :: nRender ----------> 
 * @m create an RMF sound segment
 * --> I N <-- @p
 * rmfCACH *pCache           - pointer to the destination cache entry filled by this function
 * int nType                 - identifier describing the sample encoding
 * compNone - use no compression
 * compAdpcm - use Apple specific IMA ADPCM (should be default)
 * int nChannels             - number of sample channels
 * int nSampleRate           - sample frequency
 * uint32_tnSourceSize - size of the source sample in bytes
 * bool bLoop                - loop the sound using the playback loop capability
 * unsigned char *pcSource   - pointer to raw source sample data
 * unsigned char *pDest      - pointer to destination buffer (or NULL for size calculation)
 * <-- OUT --> @r
 * number of bytes used for encoding
\*/
uint32_t CRMFSound::nRender(rmfCACH *pCache,int nType,int nChannels,int nSampleRate,uint32_t nSourceSize,bool bLoop,unsigned char *pcSource,unsigned char *pDest)
{
	uint32_t nUsed=0;
	uint32_t nPseudoSourceSize;
	uint32_t nADPCMSize=nCompress(nType,nChannels,nSourceSize);
	int nLen=(int)m_sName.length();

	unsigned char cESNDData[84]=
	{
		0x00,0x01,0x00,0x01,0x00,0x05,0x00,0x00,0x00,0xE0,0x00,0x01,0x80,0x51,0x00,0x00,
		0x00,0x00,0x00,0x14,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x3C,0x00,0xFF,0xFF,0xFF,0x40,0x0C,
		0xFA,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x69,0x6D,0x61,0x34,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,
		0x00,0x00,0x00,0x02
	};
	if (pDest)
	{
		cESNDData[27]=nChannels;
		cESNDData[28]=(nSampleRate>>8)&0xFF;
		cESNDData[29]=(nSampleRate)&0xFF;

		nPseudoSourceSize=(nSourceSize/2);

		uint32_t nMultiplier=(nSourceSize/2)/64;
		if ((nSourceSize/2)%64)
			nMultiplier++;

		if (bLoop)
		{
			Log2(verbLevMessages,IDS_PRG_ENCLOOP);
			uint32_t nSizeCode=htonl(nSourceSize/2);
			memcpy(cESNDData+36,&nSizeCode,4);
		}
		//*(uint32_t*)(cESNDData+42)=htonl(nMultiplier);
		uint32_t nMultiCode=htonl(nMultiplier);
		memcpy(cESNDData+42,&nMultiCode,4);
		hexdump("esnd headr: ",cESNDData,84);

		ASSERT(pCache);
		memcpy(pDest,"esnd",4);
		memcpy(pCache->sTag,"esnd",4);
		pDest+=4;
	}
	nUsed+=4;
	if (pDest)
	{
		pCache->nFirstValue=0;
		pDest=CRMFBasics::pRenderInteger(pDest,pCache->nFirstValue);
	}
	nUsed+=4;
	if (pDest)
	{
		*(pDest++)=(unsigned char)nLen;
		pCache->nNameOffset=nUsed;
		memcpy(pDest,(unsigned char *)m_sName.c_str(),nLen);
		pDest+=nLen;
	}
	nUsed+=nLen+1;
	if (pDest)
	{
		pCache->nDataSize=nADPCMSize+84;
		pDest=CRMFBasics::pRenderInteger(pDest,pCache->nDataSize);
	}
	nUsed+=4;
	if (pDest)
	{
		unsigned char *pEncrypt=pDest;
		pCache->nDataOffset=nUsed;
		memcpy(pDest,cESNDData,84);
		pDest+=84;
		hexdump("uncompressed: ",pcSource,64);
		//compress sound data
		Log2(verbLevMessages,IDS_PRG_COMPRESSING);
		nCompress(nType,nChannels,nSourceSize,pcSource,pDest,nADPCMSize);
		hexdump("  compressed: ",pDest,64);
		//encrypt sound data
		CRMFBasics::EncryptBinary(pEncrypt,pEncrypt,nADPCMSize+84);
		hexdump("   encrypted: ",pDest,64);
		pDest+=nADPCMSize;
	}
	nUsed+=nADPCMSize+84;
	return nUsed;
}
