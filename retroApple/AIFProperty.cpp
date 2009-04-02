/*\
 * AIFProperty.cpp
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
#include "../retroBase/MyString.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MobileContent.h"
#include "AIFFile.h"
#include "AIFProperty.h"

CAIFProperty::CAIFProperty(void)
{
}

CAIFProperty::~CAIFProperty(void)
{
}

void CAIFProperty::InitFromContent(LPCTSTR szPath, uint32_t nSize,CMobileContent *pm)
{
	CMobileProperty::InitFromContent((char *)szPath,nSize,pm);
	CAIFFile *pAIF=(CAIFFile *)pm;
	ASSERT(pAIF);
	ASSERT(typeid(pAIF) == typeid(CAIFFile *));
	if (pAIF)
	{
		SetProperty(prnumBitsPerSample,pAIF->nGetBitsPerSample());
		SetProperty(prnumChannels,pAIF->nGetChannels());
		SetProperty(prnumSampleRate,pAIF->nGetSamplesPerSecond());
		SetProperty(prnumSampleSize,(pAIF->nGetSamples()*(pAIF->nGetBitsPerSample()/8)*pAIF->nGetChannels()));
		SetProperty(prnumPlaylength,pAIF->nGetPlaytime());
		SetProperty(prstrCopyright,pAIF->m_Header.pcCopyright);
		SetProperty(prstrArranged,pAIF->m_Header.pcAuthor);
		SetProperty(prstrName,pAIF->m_Header.pcName);
		SetProperty(prstrComment,pAIF->m_Header.pcAnnotation);
	}
}
