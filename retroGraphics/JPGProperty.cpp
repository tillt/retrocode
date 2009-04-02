#include "stdafx.h"
#include <iostream>
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/FIDProperty.h"
#include "JPGProperty.h"
#include "JPGFile.h"
using namespace std;

CJPGProperty::CJPGProperty(void) 
{
}

CJPGProperty::~CJPGProperty(void)
{
}

void CJPGProperty::InitFromContent(LPCTSTR pszPath,unsigned int nSize,CMobileContent *pm)
{
	CMobileProperty::InitFromContent(pszPath,nSize,pm);
	CJPGFile *pJPG=(CJPGFile *)pm;
	ASSERT(pJPG);
	if (pJPG)
	{
		char buffer[255];
		_itoa(pJPG->nGetSubFormat(),buffer,10);
		SetProperty_long(prstrSubFormat,CJPGFile::sGetFormatName(pJPG->nGetSubFormat()).c_str(),"id",buffer);
		SetProperty(prnumWidth,(uint32_t)pJPG->nGetWidth());
		SetProperty(prnumHeight,(uint32_t)pJPG->nGetHeight());
		SetProperty(prnumColors,(uint32_t)pJPG->nGetColors());
		SetProperty(prboolProgressive,(uint32_t)pJPG->bIsProgressive());
		SetProperty(prnumResolutionX,(uint32_t)pJPG->nGetResolutionX());
		SetProperty(prnumResolutionY,(uint32_t)pJPG->nGetResolutionY());
	}
}
