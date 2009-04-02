#include "stdafx.h"
#include <iostream>
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/FIDProperty.h"
#include "PNGFile.h"
#include "PNGProperty.h"

CPNGProperty::CPNGProperty(void) 
{
}

CPNGProperty::~CPNGProperty(void)
{
}

void CPNGProperty::InitFromContent(LPCTSTR pszPath,unsigned int nSize,CMobileContent *pm)
{
	CMobileProperty::InitFromContent(pszPath,nSize,pm);
	CPNGFile *pPNG=(CPNGFile *)pm;
	ASSERT(pPNG);
	if (pPNG)
	{
		char buffer[255];
		_itoa(1+pPNG->nGetSubFormat(),buffer,10);
		SetProperty_long(prstrSubFormat,CPNGFile::sGetFormatName(1+pPNG->nGetSubFormat()).c_str(),"id",buffer);
		SetProperty(prnumWidth,(uint32_t)pPNG->nGetWidth());
		SetProperty(prnumHeight,(uint32_t)pPNG->nGetHeight());
		SetProperty(prnumBitsPerPixel,(uint32_t)pPNG->nGetBitsPerPixel());
		SetProperty(prnumColors,(uint32_t)pPNG->nGetColors());
		SetProperty(prboolInterlaced,pPNG->bIsInterlaced());
	}
}
