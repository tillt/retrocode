/*\
 * ButterWorth.cpp
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
/*\
 * Code based on the butterworth implementation in
 * Sound Processing Kit - A C++ Class Library for Audio Signal Processing
 * Copyright (C) 1995-1998 Kai Lassfolk
 * as described in
 * Computer music: synthesis, composition, and performance
 * Charles Dodge, Thomas A. Jerse  [2nd ed.]
 * Page 214
\*/
#include "stdafx.h"
#include <math.h>
#include "../retroBase/Integer.h"
#include "Filter.h"
#include "ButterWorth.h"

CButterWorth::CButterWorth()
{
	m_dX[0] = 0.0;
	m_dX[1] = 0.0;
	m_dY[0] = 0.0;
	m_dY[1] = 0.0;
}

CButterWorth::~CButterWorth()
{
}


/*\ 
 * <---------- CButterWorth::Init ----------> 
 * @m 
 * --> I N <-- @p
 * int nMode - butterLowpass, butterHighpass, butterBandpass
 * double nFrequency - cutoff frequency
 * int nRate - sample frequency
 * double nBandwidth - bandpass range
\*/ 

/// <summary>creates a readable string from the sample compatibility data found inside the mobile sample content object</summary>
/// <param name="nMode">pointer to CMobileSampleContent object</param>
/// <param name="nFrequency">cutoff frequency</param>
/// <param name="nRate">sample frequency</param>
/// <param name="nBandwidth">bandpass range</param>
/// <returns>readable string showing the allowed sample format</returns>
void CButterWorth::Init(int nMode,double nFrequency,int nRate,double nBandwidth=0)
{
	double c,d;
	switch(nMode)
	{
		case butterLowpass:
			c = 1.0 / tan (M_PI * nFrequency / nRate);
			m_dA[0] = 1.0 / (1.0 + sqrt(2.0) * c + c * c);
			m_dA[1] = 2.0 * m_dA[0];
			m_dA[2] = m_dA[0];
			m_dB[0] = 2 * (1.0 - c * c) * m_dA[0];
			m_dB[1] = (1.0 - sqrt(2.0) * c + c * c) * m_dA[0];
		break;
		case butterHighpass:
			c = tan (M_PI * nFrequency / nRate);
			m_dA[0] = 1.0 / (1.0 + sqrt (2.0) * c + c * c);
			m_dA[1] = -2.0 * m_dA[0];
			m_dA[2] = m_dA[0];
			m_dB[0] = 2 * (c * c - 1.0) * m_dA[0];
			m_dB[1] = (1.0 - sqrt(2.0) * c + c * c) * m_dA[0]; 
		break;
		case butterBandpass:
			c = 1.0 / tan (M_PI * nBandwidth / nRate);
			d = 2 * cos (2 * M_PI * nFrequency / nRate);
			m_dA[0] = 1.0 / (1.0 + c);
			m_dA[1] = 0.0;
			m_dA[2] = -m_dA[0];
			m_dB[0] = -c * d * m_dA[0];
			m_dB[1] = (c - 1.0) * m_dA[0];
		break;
	}
}


/*\ 
 * <---------- CButterWorth::Process ----------> 
 * @m filter processing function (may be used packet-wise)
 * --> I N <-- @p
 * signed short int *ibuf - input buffer
 * signed short int *obuf - output buffer
 * uint32_tnSampleCount - number of samples to process
 \*/ 
void CButterWorth::Process(signed short int *ibuf,signed short int *obuf, uint32_t nSampleCount)
{
	double in;
	double out;
	/*
		X(1) = X(2) = 0
		Y(1) = Y(2) = 0
                    1
		c = ---------------------
			tan(pi * freq / rate)

					1	
		A(1) = ------------------
			1 + V2 * c + c * c

		A(2) = 2 * A(1)
		A(3) = A(1)
	*/
	while (nSampleCount--)
	{
		in = (double)(((signed long int)(*ibuf++))<<16);
		out = m_dA[0] * in + m_dA[1] * m_dX[0] + m_dA[2] * m_dX[1] - m_dB[0] * m_dY[0] - m_dB[1] * m_dY[1];
		m_dX[1] = m_dX[0];
		m_dX[0] = in;
		m_dY[1] = m_dY[0];
		m_dY[0] = out;
		*obuf++ = nClip((signed long int)(out/65536));
	};
} 
