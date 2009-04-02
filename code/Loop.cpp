/*\
 * Loop.cpp
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
#include "Loop.h"

CLoop::CLoop() 
{
}

CLoop::~CLoop()
{
}

void CLoop::Init(int nSampleRate,int nChannels,int nBitsPerSample,uint32_t nDestPlaytime,void *pSource,uint32_t nSourceSampleCount)
{
	CCrop::Init(nSampleRate,nChannels,nBitsPerSample,0,nDestPlaytime,false,pSource,nSourceSampleCount);
}

uint32_t CLoop::nGetDestSize(uint32_t nSourceSize)
{
	m_nSourceData=nSourceSize;
	ASSERT(m_nDestPlayTime);
	//get demanded data amount
	m_nDestData=(unsigned long)((((uint64_t)m_nSampleRate*(m_nChannels*(m_nBitsPerSample>>3)))*m_nDestPlayTime)/1000);
	return m_nDestData;
}

void CLoop::Process(signed short int *ibuf,signed short int *obuf, uint32_t nSampleCount)
{
	signed short *inBuffer=ibuf;
	uint32_t nSourceCount;
	uint32_t nDestCount=m_nDestData/(m_nChannels*(m_nBitsPerSample>>3));

	signed short int inLeft,inRight;

	nSourceCount=nSampleCount;
	while (nDestCount--)
	{
		if (!nSourceCount--)
		{
			ibuf=inBuffer;
			nSourceCount=nSampleCount;
		}
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
		ibuf+=m_nChannels;
	};
} 
