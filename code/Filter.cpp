/*\
 * Filter.cpp
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
#include "../retroBase/Integer.h"
#include "Filter.h"

CFilter::CFilter() : m_nClipped(0)
{

}

CFilter::~CFilter()
{
}

signed int CFilter::ndBToAmp(const double dB)
{ 
	return (signed int)(0x8000 * pow(10.0,dB/20.0)); 
}

double CFilter::fAmpTodB(const int32_t nAmp)
{
	return 20.0*log10(fabs((double)nAmp / 0x8000));
}

/*\
 * <---------- nClip ---------->
 * @m make sure that sample values are in proper limits
 * --> I N <-- @p
 * signed long int nSample - sample input value 
 * <-- OUT --> @r
 * signed short int - sample output value
\*/
signed short int CFilter::nClip(const int32_t nSample)
{
	if (nSample > 0x7FFF)
	{
		m_nClipped++;
		return 0x7FFF;
	}
	else if (nSample < -0x8000)
	{
		m_nClipped++;
		return -0x8000;
	}
	return (signed short int)nSample;
}
