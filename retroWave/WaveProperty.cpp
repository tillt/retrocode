/*\
 * WaveProperty.cpp
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
#include "../retroBase/Basics.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/Adpcm.h"
#include "WaveFile.h"
#include "WaveProperty.h"

CWaveProperty::CWaveProperty(void) 
{
	TRACEIT2("constructor");
}

CWaveProperty::~CWaveProperty(void)
{
	TRACEIT2("destructor");
}

void CWaveProperty::InitFromContent(LPCTSTR pszPath,unsigned int nSize,CMobileContent *pm)
{
	CMobileProperty::InitFromContent(pszPath,nSize,pm);
	CWaveFile *pWave=(CWaveFile *)pm;
	ASSERT(pWave);
	if (pWave)
	{
		char buffer[255];
		_itoa(pWave->nGetFormat(),buffer,10);
		SetProperty_long(prstrSubFormat,CWaveFile::sGetFormatName(pWave->nGetFormat()).c_str(),"id",buffer);
		SetProperty(prnumChannels,pWave->m_nCSChannels);
		SetProperty(prnumSampleRate,pWave->m_nCSSamplesPerSecond);
		SetProperty(prnumBitsPerSample,pWave->m_nCSBitsPerSample);
		SetProperty(prnumPlaylength,pWave->nGetSamplePlaytime());
		for (int i=0;i < CMobileSampleContent::infoLast;i++)
		{
			if (pWave->m_strInfo[i].empty())
				setProperty(strGetInfoToPropertyStringIndex(i).c_str(),pWave->m_strInfo[i].c_str());
		}
	}
}
