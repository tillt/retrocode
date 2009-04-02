/*\
 * RMFProperty.cpp
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
#include "../include/Resource.h"
#include "../retroBase/MyString.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MobileContent.h"
#include "RMFBasics.h"
#include "RMFProperty.h"
#include "RMFFile.h"

CRMFProperty::CRMFProperty(void)
{
}

CRMFProperty::~CRMFProperty(void)
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
void CRMFProperty::InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm)
{
	CMobileProperty::InitFromContent(szPath,nSize,pm);
	CRMFFile *pRMF=(CRMFFile *)pm;
	ASSERT(pRMF);
	ASSERT(typeid(pRMF) == typeid(CRMFFile *));
	if (pRMF)
	{
		char buffer[255];
		_itoa(pRMF->nGetFormat(),buffer,10);
		SetProperty_long(prstrSubFormat,CRMFFile::sGetFormatName(pRMF->nGetFormat()).c_str(),"id",buffer);
		SetProperty(prboolContainsSamples,pRMF->bUsesSamples());
		SetProperty(prnumSampleRate,pRMF->nGetSamplesPerSecond());
		SetProperty(prnumChannels,pRMF->nGetChannels());
		SetProperty(prnumBitsPerSample,pRMF->nGetBitsPerSample());
		SetProperty(prnumPlaylength,(int)pRMF->nGetPlaytime() == 0 ? pRMF->nGetSamplePlaytime() : pRMF->nGetPlaytime());
		if (pRMF->nGetFormat() == CRMFFile::rmfformatMpeg)
			SetProperty(prnumBitRate,pRMF->nGetBitRate());
		SetProperty(prnumSamplePlaytime,pRMF->nGetSamplePlaytime());
		for (int i=0;i < CMobileSampleContent::infoLast;i++)
		{
			if (!pRMF->sGetInfoText(i).empty())
				setProperty(strGetInfoToPropertyStringIndex(i).c_str(),pRMF->sGetInfoText(i).c_str());
		}
	}
}
