#include "stdafx.h"
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/FIDProperty.h"
#include "ThemeBaseContent.h"
#include "THMProperty.h"
#include "THMFile.h"

CTHMProperty::CTHMProperty(void)
{
	CTHMProperty::InitPropertyMapping();
}

CTHMProperty::~CTHMProperty(void)
{
}

void CTHMProperty::InitFromContent(LPCTSTR pszPath,unsigned int nSize,CMobileContent *pm)
{
	CMobileProperty::InitFromContent(pszPath,nSize,pm);
	CTHMFile *pTHM=(CTHMFile *)pm;
	ASSERT(pTHM);
	if (pTHM)
	{
		char buffer[255];
		_itoa(pTHM->nGetSubFormat(),buffer,10);
		SetProperty_long(prstrSubFormat,pTHM->sGetFormatName().c_str(),"id",buffer);
		SetProperty(prstrName,pTHM->sGetTitle().c_str());
		SetProperty(prstrArranged,pTHM->sGetAuthor().c_str());
		SetProperty(prstrTechnician,pTHM->sGetEmail().c_str());
		SetProperty(prstrCopyright,pTHM->sGetCompany().c_str());
		SetProperty(prboolContainsWallpaper,pTHM->bContainsWallpaper());
		SetProperty(prnumWidth,(uint32_t)pTHM->nGetWidth());
		SetProperty(prnumHeight,(uint32_t)pTHM->nGetHeight());
		SetProperty(prnumColors,(uint32_t)pTHM->nGetColors());
		SetProperty(prboolContainsBackground,pTHM->bContainsBackground());
		SetProperty(prboolContainsRingtone,pTHM->bContainsRingtone());
		SetProperty(prboolContainsScreensaver,pTHM->bContainsScreensaver());
	}
}
