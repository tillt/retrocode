#include "stdafx.h"
#include <map>
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/FIDProperty.h"
#include "MonoFile.h"
#include "EMSProperty.h"
#include "EMSFile.h"

CEMSProperty::CEMSProperty(void)
{
}

CEMSProperty::~CEMSProperty(void)
{
}

void CEMSProperty::InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm)
{
	CMobileProperty::InitFromContent(szPath,nSize,pm);
	CEMSFile *pEMS=(CEMSFile *)pm;
	ASSERT(pEMS);
	ASSERT(typeid(pEMS) == typeid(CEMSFile *));
	if (pEMS)
	{
		char buffer[255];
		_itoa(pEMS->nGetEncoding(),buffer,10);
		SetProperty_long(prstrEncoding,CEMSFile::sGetEncodingName(pEMS->nGetEncoding()).c_str(),"id",buffer);
		SetProperty(prstrName,pEMS->sGetName().c_str());
		SetProperty(prnumEventCount,(uint32_t)pEMS->nGetEventCount());
		SetProperty(prnumPlaylength,(uint32_t)pEMS->nGetPlaytime());
		SetProperty(prnumSmsCount,(uint32_t)pEMS->nGetSMSCount());
		SetProperty(prboolContainsIMelodyHead,pEMS->bUsesIMelodyHead());
		SetProperty(prboolContainsTempo,pEMS->bUsesTempo());
		SetProperty(prboolContainsName,pEMS->bUsesName());
	}
}
