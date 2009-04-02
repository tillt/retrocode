#include "stdafx.h"
#include <iostream>
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/FIDProperty.h"
#include "BMPFile.h"
#include "BMPProperty.h"

CBMPProperty::CBMPProperty(void) 
{
}

CBMPProperty::~CBMPProperty(void)
{
}

void CBMPProperty::InitFromContent(LPCTSTR pszPath,unsigned int nSize,CMobileContent *pm)
{
	CMobileProperty::InitFromContent(pszPath,nSize,pm);
	CBMPFile *pBMP=(CBMPFile *)pm;
	ASSERT(pBMP);
	if (pBMP)
	{
		char buffer[255];
		_itoa(pBMP->nGetSubFormat(),buffer,10);
		SetProperty_long(prstrSubFormat,CBMPFile::sGetFormatName(pBMP->nGetSubFormat()).c_str(),"id",buffer);
		SetProperty(prnumWidth,(uint32_t)pBMP->nGetWidth());
		SetProperty(prnumHeight,(uint32_t)pBMP->nGetHeight());
		SetProperty(prnumBitsPerPixel,(uint32_t)pBMP->nGetBitsPerPixel());
		SetProperty(prnumColors,(uint32_t)pBMP->nGetColors());
		if (pBMP->nGetResolutionX() > 0)
			SetProperty(prnumResolutionX,(uint32_t)pBMP->nGetResolutionX());
		if (pBMP->nGetResolutionY() > 0)
			SetProperty(prnumResolutionY,(uint32_t)pBMP->nGetResolutionY());
		_itoa(pBMP->nGetCompression(),buffer,10);
		SetProperty_long(prnumCompression,CBMPFile::sGetCompressionName(pBMP->nGetCompression()).c_str(),"id",buffer);
	}
}
