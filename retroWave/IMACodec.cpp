/*\
 * IMACodec.cpp
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
#include <iostream>
#include <map>
#include <math.h>
#include <strstream>
#include <fstream>
#include "Basics.h"
#include "WaveFile.h"
#include "WaveProperty.h"
#include "Adpcm.h"

/*\ 
 * <---------- *CWaveFile::pcIMAAdpcmCompress ----------> 
 * @m 
 * --> I N <-- @p
 * signed short *pIn - 
 * unsigned long nSize - 
 * unsigned long *pnOutSize - 
 * <-- OUT --> @r
 * unsigned char  - 
 \*/ 
/*
unsigned char *CWaveFile::pcIMAAdpcmCompress(signed short *pIn,unsigned long nSize,unsigned long *pnOutSize)
{
	unsigned char *pcOut=NULL;
	signed short *pcExtra=NULL;
	unsigned char *p;
	int iStateIndex;
	int nBlockAlign;
	unsigned long nSamples;

	*pnOutSize=0;
	iStateIndex=0;
	pcOut=(unsigned char *)calloc(nSize/2,1);
	p=pcOut;

	nSamples=m_Adpcm.wSamplesPerBlock;
	nBlockAlign=m_Adpcm.wave->nBlockAlign;

	TRACE2("compressing PCM data into IMA ADPCM...\n");
	while(nSize >= (unsigned long)m_Adpcm.wSamplesPerBlock*2)
	{
		unsigned long nLen;
		//compress block
		IMAAdpcmCompressBlock(	m_Header.nChannels,
								pIn,
								nSamples,
								&iStateIndex,
								p);
		*pnOutSize+=nBlockAlign;
		p+=nBlockAlign;
		nLen=nSamples;
		pIn+=nLen;
		nSize-=nLen*2;
	};
	//still unencoded samples left? (total sample size not devideable by blocksize)
	if (nSize)
	{	//yes->...
		pcExtra=new signed short [m_Adpcm.wSamplesPerBlock];				//alloc extra block
		ZeroMemory(pcExtra,m_Adpcm.wSamplesPerBlock*2);						//empty total block
		CopyMemory(pcExtra,pIn,nSize);										//copy remaining, unencoded samples into the block
		//compress block
		IMAAdpcmCompressBlock(	m_Header.nChannels,pcExtra,nSamples,&iStateIndex,p);
		*pnOutSize+=nBlockAlign;	//raise output size
		delete [] pcExtra;
	}
	return pcOut;
}
*/