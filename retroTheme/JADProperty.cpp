#include "stdafx.h"
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/FIDProperty.h"
#include "ThemeBaseContent.h"
#include "JADProperty.h"
#include "JADFile.h"

CJADProperty::CJADProperty(void)
{
	CJADProperty::InitPropertyMapping();
}

CJADProperty::~CJADProperty(void)
{
}


void CJADProperty::InitFromContent(LPCTSTR pszPath,unsigned int nSize,CMobileContent *pm)
{
	CMobileProperty::InitFromContent(pszPath,nSize,pm);
	CJADFile *pJAD=(CJADFile *)pm;
	ASSERT(pJAD);
	if (pJAD)
	{
		char buffer[255];
		_itoa(pJAD->nGetSubFormat(),buffer,10);
		SetProperty_long(prstrSubFormat,pJAD->sGetFormatName(pJAD->nGetSubFormat()).c_str(),"id",buffer);
		SetProperty(prstrName,pJAD->sGetTitle().c_str());
		SetProperty(prstrCopyright,pJAD->sGetCompany().c_str());
	}
}
