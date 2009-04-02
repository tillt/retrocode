#include "stdafx.h"
#include "../retroBase/MyString.h"
#include "../retroBase/Integer.h"
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/FIDProperty.h"
#include "ThemeBaseContent.h"
#include "UTZProperty.h"
#include "UTZFile.h"

CUTZProperty::CUTZProperty(void)
{
	CUTZProperty::InitPropertyMapping();
}

CUTZProperty::~CUTZProperty(void)
{
}


void CUTZProperty::InitFromContent(LPCTSTR pszPath,unsigned int nSize,CMobileContent *pm)
{
	CMobileProperty::InitFromContent(pszPath,nSize,pm);
	CUTZFile *pUTZ=(CUTZFile *)pm;
	ASSERT(pUTZ);
	if (pUTZ)
	{
		char buffer[255];
		_itoa(pUTZ->nGetSubFormat(),buffer,10);
		SetProperty_long(prstrSubFormat,pUTZ->sGetFormatName().c_str(),"id",buffer);
		SetProperty(prstrName,pUTZ->sGetTitle().c_str());
		SetProperty(prstrArranged,pUTZ->sGetAuthor().c_str());
		SetProperty(prstrCopyright,pUTZ->sGetCompany().c_str());
		SetProperty(prboolContainsScreensaver,pUTZ->bContainsScreensaver());
		SetProperty(prboolContainsRingtone,pUTZ->bContainsRingtone());
		SetProperty(prboolContainsBackground,pUTZ->bContainsBackground());
		SetProperty(prnumWidth,(uint32_t)pUTZ->nGetWidth());
		SetProperty(prnumHeight,(uint32_t)pUTZ->nGetHeight());
		SetProperty(prnumColors,(uint32_t)pUTZ->nGetColors());
	}
}
