#include "stdafx.h"
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/FIDProperty.h"
#include "ThemeBaseContent.h"
#include "NTHProperty.h"
#include "NTHFile.h"

CNTHProperty::CNTHProperty(void)
{
	CNTHProperty::InitPropertyMapping();
}

CNTHProperty::~CNTHProperty(void)
{
}


void CNTHProperty::InitFromContent(LPCTSTR pszPath,unsigned int nSize,CMobileContent *pm)
{
	CMobileProperty::InitFromContent(pszPath,nSize,pm);
	CNTHFile *pNTH=(CNTHFile *)pm;
	ASSERT(pNTH);
	if (pNTH)
	{
		char buffer[255];
		_itoa(CNTHFile::nGetSubFormat(pNTH->sGetVersion().c_str()),buffer,10);
		SetProperty_long(prstrSubFormat,CNTHFile::sGetFormatName(CNTHFile::nGetSubFormat(pNTH->sGetVersion().c_str())).c_str(),"id",buffer);
		SetProperty(prstrName,pNTH->sGetTitle().c_str());
		SetProperty(prboolContainsBackground,pNTH->bContainsBackground());
		SetProperty(prboolContainsRingtone,pNTH->bContainsRingtone());
		SetProperty(prboolContainsWallpaper,pNTH->bContainsWallpaper());
		SetProperty(prboolContainsScreensaver,pNTH->bContainsScreensaver());
		SetProperty(prnumWidth,(uint32_t)pNTH->nGetWidth());
		SetProperty(prnumHeight,(uint32_t)pNTH->nGetHeight());
		SetProperty(prnumColors,(uint32_t)pNTH->nGetColors());
	}
}
