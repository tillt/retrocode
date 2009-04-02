#include "stdafx.h"
#include "../retroBase/MyString.h"
#include "../retroBase/Integer.h"
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/FIDProperty.h"
#include "ThemeBaseContent.h"
#include "THMSamsungProperty.h"
#include "THMSamsungFile.h"

CTHMSamsungProperty::CTHMSamsungProperty(void)
{
	CTHMSamsungProperty::InitPropertyMapping();
}

CTHMSamsungProperty::~CTHMSamsungProperty(void)
{
}

void CTHMSamsungProperty::InitFromContent(LPCTSTR pszPath,unsigned int nSize,CMobileContent *pm)
{
	CMobileProperty::InitFromContent(pszPath,nSize,pm);
	CTHMSamsungFile *pTHM=(CTHMSamsungFile *)pm;
	ASSERT(pTHM);
	if (pTHM)
	{
		char buffer[255];
		_itoa(pTHM->nGetSubFormat(),buffer,10);
		SetProperty_long(prstrSubFormat,pTHM->sGetFormatName().c_str(),"id",buffer);
		SetProperty(prstrName,pTHM->sGetTitle().c_str());
		SetProperty(prboolContainsWallpaper,pTHM->bContainsWallpaper());
		SetProperty(prboolContainsBackground,pTHM->bContainsBackground());
		SetProperty(prboolContainsRingtone,pTHM->bContainsRingtone());
		SetProperty(prboolContainsBootup,pTHM->bContainsBootup());
		SetProperty(prboolContainsShutdown,pTHM->bContainsShutdown());
		SetProperty(prnumWidth,(uint32_t)pTHM->nGetWidth());
		SetProperty(prnumHeight,(uint32_t)pTHM->nGetHeight());
		SetProperty(prnumColors,(uint32_t)pTHM->nGetColors());
	}
}
