/*\
 * AVIProperty.cpp
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
#include "../retroBase/Basics.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/Adpcm.h"
#include "WaveFile.h"
#include "AVIFile.h"
#include "AVIProperty.h"

CAVIProperty::CAVIProperty(void) 
{
	TRACEIT2("constructor");
}

CAVIProperty::~CAVIProperty(void)
{
	TRACEIT2("destructor");
}

void CAVIProperty::InitFromContent(LPCTSTR pszPath,unsigned int nSize,CMobileContent *pm)
{
	CMobileProperty::InitFromContent(pszPath,nSize,pm);
	CAVIFile *pAVI=(CAVIFile *)pm;
	ASSERT(pAVI);
	if (pAVI)
	{
		unsigned int nTag;
		char buffer[255];
		CAVIStream *pVideoStream;
		CAVIStream *pAudioStream;
		pVideoStream=pAVI->pGetStream(pAVI->iGetFirstVideoStream());
		pAudioStream=pAVI->pGetStream(pAVI->iGetFirstAudioStream());
		uint32_t nFormat;
		tstring strFormat;
		
		nFormat=0;
		if (pVideoStream)
		{
			nTag=pVideoStream->nGetCodecTag();
			nFormat|=(1+pVideoStream->nGetCodecId())<< 16;
			strFormat+=pVideoStream->sGetCodecName(pVideoStream->nGetCodecId());
			if (((nTag>>24)&0xFF) >= ' ')
			{
				sprintf(buffer," : %c%c%c%c",nTag&0xFF,(nTag>>8)&0xFF,(nTag>>16)&0xFF,(nTag>>24)&0xFF);
				strFormat+=buffer;
			}
		}
		if (pAudioStream)
		{
			nFormat|=pAudioStream->pGetWaveHeader()->wFormatTag;
			if (!strFormat.empty())
				strFormat+=", ";
			strFormat+=CWaveFile::sGetFormatName(pAudioStream->pGetWaveHeader()->wFormatTag);
		}
		_itoa(nFormat,buffer,10);
		SetProperty_long(prstrSubFormat,strFormat.c_str(),"id",buffer);

		if (pVideoStream)
		{
			SetProperty(prnumFrameWidth,pVideoStream->nGetWidth());
			SetProperty(prnumFrameHeight,pVideoStream->nGetHeight());
			SetProperty(prnumFrameBitPerPixel,pVideoStream->nGetBitPerPixel());
			SetProperty(prnumFrameRate,pVideoStream->nGetFrameRate());

			uint32_t nVideoBitRate=pAVI->nGetBitRate();
			if (nVideoBitRate == 0)
				nVideoBitRate=(uint32_t)(pAVI->llGetTotalStreamDataSize()*8 / pVideoStream->nGetFrameCount()) * pVideoStream->nGetFrameRate();
			if (nVideoBitRate > 0)
			{
				if (pAudioStream)
					nVideoBitRate-=pAudioStream->pGetWaveHeader()->nAvgBytesPerSec*8;
			}
			SetProperty(prnumFrameBitRate,nVideoBitRate);
		}
		if (pAudioStream)
		{
			SetProperty(prnumChannels,(uint32_t)pAudioStream->pGetWaveHeader()->wChannels);
			SetProperty(prnumSampleRate,pAudioStream->pGetWaveHeader()->nSamplesPerSec);
			if ((int)pAudioStream->pGetWaveHeader()->wBitsPerSample)
				SetProperty(prnumBitsPerSample,(uint32_t)pAudioStream->pGetWaveHeader()->wBitsPerSample);
			SetProperty(prnumBitRate,pAudioStream->pGetWaveHeader()->nAvgBytesPerSec*8);
		}
		SetProperty(prnumPlaylength,(uint32_t)pVideoStream->llGetPlaytime());
		for (int i=0;i < CMobileSampleContent::infoLast;i++)
		{
			if (!pAVI->m_strInfo[i].empty())
				setProperty(strGetInfoToPropertyStringIndex(i).c_str(),pAVI->m_strInfo[i].c_str());
		}
	}
}
