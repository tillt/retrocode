/*\
 * VOXFile.cpp
 * Copyright (C) 2004-2007, MMSGURU - written by Till Toenshoff
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
\*/
#include "stdafx.h"
#include <map>
#include <iostream>
#include <fstream>
#include "../include/Resource.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MyString.h"
#include "../retroBase/Basics.h"
#include "../retroBase/Adpcm.h"
#include "../retroBase/MobileContent.h"
#include "MFMSample.h"
#include "MFMFile.h"
#include "VOXFile.h"
#include "../retroBase/CommonAccess.h"
#include "VOXProperty.h"

DYNIMPPROPERTY(CVOXFile,CVOXProperty)

/*\ 
 * <---------- CVOXFile::CVOXFile ----------> 
 * @m 
\*/ 
 CVOXFile::CVOXFile(void)
{
	m_nMagicSize=4;
	m_pcMagic="VOX";
	m_sFormatName="VOX";
	m_sDefaultExtension=_T("vox");
	m_sFormatDescription.Load(IDS_FORMDESC_VOX);
	m_sFormatCredits=_T("");
}

/*\ 
 * <---------- CVOXFile::~CVOXFile----------> 
 * @m 
\*/ 
CVOXFile::~CVOXFile(void)
{
}

/*\ 
 * <---------- CVOXFile::read ----------> 
 * @m 
 * --> I N <-- @p
 * istream &ar - input stream reference
\*/ 
void CVOXFile::Read(istream &ar)
{
	unsigned char *pSource=NULL;
	uint32_t nSourceSize=0;

	nSourceSize=m_nFileSize;
	if (nSourceSize)
	{
		pSource=new unsigned char [nSourceSize];
		try
		{
			ar.read((char *)pSource,nSourceSize);
		}
		catch (istream::failure const &e)
		{
			Log2(verbLevErrors,"File Access Exception\n\t%s\n",e.what());
			return;
		}
	}
	m_pCurrentSample=new CMFMSample(pSource,nSourceSize);
	delete [] pSource;		//not needed anymore, now stored within MFM-Sample Objec

	m_nCSSize=nSourceSize*4;
	if ((m_pcCSBuffer=CMobileSampleContent::Alloc(m_nCSSize)) != NULL)
		m_pCurrentSample->Decode((short *)m_pcCSBuffer,m_nCSSize);
	m_nCSBitsPerSample=16;
	m_nCSChannels=1;
	m_nCSSamplesPerSecond=8000;
}

/*\ 
 * <---------- CVOXFile::RenderDestination ----------> 
 * @m 
 * --> I N <-- @p
 * ostream &out - reference to output stream
 * CMobileSampleContent *pSource - source sample data
\*/ 
void CVOXFile::Write(ostream &out)
{
	CMFMSample *pSample;

	if (m_pCSSource->m_nCSBitsPerSample != 16 && m_pCSSource->m_nCSBitsPerSample != 8)
	{
		TRACEIT2("sample width incompatible\n");
		throw new CFormatException(CFormatException::formaterrIncompatibleSampleFormat,"sample width incompatible");
	}
	if (m_pCSSource->m_nCSChannels != 1)
	{
		TRACEIT2("channel count incompatible\n");
		throw new CFormatException(CFormatException::formaterrIncompatibleSampleFormat,"channel count incompatible");
	}
	if (m_pCSSource->m_nCSSamplesPerSecond != 8000)
	{
		TRACEIT2("sample rate incompatible\n");
		throw new CFormatException(CFormatException::formaterrIncompatibleSampleFormat,"sample rate incompatible");
	}

	pSample=new CMFMSample(NULL,0);
	pSample->Encode((short *)m_pCSSource->m_pcCSBuffer,m_pCSSource->m_nCSSize);
	out.write((char *)pSample->pcGetAdpcm(),pSample->nGetSize());
	delete pSample;

	m_nCSChannels=m_pCSSource->m_nCSChannels;
	m_nCSBitsPerSample=4;
	m_nCSSamplesPerSecond=m_pCSSource->m_nCSSamplesPerSecond;
	m_nCSSize=pSample->nGetSize();
	m_nFileSize=out.tellp();
}

/*\
 * <---------- bMagicHead ---------->
 * @m vox files have no header - bypass magic comparsion and check file extension instead
 * --> I N <-- @p
 * std::istream &ar - 
 * uint32_tnSize - 
 * <-- OUT --> @r
 * bool - 
\*/
bool CVOXFile::bMagicHead(std::istream &ar,uint32_t nSize)
{
	CMyString strExt=CCommonAccess::sGetFileExtension(m_sFileName.c_str());
	strExt.ToLower();
	return strExt.compare(m_sDefaultExtension) == 0;
}

