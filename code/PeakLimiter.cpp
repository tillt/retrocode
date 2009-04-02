/*\
 * Peaklimiter.cpp
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
#include "PeakLimiter.h"

CPeaklimit::CPeaklimit() : m_nLimited(0)
{
}

CPeaklimit::~CPeaklimit()
{
}

/*\
 * <---------- Init ---------->
 * @m configure peak-limiter parameters
 * --> I N <-- @p
 * int nMode - peakLimit, peakVolume
 * float fGain - amplification
 * float fLimitGain - limiting threshold (used only for nMode=peakLimit)
\*/
void CPeaklimit::Init(int nMode,float fGain, float fLimitGain)
{
	m_nMode=nMode;
	m_dGain=(float)fGain;
	m_dGain=exp(fGain*(float)0.1151292546497022842009e0); 
	ASSERT(m_dGain > 1.0e0);
	m_dLimitGain=(float)fLimitGain;
	ASSERT(m_dLimitGain < 1.0e0);
	m_dThreshhold=0x7fffffffL * ((float)1.0e0 - m_dLimitGain) / (fabs(m_dGain) - m_dLimitGain);
}

void CPeaklimit::Process(signed short int *ibuf,signed short int *obuf, uint32_t nSampleCount)
{
    double sample;
  
	switch (m_nMode)
	{
		case peakLimit:
		{
			TRACEIT2("processing %d samples in limiter mode\n",nSampleCount);
			while (nSampleCount--)
			{
	    		sample = (double)((signed long)(*ibuf++)<<16);	
					
	    		if (sample > m_dThreshhold)									//sample exceeds thresshold, its a peak that needs limiting
	    		{
	    			sample =  ((double)0x7fffffffL - m_dGain * (0x7fffffffL - sample));
	    			m_nLimited++;
	    		}
	    		else if (sample < -m_dThreshhold)							//sample exceeds thresshold, its a peak that needs limiting
	    		{
	    			sample = -((double)0x7fffffffL - m_dGain * (0x7fffffffL + sample));
	    			m_nLimited++;
	    		}
	    		else
	    			sample = (double)m_dGain * sample;						//sample gets normal amplification
				*obuf++ = nClip((signed long int)sample/65536);
			};
			TRACEIT2("limited %d samples\n",m_nLimited);
		}
		break;
		case peakVolume:
		{
			TRACEIT2("processing %d samples in amplifier mode\n",nSampleCount);
    		while (nSampleCount--)
				*obuf++ = nClip( (signed long int) (m_dGain * (((signed long int)(*ibuf++)<<16)/65536)) );
		}
		break;
    }
}
