/*\
 * VOXProperty.cpp
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
#include "../retroBase/MyString.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MobileContent.h"
#include "MFMFile.h"
#include "VOXFile.h"
#include "VOXProperty.h"

CVOXProperty::CVOXProperty(void) 
{
}

CVOXProperty::~CVOXProperty(void)
{
}

void CVOXProperty::InitFromContent(LPCTSTR pszPath,unsigned int nSize,CMobileContent *pm)
{
	CMobileProperty::InitFromContent(pszPath,nSize,pm);
	CVOXFile *pMfm=(CVOXFile *)pm;
	ASSERT(pMfm);
	if (pMfm)
	{
		SetProperty_long(prstrSubFormat,"Dialogic/OKI ADPCM","id","1");
		SetProperty(prnumSampleRate,pMfm->m_nCSSamplesPerSecond);
		SetProperty(prnumBitsPerSample,pMfm->m_nCSBitsPerSample);
		SetProperty(prnumChannels,pMfm->m_nCSChannels);
		SetProperty(prnumPlaylength,pMfm->nGetSamplePlaytime());
	}
}
