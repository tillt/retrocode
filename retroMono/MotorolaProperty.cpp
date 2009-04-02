#include "stdafx.h"
#include <map>
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/FIDProperty.h"
#include "MonoFile.h"
#include "MotorolaProperty.h"
#include "MotorolaFile.h"

CMotorolaProperty::CMotorolaProperty(void)
{
}

CMotorolaProperty::~CMotorolaProperty(void)
{
}

void CMotorolaProperty::InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm)
{
	CMobileProperty::InitFromContent(szPath,nSize,pm);
	CMotorolaFile *pRingtone=(CMotorolaFile *)pm;
	ASSERT(pRingtone);
	ASSERT(typeid(pRingtone) == typeid(CMotorolaFile *));
	if (pRingtone)
	{
		char buffer[255];
		_itoa(pRingtone->nGetEncoding(),buffer,10);
		SetProperty_long(prstrEncoding,CMonoContent::sGetEncodingName(pRingtone->nGetEncoding()).c_str(),"id",buffer);
		SetProperty(prnumEventCount,(uint32_t)pRingtone->nGetEventCount());
		SetProperty(prnumPlaylength,(uint32_t)pRingtone->nGetPlaytime());
		SetProperty(prnumSmsCount,(uint32_t)pRingtone->nGetSMSCount());
	}
}
