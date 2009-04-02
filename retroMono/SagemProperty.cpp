#include "stdafx.h"
#include <map>
#include "../retroBase/MyString.h"
#include "../retroBase/Integer.h"
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/FIDProperty.h"
#include "MonoFile.h"
#include "SagemProperty.h"
#include "SagemFile.h"

CSagemProperty::CSagemProperty(void)
{
}

CSagemProperty::~CSagemProperty(void)
{
}

void CSagemProperty::InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm)
{
	CMobileProperty::InitFromContent(szPath,nSize,pm);
	CSagemFile *pRingtone=(CSagemFile *)pm;
	ASSERT(pRingtone);
	ASSERT(typeid(pRingtone) == typeid(CSagemFile *));
	if (pRingtone)
	{
		char buffer[255];
		_itoa(pRingtone->nGetEncoding(),buffer,10);
		SetProperty_long(prstrEncoding,CMonoContent::sGetEncodingName(pRingtone->nGetEncoding()).c_str(),"id",buffer);
		SetProperty(prstrName,pRingtone->sGetName().c_str());
		SetProperty(prnumEventCount,(uint32_t)pRingtone->nGetEventCount());
		SetProperty(prnumPlaytime,(uint32_t)pRingtone->nGetPlaytime());
		SetProperty(prnumSmsCount,(uint32_t)pRingtone->nGetSMSCount());
		SetProperty(prnumLoopDelay,(uint32_t)pRingtone->nGetLoopDelay());
	}
}
