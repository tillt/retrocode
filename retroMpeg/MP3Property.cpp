#include "stdafx.h"
#include <map>
#include "../retroBase/Basics.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MyString.h"
#include "../retroBase/MobileContent.h"
#include "MP3File.h"
#include "MP3Property.h"

CMP3Property::CMP3Property(void) 
{
}

CMP3Property::~CMP3Property(void)
{
}

void CMP3Property::InitFromContent(LPCTSTR pszPath,unsigned int nSize,CMobileContent *pm)
{
	CMobileProperty::InitFromContent(pszPath,nSize,pm);
	CMP3File *pMpeg=(CMP3File *)pm;	
	ASSERT(pMpeg);
	if (pMpeg)
	{
		char buffer[255];
		_itoa(pMpeg->nGetSubFormat(),buffer,10);
		SetProperty_long(prstrSubFormat,CMP3File::sGetFormatName(pMpeg->nGetSubFormat()).c_str(),"id",buffer);
		PartialInitFromContent(pMpeg);
	}
}

void CMP3Property::PartialInitFromContent(CMP3File *pMpeg)
{
	ASSERT(pMpeg);
	if (pMpeg)
	{
		SetProperty(prnumChannels,pMpeg->nGetChannels());
		SetProperty(prnumSampleRate,pMpeg->nGetSamplesPerSecond());
		SetProperty(prnumBitRate,pMpeg->nGetBitRate());
		SetProperty(prnumPlaylength,pMpeg->nGetPlaytime());
		SetProperty(prboolVariableBitRate,pMpeg->bGetVBR());
		if (pMpeg->nGetChannels() > 1)
			SetProperty(prboolJointStereo,pMpeg->bGetJointStereo());
		SetProperty(prboolContainsCRC,pMpeg->bGetCRC());
		SetProperty(prboolCopyrighted,pMpeg->bGetCopyrightBit());
		SetProperty(prboolOriginalRecording,pMpeg->bGetOriginalBit());
		SetProperty(prboolPrivate,pMpeg->bGetPrivateBit());
		if (pMpeg->bGetVBR())
		{
			SetProperty(prnumMinBitRate,pMpeg->nGetMinBitRate());
			SetProperty(prnumMaxBitRate,pMpeg->nGetMaxBitRate());
		}
		for (int i=0;i < CMobileSampleContent::infoLast;i++)
		{
			if (!pMpeg->sGetInfoText(i).empty())
				setProperty(strGetInfoToPropertyStringIndex(i).c_str(),pMpeg->sGetInfoText(i).c_str());
		}
	}
}
