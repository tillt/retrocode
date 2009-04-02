#include "stdafx.h"
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/FIDProperty.h"
#include "ThemeBaseContent.h"
#include "SDFProperty.h"
#include "SDFFile.h"

CSDFProperty::CSDFProperty(void)
{
	CSDFProperty::InitPropertyMapping();
}

CSDFProperty::~CSDFProperty(void)
{
}

void CSDFProperty::InitFromContent(LPCTSTR pszPath,unsigned int nSize,CMobileContent *pm)
{
	CMobileProperty::InitFromContent(pszPath,nSize,pm);
	CSDFFile *pTHM=(CSDFFile *)pm;
	ASSERT(pTHM);
	if (pTHM)
	{
		//char buffer[255];
		//_itoa(pTHM->nGetSubFormat(),buffer,10);
		//SetProperty_long(prstrSubFormat,pTHM->sGetFormatName().c_str(),"id",buffer);
		SetProperty(prstrName,pTHM->sGetTitle().c_str());
		SetProperty(prstrArtist,pTHM->sGetAuthor().c_str());
		SetProperty(prstrVendor,pTHM->sGetVendor().c_str());
		SetProperty(prstrDateCreated,pTHM->sGetPublished().c_str());
		SetProperty(prstrLicenseURL,pTHM->sGetUrl().c_str());
		SetProperty(prstrCopyright,pTHM->sGetCopyright().c_str());
		SetProperty(prstrComment,pTHM->sGetDescription().c_str());

		//SetProperty(prboolContainsWallpaper,pTHM->bContainsWallpaper());
		SetProperty(prboolContainsBackground,pTHM->bContainsBackground());
		SetProperty(prboolContainsScreensaver,pTHM->bContainsScreensaver());
		SetProperty(prboolContainsBootup,pTHM->bContainsBootup());
		SetProperty(prboolContainsShutdown,pTHM->bContainsShutdown());

		SetProperty(prnumWidth,(uint32_t)pTHM->nGetWidth());
		SetProperty(prnumHeight,(uint32_t)pTHM->nGetHeight());
		SetProperty(prnumColors,(uint32_t)pTHM->nGetColors());
	}
}
