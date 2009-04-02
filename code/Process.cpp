/*\
 * Process.cpp
 * Copyright (C) 2004-2009, MMSGURU - written by Till Toenshoff
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
#include "../include/Resource.h"
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/CommonAccess.h"
#include "../retroBase/Modules.h"
#include "../retroBase/Endian.h" 
#include "code.h"
#include "Filter.h"
#include "ButterWorth.h"
#include "PeakLimiter.h"
#include "ChannelConverter.h"
#include "RateConverter.h"
#include "Normalizer.h"
#include "ExtNormalizer.h"
#include "EightToSixteenBit.h"
#include "Crop.h"
#include "Loop.h"
#include "Fade.h"
#include "Ringback.h"

void ConvertTo16Bit(CMobileSampleContent *pSource)
{
	signed short int *pDest=NULL;
	uint32_t nDestSize=0;
	uint32_t nSourceSamples=pSource->m_nCSSize/((pSource->m_nCSBitsPerSample>>3)*pSource->m_nCSChannels);

	CEightToSixteenBit filter;
	
	filter.Init(pSource->m_nCSChannels);
	
	nDestSize=filter.nGetDestSize(pSource->m_nCSSize);
	TRACEIT2("buffer size: %d\n",nDestSize);
	ASSERT(nDestSize);
	pDest=(signed short *)pSource->Alloc(nDestSize+2);
	
	filter.Process((short *)pSource->m_pcCSBuffer,pDest,nSourceSamples);
	
	pSource->Reset(pDest,nDestSize);
	pSource->m_nCSBitsPerSample=16;
}

void Normalize(CMobileSampleContent *pSource)
{
	CNormalizer filter;
	Log2(verbLevMessages,IDS_PRG_NORMALIZING);
	ASSERT(pSource->m_nCSBitsPerSample);
	filter.Process((short *)pSource->m_pcCSBuffer,(short *)pSource->m_pcCSBuffer,pSource->m_nCSSize/(pSource->m_nCSBitsPerSample>>3));
}

void RmsNormalize(CMobileSampleContent *pSource,converting_parameters *parm)
{
	CExtNormalizer filter;
	Log2(verbLevMessages,"rms normalizer to %ddB ...\n",parm->m_fParameter[paraFloatVolRmsNorm]);
	filter.Init(parm->m_fParameter[paraFloatVolRmsNorm]);
	filter.Process((short *)pSource->m_pcCSBuffer,(short *)pSource->m_pcCSBuffer,pSource->m_nCSSize/(pSource->m_nCSBitsPerSample>>3));
}

void ConvertChannels(CMobileSampleContent *pSource,converting_parameters *parm)
{
	if (parm->m_nParameter[paraNumChannels] <= 2)
	{
		if (pSource->m_nCSChannels == parm->m_nParameter[paraNumChannels])
		{
			Log2(verbLevMessages,IDS_PRG_NOCHANCHG);
		}
		else
		{
			signed short int *pDest=NULL;
			uint32_t nDestSize=0;
			CChannelConverter filter;
			Log2(verbLevMessages,IDS_PRG_CHANCHG,parm->m_nParameter[paraNumChannels]);
			filter.Init(pSource->m_nCSChannels,parm->m_nParameter[paraNumChannels]);
			nDestSize=filter.nGetDestSize(pSource->m_nCSSize);
			TRACEIT2("buffer size: %d\n",nDestSize);
			ASSERT(nDestSize);
			pDest=(signed short *)pSource->Alloc(nDestSize+2);
			filter.Process((short *)pSource->m_pcCSBuffer,pDest,pSource->m_nCSSize/((pSource->m_nCSBitsPerSample>>3)*pSource->m_nCSChannels));
			pSource->Reset(pDest,nDestSize);
			pSource->m_nCSChannels=parm->m_nParameter[paraNumChannels];
		}
	}
	else
	{
		Log2(verbLevWarnings,IDS_PRG_CHANCNTILL);
	}
}

void ConvertSampleRate(CMobileSampleContent *pSource,converting_parameters *parm)
{
	if (parm->m_nParameter[paraNumSamplerate] < 65535)
	{
		if (pSource->m_nCSSamplesPerSecond == parm->m_nParameter[paraNumSamplerate])
		{
			Log2(verbLevMessages,IDS_PRG_NORATECHG);
		}
		else
		{
			CRateConverter filter;
			signed short int *pDest=NULL;
			uint32_t nDestSize;
			filter.Init(pSource->m_nCSChannels,pSource->m_nCSSamplesPerSecond,parm->m_nParameter[paraNumSamplerate]);
			nDestSize=filter.nGetDestSize(pSource->m_nCSSize);
			Log2(verbLevMessages,"adjusting sample rate towards %dHz (filesize %d to %d)...\n\n",parm->m_nParameter[paraNumSamplerate],pSource->m_nCSSize,nDestSize);
			TRACEIT2("buffer size: %d\n",nDestSize);
			ASSERT(nDestSize);
			pDest=(signed short *)pSource->Alloc(nDestSize+4);
			ZeroMemory(pDest,nDestSize);
			filter.Process((short *)pSource->m_pcCSBuffer,pDest,pSource->m_nCSSize / (pSource->m_nCSBitsPerSample>>3));
			pSource->Reset(pDest,nDestSize);
			pSource->m_nCSSamplesPerSecond=parm->m_nParameter[paraNumSamplerate];
		}
	}
	else
	{
		Log2(verbLevWarnings,IDS_PRG_RATEEXC);
	}
}

void GainControl(CMobileSampleContent *pSource,converting_parameters *parm)
{
	if (parm->m_fParameter[paraFloatVolGain] > 1.0)
	{
		if (parm->m_fParameter[paraFloatVolLimitGain] < 1.0)
		{
			int nMode=parm->m_fParameter[paraFloatVolLimitGain] > 0.0 ? CPeaklimit::peakLimit : CPeaklimit::peakVolume;
			const char *sModeName[]={"unknown","amplifier","peak limiter"};
			CPeaklimit filter;
			Log2(verbLevMessages,IDS_PRG_GAINCONTROL,sModeName[nMode+1]);
			filter.Init(nMode,parm->m_fParameter[paraFloatVolGain],parm->m_fParameter[paraFloatVolLimitGain]);
			filter.Process((short *)pSource->m_pcCSBuffer,(short *)pSource->m_pcCSBuffer,pSource->m_nCSSize/(pSource->m_nCSBitsPerSample>>3));
		}
		else
		{
			Log2(verbLevWarnings,IDS_PRG_LIMITGAINEXC);
		}
	}
	else
	{
		Log2(verbLevWarnings,IDS_PRG_OUTPUTGAINEXC);
	}
}

void Crop(CMobileSampleContent *pSource,converting_parameters *parm)
{
	bool bCrop=false;
	//MP3 auto prepends and appends zero bytes to our precious sample, should get autocrop enabled to compensate
	if (parm->m_bSwitch[paraSwitchAutoCrop])
	{
		Log2(verbLevMessages,IDS_PRG_LOOPADJUST);
		bCrop=true;
	}
	else
	{
		if (parm->m_nParameter[paraNumSmpPlaytime] != 0)
		{
			if (pSource->nGetSamplePlaytime() <= (unsigned long)parm->m_nParameter[paraNumSmpPlaytime])
			{
				Log2(verbLevMessages,IDS_PRG_NOTRIM);
			}
			else
				bCrop=true;

			if ((signed long)parm->m_nParameter[paraNumSmpPlaytime] < 0)
			{
				if (pSource->nGetSamplePlaytime() > (unsigned long)(-(signed long)(parm->m_nParameter[paraNumSmpPlaytime])+parm->m_nParameter[paraNumSmpOffset]))
				{
					parm->m_nParameter[paraNumSmpPlaytime]=(pSource->nGetSamplePlaytime()+(signed long)parm->m_nParameter[paraNumSmpPlaytime])-parm->m_nParameter[paraNumSmpOffset];
					bCrop=true;
				}
				else
				{
					Log2(verbLevMessages,"\n");
				}
			}
		}
		if (parm->m_nParameter[paraNumSmpOffset] > 0)
		{
			if ((unsigned long)parm->m_nParameter[paraNumSmpOffset] >= pSource->nGetSamplePlaytime())
			{
				Log2(verbLevMessages,IDS_PRG_NOOFFSET);
			}
			else
				bCrop=true;
		}
		if (bCrop)
		{
			Log2(verbLevMessages,IDS_PRG_TRIM,parm->m_nParameter[paraNumSmpOffset],parm->m_nParameter[paraNumSmpPlaytime]);
		}
	}
	if (bCrop)
	{
		signed short int *pDest=NULL;
		uint32_t nDestSize=0;
		uint32_t nSourceSamples=pSource->m_nCSSize/((pSource->m_nCSBitsPerSample>>3)*pSource->m_nCSChannels);
		CCrop filter;

		Log2(verbLevMessages,"really cropping...\n");

		filter.Init(pSource->m_nCSSamplesPerSecond,
					pSource->m_nCSChannels,
					pSource->m_nCSBitsPerSample,
					parm->m_nParameter[paraNumSmpOffset],
					parm->m_nParameter[paraNumSmpPlaytime],
					parm->m_bSwitch[paraSwitchAutoCrop],
					pSource->m_pcCSBuffer,
					nSourceSamples);
		nDestSize=filter.nGetDestSize(pSource->m_nCSSize);
		Log2(verbLevMessages,"destination size: %d bytes\n",nDestSize);
		TRACEIT2("buffer size: %d\n",nDestSize);
		ASSERT(nDestSize);
		pDest=(signed short *)pSource->Alloc(nDestSize+2);
		filter.Process((short *)pSource->m_pcCSBuffer,pDest,nSourceSamples);
		pSource->Reset(pDest,nDestSize);
	}
}

void FadeIn(CMobileSampleContent *pSource,converting_parameters *parm)
{
	CFade filter;
	filter.Init(pSource->m_nCSSamplesPerSecond,
				pSource->m_nCSBitsPerSample,
				pSource->m_nCSChannels,
				parm->m_nParameter[paraNumSmpFadeIn],
				CFade::directionIn);
	Log2(verbLevMessages,IDS_PRG_FADEIN,parm->m_nParameter[paraNumSmpFadeIn]);
	filter.Process((short *)pSource->m_pcCSBuffer,(short *)pSource->m_pcCSBuffer,pSource->m_nCSSize/(pSource->m_nCSChannels*(pSource->m_nCSBitsPerSample>>3)));
}

void FadeOut(CMobileSampleContent *pSource,converting_parameters *parm)
{
	CFade filter;
	filter.Init(pSource->m_nCSSamplesPerSecond,
				pSource->m_nCSBitsPerSample,
				pSource->m_nCSChannels,
				parm->m_nParameter[paraNumSmpFadeOut],
				CFade::directionOut);
	Log2(verbLevMessages,IDS_PRG_FADEOUT,parm->m_nParameter[paraNumSmpFadeOut]);
	filter.Process((short *)pSource->m_pcCSBuffer,(short *)pSource->m_pcCSBuffer,pSource->m_nCSSize/(pSource->m_nCSChannels*(pSource->m_nCSBitsPerSample>>3)));
}

void AutoAdapt(CMobileSampleContent *pDest,CMobileSampleContent *pSource,converting_parameters *parm)
{
	//only go through this if the source sample is incompatible in the first place
	if (!pDest->m_encodingCaps.bIsCompatible(pSource->m_nCSBitsPerSample,pSource->m_nCSChannels,pSource->m_nCSSamplesPerSecond))
	{
		Log2(verbLevMessages,"auto adapt sample towards destination format limitations\n");
		parm->m_bSwitch[paraSwitchNormalize]=true;
		parm->m_nParameter[paraNumChannels]=pSource->m_nCSChannels;
		parm->m_nParameter[paraNumSamplerate]=pSource->m_nCSSamplesPerSecond;
		pDest->m_encodingCaps.determineFittestFormat((uint32_t *)&parm->m_nParameter[paraNumChannels],(uint32_t *)&parm->m_nParameter[paraNumSamplerate]);
		Log2(verbLevDebug3,"best fitting format is: %d channels, %d hz\n",parm->m_nParameter[paraNumChannels],parm->m_nParameter[paraNumSamplerate]);
		if (parm->m_nParameter[paraNumSamplerate] < pSource->m_nCSSamplesPerSecond)
			parm->m_nParameter[paraNumLowpassFreq] = parm->m_nParameter[paraNumSamplerate] / 2;
	}
}

void ButterworthFilter(CMobileSampleContent *pSource,converting_parameters *parm)
{
	int nMode=0;
	CButterWorth filter;
	unsigned int nFrequency,nWidth;
	const char *sModeName[]={	"unknown","lowpass","highpass","bandpass"};
	nWidth=0;
	if (parm->m_nParameter[paraNumLowpassFreq] != 0)
	{
		nMode=1+CButterWorth::butterLowpass;
		nFrequency=parm->m_nParameter[paraNumLowpassFreq];
	}
	if (parm->m_nParameter[paraNumHighpassFreq] != 0)
	{
		nMode=1+CButterWorth::butterHighpass;
		nFrequency=parm->m_nParameter[paraNumHighpassFreq];
	}
	if (parm->m_nParameter[paraNumBandpassFreq] != 0)
	{
		nMode=1+CButterWorth::butterBandpass;
		nFrequency=parm->m_nParameter[paraNumBandpassFreq];
		nWidth=parm->m_nParameter[paraNumBandpassWidth];
	}
	Log2(verbLevMessages,IDS_PRG_FILTER,sModeName[nMode],nFrequency);
	filter.Init(nMode-1,nFrequency,pSource->m_nCSSamplesPerSecond,nWidth);
	filter.Process((short *)pSource->m_pcCSBuffer,(short *)pSource->m_pcCSBuffer,pSource->m_nCSSize/(pSource->m_nCSBitsPerSample>>3));
}

void Loop(CMobileSampleContent *pSource,converting_parameters *parm)
{
	CLoop filter;
	signed short int *pDest=NULL;
	uint32_t nDestSize;
	filter.Init(pSource->m_nCSSamplesPerSecond,
				pSource->m_nCSChannels,
				pSource->m_nCSBitsPerSample,
				parm->m_nParameter[paraNumSmpLooptime],
				(short *)pSource->m_pcCSBuffer,
				pSource->m_nCSSize/(pSource->m_nCSChannels*(pSource->m_nCSBitsPerSample>>3)));
	nDestSize=filter.nGetDestSize(pSource->m_nCSSize);
	Log2(verbLevMessages,IDS_PRG_LOOPFIT,parm->m_nParameter[paraNumSmpLooptime]);
	TRACEIT2("buffer size: %d\n",nDestSize);
	ASSERT(nDestSize);
	pDest=(signed short *)pSource->Alloc(nDestSize+1);
	ZeroMemory(pDest,nDestSize);
	filter.Process((short *)pSource->m_pcCSBuffer,pDest,pSource->m_nCSSize/(pSource->m_nCSChannels*(pSource->m_nCSBitsPerSample>>3)));
	pSource->Reset(pDest,nDestSize);
}

void RingbackMixer(CMobileSampleContent *pSource,converting_parameters *parm)
{
	CRingback rb;
	rb.Init(pSource->m_nCSSamplesPerSecond,pSource->m_nCSBitsPerSample,pSource->m_nCSChannels,parm->m_strParameter[paraStrRingbackXml].c_str(),parm->m_strParameter[paraStrRingbackCountry].c_str());
	Log2(verbLevMessages,IDS_PRG_RINGBACK);
	rb.Process((short *)pSource->m_pcCSBuffer,(short *)pSource->m_pcCSBuffer,pSource->m_nCSSize/(pSource->m_nCSBitsPerSample>>3));

	if (parm->m_fParameter[paraFloatVolRmsNorm] == 0.0 && parm->m_bSwitch[paraSwitchNormalize])
		Normalize(pSource);
}
