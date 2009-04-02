/*\
 * AACProperty.cpp
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
#include "AACFile.h"
#include "MP4Property.h"
#include "AACProperty.h"

CAACProperty::CAACProperty(void) 
{
}

CAACProperty::~CAACProperty(void)
{
}

void CAACProperty::InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm)
{
	CMobileProperty::InitFromContent(szPath,nSize,pm);
	CAACFile *pAAC=(CAACFile *)pm;
	ASSERT(pAAC);
	if (pAAC)
	{
		char buffer[255];
		_itoa(pAAC->info.object_type,buffer,10);
		SetProperty_long(prstrSubFormat,CAACFile::sGetFormatName(pAAC->info.object_type).c_str(),"id",buffer);
		SetProperty(prnumChannels,(uint32_t)pAAC->info.channels);
		_itoa(pAAC->info.headertype,buffer,10);
		SetProperty_long(prstrHeaderType,CAACFile::sGetHeaderName(pAAC->info.headertype).c_str(),"id",buffer);
		SetProperty(prnumPlaylength,(uint32_t)pAAC->info.length);
		SetProperty(prnumBitRate,(uint32_t)pAAC->info.bitrate);
		SetProperty(prnumSampleRate,(uint32_t)pAAC->info.sampling_rate);
		SetProperty(prboolVariableBitRate,(uint32_t)pAAC->info.bVariableBitrate);
		SetProperty(prnumMpegVersion,(uint32_t)pAAC->info.version);
	}
}
