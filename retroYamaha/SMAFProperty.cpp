/*\
 * SMAFProperty.cpp
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
#include "SMAFDecoder.h"
#include "SMAFProperty.h"
#include "SMAFFile.h"

CSMAFProperty::CSMAFProperty(void)
{
}

CSMAFProperty::~CSMAFProperty(void)
{
}

void CSMAFProperty::InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm)
{
	CMobileProperty::InitFromContent(szPath,nSize,pm);
	CSMAFFile *pSmaf=(CSMAFFile *)pm;
	ASSERT(pSmaf);
	ASSERT(typeid(pSmaf) == typeid(CSMAFFile *));
	if (pSmaf)
	{
		char buffer[255];
		_itoa(pSmaf->nGetFormat(),buffer,10);
		SetProperty_long(prstrSubFormat,CSMAFFile::sGetFormatName(pSmaf->nGetFormat()).c_str(),"id",buffer);
		SetProperty(prboolStatusSave,pSmaf->bGetStatusSave());
		SetProperty(prboolStatusCopy,pSmaf->bGetStatusCopy());
		SetProperty(prboolStatusEdit,pSmaf->bGetStatusEdit());
		SetProperty(prboolContainsSamples,pSmaf->bUsesSamples());
		SetProperty(prboolContainsPicture,pSmaf->bUsesGraphix());
		SetProperty(prboolContainsSynthesizer,pSmaf->bUsesSynthesizer());
		_itoa(pSmaf->nGetEncoding(),buffer,10);
		SetProperty_long(prstrEncoding,CSMAFFile::sGetEncodingName(pSmaf->nGetEncoding()).c_str(),"id",buffer);
		SetProperty(prboolContainsHumanVoice,pSmaf->bUsesHumanVoice());
		//SetProperty(prnumSequenceFormat,pSmaf->nGetSequenceFormat());
		SetProperty(prnumPlaylength,pSmaf->nGetPlaytime());
		if (pSmaf->bUsesSamples())
		{
			SetProperty(prnumSamplePlaytime,pSmaf->nGetSamplePlaytime());
			SetProperty(prnumSampleRate,pSmaf->nGetSampleRate());
			SetProperty(prnumChannels,pSmaf->nGetChannels());
		}
		for (int i=0;i < CMobileSampleContent::infoLast;i++)
		{
			if (!pSmaf->sGetInfoText(i).empty())
				setProperty(strGetInfoToPropertyStringIndex(i).c_str(),pSmaf->sGetInfoText(i).c_str());
		}
	}
}
