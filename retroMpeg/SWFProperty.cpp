#include "stdafx.h"
#include "../retroBase/Basics.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MyString.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/PacketCollection.h"
#include "../retroBase/Endian.h"
#include "../retroBase/Packer.h"
#include "MP3File.h"
#include "SWFFile.h"
#include "MP3Property.h"
#include "SWFProperty.h"

CSWFProperty::CSWFProperty(void) 
{
}

CSWFProperty::~CSWFProperty(void)
{
}

void CSWFProperty::InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm)
{
	CMobileProperty::InitFromContent(szPath,nSize,pm);
	CSWFFile *pSWF=(CSWFFile *)pm;
	ASSERT(pSWF);
	if (pSWF)
	{
		char buffer[255];
		_itoa(pSWF->nGetFormat(),buffer,10);
		SetProperty_long(prstrSubFormat,CSWFFile::sGetFormatName(pSWF->nGetFormat()).c_str(),"id",buffer);
		if (pSWF->bContainsMP3())
			PartialInitFromContent((CMP3File *)pm);
		else
		{
			SetProperty(prnumChannels,pSWF->nGetChannels());
			SetProperty(prnumSampleRate,pSWF->nGetSamplesPerSecond());
			SetProperty(prnumPlaylength,pSWF->nGetPlaytime());
			SetProperty(prnumBitsPerSample,pSWF->nGetBitsPerSample());
		}
		SetProperty(prboolLoop,(pSWF->nGetLoopCount() > 0));
		SetProperty(prnumLoopCount,pSWF->nGetLoopCount());
		SetProperty(prnumFrameRate,pSWF->nGetFrameRate());
		SetProperty(prnumFrameWidth,pSWF->nGetFrameWidth()/20);
		SetProperty(prnumFrameHeight,pSWF->nGetFrameHeight()/20);
	}
}

