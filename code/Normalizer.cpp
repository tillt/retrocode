/*\
 * Normalizer.cpp
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
#include "Normalizer.h"

CNormalizer::CNormalizer()
{
}

CNormalizer::~CNormalizer()
{
}

/*\
 * <---------- CNormalizer :: Process ----------> 
 * @m two pass normalizer
 * --> I N <-- @p
 * signed short int *ibuf     - pointer to input buffer
 * signed short int *obuf     - pointer to output buffer
 * uint32_tnSampleCount - number of samples to process
\*/
void CNormalizer::Process(signed short int *ibuf,signed short int *obuf, uint32_t nSampleCount)
{
	double nOut;
	uint32_t nToDo;
	signed short int nIn;
	uint32_t nAbsolute;
	uint32_t nPeak=0;
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
			TRACEIT2("peak %04X (%1.2fdB) at %08d\n",nAbsolute,fAmpTodB(nAbsolute),nSampleCount-nToDo);
			nPeak=nAbsolute;	//remember this peak as the new max
		}
	};
	//is the sample on "normal" level allready?
	if (nPeak > 0.0 && nPeak < 32768.0)
	{	//nope->amplify
		Log2(verbLevMessages,"sample peaked with %1.2fdB\n",fAmpTodB(nPeak));
		ibuf=pnInput;
		nToDo=nSampleCount;
		while (nToDo--)
		{
			nIn = (*ibuf++);
			nOut=(((double)nIn*0x8000))/nPeak;
			(*obuf++)= nClip((int32_t)nOut);
		};
	}
	else
	{
		Log2(verbLevWarnings,"sample allready at max level, no normalizing needed\n");
	}
} 
