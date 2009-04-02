/*\
 * EightToSixteenBit.cpp
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
#include "EightToSixteenBit.h"

CEightToSixteenBit::CEightToSixteenBit() : m_nChannels(0)
{
}

CEightToSixteenBit::~CEightToSixteenBit()
{
}

void CEightToSixteenBit::Init(int nChannels)
{
	m_nChannels=nChannels;
}

uint32_t CEightToSixteenBit::nGetDestSize(uint32_t nSourceSize)
{
	return nSourceSize*2;
}

void CEightToSixteenBit::Process(signed short int *ibuf,signed short int *obuf, uint32_t nSampleCount)
{
	unsigned char *in=(unsigned char *)ibuf;
	signed short int inLeft,inRight;
	
	while (nSampleCount--)
	{
		inLeft = (((signed int)*in) << 8) - 32768;
		if (m_nChannels == 2)
		{
			inRight = ((int)*(in+1) << 8) - 32768;
			*obuf++ = inLeft;
			*obuf++ = inRight;
		}
		else
			*obuf++ = inLeft;
		in+=m_nChannels;
	};
} 
