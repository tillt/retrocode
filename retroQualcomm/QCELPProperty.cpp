/*\
 * QCELPProperty.cpp
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
#include "../retroBase/MyString.h"
#include "../retroBase/MobileContent.h"
#include "QCELPFile.h"
#include "QCELPProperty.h"

CQcelpProperty::CQcelpProperty(void) 
{
}

CQcelpProperty::~CQcelpProperty(void)
{
}

/*\ 
 * <---------- CQcelpProperty::InitFromContent ----------> 
 * @m 
 * --> I N <-- @p
 * LPCTSTR pszPath - 
 * unsigned int nSize - 
 * CMobileContent *pm - 
\*/ 
void CQcelpProperty::InitFromContent(LPCTSTR pszPath,unsigned int nSize,CMobileContent *pm)
{
	CMobileProperty::InitFromContent(pszPath,nSize,pm);
	CQCELPFile *pQcelp=(CQCELPFile *)pm;
	ASSERT(pQcelp);
	if (pQcelp)
	{
		char buffer[255];
		_itoa(pQcelp->nGetSubFormat(),buffer,10);
		SetProperty_long(prstrSubFormat,CQCELPFile::sGetFormatName(pQcelp->nGetSubFormat()).c_str(),"id",buffer);
		SetProperty(prnumSampleRate,pQcelp->nGetSamplesPerSecond());
		SetProperty(prnumChannels,(uint32_t)1);
		SetProperty(prnumBitsPerSample,pQcelp->nGetBitsPerSample());
		SetProperty(prnumPlaylength,pQcelp->nGetPlaytime(nSize,pQcelp->nGetAvgBps()/1000));
	}
}
