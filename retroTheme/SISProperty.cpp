#include "stdafx.h"
#include <iostream>
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/FIDProperty.h"
#include "SISFile.h"
#include "SISProperty.h"

CSISProperty::CSISProperty(void)
{
	CSISProperty::InitPropertyMapping();
}

CSISProperty::~CSISProperty(void)
{
}

void CSISProperty::InitFromContent(LPCTSTR pszPath,unsigned int nSize,CMobileContent *pm)
{
	CMobileProperty::InitFromContent(pszPath,nSize,pm);
	CSISFile *pSIS=(CSISFile *)pm;
	ASSERT(pSIS);
	if (pSIS)
	{
		char buffer[255];
		_itoa(pSIS->nGetSubFormat(),buffer,10);
		SetProperty_long(prstrSubFormat,pSIS->sGetFormatName(pSIS->nGetSubFormat()).c_str(),"id",buffer);
		SetProperty(prstrName,pSIS->sGetTitle().c_str());
		SetProperty(prstrComment,pSIS->sGetRemark().c_str());
		//SetProperty(prstrCopyright,pSIS->sGetCompany().c_str());
		SetProperty(prboolContainsScreensaver,pSIS->bContainsScreensaver());
		SetProperty(prboolContainsApplication,pSIS->bContainsApplication());
		SetProperty(prboolContainsRingtone,pSIS->bContainsRingtone());
		SetProperty(prboolContainsBackground,pSIS->bContainsBackground());
		SetProperty(prnumWidth,(uint32_t)pSIS->nGetWidth());
		SetProperty(prnumHeight,(uint32_t)pSIS->nGetHeight());
		SetProperty(prnumColors,(uint32_t)pSIS->nGetColors());
	}
}
