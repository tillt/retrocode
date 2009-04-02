/*\
 * ChannelConverter.cpp
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
#include "ChannelConverter.h"

CChannelConverter::CChannelConverter() : m_nSourceChannels(0),m_nDestChannels(0)
{
}

CChannelConverter::~CChannelConverter()
{
}

void CChannelConverter::Init(int nSourceChannels,int nDestChannels)
{
	m_nSourceChannels=nSourceChannels;
	m_nDestChannels=nDestChannels;
}

uint32_t CChannelConverter::nGetDestSize(uint32_t nSourceSize)
{
	return (nSourceSize/m_nSourceChannels)*m_nDestChannels;	
}

void CChannelConverter::Process(signed short int *ibuf,signed short int *obuf, uint32_t nSampleCount)
{
	signed short int inLeft,inRight;
	while (nSampleCount--)
	{
		inLeft = (*ibuf++);
		if (m_nSourceChannels == 2)
			inRight = (*ibuf++);
		else
			inRight = inLeft;

		if (m_nDestChannels == 2)
		{
			*obuf++ = inLeft;
			*obuf++ = inRight;
		}
		else
		{
			*obuf++ = nClip((signed long int)(inLeft+inRight)/2);
		}
	};
} 

