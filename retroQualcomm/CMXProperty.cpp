/*\
 * CMXProperty.cpp
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
#include <map>
#include "../retroBase/Basics.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MyString.h"
#include "../retroBase/MobileContent.h"
#include "CMXProperty.h"
#include "CMXFile.h"

CCMXProperty::CCMXProperty(void)
{
}

CCMXProperty::~CCMXProperty(void)
{
}

/*\
 * <---------- InitFromContent ---------->
 * @m 
 * --> I N <-- @p
 * LPCTSTR szPath - 
 * unsigned int nSize - 
 * CMobileContent *pm - 
\*/
void CCMXProperty::InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm)
{
	TRACEIT2("start\n");
	CMobileProperty::InitFromContent(szPath,nSize,pm);
	CCMXFile *pCmx=(CCMXFile *)pm;
	ASSERT(pCmx);
	ASSERT(typeid(pCmx) == typeid(CCMXFile *));
	if (pCmx)
	{
		TRACEIT2("m_nSubFormat=%d\n",pCmx->nGetFormat());
		char buffer[255];
		bool bSamples=pCmx->bUsesSamples();
		_itoa(pCmx->nGetFormat(),buffer,10);
		SetProperty_long(prstrSubFormat,CCMXFile::sGetFormatName(pCmx->nGetFormat()).c_str(),"id",buffer);
		TRACEIT2("m_nEncoding=%d\n",pCmx->nGetEncoding());
		//m_nEncoding=pCmx->nGetEncoding();
		SetProperty(prboolContainsSamples,bSamples);
		SetProperty(prboolContainsPicture,pCmx->bUsesGraphix());
		if (bSamples)
		{
			SetProperty(prnumSampleRate,pCmx->m_nCSSamplesPerSecond);
			SetProperty(prnumChannels,pCmx->m_nCSChannels);
			SetProperty(prnumBitsPerSample,pCmx->m_nCSBitsPerSample);
			SetProperty(prnumSamplePlaytime,pCmx->nGetSamplePlaytime());
		}
		SetProperty(prnumPlaylength,pCmx->nGetPlaytime() == 0 ? pCmx->nGetSamplePlaytime() : pCmx->nGetPlaytime());
		for (int i=0;i < CMobileSampleContent::infoLast;i++)
		{
			if (!pCmx->m_strInfo[i].empty())
				setProperty(strGetInfoToPropertyStringIndex(i).c_str(),pCmx->m_strInfo[i].c_str());
		}
	}
	TRACEIT2("done\n");
}
