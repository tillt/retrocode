/*\
 * Gain.cpp
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
#include "Basics.h"
#include "Normalizer.h"

CGain::CGain()
{
}

CGain::~CGain()
{
}

void CGain::Process(signed short int *ibuf,signed short int *obuf, unsigned long nSampleCount)
{
	double nOut;
	unsigned long nToDo;
	signed short int nIn;
	unsigned long int nAbsolute;
	unsigned long int nPeak=0;
	signed short *pnInput=ibuf;
	nToDo=nSampleCount;
	while (nToDo--)
	{
		nIn = (*ibuf++);
		if (nIn < 0)			//get absolute value of this sample
			nAbsolute=0-nIn;
		else
			nAbsolute=nIn;
		if (nAbsolute > nPeak)	//sample value higher than current max?
		{
			TRACEIT2("peak %04X at %08d\n",nAbsolute,nSampleCount-nToDo);
			nPeak=nAbsolute;	//remember this peak as the new max
		}
	};
	//is the sample on "normal" level allready?
	if (nPeak > 0 && nPeak < 0x8000)
	{	//nope->amplify
		ibuf=pnInput;
		nToDo=nSampleCount;
		while (nToDo--)
		{
			nIn = (*ibuf++);
			nOut=(((double)nIn*0x8000))/nPeak;
			(*obuf++)= nClip((signed long int)nOut);
		};
	}
	else
	{
		TRACEIT2("sample allready at max level\n");
	}
} 

