/*\
 * CMXSample.cpp
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
#include <string>
#ifndef WIN32
#include <netinet/in.h>
#else
#include <winsock2.h>
#endif
#include "../retroBase/Basics.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MyString.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/Adpcm.h"
#include "CMXSample.h"
#include "CMXSample.h"
#include "QCELPFile.h"
#ifndef USE_QUALCOMM_LIBRARY
#include "ANSI733Codec.h"
#endif

/*\ 
 * <---------- CCMXPacket::CCMXPacket ----------> 
 * @m 
 * --> I N <-- @p
 * unsigned char *pData
 * uint32_tnSize
 * <-- OUT --> @r
 * 
 \*/ 
CCMXPacket::CCMXPacket(unsigned char *pData,uint32_t nSize) 
{
	m_pData=new unsigned char [nSize];
	if (m_pData)
	{
		CopyMemory(m_pData,pData,nSize);
		m_nSize=nSize;
	}
}

CCMXPacket::~CCMXPacket(void)
{
	if (m_pData)
		delete [] m_pData;
}

/*\
 * <---------- nGetDelta ---------->
 * @m 
 * --> I N <-- @p
 * int nFormat - 
 * int nSampleRate - 
 * int nSampleSize - 
 * int nDoneDelta - 
 * int nDoneSize - 
 * <-- OUT --> @r
 * uint32_t- 
\*/
uint32_t CCMXPacket::nGetDelta(uint32_t nFormat,uint32_t nSampleRate,uint32_t nSampleSize,uint32_t nDoneDelta,uint32_t nDoneSize)
{
	uint32_t nDelta;
	if (nFormat == fmtQCELP)
	{
		nDelta=(nGetSize()/35)*2;	//216320
		ASSERT(!(nGetSize()%35));
	}
	if (nFormat == fmtIMA)
	{
		//nDelta=(nGetSize()/(nSampleRate/200));	//216320
		//(nSampleSize*1000)/nSampleRate
		//nSamples=(nBlockSize*2)-7;
		nDelta=(nGetSize()*200)/nSampleRate;
		//ASSERT(!(nGetSize()%(nSampleRate/200)));
	}
	return nDelta;
}

CCMXSample::CCMXSample(uint32_t nFormat,uint32_t nSampleRate)
{
	CAdpcm::initImaTable();

	m_nSizeSum=0;
	m_nFormat=nFormat;
	m_nSampleRate=nSampleRate;
}

CCMXSample::~CCMXSample(void)
{
	unsigned int i;
	for (i=0;i < (unsigned int)m_Packets.size();i++)
	{
		ASSERT (m_Packets[i]);
		if (m_Packets[i] != NULL)
			delete m_Packets[i];
	}
	if (m_Packets.size())
		m_Packets.erase(m_Packets.begin(), m_Packets.end());
}


/*\ 
 * <---------- CCMXSample::AddPacket ----------> 
 * @m 
 * --> I N <-- @p
 * unsigned char *pData - 
 * uint32_tint nSize - 
\*/ 
void CCMXSample::AddPacket(unsigned char *pData,uint32_t nSize)
{
	CCMXPacket *pPacket=new CCMXPacket(pData,nSize);
	m_nSizeSum+=nSize;
	m_Packets.push_back(pPacket);
}

/*\
 * <---------- ExportRaw ---------->
 * @m 
\*/
void CCMXSample::ExportRaw (void)
{
	int i;
	FILE *fp;
	char cName[255]="sound.raw";

	TRACEIT2("exporting sound data as \"%s\"\n",cName);

	if ((fp=fopen(cName,"wb")) != NULL)
	{
		for(i=0;i < (int)m_Packets.size();i++)
			fwrite(m_Packets[i]->pGetData(),m_Packets[i]->nGetSize(),1,fp);
		fclose(fp);
	}
	else
	{
		Log2(verbLevErrors,"couldnt create output file \"%s\"\n",cName);
		throw new CFormatException(CFormatException::formaterrUnknown,"couldnt create output file");
	}
}

/*\
 * <---------- DecodeQCELP ---------->
 * @m 
 * --> I N <-- @p
 * signed short int **pcDest - 
 * uint32_tint *pSize - 
\*/
void CCMXSample::DecodeQCELP(signed short int **pcDest,uint32_t *pSize)
{
	unsigned int i;
	uint32_t nPacketSize;
	signed short int *pd;
	uint32_t nBlockCount,nUsedCount;
	uint32_t nSamples=((uint32_t)m_Packets.size()*25)*160;
	CQCELPFile qc;
#ifdef USE_QUALCOMM_LIBRARY
	IQsclCodec *codec;
#else
	CANSI733Codec *codec;
#endif	
	*pSize=nSamples*2;
	if (pcDest)
	{
		*pcDest=(signed short *)CMobileSampleContent::Alloc(nSamples*2);
		ZeroMemory(*pcDest,nSamples*2);
		pd=*pcDest;
#ifdef USE_QUALCOMM_LIBRARY
		if ((codec=(IQsclCodec *)qc.pInitLibrary()) != NULL)
#else
		if ((codec=(CANSI733Codec *)qc.pInitLibrary()) != NULL)
#endif
		{
#ifdef USE_QUALCOMM_LIBRARY
			qc.InitDecode(codec,false);
#else
			codec->Init(false,false);
#endif
			for (i=0;i < m_Packets.size();i++)
			{
				nPacketSize=m_Packets[i]->nGetSize();
				nBlockCount=nPacketSize/35;
				if (nPacketSize%35)
					++nBlockCount;
#ifdef USE_QUALCOMM_LIBRARY
				nUsedCount=qc.nDecode(codec,m_Packets[i]->pGetData(),pd,nBlockCount,*pSize);
#else
				nUsedCount=codec->nDecode(pd,m_Packets[i]->pGetData(),nBlockCount,nPacketSize);
#endif
				pd+=nUsedCount;
			}
			TRACEIT2("qcelp decoded...\n");
			m_nSampleRate=8000;
			Log2(verbLevDebug2,"size reserved: %d, size used: %d\n",*pSize,(pd - *pcDest)*2);
			*pSize=((uint32_t)(pd - *pcDest))*2;
			TRACEIT2("deinit qcelp...\n");
#ifdef USE_QUALCOMM_LIBRARY
			qc.DeinitLibrary();
#else
			qc.DeinitLibrary(codec);
#endif
			TRACEIT2("done\n");
		}
	}
}

/*\ 
 * <---------- CCMXSample::Decode ----------> 
 * @m 
\*/ 
void CCMXSample::DecodeIMA(signed short int **pcDest,uint32_t *pSize)
{
	unsigned int i;
	signed short int *pd=NULL;
	uint32_t nSamples=(uint32_t)nGetSize()*2;

	if (pcDest)
	{
		*pcDest=(signed short *)CMobileSampleContent::Alloc(nSamples*2);
		pd=*pcDest;
	}
	m_nSampleRate=0;
	*pSize=nSamples*2;
	if(pcDest)
	{
		for (i=0;i < m_Packets.size();i++)
		{
			uint32_t nPacketSize=m_Packets[i]->nGetSize();
			uint32_t nOutSize=0;
			
			int nSamplesPerBlock;
			uint32_t nBlockAlign;

			unsigned short int nRateValue=ntohs(*(unsigned short int*)m_Packets[i]->pGetData());
			if (!m_nSampleRate)
			{
				switch(nRateValue&0xC000)
				{
					case 0x8000:	m_nSampleRate=32000;	break;
					case 0x4000:	m_nSampleRate=16000;	break;
					default:		m_nSampleRate=8000;		break;
				}
			}
			nBlockAlign=nRateValue&0x0FFF;
			Log2(verbLevDebug2,"block align: %d\n",nBlockAlign);
			nSamplesPerBlock=(nBlockAlign*2)-7;
			CAdpcm::pwIMAAdpcmDecompress(pd,m_Packets[i]->pGetData()+2,nPacketSize-2,&nOutSize,nBlockAlign,nSamplesPerBlock,1);
			pd+=nOutSize/2;
		}
		Log2(verbLevDebug2,"size reserved: %d, size used: %d\n",*pSize,(pd - *pcDest)*2);
		*pSize=((uint32_t)(pd - *pcDest))*2;
	}
}

/*\
 * <---------- EncodeQCELP ---------->
 * @m 
 * --> I N <-- @p
 * signed short int *pcSource - 
 * uint32_tint nSize - 
\*/
void CCMXSample::EncodeQCELP(signed short int *pcSource,uint32_t nSize)
{
	uint32_t nLen,nUsedSize,nPackets;
#ifdef USE_QUALCOMM_LIBRARY
	IQsclCodec *codec;
#else
	CANSI733Codec *codec;
#endif
	CQCELPFile qc;
	char *ps,*pBuffer=new char [((nSize/320)+2)*35];

#ifdef USE_QUALCOMM_LIBRARY
	if ((codec=(IQsclCodec *)qc.pInitLibrary()) != NULL)
#else
	if ((codec=(CANSI733Codec *)qc.pInitLibrary()) != NULL)
#endif
	{
		ps=pBuffer;
#ifdef USE_QUALCOMM_LIBRARY
		qc.InitEncode(codec);
		nUsedSize=qc.nEncode(codec,1,16,8000,pcSource,pBuffer,nSize);
#else
		codec->Init(true,false);
		nPackets=codec->nEncode((unsigned char *)pBuffer,&nUsedSize,pcSource,nSize);
#endif
		//playtime in delta ticks for the whole sample
		m_nDelta=(nSize*50)/8000;
		while(nUsedSize)
		{
			nLen=min(nUsedSize,25*35);
			nUsedSize-=nLen;
			AddPacket((unsigned char *)ps,nLen);
			ps+=nLen;
		};
#ifdef USE_QUALCOMM_LIBRARY
		qc.DeinitLibrary();
#else
		qc.DeinitLibrary(codec);
#endif
	}
	delete [] pBuffer;
}

/*\
 * <---------- EncodeIMA ---------->
 * @m 
 * --> I N <-- @p
 * signed short int *pcSource - 
 * uint32_tint nSize - 
\*/
void CCMXSample::EncodeIMA(int16_t *pcSource,uint32_t nSize)
{
	uint32_t nLen,nUsedSize;

	char *pBuffer;
	unsigned char *pCompressed,*ps;
	int32_t nRateValue,nBlockSize,nSamples,nBlocksPerFragment;
	switch(m_nSampleRate)
	{
		case 32000:
			nBlockSize=0x200;
			nRateValue=0x8000|nBlockSize;
			nBlocksPerFragment=0x0F;
		break;
		case 16000:
			nBlockSize=0x100;
			nRateValue=0x4000|nBlockSize;
			nBlocksPerFragment=0x0F;
		break;
		default:
			nBlockSize=0x100;
			nRateValue=0x0000|nBlockSize;
			nBlocksPerFragment=0x07;
	}
	pBuffer=new char [(nBlockSize*nBlocksPerFragment)+2];
	nSamples=(nBlockSize*2)-7;
	//playtime in delta ticks for the whole sample
	m_nDelta=(nSize*50)/m_nSampleRate;
	pCompressed=CAdpcm::pcIMAAdpcmCompress(pcSource,nSize,&nUsedSize,nBlockSize,nSamples,1);
	ps=pCompressed;
	while(nUsedSize)
	{
		nLen=min(nUsedSize,(uint32_t)nBlocksPerFragment*nBlockSize);
		nUsedSize-=nLen;
		
		*(uint16_t *)pBuffer=htons(nRateValue);
		CopyMemory(pBuffer+2,ps,nLen);
		
		AddPacket((unsigned char *)pBuffer,nLen+2);
		ps+=nLen;
	};
	delete [] pBuffer;
}

/*\
 * <---------- Decode ---------->
 * @m 
 * --> I N <-- @p
 * signed short int **pcDest - 
 * uint32_tint *pSize - 
\*/
void CCMXSample::Decode(signed short int **pcDest,uint32_t *pSize)
{
	switch(m_nFormat)
	{
		case CCMXPacket::fmtQCELP:
			DecodeQCELP(pcDest,pSize);
		break;
		case CCMXPacket::fmtIMA:
			DecodeIMA(pcDest,pSize);
		break;
	}
}

/*\
 * <---------- Encode ---------->
 * @m 
 * --> I N <-- @p
 * signed short int *pcSource - 
 * uint32_tint nSize - 
\*/
void CCMXSample::Encode(signed short int *pcSource,uint32_t nSize)
{
	switch (m_nFormat)
	{
		case CCMXPacket::fmtQCELP:
			EncodeQCELP(pcSource,nSize);
		break;
		case CCMXPacket::fmtIMA:
			EncodeIMA(pcSource,nSize);
		break;
	}
}

/*\
 * <---------- nGetFragmentDelta ---------->
 * @m 
 * --> I N <-- @p
 * uint32_tint nFragmentPosition - 
 * uint32_tint nFragmentTime - 
 * <-- OUT --> @r
 * uint32_t- 
\*/
uint32_t CCMXSample::nGetFragmentDelta(uint32_t nFragmentPosition,uint32_t nFragmentTime)
{
	uint32_t nDelta=0;
	if(m_nSizeSum)
		nDelta=((m_nDelta*nFragmentPosition) / m_nSizeSum);
	nDelta-=nFragmentTime;
	return nDelta;
}
