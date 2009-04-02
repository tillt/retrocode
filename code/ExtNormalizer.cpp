/*\
 * ExtNormalizer.cpp
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
#ifndef WIN32
//#include <ieeefp.h>
#endif
#include <float.h>
#include "../retroBase/Basics.h"
#include "../retroBase/Integer.h"
#include "Filter.h"
#include "PeakLimiter.h"
#include "ExtNormalizer.h"

#ifndef WIN32
#define _finite finite
#endif

CExtNormalizer::CExtNormalizer()
{
	m_fRMSNormDb=-0.0;
	m_fRMSThreshDb=-96.0;
}

CExtNormalizer::~CExtNormalizer()
{
}

/*\
 * <---------- Init ---------->
 * @m setup
 * --> I N <-- @p
 * double fVal - target RMS level
\*/
void CExtNormalizer::Init(double fVal)
{
	m_fRMSNormDb=fVal;
}

/*\
 * <---------- bUpdateStats ---------->
 * @m calc peak and rms level
 * --> I N <-- @p
 * signed short int *ibuf - input data
 * uint32_tnSampleCount - number of samples
 * <-- OUT --> @r
 * bool - true=ok
\*/
bool CExtNormalizer::bUpdateStats(signed short int *ibuf,uint32_t nSampleCount)
{
	uint32_t nToDo;
	signed short int nIn;
	uint32_t nThreshAbs;
	uint32_t nAbsolute;
	uint32_t nCounted;
	double fSum,fRes;
	fSum=0.0;
	m_nPeak=0;
	m_nPeakOffset=0;

	nThreshAbs=ndBToAmp(m_fRMSThreshDb);
	TRACEIT2("threshhold %f, abs: %04Xh\n",m_fRMSThreshDb,nThreshAbs);
	
	if (nSampleCount)
	{
		nToDo=nSampleCount;
		nCounted=0;
		/*\
		 *
		 * Root Mean Square level function taken from 
		 * http://en.wikipedia.org/wiki/Root_mean_square
		 *
		 *              +-------------------------------'
		 *          -|  | X(1)^2 + X(2)^2 + --- + X(n)^2
		 * Xrms(n) = |/   ------------------------------
		 *           |                 n
		 *
		 * 0x7FFFFFFFFFFFFFFF = largest possible signed 64 bit number (how about doubles?)
		 *
		 * todo: Make sure this works on large samples
		 * Well, it does work with a sample length of theoretical 0x100020003 (4.295.098.371) samples on the upper part.
		 * On the lower part, n cannot exceed 0xFFFFFFFF (4.294.967.295) samples.
		 *
		\*/
		while (nToDo--)
		{
			nIn=(*ibuf++);
			if (nIn < 0)							//get absolute value of this sample
				nAbsolute=0-nIn;
			else
				nAbsolute=nIn;
			if (nAbsolute > nThreshAbs)
			{
				fSum+=(signed long int)nIn*(signed long int)nIn;
				++nCounted;
			}
			if (nAbsolute > m_nPeak)				//sample value higher than current max?
			{
				TRACEIT2("peak %04X (%1.2fdB) at %08d\n",nAbsolute,fAmpTodB(nAbsolute),nSampleCount-nToDo);
				m_nPeak=nAbsolute;					//remember this peak as the new max
				m_nPeakOffset=nSampleCount-nToDo;
			}
		};
		if (nCounted)
			fRes=sqrt(fSum/nSampleCount);
		else
			fRes=0.0;
	}
	m_fRMSLevelDb=fAmpTodB((const uint32_t)fRes);
	if (_finite(m_fRMSLevelDb) == 0)
		m_fRMSLevelDb = -96.0;
	return true;
}

/*\
 * <---------- Process ---------->
 * @m calc rms, gain adjust, recalc rms
 * --> I N <-- @p
 * signed short int *ibuf - input data
 * signed short int *obuf - output buffer
 * uint32_tnSampleCount - sample count
\*/
void CExtNormalizer::Process(signed short int *ibuf,signed short int *obuf, uint32_t nSampleCount)
{
	//uint32_tint nToDo=nSampleCount;
	signed short int *pcInput=ibuf;

	bUpdateStats(pcInput,nSampleCount);
	Log2(verbLevMessages,"old Peak: %0.2fdb\n",fAmpTodB(m_nPeak));
	Log2(verbLevMessages,"old RMS: %0.2fdb (threshold: %0.2fdb)\n",m_fRMSLevelDb,m_fRMSThreshDb);

	double dGain=(double)ndBToAmp(m_fRMSNormDb)/ndBToAmp(m_fRMSLevelDb);
	Log2(verbLevDebug3,"Gain Proposal: %0.2f\n",dGain);
	
	//CPeaklimit::Init(peakLimit,dGain,0.5);		
	CPeaklimit::Init(peakVolume,(float)dGain,0.5);	//initialize parent object
	m_dGain=(float)dGain;							//patch gain value to fit RMS normalizing result
	CPeaklimit::Process(ibuf,obuf,nSampleCount);	//run gain adjust
	
	bUpdateStats(pcInput,nSampleCount);
	Log2(verbLevMessages,"new Peak: %0.2fdb\n",fAmpTodB(m_nPeak));
	Log2(verbLevMessages,"new RMS: %0.2fdb (threshold: %0.2fdb)\n",m_fRMSLevelDb,m_fRMSThreshDb);
} 
