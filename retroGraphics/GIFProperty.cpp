#include "stdafx.h"
#include <iostream>
#include "../retroBase/Basics.h"
#include "../retroBase/MyString.h"
#include "../retroBase/MobileContent.h"
#include "GIFFile.h"
#include "../retroBase/FIDProperty.h"
#include "GIFProperty.h"

using namespace std;

CGIFProperty::CGIFProperty(void) 
{
}

CGIFProperty::~CGIFProperty(void)
{
}

void CGIFProperty::InitFromContent(LPCTSTR pszPath,unsigned int nSize,CMobileContent *pm)
{
	CMobileProperty::InitFromContent(pszPath,nSize,pm);
	CGIFFile *pGIF=(CGIFFile *)pm;
	ASSERT(pGIF);
	if (pGIF)
	{
		char buffer[255];
		_itoa(pGIF->nGetSubFormat(),buffer,10);
		SetProperty_long(prstrSubFormat,CGIFFile::sGetFormatName(pGIF->nGetSubFormat()).c_str(),"id",buffer);
		SetProperty(prnumWidth,(uint32_t)pGIF->nGetWidth());
		SetProperty(prnumHeight,(uint32_t)pGIF->nGetHeight());
		SetProperty(prnumPlaytime,(uint32_t)pGIF->nGetPlaytime());
		SetProperty(prnumBitsPerPixel,(uint32_t)pGIF->nGetBitsPerPixel());
		SetProperty(prnumColors,(uint32_t)pGIF->nGetColors());
		SetProperty(prnumAverageInterFrameDelay,(uint32_t)pGIF->nGetAverageInterFrameDelay());
		SetProperty(prnumFrames,(uint32_t)pGIF->nGetFrameCount());
		SetProperty(prstrComment,pGIF->sGetComment().c_str());
		SetProperty(prboolInterlaced,pGIF->bIsInterlaced());
		SetProperty(prboolTransparent,pGIF->bIsTransparent());
		SetProperty(prnumLoopCount,(uint32_t)pGIF->nGetLoopCount());
	}
}
