#include "stdafx.h"
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/FIDProperty.h"
#include "MBMFile.h"
#include "MBMProperty.h"

CMBMProperty::CMBMProperty(void)
{
	CMBMProperty::InitPropertyMapping();
}

CMBMProperty::~CMBMProperty(void)
{
}


void CMBMProperty::InitFromContent(LPCTSTR pszPath,unsigned int nSize,CMobileContent *pm)
{
	char buffer[10];
	CMobileProperty::InitFromContent(pszPath,nSize,pm);
	CMBMFile *pMBM=(CMBMFile *)pm;
	ASSERT(pMBM);
	if (pMBM)
	{
		_itoa(pMBM->nGetSubFormat(),buffer,10);
		SetProperty_long(prstrSubFormat,pMBM->sGetFormatName(pMBM->nGetSubFormat()).c_str(),"id",buffer);
		SetProperty(prnumWidth,(uint32_t)pMBM->nGetWidth());
		SetProperty(prnumHeight,(uint32_t)pMBM->nGetHeight());
		SetProperty(prnumColors,(uint32_t)pMBM->nGetColors());
	}
}
