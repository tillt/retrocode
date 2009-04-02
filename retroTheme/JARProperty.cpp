#include "stdafx.h"
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/FIDProperty.h"
#include "ThemeBaseContent.h"
#include "JARProperty.h"
#include "JARFile.h"

CJARProperty::CJARProperty(void)
{
	CJARProperty::InitPropertyMapping();
}

CJARProperty::~CJARProperty(void)
{
}


void CJARProperty::InitFromContent(LPCTSTR pszPath,unsigned int nSize,CMobileContent *pm)
{
	CMobileProperty::InitFromContent(pszPath,nSize,pm);
	CJARFile *pJAR=(CJARFile *)pm;
	ASSERT(pJAR);
	if (pJAR)
	{
		char buffer[255];
		_itoa(pJAR->nGetSubFormat(),buffer,10);
		SetProperty_long(prstrSubFormat,pJAR->sGetFormatName(pJAR->nGetSubFormat()).c_str(),"id",buffer);
		SetProperty(prstrName,pJAR->sGetTitle().c_str());
		SetProperty(prstrCopyright,pJAR->sGetCompany().c_str());
	}
}
