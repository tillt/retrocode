#include "stdafx.h"
#include <map>
#include "../retroBase/Basics.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MyString.h"
#include "../retroBase/MobileContent.h"

#ifdef __cplusplus
extern "C"{
#endif 
#include "ffmpeg/avformat.h"
#include "ffmpeg/avcodec.h"
#ifdef __cplusplus
}
#endif

#include "FFMPEGFile.h"
#include "FFMPEGProperty.h"

CFFMPEGProperty::CFFMPEGProperty(void) 
{
}

CFFMPEGProperty::~CFFMPEGProperty(void)
{
} 

void CFFMPEGProperty::InitFromContent(LPCTSTR pszPath,unsigned int nSize,CMobileContent *pm)
{
	CMobileProperty::InitFromContent(pszPath,nSize,pm);
	CFFMPEGFile *pMpeg=(CFFMPEGFile *)pm;	
	ASSERT(pMpeg);
	if (pMpeg)
	{
		char buffer[255];
		_itoa(pMpeg->nGetFormat(),buffer,10);
		SetProperty_long(prstrSubFormat,CFFMPEGFile::sGetFormatName(pMpeg->nGetFormat()).c_str(),"id",buffer);
		SetProperty(prnumSampleRate,pMpeg->m_nCSSamplesPerSecond);
		SetProperty(prnumChannels,pMpeg->m_nCSChannels);
		SetProperty(prnumBitsPerSample,pMpeg->m_nCSBitsPerSample);
		SetProperty(prnumPlaylength,pMpeg->nGetPlaytime());
		SetProperty(prnumBitRate,pMpeg->m_nBitRate);
		for (int i=0;i < CMobileSampleContent::infoLast;i++)
		{
			if (!pMpeg->m_strInfo[i].empty())
				setProperty(strGetInfoToPropertyStringIndex(i).c_str(),pMpeg->m_strInfo[i].c_str());
		}
	}
}
