/*\
 * MSCodec.cpp
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
#include "../retroBase/Basics.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/Adpcm.h"
#include "WaveFile.h"
#include "WaveProperty.h"


/*============================================================================================
**	MS ADPCM Block Layout.
**	======================
**	Block is usually 256, 512 or 1024 bytes depending on sample rate.
**	For a mono file, the block is laid out as follows:
**		byte	purpose
**		0		block predictor [0..6]
**		1,2		initial idelta (positive)
**		3,4		sample 1
**		5,6		sample 0
**		7..n	packed bytecodes
**
**	For a stereo file, the block is laid out as follows:
**		byte	purpose
**		0		block predictor [0..6] for left channel
**		1		block predictor [0..6] for right channel
**		2,3		initial idelta (positive) for left channel
**		4,5		initial idelta (positive) for right channel
**		6,7		sample 1 for left channel
**		8,9		sample 1 for right channel
**		10,11	sample 0 for left channel
**		12,13	sample 0 for right channel
**		14..n	packed bytecodes
==============================================================================================*/

/*\ 
 * <---------- *CWaveFile::pcMSAdpcmCompress ----------> 
 * @m 
 * --> I N <-- @p
 * signed short *pIn - sample buffer
 * uint32_tnSize - sample buffer size in bytes
 * uint32_t*pnOutSize - output size in bytes
 * <-- OUT --> @r
 * unsigned char *- pointer to output buffer
\*/ 
unsigned char *CWaveFile::pcMSAdpcmCompress(int16_t *pIn,uint32_t nSize,uint32_t *pnOutSize)
{
	unsigned char *pcOut=NULL;
	int16_t *pcExtra=NULL;
	unsigned char *p;
	int32_t iStepIndex[2];
	uint32_t nBlockAlign;
	uint32_t nChannels;
	uint32_t nSamples;

	*pnOutSize=0;
	iStepIndex[0]=0;
	iStepIndex[1]=0;
	pcOut=(unsigned char *)CMobileSampleContent::Alloc(nSize/2);
	p=pcOut;

	nSamples=m_Adpcm.wSamplesPerBlock;
	nBlockAlign=m_Adpcm.wave->wBlockAlign;
	nChannels=m_Header.wChannels;

	Log2(5,"compressing PCM data into MS ADPCM...\n");
	while(nSize >= (uint32_t)nChannels*nSamples*2)
	{
		//compress block
		m_codec.MSAdpcmCompressBlock(	nChannels,
										pIn,
										nSamples,
										&iStepIndex[0],
										p,
										nBlockAlign);
		*pnOutSize+=nBlockAlign;
		p+=nBlockAlign;
		pIn+=nSamples*nChannels;
		nSize-=nSamples*nChannels*2;
	};
	//still unencoded samples left? (total sample size not devideable by blocksize)
	if (nSize)
	{	//yes->...
		pcExtra=new int16_t[nChannels*nSamples];			//alloc extra block
		ZeroMemory(pcExtra,nChannels*nSamples*2);				//empty total block
		CopyMemory(pcExtra,pIn,nSize);							//copy remaining, unencoded samples into the block
		//compress block
		m_codec.MSAdpcmCompressBlock(	nChannels,
										pcExtra,
										nSamples,
										&iStepIndex[0],
										p,
										nBlockAlign);
		*pnOutSize+=nBlockAlign;								//raise output size
		delete [] pcExtra;
	}
	return pcOut;
}

/*\ 
 * <---------- *CWaveFile::pwMSAdpcmDecompress ----------> 
 * @m 
 * --> I N <-- @p
 * unsigned char *pIn - 
 * uint32_tnSize - 
 * uint32_t*pnOutSize - 
 * <-- OUT --> @r
 * signed short  - 
\*/ 
signed short *CWaveFile::pwMSAdpcmDecompress(unsigned char *pIn,uint32_t nSize,uint32_t *pnOutSize)
{
	int16_t *pwOut=NULL;
	int16_t *p;
	*pnOutSize=0;
	pwOut=(int16_t *)CMobileSampleContent::Alloc(nSize*5);
	p=pwOut;
	
	uint32_t nSamples=m_Adpcm.wSamplesPerBlock;
	uint32_t nBlockAlign=m_Adpcm.wave->wBlockAlign;

	Log2(5,"decompressing MS ADPCM data...\n");
	while(nSize)
	{
		uint32_t nLen;
		if (nSize < nBlockAlign)
		{
			Log2(3,"short adpcm block (samples %d, size left: %d)\n",nSamples,nSize);
			nSamples=(nSize-(6*m_Header.wChannels))*2;
		}
		//decompress block
		m_codec.MSAdpcmDecompressBlock(	m_Header.wChannels,
										m_Adpcm.wNumCoef,
										(int16_t *)m_Adpcm.paCoeff,
										pIn,
										p,
										nSamples	);
		*pnOutSize+=m_Header.wChannels*nSamples*sizeof(int16_t);
		p+=m_Header.wChannels*nSamples;
		//nLen=(nSamples/2)+(6*m_Header.wChannels);
		nLen=nBlockAlign;
		pIn+=nLen;
		nSize-=min(nLen,nSize);
	};
	return pwOut;
}
