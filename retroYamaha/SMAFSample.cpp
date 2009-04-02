/*\
 * SMAFSample.cpp
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
#include <iostream>
#include "../retroBase/Basics.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MyString.h"
#include "../retroBase/MobileContent.h"
#include "SMAFSample.h"

const signed int CSMAFSample::m_nStepTable[8]=
{
	0x00003980,
	0x00003980,
	0x00003980,
	0x00003980,
	0x00004CC0,
	0x00006640,
	0x00008000,
	0x00009980
};

CSMAFSample::CSMAFSample(uint32_t nSampleRate,uint32_t nChannels) : m_nSampleRate(nSampleRate), m_nChannels(nChannels)
{
	m_pnRawData=NULL;
	m_pnAdpcmData=NULL;
}

CSMAFSample::~CSMAFSample()
{
	if (m_pnRawData != NULL)
		CMobileSampleContent::Free(m_pnRawData);
	if (m_pnAdpcmData != NULL)
		CMobileSampleContent::Free(m_pnAdpcmData);
}

uint32_t CSMAFSample::nGetPlaytime()
{
	return (m_nRawSize*500)/m_nSampleRate;
}

/*\
 * <---------- CSMAFSample :: nEncode ----------> 
 * @m encode 16 bit PCM data into Yamaha's ADPCM format
 * --> I N <-- @p
 * signed short int *pWave - pointer to raw source pcm data
 * unsigned int nWaveSize  - original size (in bytes)
 * char *pDest             - pointer to destination buffer
 * <-- OUT --> @r
 * number of bytes used for encoding
\*/
int CSMAFSample::nEncode(signed short int *pWave,uint32_t nWaveSize,char *pDest)
{
	char *pAdpcm=pDest;
	int16_t *pnLimit=pWave+(nWaveSize/2);
	uint32_t nSampleIndex=0;
	unsigned char cNibble;
	uint32_t iChannel=0;
	uint32_t nBitIndex=0;
	int32_t nDifference;
	char cBitflag[4]={0,0,0,0};
	int32_t nLastSample[2]={0x00000000,0x00000000};
	int32_t nLastValue[2]={0x007F,0x007F};

	if (!pWave)
	{
		Log2(verbLevErrors,"illegal pointer given as parameter\n");
		return 0;
	}
	if (!nWaveSize)
	{
		Log2(verbLevErrors,"illegal input data size\n");
		return 0;
	}
	int16_t *pPCM=(int16_t *)pWave;
	//TRACEIT2("first sample value %04X\n",*pPCM);
	hexdump("first sample values",(unsigned char *)pPCM,16);
	while (pPCM < pnLimit)
	{
		//calc the difference between the last encoded sample and the current sample
		nDifference=((int16_t)*pPCM)-nLastSample[iChannel];
		
		//encode the sign
		if (nDifference < 0)
		{
			nDifference=0-nDifference;		//get the absolute value of the difference
			cBitflag[3]=1;					//remember that it is negative
		}
		else
			cBitflag[3]=0;					//remember that it is positive

		int nDoubleCount=0;
		int nIterator=nLastValue[iChannel]>>2;
		int nAdapter=nIterator;
		while (nAdapter <= nDifference)
		{
			nAdapter+=nIterator;
			if (++nDoubleCount == 7)
				break;
		};
		cBitflag[0]=(unsigned char)nDoubleCount&1;
		cBitflag[1]=(unsigned char)(nDoubleCount>>1) & 1;
		cBitflag[2]=(unsigned char)(nDoubleCount>>2) & 1;
		
		signed int nDelta = (int16_t)cBitflag[0]*(nLastValue[iChannel] >> 2);
		nDelta+=(int16_t)cBitflag[1]*(nLastValue[iChannel] >> 1);
		nDelta+=(int16_t)cBitflag[2]*nLastValue[iChannel];
		nDelta+=(int16_t)nLastValue[iChannel] >> 3;
		if (cBitflag[3])
			nDelta=nLastSample[iChannel]-nDelta;
		else
			nDelta+=nLastSample[iChannel];
		if (nDelta < -0x8000)
			nDelta=-0x8000;
		if (nDelta > 0x7FFF)
			nDelta=0x7FFF;
		nLastSample[iChannel]=nDelta;

		//encode bit 0, 1 and 2 into our output nibble
		cNibble=(cBitflag[2]<<2) | (cBitflag[1]<<1) | cBitflag[0];

		//recalc last value based on the encoded adpcm data
		nLastValue[iChannel]=(m_nStepTable[cNibble&0x07]*nLastValue[iChannel])>>14;

		//cap the value
		if (nLastValue[iChannel] < 0x007F)
			nLastValue[iChannel]=0x007F;
		if (nLastValue[iChannel] > 0x6000)
			nLastValue[iChannel]=0x6000;

		//add sign to adpcm nibble
		cNibble|=(cBitflag[3] << 3);

		//write adpcm-nibble into output stream
		*pDest = nBitIndex ? *pDest | (cNibble << 4) : cNibble;

		//iterate
		++pPCM;
		nBitIndex^=4;
		if (!nBitIndex)
			++pDest;
		if (++iChannel >= m_nChannels)
			iChannel=0;
	};
	if (nBitIndex)
		++pDest;
	return (unsigned long)((char *)pDest-pAdpcm);
}

/*\
 * <---------- CSMAFSample :: Decode ----------> 
 * @m decode Yamaha's ADPCM format into 16 bit PCM data
 * --> I N <-- @p
 * const unsigned char *pcBuffer - source adpcm data
 * int nSize                     - source size in bytes
\*/
void CSMAFSample::Decode(const unsigned char *pcBuffer,uint32_t nSize)
{
	const unsigned char *pcLimit=pcBuffer+nSize;
	unsigned char cIn;
	unsigned char cNibble;
	int16_t *pnOut;
	int32_t nOut[2]={0x0000,0x0000};
	int32_t nDelta[2]={0x000,0x0000};
	int32_t nMultiplier[2]={0x007f,0x007f};
	uint32_t nBitIndex=0;
	uint32_t iChannel=0;

	if (m_pnRawData != NULL)
		delete [] m_pnRawData;

	m_nRawSize=nSize*4;
	m_pnRawData=(int16_t *)CMobileSampleContent::Alloc(m_nRawSize*2);
	if (m_pnRawData == NULL)
		return;
	pnOut=m_pnRawData;

	TRACEIT("CSMAFSample::Decode - unfolding PCM data\n");
	while(pcBuffer < pcLimit)
	{
		//get next nibble
		if (!nBitIndex)
		{
			cIn=*pcBuffer;
			++pcBuffer;
		}
		cNibble=(cIn>>nBitIndex)&0x0F;
		nBitIndex^=4;

		//calc difference to last sample
		nDelta[iChannel]=nMultiplier[iChannel] >> 3;									//multiplier / 8
		nDelta[iChannel]+=(cNibble & 0x01) * (nMultiplier[iChannel] >> 2);				//nibble bit0 * (multiplier / 4)
		nDelta[iChannel]+=((cNibble >> 1) & 0x01) * (nMultiplier[iChannel] >> 1);		//nibble bit1 * (multiplier / 2)
		nDelta[iChannel]+=((cNibble >> 2) & 0x01) * nMultiplier[iChannel];				//nibble bit2 * multiplier
		
		//calc new sample using the sign bit
		if (cNibble & 0x08)
			nOut[iChannel]-=nDelta[iChannel];
		else
			nOut[iChannel]+=nDelta[iChannel];
		
		//limit sample
		if (nOut[iChannel] > 0x7FFF)
			nOut[iChannel]=0x7FFF;
		if (nOut[iChannel] < -0x8000)
			nOut[iChannel]=-0x8000;
		
		//write sample
		*(pnOut++)=(int16_t)nOut[iChannel];

		//calc next multiplier and shift it down for reusing the top 18 bit
		nMultiplier[iChannel]=(m_nStepTable[cNibble&0x07] * nMultiplier[iChannel]) >> 14;

		//limit multiplier
		if (nMultiplier[iChannel] < 0x007F)
			nMultiplier[iChannel] = 0x007F;
		if (nMultiplier[iChannel] > 0x6000)
			nMultiplier[iChannel] = 0x6000;

		if (++iChannel >= (uint32_t)m_nChannels)
			iChannel=0;
	};
	TRACEIT("CSMAFSample::Decode - sample decoded\n");
}
