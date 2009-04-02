#include "stdafx.h"
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/FIDProperty.h"
#include "ThemeBaseContent.h"
#include "MTFProperty.h"
#include "MTFFile.h"

CMTFProperty::CMTFProperty(void)
{
	CMTFProperty::InitPropertyMapping();
}

CMTFProperty::~CMTFProperty(void)
{
}


void CMTFProperty::InitFromContent(LPCTSTR pszPath,unsigned int nSize,CMobileContent *pm)
{
	char buffer[10];
	CMobileProperty::InitFromContent(pszPath,nSize,pm);
	CMTFFile *pMTF=(CMTFFile *)pm;
	ASSERT(pMTF);
	if (pMTF)
	{
		_itoa(pMTF->nGetSubFormat(),buffer,10);
		SetProperty_long(prstrSubFormat,pMTF->sGetFormatName().c_str(),"id",buffer);
		SetProperty(prboolContainsScreensaver,pMTF->bContainsScreensaver());
		SetProperty(prboolContainsRingtone,pMTF->bContainsRingtone());
		SetProperty(prboolContainsWallpaper,pMTF->bContainsWallpaper());
		SetProperty(prnumWidth,(uint32_t)pMTF->nGetWidth());
		SetProperty(prnumHeight,(uint32_t)pMTF->nGetHeight());
		SetProperty(prnumColors,(uint32_t)pMTF->nGetColors());
	}
}
