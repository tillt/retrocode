#include "stdafx.h"
#include <map>
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/FIDProperty.h"
#include "MonoFile.h"
#include "NokiaProperty.h"
#include "NokiaFile.h"

CNokiaProperty::CNokiaProperty(void)
{
}

CNokiaProperty::~CNokiaProperty(void)
{
}

void CNokiaProperty::InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm)
{
	CMobileProperty::InitFromContent(szPath,nSize,pm);
	CNokiaFile *pNokia=(CNokiaFile *)pm;
	ASSERT(pNokia);
	ASSERT(typeid(pNokia) == typeid(CNokiaFile *));
	if (pNokia)
	{
		char buffer[255];
		_itoa(pNokia->nGetEncoding(),buffer,10);
		SetProperty_long(prstrEncoding,CNokiaFile::sGetEncodingName(pNokia->nGetEncoding()).c_str(),"id",buffer);
		SetProperty(prstrName,pNokia->sGetName().c_str());
		SetProperty(prnumEventCount,(uint32_t)pNokia->nGetEventCount());
		SetProperty(prboolLoop,(uint32_t)pNokia->bGetLoop() > 0);
		SetProperty(prnumPlaylength,(uint32_t)pNokia->nGetPlaytime());
		SetProperty(prnumSmsCount,(uint32_t)pNokia->nGetSMSCount());
	}
}
