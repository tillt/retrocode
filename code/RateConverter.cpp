/*\
 * RateConverter.cpp
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
#include <math.h>
#include "../retroBase/Basics.h"
#include "../retroBase/Integer.h"
#include "Filter.h"
#include "RateConverter.h"

//do not change below 24 bit as that surely will trigger buffer problems when converting e.g. 48kHz to 44.1kHz
//no not change above 60 bit as that will render distorted audio when converting e.g. 48kHz to 4kHz
#define BIT_PER_FRACTION	48

CRateConverter::CRateConverter()
{
}

CRateConverter::~CRateConverter()
{
}

void CRateConverter::Init(int nChannels,int nInRate,int nOutRate)
{
	m_nSamplesPerSecond[rateIn]=nInRate;
	m_nSamplesPerSecond[rateOut]=nOutRate;
	m_nChannels=nChannels;

	long long incr=(uint64_t)((double)nInRate / (double)nOutRate * ((uint64_t) 1 << BIT_PER_FRACTION));
	m_llOposIncFrac = incr & (((uint64_t) 1 << BIT_PER_FRACTION)-1);										//static part of the per original sample size fraction
	m_llOposInc = incr >> BIT_PER_FRACTION;																	//static part of the per original sample size increment
}

/*\
 * <---------- CRateConverter :: nGetDestSize ----------> 
 * @m calculate size of bytes needed for the destination sample
 * NOTE: currently this is just an estimation and it is not proven to return a size sufficient for the conversion - this should be changed
 * --> I N <-- @p
 * uint32_tnSourceSize - size of source sample in bytes
 * <-- OUT --> @r
 * size of destination sample in bytes
\*/
uint32_t CRateConverter::nGetDestSize(uint32_t nSourceSize)
{
	uint32_t nSourceSamples=nSourceSize/2;
	uint32_t nDestSamples;
	if (nSourceSize%2)
		++nSourceSamples;
	nDestSamples=1+(unsigned long)(((uint64_t)nSourceSamples*m_nSamplesPerSecond[rateOut])/m_nSamplesPerSecond[rateIn]);
	if (((uint64_t)nSourceSamples*m_nSamplesPerSecond[rateOut])%m_nSamplesPerSecond[rateIn])
		nDestSamples+=2;
	return nDestSamples*2;	
}

/*\
 * <---------- CRateConverter :: Process ----------> 
 * @m convert sample rate
 * --> I N <-- @p
 * signed short int *ibuf     - input sample buffer
 * signed short int *obuf     - output sample buffer
 * uint32_tnSampleCount - number of samples to process
\*/
void CRateConverter::Process(signed short int *ibuf,signed short int *obuf, uint32_t nSampleCount)
{
	ASSERT(obuf);
	ASSERT(ibuf);

	TRACEIT2("interpolating sample rate from %d to %d Hz\n",m_nSamplesPerSecond[rateIn],m_nSamplesPerSecond[rateOut]);

	signed short int icur;
	signed short int ilast[2]={0,0};
    long long tmp;
    double dFractionWeight;
	signed short int *pcIn[2]={ibuf,ibuf+1};
	signed short int *start=obuf;
	long long ip=0;
	long long ipos=0;
	long long opos=0; 
	long long oposfrac=0;

    while (nSampleCount) 
	{
		dFractionWeight=(double) oposfrac / ((uint64_t) 1 << BIT_PER_FRACTION);
		tmp = oposfrac + m_llOposIncFrac;
		for (int i=0;i < m_nChannels;i++)
		{
			ipos=ip;
			//skip unneeded samples (if any)
			while (nSampleCount && ipos <= opos) 
			{
				ilast[i] = *pcIn[i];
				pcIn[i]+=m_nChannels;
				ipos++;
				--nSampleCount;
			};
			//get current sample (while having the last one stored in ilast)
			if (nSampleCount)
				icur = *pcIn[i];
			else
				icur = ilast[i];
			//output interpolated sample (mixing this and the last value)
			*obuf++=(signed short int)((double)ilast[i] * (1.0 - dFractionWeight) + (double) icur * dFractionWeight);	    
		}
		ip=ipos;
		//calc the new fraction weight
		opos += m_llOposInc + (tmp >> BIT_PER_FRACTION);					//raise input position pre comma part
		oposfrac = tmp & (((uint64_t) 1 << BIT_PER_FRACTION)-1);			//the "error" is save within the lower 16 bit - it is accumulated within oposfrac
    };
	TRACEIT2("output size = %d\n",(obuf-start)*2);
}
