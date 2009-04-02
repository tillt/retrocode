#include "stdafx.h"
#include "../retroBase/Basics.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MyString.h"
#include "../retroBase/MobileContent.h"
#include "AMRFile.h"
#include "AMRProperty.h"

CAMRProperty::CAMRProperty(void) 
{
}

CAMRProperty::~CAMRProperty(void)
{
}

void CAMRProperty::InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm)
{
	CMobileProperty::InitFromContent(szPath,nSize,pm);
	CAMRFile *pAMR=(CAMRFile *)pm;
	ASSERT(pAMR);
	if (pAMR)
	{
		m_nMode=pAMR->nGetMode();
		char buffer[255];
		_itoa(pAMR->nGetFormat(),buffer,10);
		SetProperty_long(prstrSubFormat,CAMRFile::sGetFormatName(pAMR->nGetFormat()).c_str(),"id",buffer);
		SetProperty(prnumChannels,pAMR->nGetChannels());
		SetProperty(prnumBitRate,pAMR->nGetBitRate(m_nMode));
		SetProperty(prnumSampleRate,pAMR->nGetSamplesPerSecond());
		SetProperty(prnumPlaylength,(uint32_t)round(pAMR->m_nFileSize*8000,pAMR->nGetBitRate(m_nMode)*pAMR->nGetChannels()));
	}
}
