/*\
 * Fade.cpp
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
#include "Filter.h"
#include "Fade.h"

CFade::CFade() : m_nFadeTime(0)
{
}

CFade::~CFade()
{
}

/*\
 * <---------- Init ---------->
 * @m 
 * --> I N <-- @p
 * int nSampleRate - sample rate
 * int nBitsPerSample - bits per sample
 * int nChannels - channels
 * unsigned int nFadeTime - fade time [ms]
 * int nDirection - directionIn | directionOut
\*/
void CFade::Init(int nSampleRate, int nBitsPerSample, int nChannels, unsigned int nFadeTime,int nDirection)
{
	m_nFadeTime=nFadeTime;
	m_nDirection=nDirection;
	m_nSampleRate=nSampleRate;
	m_nBitsPerSample=nBitsPerSample;
	m_nChannels=nChannels;
}

/*\
 * <---------- Process ---------->
 * @m 
 * --> I N <-- @p
 * signed short int *ibuf - 
 * signed short int *obuf - 
 * uint32_tnSampleCount - 
\*/
void CFade::Process(signed short int *ibuf,signed short int *obuf, uint32_t nSampleCount)
{
	uint32_t nPosStart; 
	uint32_t nPosEnd;
	uint32_t nPosition;
	signed short int inLeft,inRight;
	uint32_t nFadingSince=(unsigned long)-1;
	uint32_t nFadingSamples=(unsigned long)-1;
	
	nFadingSamples=(unsigned long)((((uint64_t)m_nSampleRate*(m_nBitsPerSample>>3))*m_nFadeTime)/1000)*m_nChannels;
	ASSERT(nFadingSamples);
	if (!nFadingSamples)
		return;
	switch(m_nDirection)
	{
		case directionIn:
			nPosStart=0;
			nPosEnd=(unsigned long)((((uint64_t)m_nSampleRate*(m_nBitsPerSample>>3))*m_nFadeTime)/1000)*m_nChannels;
		break;
		case directionOut:
			nPosStart=(unsigned long)((((uint64_t)m_nSampleRate*(m_nBitsPerSample>>3))*m_nFadeTime)/1000)*m_nChannels;
			if (nPosStart > (nSampleCount*((m_nBitsPerSample>>3)*m_nChannels)))
				nPosStart=0;
			else
				nPosStart=(nSampleCount*((m_nBitsPerSample>>3)*m_nChannels))-nPosStart;
			nPosEnd=nSampleCount*((m_nBitsPerSample>>3)*m_nChannels);
		break;
	}
	nPosition=0;
	while (nSampleCount--)
	{
		if (nPosition >= nPosStart && nPosition < nPosEnd)
		{
			if (nFadingSince == (unsigned long)-1)
				nFadingSince=0;
			switch(m_nDirection)
			{
				case directionIn:
					inLeft = (signed short)(((uint64_t)nFadingSince * (signed long)*ibuf) / nFadingSamples);
					if (m_nChannels == 2)
						inRight = (signed short)(((uint64_t)nFadingSince * (signed long)*(ibuf+1)) / nFadingSamples);
				break;
				case directionOut:
					inLeft = (signed short)(((nFadingSamples-nFadingSince) * (uint64_t)*ibuf) / nFadingSamples);
					if (m_nChannels == 2)
						inRight = (signed short)(((nFadingSamples-nFadingSince) * (uint64_t)*(ibuf+1)) / nFadingSamples);
				break;
			}
			if (m_nChannels == 2)
			{
				*obuf = inLeft;
				*(obuf+1) = inRight;
			}
			else
				*obuf = inLeft;
			nFadingSince+=(m_nBitsPerSample>>3)*m_nChannels;
		}
		obuf+=m_nChannels;
		ibuf+=m_nChannels;
		nPosition+=(m_nBitsPerSample>>3)*m_nChannels;
	};
}
