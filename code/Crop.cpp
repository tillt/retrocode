/*\
 * Crop.cpp
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
#include "Crop.h"

CCrop::CCrop() : m_nChannels(0), m_nDestData(0), m_nPosStart(0), m_nPosEnd(0)
{
}

CCrop::~CCrop()
{
}

void CCrop::Init(int nSampleRate,int nChannels,int nBitsPerSample,uint32_t nOffset,uint32_t nDestPlaytime,bool bAutocrop,void *pSource,uint32_t nSampleCount)
{
	m_nSampleRate=nSampleRate;
	m_nBitsPerSample=nBitsPerSample;
	m_nChannels=nChannels;

	if (bAutocrop)
	{
		bool bFoundFirstNonSilent=false;
		//bool bFoundLastNonSilent=false;
		uint32_t nPosition=0;
		uint32_t nFirstNonSilent=0;
		uint32_t nLastNonSilent=0;

		signed short int *p=(signed short int *)pSource;

		while (nPosition < (nSampleCount*nChannels)*(nBitsPerSample>>3))
		{
			//trigger limit at ~0.5dB
			if (*p >= 178 || *p <= -177)
			{
				if (!bFoundFirstNonSilent)
				{
					nFirstNonSilent=nPosition;
					bFoundFirstNonSilent=true;
				}
				else
					nLastNonSilent=nPosition;
			}
			++p;
			nPosition+=(nBitsPerSample>>3);
		};
		if (bFoundFirstNonSilent)
		{
			m_nPosStart=nFirstNonSilent;
			m_nPosEnd=nLastNonSilent;
		}
	}
	else
	{
		m_nOffsetTime=nOffset;
		m_nDestPlayTime=nDestPlaytime;
	}
}

uint32_t CCrop::nGetDestSize(uint32_t nSourceSize)
{
	m_nSourceData=nSourceSize;
	m_nDestData=nSourceSize;

	if (m_nPosStart == 0 && m_nPosEnd == 0)
	{
		uint32_t nTimePerData=(uint32_t)(((uint64_t)nSourceSize*1000)/((m_nSampleRate*(m_nBitsPerSample>>3))*m_nChannels));
		if (m_nDestPlayTime == 0)
			m_nDestPlayTime=nTimePerData;
		if (m_nDestPlayTime + m_nOffsetTime > nTimePerData)
		{
			if (m_nOffsetTime > nTimePerData)
				m_nOffsetTime=0;
			if (m_nDestPlayTime + m_nOffsetTime > nTimePerData)
			{
				//prevent appending
				m_nDestPlayTime=nTimePerData-m_nOffsetTime;
			}
		}
		//get demanded data amount
		unsigned int nDataPerDemand=(unsigned long)((((uint64_t)m_nSampleRate*(m_nChannels*(m_nBitsPerSample>>3)))*m_nDestPlayTime)/1000);
		//sample bigger than that?
		if (nSourceSize > nDataPerDemand)
			m_nDestData=(unsigned long)((((uint64_t)m_nSampleRate*(m_nChannels*(m_nBitsPerSample>>3)))*min(m_nDestPlayTime,nTimePerData))/1000);
		else
			m_nDestData=nDataPerDemand;
	}
	else
		m_nDestData=m_nPosEnd-m_nPosStart;
	return m_nDestData;
}

void CCrop::Process(signed short int *ibuf,signed short int *obuf, uint32_t nSampleCount)
{
	uint32_t nPosition;
	signed short int inLeft,inRight;
	
	if (m_nPosStart == 0 && m_nPosEnd == 0)
	{
		m_nPosStart=(unsigned long)((((uint64_t)m_nSampleRate*(m_nBitsPerSample>>3))*m_nOffsetTime)/1000)*m_nChannels;
		m_nPosEnd=(unsigned long)((((uint64_t)m_nSampleRate*(m_nBitsPerSample>>3))*m_nDestPlayTime)/1000)*m_nChannels;
		m_nPosEnd+=m_nPosStart;
	}
	nPosition=0;
	while (nSampleCount--)
	{
		if (nPosition >= m_nPosStart && nPosition < m_nPosEnd)
		{
			inLeft = *ibuf;
			if (m_nChannels == 2)
				inRight = *(ibuf+1);
			if (m_nChannels == 2)
			{
				*obuf++ = inLeft;
				*obuf++ = inRight;
			}
			else
				*obuf++ = inLeft;
		}
		ibuf+=m_nChannels;
		nPosition+=(m_nBitsPerSample>>3)*m_nChannels;
	};
} 
