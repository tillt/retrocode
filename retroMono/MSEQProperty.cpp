/*\
 * MSEQProperty.cpp
 * Copyright (C) 2002-2004, MMSGURU - written by Till Toenshoff
\*/
/*\
 * $Log: MSEQProperty.cpp,v $
 * Revision 1.2  2009/01/14 10:24:02  lobotomat
 * cleaned includes
 *
 * Revision 1.1.1.1  2007/11/27 06:35:47  lobotomat
 * initial check in
 *
 * Revision 1.1  2006/12/19 08:49:25  cvsuser
 * no message
 *
 * Revision 1.1.1.1  2006/04/17 20:31:40  Lobotomat
 * no message
 *
 * Revision 1.2  2005/01/13 04:56:36  lobotomat
 * added MP4, AAC, 3GP - fixed SMAF parser
 *
 * Revision 1.1  2004/12/01 09:34:51  lobotomat
 * added MSEQ and XMF support
 *
 *
 *
\*/
#include "stdafx.h"
#include <map>
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/FIDProperty.h"
#include "MonoFile.h"
#include "MSEQProperty.h"
#include "MSEQFile.h"

CMSEQProperty::CMSEQProperty(void)
{
}

CMSEQProperty::~CMSEQProperty(void)
{
}

void CMSEQProperty::InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm)
{
	CMobileProperty::InitFromContent(szPath,nSize,pm);
	CMSEQFile *pMseq=(CMSEQFile *)pm;
	ASSERT(pMseq);
	ASSERT(typeid(pMseq) == typeid(CMSEQFile *));
	if (pMseq)
	{
		SetProperty(prnumLoopCount,(uint32_t)pMseq->nGetLoopCount());
	}
}
