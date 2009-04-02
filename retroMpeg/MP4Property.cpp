/*\
 * MP4Property.cpp
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
#include "MP4File.h"
#include "MP4Property.h"

CMP4Property::CMP4Property(void) 
{
	TRACEIT2("constructor\n");
}

CMP4Property::~CMP4Property(void)
{
	TRACEIT2("destructor\n");
}

void CMP4Property::InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm)
{
	CMobileProperty::InitFromContent(szPath,nSize,pm);
	CMP4File *pMP4=(CMP4File *)pm;
	ASSERT(pMP4);
	if (pMP4)
	{
		char buffer[255];
		_itoa(pMP4->m_nObjectType,buffer,10);
		SetProperty_long(prstrSubFormat,CMP4File::sGetFormatName(pMP4->m_nObjectType).c_str(),"id",buffer);
		SetProperty(prnumChannels,pMP4->m_nCSChannels);
		SetProperty(prnumSampleRate,pMP4->m_nCSSamplesPerSecond);
		SetProperty(prnumBitRate,pMP4->m_nBitRate);
		SetProperty(prnumPlaylength,pMP4->m_nPlayTime);
		if (pMP4->m_nObjectType & 0xFFF0)
		{
			SetProperty(prnumFrameRate,pMP4->m_nFrameRate);
			SetProperty(prnumFrameWidth,pMP4->m_nWidth);
			SetProperty(prnumFrameHeight,pMP4->m_nHeight);
		}
		for (int i=0;i < CMobileSampleContent::infoLast;i++)
		{
			if (!pMP4->sGetInfoText(i).empty())
				setProperty(strGetInfoToPropertyStringIndex(i).c_str(),pMP4->sGetInfoText(i).c_str());
		}
		//SetProperty(prnumF,pMP4->m_nFrameRate);
		TRACEIT2("playtime: %d\n",pMP4->m_nPlayTime);
	}
}
