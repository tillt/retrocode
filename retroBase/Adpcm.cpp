/*\
 * Adpcm.cpp
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
/*
	Many parts of this were directly adopted from SOX.
*/
#include "stdafx.h"
#include <iostream>
#include <map>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#ifdef WIN32
#include <strstream>
#endif
#include <fstream>
#include "Basics.h"
#include "Adpcm.h"
#include "MobileContent.h"
#include "Endian.h"

//these are step-size adjust factors, where 1.0 is scaled to 0x100
const unsigned int CAdpcm::m_nMSStepAdjustTable[16] = 
{
    230, 230, 230, 230, 307, 409, 512, 614,
    768, 614, 512, 409, 307, 230, 230, 230
};

const signed short int CAdpcm::m_nMSAdpcmCoef[7][2] = 
{
	{ 256,   0},
	{ 512,-256},
	{   0,   0},
	{ 192,  64},
	{ 240,   0},
	{ 460,-208},
	{ 392,-232}
};

//adpcm index table
const int CAdpcm::m_nIMAIndexTable[16] =
{
	-1, -1, -1, -1, 2, 4, 6, 8,
	-1, -1, -1, -1, 2, 4, 6, 8,
};

const int CAdpcm::m_nIMAStepSizeTable[89]=
{
	0x0007, 0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E,
	0x0010, 0x0011, 0x0013, 0x0015, 0x0017, 0x0019, 0x001C, 0x001F,
	0x0022, 0x0025, 0x0029, 0x002D, 0x0032, 0x0037, 0x003C, 0x0042,
	0x0049, 0x0050, 0x0058, 0x0061, 0x006B, 0x0076, 0x0082, 0x008F,
	0x009D, 0x00AD, 0x00BE, 0x00D1, 0x00E6, 0x00FD, 0x0117, 0x0133,
	0x0151, 0x0173, 0x0198, 0x01C1, 0x01EE, 0x0220, 0x0256, 0x0292,
	0x02D4, 0x031C, 0x036C, 0x03C3, 0x0424, 0x048E, 0x0502, 0x0583,
	0x0610, 0x06AB, 0x0756, 0x0812, 0x08E0, 0x09C3, 0x0ABD, 0x0BD0,
	0x0CFF, 0x0E4C, 0x0FBA, 0x114C, 0x1307, 0x14EE, 0x1706, 0x1954,
	0x1BDC, 0x1EA5, 0x21B6, 0x2515, 0x28CA, 0x2CDF, 0x315B, 0x364B,
	0x3BB9, 0x41B2, 0x4844, 0x4F7E, 0x5771, 0x602F, 0x69CE, 0x7462,
	0x7FFF
};
	
#define lsbshortldi(x,p) { (x)=((signed short int )((signed int)(p)[0] + ((signed int)(p)[1]<<8))); (p) += 2; }

CAdpcm::CAdpcm(void) : m_bBraindead(false), m_cFillByte(0x00)
{
}

CAdpcm::~CAdpcm(void)
{
	TRACEIT2("killing\n");
}

const signed short int *CAdpcm::pnGetMSCoef(void)
{
	return &m_nMSAdpcmCoef[0][0];
}

const int *CAdpcm::pnGetIMAStepSizeTable(void) 
{ 
	return m_nIMAStepSizeTable;	
}

void CAdpcm::init(bool bBraindead,unsigned char cFillByte)
{
	m_bBraindead=bBraindead;
	m_cFillByte=cFillByte;
	initImaTable();
}

void CAdpcm::initImaTable(void)
{
    int i,j,k;
    for (i=0;i <= ISSTMAX;i++) 
	{
        for (j=0;j < 8;j++) 
		{
            k = i + ((j < 4) ? -1 : (2*j-6));
            if (k < 0) 
				k=0;
            else if (k > ISSTMAX) 
				k=ISSTMAX;
            m_cIMAStateAdjustTable[i][j] = k;
        }
    }
} 

/*\ 
 * <---------- CAdpcm::IMAAdpcmDecompressSample ----------> 
 * @m 
 * --> I N <-- @p
 * int ch - channel number to decode, REQUIRE 0 <= ch < chans
 * int chans - total channels           
 * const unsigned char *ibuff - input buffer[blockAlign]
 * signed short *obuff - obuff[n] will be output samples
 * int n - samples to decode PER channel, REQUIRE n % 8 == 1
 * int o_inc - index difference between successive output samples
 \*/ 
void CAdpcm::IMAAdpcmDecompressSample(uint32_t iChannel,uint32_t nChannels,const unsigned char *pcInput,int16_t *pwOutput,int32_t nSamplesPerBlock)
{
    const unsigned char *ip;
    int16_t *op;
    int32_t i_inc;
    int32_t i, val, state;

    ip = pcInput + 4*iChannel;			// input pointer to 4-byte block state-initializer
    i_inc = 4 * (nChannels-1);			// amount by which to incr ip after each 4-byte read
    val = (int16_t)(ip[0] + (ip[1]<<8));	// need cast for sign-extend
    state = ip[2];
    if (state > ISSTMAX) 
	{
		Log2(verbLevWarnings,"ADPCM block ch%d initial-state (%d) out of range\n", iChannel, state);
        state = 0;
    }
    // specs say to ignore ip[3] , but write it as 0
    ip += 4+i_inc;

    op = pwOutput;
    *op = val;				// 1st output sample for this channel
    op += nChannels;

    for (i = 1; i < nSamplesPerBlock; i++) 
	{
        int step,dp,c,cm;
        if (i&1) 
		{   // 1st of pair
			if (m_bBraindead)
				cm = *ip >> 4 ;
			else
				cm = *ip & 0x0f;
        } 
		else 
		{
			if (m_bBraindead)
				cm = (*ip++) & 0x0f;
			else
				cm = (*ip++)>>4;
			if ((i&7) == 0)		/* ends the 8-sample input block for this channel */
				ip += i_inc;	/* skip ip for next group */ 
        }
        c = cm & 0x07;
        step = m_nIMAStepSizeTable[state];
        state = m_cIMAStateAdjustTable[state][c];		// update the state for the next sample
        dp = 0;
        if (c & 4) 
			dp += step;
        step = step >> 1;
        if (c & 2) 
			dp += step;
        step = step >> 1;
        if (c & 1) 
			dp += step;
        step = step >> 1;
        dp += step;
		if (c != cm) 
		{
            val -= dp;
            if (val < -0x8000) 
				val = -0x8000;
        } 
		else 
		{
            val += dp;
            if (val > 0x7fff) 
				val = 0x7fff;
        }
        *op = val;
        op += nChannels;
    }
    return;
} 

/*\ 
 * <---------- CAdpcm::IMAAdpcmDecompressBlock ----------> 
 * @m outputs interleaved samples into one output buffer
 * --> I N <-- @p
 * int chans - total channels
 * const unsigned char *ibuff - input buffer
 * signed short int *obuff - output buffer
 * int n - sample count
\*/ 
void CAdpcm::IMAAdpcmDecompressBlock(uint32_t nChannels,const unsigned char *pcInput,int16_t *pwOutput,int32_t nSamplesPerBlock)
{
	for (uint32_t iChannel=0;iChannel < nChannels;iChannel++)
		IMAAdpcmDecompressSample(iChannel, nChannels, pcInput, pwOutput+iChannel, nSamplesPerBlock);
} 

/*\
 * <---------- CAdpcm :: nIMAOptimizeState ----------> 
 * @m 
 * --> I N <-- @p
 * int nLevels                     - 
 *  int iChannel                   - 
 * int nChannels                   - 
 * const signed short int *pwInput - 
 * int nSamplesPerBlock            - 
 * int nIOState                    - 
 * <-- OUT --> @r
 * 
\*/
int32_t CAdpcm::nIMAOptimizeState(int32_t nLevels, uint32_t iChannel,uint32_t nChannels,const int16_t *pwInput,uint32_t nSamplesPerBlock,int32_t nIOState)
{
	int32_t nSNext;				//
	int32_t s0,d0;				//
	int32_t s32,d32;			//

	int32_t low,hi,w;			//
	int32_t low0,hi0;			//

	//init the state recalc with the current value
	s32 = s0 = nIOState;
	nSNext = s0;
	d32 = d0 = IMAAdpcmCompressChannelBlock(iChannel, nChannels, *pwInput, pwInput, nSamplesPerBlock, &nSNext, NULL);

	w = 0;
	low=hi=s0;
	low0 = low-nLevels; 
	if (low0<0) 
		low0=0;
	hi0 = hi+nLevels; 
	if (hi0 > ISSTMAX) 
		hi0=ISSTMAX;
	while (low > low0 || hi < hi0) 
	{
		if (!w && low > low0) 
		{
			int d;
			nSNext = --low;
			d = IMAAdpcmCompressChannelBlock(iChannel, nChannels, *pwInput, pwInput, nSamplesPerBlock, &nSNext, NULL);			
			if (d < d0)			//is the rms error smaller?
			{					//yes->optimizing found a closer (lower) status for the next block-start
				d0=d; 
				s0=low;
				low0 = low-nLevels; 
				if (low0 < 0) 
					low0=0;
				hi0 = low+nLevels; 
				if (hi0 > ISSTMAX) 
					hi0=ISSTMAX;
			}
		}
		if (w && hi < hi0) 
		{
			int d;
			nSNext = ++hi;
			d = IMAAdpcmCompressChannelBlock(iChannel, nChannels, *pwInput, pwInput, nSamplesPerBlock, &nSNext, NULL);
			if (d < d0)			//is the rms error smaller?
			{					//yes->optimizing found a closer (higher) status for the next block-start
				d0=d; 
				s0=hi;
				low0 = hi-nLevels; 
				if (low0 < 0) 
					low0=0;
				hi0 = hi+nLevels; 
				if (hi0 > ISSTMAX) 
					hi0=ISSTMAX;
			}
		}
		w=1-w;
	};
	return s0;
}

/*\ 
 * <---------- CAdpcm::IMAAdpcmCompressSample ----------> 
 * @m encode a sample (mono or stereo) into IMA ADPCM
 * --> I N <-- @p
 * int ch - channel number to encode
 * int chans - channel count
 * signed short wPredictionValue - value to use as starting prediction0
 * const signed short int *ibuff - ibuff[] is interleaved input samples
 * int n - samples to encode PER channel, REQUIRE n % 8 == 1 
 * int *iostate - input/output state, REQUIRE 0 <= *st <= ISSTMAX
 * unsigned char *obuff - output buffer[blockAlign], or NULL for no output
 * <-- OUT --> @r
 * int - RMS error value
 \*/ 
int32_t CAdpcm::IMAAdpcmCompressChannelBlock(uint32_t iChannel,uint32_t nChannels,int16_t wPredictionValue,const int16_t *pwInput, uint32_t nSamplesPerBlock,int32_t *pwIOState,unsigned char *pcOutput)
{
    const int16_t *ip, *itop;
    unsigned char *op;			//
    int32_t o_inc = 0;		// set 0 only to shut up gcc's 'might be uninitialized' 
    int32_t i, val;			//
    int32_t state;			//
    double d2;					// long long is okay also, speed abt the same 

	//TRACEIT2("nSamplesPerBlock %d\n",nSamplesPerBlock);
    ip = pwInput + iChannel;	// point ip to 1st input sample for this channel 
    itop = pwInput + nSamplesPerBlock * nChannels;
    val = *ip - wPredictionValue;	// difference of first and second sample
	ip += nChannels;				// 1st input sample for this channel 
    d2 = val*val;					// d2 will be sum of squares of errors, given input v0 and *st 
    val = wPredictionValue;       

	//TRACEIT2("itop: %08X\n",itop);
    op = pcOutput;				// output pointer (or NULL) 
    if (op) 
	{							// NULL means don't output, just compute the rms error 
        op += 4*iChannel;		// where to put this channel's 4-byte block state-initializer 
        o_inc = 4*(nChannels-1);// amount by which to incr op after each 4-byte written 
        *op++ = val; 
		*op++ = val>>8;
        *op++ = *pwIOState; 
		*op++ = m_cFillByte;	// they could have put a mid-block state-correction here  
        op += o_inc;			// _sigh_   NEVER waste a byte.      It's a rule!
								//
								// waste is not if one individual doesnt understand the purpose of something ->
								// that byte can now nicely be used for resyncronizing to broken streams ->
								// this is not waste or i am a messy - which by the way, i am 
    }

	ASSERT(pwIOState);
	state = *pwIOState;

    for (i = 0; ip < itop; ip+=nChannels) 
	{
        int32_t step,d,dp,c;

        d = *ip - val;  // difference between last prediction and current sample 

		ASSERT(state < ISSTMAX+1);
        step = m_nIMAStepSizeTable[state];
        c = (abs(d)<<2)/step;
        if (c > 7) 
			c = 7;
        // Update the state for the next sample 
        state = m_cIMAStateAdjustTable[state][c];

        if (op) 
		{   // if we want output, put it in proper place 
            int32_t cm = c;
            if (d < 0) 
				cm |= 8;
            if (i & 1) 
			{							// odd numbered output 
				if (m_bBraindead)
					*op++ |= cm;
				else
					*op++ |= cm<<4;
				if (i == 7)				// ends the 8-sample output block for this channel 
					op += o_inc;		// skip op for next group 
            } 
			else 
			{
				if (m_bBraindead)
					*op = (cm<<4);
				else
					*op = cm;
            }
            i = (i+1) & 0x07;
        }

        dp = 0;
        if (c & 4) 
			dp += step;
        step >>= 1;
        if (c & 2) 
			dp += step;
        step >>= 1;
        if (c & 1) 
			dp += step;
        step >>= 1;
        dp += step;
		//dp = ((c+c+1) * step) >> 3; /* faster than bit-test & add on my cpu */

	    if (d < 0) 
		{
            val -= dp;
            if (val < -0x8000) 
				val = -0x8000;
        } 
		else 
		{
            val += dp;
            if (val > 0x7fff) 
				val = 0x7fff;
        }
        {
            int32_t x = *ip - val;			//calc error
            d2 += x*x;							//square error
        }
    }
	d2 /= nSamplesPerBlock;						// be sure it's non-negative 
    *pwIOState = state;
    return (int32_t) sqrt(d2);						//return rms of error
}


/* mash one block.  if you want to use opt>0, 9 is a reasonable value */
/* total channels */
/* ip[] is interleaved input samples */
/* samples to encode PER channel, REQUIRE n % 8 == 1 */
/* input/output state, REQUIRE 0 <= *st <= ISSTMAX */
/* output buffer[blockAlign] */


/*\
 * <---------- CAdpcm :: IMAAdpcmCompressBlock ----------> 
 * @m compress one block.  if you want to use opt>0, 9 is a reasonable value
 * --> I N <-- @p
 * int nChannels                   - 
 * const signed short int *pwInput - 
 * int n                           - 
 * int *pwIOState                  - 
 * unsigned char *pcOutput         - 
\*/
void CAdpcm::IMAAdpcmCompressBlock(uint32_t nChannels,const int16_t *pwInput,uint32_t nSamplesPerBlock,int32_t *pwIOState,unsigned char *pcOutput)
{
    for (uint32_t iChannel=0;iChannel < nChannels; iChannel++)
	{
		*pwIOState=nIMAOptimizeState(20,iChannel,nChannels,pwInput,nSamplesPerBlock,*pwIOState);
		IMAAdpcmCompressChannelBlock(iChannel, nChannels, *pwInput, pwInput, nSamplesPerBlock, pwIOState, pcOutput);
	}
} 

/*\ 
 * <---------- CAdpcm::MSAdpcmDecompressBlock ----------> 
 * @m decompress source block data
 * --> I N <-- @p
 * int nChannels - number of channels to decode
 * int nCoef - number of used coefficients in list
 * const short *iCoef - pointer to coefficient list data
 * const unsigned char *ibuff - adpcm input data pointer
 * signed short *obuff - raw pcm output data pointer (16bit)
 * int n - number of samples to decode
\*/ 
void CAdpcm::MSAdpcmDecompressBlock(uint32_t nChannels,uint32_t nCoef,const int16_t *iCoef,const unsigned char *ibuff,int16_t *obuff,int32_t n)
{
	const unsigned char *ip;
	uint32_t ch;
	MsState state[4];		/* One decompressor state for each channel */

	/* Read the four-byte header for each channel */
	ip = ibuff;
	for (ch = 0; ch < nChannels; ch++) 
	{
		unsigned char bpred = *ip++;
		if (bpred >= nCoef) 
		{
			Log2(verbLevDebug1,"MSADPCM bpred >= nCoef, arbitrarily using 0\n");
			bpred = 0;
		}
		state[ch].iCoef[0] = iCoef[(int)bpred*2+0];
		state[ch].iCoef[1] = iCoef[(int)bpred*2+1];
	}

	for (ch = 0; ch < nChannels; ch++)
		lsbshortldi(state[ch].step, ip);

	/* sample1's directly into obuff */
	for (ch = 0; ch < nChannels; ch++)
		lsbshortldi(obuff[nChannels+ch], ip);

	/* sample2's directly into obuff */
	for (ch = 0; ch < nChannels; ch++)
		lsbshortldi(obuff[ch], ip);

	{
		uint32_t ch;
		unsigned char b;
		int16_t *op, *top, *tmp;

		/* already have 1st 2 samples from block-header */
		op = obuff + 2*nChannels;
		top = obuff + n*nChannels;

		ch = 0;
		while (op < top) 
		{
			b = *ip++;
			tmp = op;
			*op++ = nMSAdpcmDecompressSample(b >> 4, state+ch, tmp[-(int32_t)nChannels], tmp[-2*(int32_t)nChannels]);
			if (++ch == nChannels) 
				ch = 0;
			tmp = op;
			*op++ = nMSAdpcmDecompressSample(b&0x0f, state+ch, tmp[-(int32_t)nChannels], tmp[-2*(int32_t)nChannels]);
			if (++ch == nChannels) 
				ch = 0;
		};
	}
} 

/*\ 
 * <---------- CAdpcm::nMSAdpcmDecompressSample ----------> 
 * @m 
 * --> I N <-- @p
 * signed char c - input nibble
 * MsState *state - current decoding status
 * signed short sample1 - pre sample
 * signed short sample2 - pre pre sample
 * <-- OUT --> @r
 * signed short  - sample data
\*/ 
int16_t CAdpcm::nMSAdpcmDecompressSample(signed char c,MsState *state,int16_t sample1,int16_t sample2)
{ 
	int32_t vlin;
	int32_t sample;
	int32_t step;

	/** Compute next step value **/
	step = state->step;
	{
		int32_t nstep;
		nstep = (m_nMSStepAdjustTable[c] * step) >> 8;
		state->step = (nstep < 16)? 16 : nstep;
	}

	/** make linear prediction for next sample **/
	vlin = ((sample1 * state->iCoef[0]) + (sample2 * state->iCoef[1])) >> 8;
	/** then add the code*step adjustment **/
	c -= (c & 0x08) * 2;
	sample = (c * step) + vlin;

	if (sample > 0x7fff) 
		sample = 0x7fff;
	else if (sample < -0x8000) 
		sample = -0x8000;

	return (int16_t)sample;
} 

/*\ 
 * <---------- CAdpcm::MSAdpcmCompressBlock ----------> 
 * @m compress
 * --> I N <-- @p
 * int chans - number of channels
 * const signed short *ip - 
 * int n - 
 * int *st - 
 * unsigned char *obuff - 
 * int blockAlign - 
\*/ 
void CAdpcm::MSAdpcmCompressBlock(uint32_t chans,const int16_t *ip,int32_t n,int32_t *st,unsigned char *obuff,int32_t blockAlign)
{
    uint32_t ch;
    unsigned char *p;

    //TRACEIT2("(chans %d, ip %p, n %d, st %p, obuff %p, bA %d)\n",chans, ip, n, st, obuff, blockAlign);
    for (p=obuff+7*chans;p < obuff+blockAlign; p++) 
		*p=0;

    for (ch=0; ch<chans; ch++)
		MSAdpcmCompressChannel(ch, chans, ip, n, st+ch, obuff);
}
 
/*\ 
 * <---------- CAdpcm::MSAdpcmCompressChannel ----------> 
 * @m 
 * --> I N <-- @p
 * int ch - 
 * int chans - 
 * const signed short int *ip - 
 * int n - 
 * int *st - 
 * unsigned char *obuff - 
\*/ 
void CAdpcm::MSAdpcmCompressChannel(uint32_t ch,uint32_t chans,const int16_t *ip,int32_t n,int32_t *st,unsigned char *obuff)
{
    int16_t v[2];
    int32_t n0,s0,s1,ss,smin;
    int32_t d,dmin,k,kmin;

    n0 = n/2; 
	if (n0 > 32) 
		n0=32;

	if (*st < 16) 
		*st = 16;
    
	v[1] = ip[ch];
    v[0] = ip[ch+chans];

    dmin = 0; 
	kmin = 0; 
	smin = 0;
    // for each of 7 standard coeff sets, we try compression
    // beginning with last step-value, and with slightly
    // forward-adjusted step-value, taking best of the 14
    for (k=0;k < 7;k++) 
	{
        int32_t d0,d1;
        ss = s0 = *st;
		d0=MSAdpcmCompressSample(ch, chans, v, m_nMSAdpcmCoef[k], ip, n, &ss, NULL); /* with step s0 */

        s1 = s0;
        MSAdpcmCompressSample(ch, chans, v, m_nMSAdpcmCoef[k], ip, n0, &s1, NULL);
        
		//TRACEIT2(" s32 %d\n",s1);

        ss = s1 = (3*s0+s1)/4;
        
		d1=MSAdpcmCompressSample(ch, chans, v, m_nMSAdpcmCoef[k], ip, n, &ss, NULL); /* with step s1 */
        
		if (!k || d0 < dmin || d1 < dmin) 
		{
            kmin = k;
            if (d0 <= d1) 
			{
                dmin = d0;
                smin = s0;
            }
			else
			{
                dmin = d1;
                smin = s1;
            }
        }
    }
	//store step index
    *st = smin;
    //TRACEIT2("kmin %d, smin %5d\n",kmin,smin);
    d=MSAdpcmCompressSample(ch, chans, v, m_nMSAdpcmCoef[kmin], ip, n, st, obuff);
	//store coefficient index
    obuff[ch] = kmin;
} 

/*\ 
 * <---------- CAdpcm::MSAdpcmCompressSample ----------> 
 * @m 
 * --> I N <-- @p
 * int ch - 
 * int chans - 
 * signed short v[2] - 
 * const short iCoef[2] - 
 * const signed short int *ibuff - 
 * int n - 
 * int *iostep - 
 * unsigned char *obuff - 
 * <-- OUT --> @r
 * int  - 
\*/ 
int32_t CAdpcm::MSAdpcmCompressSample(uint32_t ch,uint32_t chans,int16_t v[2],const int16_t iCoef[2],const int16_t *ibuff,int32_t n,int32_t *iostep,unsigned char *obuff)
{
    const signed short *ip, *itop;
    unsigned char *op;
    int32_t ox = 0;
    int32_t i, d, v0, v1, step;
    double d2;					/* long long is okay also, speed abt the same */

    ip = ibuff + ch;			//point ip to 1st input sample for this channel
    itop = ibuff + n*chans;
    v0 = v[0];
    v1 = v[1];
    d = *ip - v1; ip += chans;	//1st input sample for this channel
    d2 = d*d;					//d2 will be sum of squares of errors, given input v0 and *st
    d = *ip - v0; ip += chans;	//2nd input sample for this channel
    d2 += d*d;

    step = *iostep;

    op = obuff;
    if (op) 
	{							/* NULL means don't output, just compute the rms error */
        op += chans;	        /* skip bpred indices */
        op += 2 * ch;			/* channel's stepsize */
        op[0] = (unsigned char)(((int16_t)step) & 0xFF); 
		op[1] = (unsigned char)((((int16_t)step)>>8) & 0xFF);
        op += 2*chans;			/* skip to v0 */
        op[0] = (unsigned char)(((int16_t)v0) & 0xFF);
		op[1] = (unsigned char)((((int16_t)v0)>>8) & 0xFF);
        op += 2*chans;			/* skip to v1 */
        op[0] = (unsigned char)((int16_t)v1 & 0xFF); 
		op[1] = (unsigned char)((((int16_t)v1)>>8) & 0xFF);
        op = obuff+7*chans;		/* point to base of output nibbles */
        ox = 4*ch;
    }
    for (i = 0; ip < itop; ip+=chans) 
	{
        int32_t vlin,d,dp,c;

	    /* make linear prediction for next sample */
        vlin = (v0 * iCoef[0] + v1 * iCoef[1]) / 256;
        d = *ip - vlin;			/* difference between linear prediction and current sample */
        dp = d + (step*8) + (step/2);
        c = 0;
        if (dp > 0) 
		{
            c = dp/step;
            if (c > 15) 
				c = 15;
        }
        c -= 8;
        dp = c * step;   /* quantized estimate of samp - vlin */
        c &= 0x0f;       /* mask to 4 bits */

        v1 = v0;		/* shift history */
        v0 = vlin + dp;
        if (v0<-0x8000) 
			v0 = -0x8000;
        else if (v0>0x7fff) 
			v0 = 0x7fff;

        d = *ip - v0;
        d2 += d*d;		/* update square-error */

        if (op) 
		{	//if we want output, put it in proper place
            op[ox/8] |= (ox&4) ? c : (c<<4);
            ox += 4*chans;
        }
        // Update the step for the next sample
        step = (m_nMSStepAdjustTable[c] * step) / 256;
        if (step < 16) 
			step = 16;
    }
    d2 /= n; /* be sure it's non-negative */
    //TRACEIT2("ch%d: st %d->%d, d %.1f\n", ch, *iostep, step, sqrt(d2));
    *iostep = step;
    return (int32_t)sqrt(d2);
}

/*\ 
 * <---------- CAdpcm::RMFAdpcmDecompress ----------> 
 * @m decompress an entire ADPCM sample stream
 * --> I N <-- @p
 * unsigned char *pcDest - output buffer
 * unsigned char *pcSource - input sample
 * uint32_tnSize - size of the input sample in bytes
 * int nChannels - number of channels to decode
\*/ 
uint32_t CAdpcm::RMFAdpcmDecompress(unsigned char *pcDest,unsigned char *pcSource,uint32_t nSize,uint32_t nChannels)
{
    unsigned char *inp;				//Input buffer pointer
    int16_t *outp;			//output buffer pointer 
	int16_t *startout;		//output buffer pointer
	uint32_t nSamplesToDecode;
    int32_t delta;			//Current adpcm output value
    int32_t step;			//Stepsize
    int32_t valpred=0;		//Predicted value
    int32_t vpdiff;			//Current change to valpred
    int32_t index;			//Current step change index
    uint32_t inputbuffer = 0;		//place to keep next 4-bit value
	uint32_t resultsize=0;
    uint32_t bufferstep=0;	

	Log2(verbLevDebug1,"decompressing apple IMA adpcm....\n");
	uint32_t iChn;

	nSize=nSize&0xFFFFFFFE;
	memcpy(&startout,pcDest,2);

	for (iChn=0;iChn < nChannels;iChn++)
	{
		int nPacketLeft;
		
		outp = ((short *)pcDest)+iChn;

		nSamplesToDecode=(nSize/nChannels)*2;
		inp = pcSource+((nSamplesToDecode*iChn)/2);

		while (nSamplesToDecode)
		{
			index = inp[1]&0x7F;		 
			if ( index > 88 ) 
				index=88;			
			inp+=2;										//use index but skip predicator estimate, its useless anyways
			nSamplesToDecode-=4;
			step = CAdpcm::pnGetIMAStepSizeTable()[index];		

			nPacketLeft=min(0x40,nSamplesToDecode);
			while(nPacketLeft)
			{	
				memcpy(&inputbuffer,inp,4);
				inp+=4;
				bufferstep = 8;
				//decode 8 nibbles in a row (one dword)
				while(bufferstep--)
				{
					//mask lowest nibble
					delta = inputbuffer & 0x0f;

					vpdiff = step >> 3;
					if (delta & 1)	vpdiff += step>>2;
					if (delta & 2)	vpdiff += step>>1;
					if (delta & 4)	vpdiff += step;
					
					if (delta & 8)
						valpred -= vpdiff;
					else
						valpred += vpdiff;

					if (valpred > 32767)
						valpred = 32767;
					else if ( valpred < -32768 )
						valpred = -32768;

					index += m_nIMAIndexTable[delta&0x0F];

					//make sure the index stays in range
					if (index < 0) 
						index=0;
					if ( index > 88 ) 
						index=88;

					//get the step size to recompute the last sample for delta calculation
					step = CAdpcm::pnGetIMAStepSizeTable()[index];					

					//shift more to next sample within the input dword
					inputbuffer>>=4;

					//store output
					memcpy(outp,&valpred,2);	//seems overblown but is needed for e.g. SPARC based hosts
					outp+=nChannels;

					nSamplesToDecode--;
					--nPacketLeft;
					if (!nSamplesToDecode)
						break;
				};
			};
		};
		resultsize=(uint32_t)(outp-startout);
	}
	if (resultsize > 2)
		resultsize-=2;
	return resultsize;
}

/*\ 
 * <---------- CAdpcm::RMFAdpcmCompress ----------> 
 * @m 
 * --> I N <-- @p
 * unsigned char *pcDest - 
 * unsigned char *pcSource - 
 * uint32_tnSize - 
 * int nChannels - 
 * uint32_tnDestSize - 
\*/ 
void CAdpcm::RMFAdpcmCompress(unsigned char *pcDest,unsigned char *pcSource,uint32_t nSize,uint32_t nChannels,uint32_t nDestSize)
{
	long nSamplesToEncode;
    int16_t *inp;			//Input buffer pointer
	int16_t *startin;		//Input buffer pointer
    unsigned char *outp;			//Output buffer pointer
	unsigned char *startout;
    int32_t val;			/* Current input sample value */
    int32_t sign;			/* Current adpcm sign bit */
    int32_t delta;			/* Current adpcm output value */
    int32_t diff;			/* Difference between val and valprev */
    int32_t step;			/* Stepsize */
    int32_t valpred;		/* Predicted output value */
    int32_t vpdiff;			/* Current change to valpred */
    int32_t index=0;		/* Current step change index */
    int32_t outputbuffer = 0;	/* place to keep previous 4-bit value */
    int32_t bufferstep;		/* toggle between outputbuffer/output */
	uint32_t iChn;

    outp = pcDest;

	nSize=nSize&0xFFFFFFFE;

	for (iChn=0;iChn < nChannels;iChn++)
	{
		int nPacketLeft;

		startout = outp = pcDest;
		startin = inp = (int16_t *)pcSource;

		nSamplesToEncode=(nSize*nChannels)/2;

	    valpred=0x0000;
		index=0;

		while (nSamplesToEncode > 0)
		{
			nPacketLeft=min(0x40,nSamplesToEncode);
		    bufferstep = 1;
			outp[0]=(unsigned char)((valpred>>8)&0xFF);
			outp[1]=(unsigned char)(valpred & 0x80) | (unsigned char)(index & 0x7F);
			outp+=2;
			while(nPacketLeft)
			{	
				step = CAdpcm::pnGetIMAStepSizeTable()[index];		
				
				//get sample
				val = *inp++;

				/* Step 1 - compute difference with previous value */
				diff = val - valpred;
				sign = (diff < 0) ? 8 : 0;
				if ( sign ) 
					diff = (-diff);

				/* Step 2 - Divide and clamp */
				/* Note:
				** This code *approximately* computes:
				**    delta = diff*4/step;
				**    vpdiff = (delta+0.5)*step/4;
				** but in shift step bits are dropped. The net result of this is
				** that even if you have fast mul/div hardware you cannot put it to
				** good use since the fixup would be too expensive.
				*/
				delta = 0;
				vpdiff = step / 8;
				
				if ( diff >= step ) 
				{
					delta = 4;
					diff -= step;
					vpdiff += step;
				}
				step >>= 1;
				if ( diff >= step  ) 
				{
					delta |= 2;
					diff -= step;
					vpdiff += step;
				}
				step = step/2;
				if ( diff >= step ) 
				{
					delta |= 1;
					vpdiff += step;
				}

				/* Step 3 - Update previous value */
				if ( sign )
					valpred -= vpdiff;
				else
					valpred += vpdiff;

				/* Step 4 - Clamp previous value to 16 bits */
				if ( valpred > 32767 )
					valpred = 32767;
				else if ( valpred < -32768 )
					valpred = -32768;

				/* Step 5 - Assemble value, update index and step values */
				delta |= sign;
				
				index += m_nIMAIndexTable[delta];
				if ( index < 0 ) 
					index = 0;
				if ( index > 88 ) 
					index = 88;
				step = CAdpcm::pnGetIMAStepSizeTable()[index];

				/* Step 6 - Output value */
				if ( bufferstep ) 
					outputbuffer = delta & 0x0f;
				else 
					*outp++ = ((delta << 4) & 0xf0) | outputbuffer;
				bufferstep = !bufferstep;
				nSamplesToEncode--;
				--nPacketLeft;
				if (!nSamplesToEncode)
					break;
			}
			if (!bufferstep)
				*outp++ = outputbuffer;
		}
	}
	TRACEIT2("input size: %d\n",(inp-startin)*2);
	TRACEIT2("output size: %d\n",outp-startout);
	TRACEIT2("dest size : %d\n",nDestSize);
}

/*\
 * <---------- *CAdpcm :: pwIMAAdpcmDecompress ----------> 
 * @m decode an IMA ADPCM sample
 * --> I N <-- @p
 * signed short *pwOut            - pointer to output buffer
 * unsigned char *pIn             - 
 * uint32_tnSize            - 
 * uint32_t*pnOutSize       - 
 * uint32_tnBlockAlign      - 
 * uint32_tnSamplesPerBlock - 
 * int nChannels                  - number of channels
 * <-- OUT --> @r
 * signed short * - pointer to output buffer
\*/
int16_t *CAdpcm::pwIMAAdpcmDecompress(int16_t *pwOut,unsigned char *pIn,uint32_t nSize,uint32_t *pnOutSize,uint32_t nBlockAlign,uint32_t nSamplesPerBlock,uint32_t nChannels)
{
	int16_t *p=pwOut;	
	*pnOutSize=0;

	Log2(verbLevDebug1,"decompressing IMA ADPCM data...\n");
	while(nSize)
	{
		uint32_t nLen;
		if (nSize < nBlockAlign)
		{
			Log2(verbLevWarnings,"short adpcm block (samples %d instead of %d)\n",(nSize-(4*nChannels))*2,nSamplesPerBlock);
			nSamplesPerBlock=(nSize-(4*nChannels))*2;
			nSize=nBlockAlign;
		}
		//decompress block
		IMAAdpcmDecompressBlock(nChannels,pIn,p,nSamplesPerBlock);
		
		*pnOutSize+=nChannels*nSamplesPerBlock*sizeof(int16_t);
		p+=nChannels*nSamplesPerBlock;

		nLen=nBlockAlign;
		pIn+=nLen;
		nSize-=nLen;
	};
	return pwOut;
}

/*\
 * <---------- *CAdpcm :: pcIMAAdpcmCompress ----------> 
 * @m encode an IMA ADPCM sample
 * --> I N <-- @p
 * signed short *pIn			  - pointer to plain PCM data
 * uint32_tnSize			  - input size in bytes
 * uint32_t*pnOutSize		  - pointer to output size in bytes
 * uint32_tnBlockAlign      - block alignment
 * uint32_tnSamplesPerBlock - number of sample values per block
 * int nChannels                  - number of channels
 * <-- OUT --> @r
 * unsigned char *- pointer to output buffer
\*/
unsigned char *CAdpcm::pcIMAAdpcmCompress(int16_t *pIn,uint32_t nSize,uint32_t *pnOutSize,uint32_t nBlockAlign,uint32_t nSamplesPerBlock,uint32_t nChannels)
{
	uint32_t nTotalAdpcmSize;
	unsigned char *pcOut=NULL;		//
	int16_t *pcExtra=NULL;			//
	unsigned char *pc;				//
	int32_t iStateIndex[2]={0,0};	//channel state storage
	
	nTotalAdpcmSize=0;				//
	*pnOutSize=0;					//

	pcOut=(unsigned char *)CMobileSampleContent::Alloc(nSize/2);
	pc=pcOut;
	while(nSize >= (uint32_t)(nChannels*nSamplesPerBlock)*2)					//still enough source samples for a block available?
	{																				//yes->loop
		IMAAdpcmCompressBlock(	nChannels,											//compress a complete block
								pIn,												
								nSamplesPerBlock,
								&iStateIndex[0],
								pc	);
		*pnOutSize+=nBlockAlign;													//
		pc+=nBlockAlign;															//
		pIn+=nSamplesPerBlock*nChannels;											//
		nSize-=(nChannels*nSamplesPerBlock)*2;										//
	};
	TRACEIT2("done with aligned size - now for the possible rest...\n");
	//still unencoded samples left? (total sample size not devideable by blocksize)
	if (nSize)
	{	//yes->...
		//make sure we have a complete block of source samples available...
		pcExtra=new int16_t[nChannels*nSamplesPerBlock];						//alloc extra block
		ZeroMemory(pcExtra,nChannels*nSamplesPerBlock*2);										//empty total block
		CopyMemory(pcExtra,pIn,nSize);												//copy remaining, unencoded samples into the block
		IMAAdpcmCompressBlock(	nChannels,											//compress block
								pcExtra,
								nSamplesPerBlock,
								&iStateIndex[0],
								pc	);												
		*pnOutSize+=nBlockAlign;													//raise output size
		delete [] pcExtra;															//free the extra block data
	}
	return pcOut;
}
