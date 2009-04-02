/*\
 * UTZFile.cpp
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
#ifdef WIN32
#include <io.h>
#include <direct.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <map>
#include <stdlib.h>
#include "../retroBase/MyString.h"
#include "../retroBase/Integer.h"
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/FIDProperty.h"
#include "ThemeBaseContent.h"
#include "unzip/unzip.h"
#include "zlib.h"
#include "ZIPArchive.h"
#include "UTZFile.h"
#include "UTZProperty.h"

DYNIMPPROPERTY(CUTZFile,CUTZProperty)

CUTZFile::CUTZFile(void)
{
	m_pcMagic="PK\003\004";
	m_nMagicSize=4;
	m_sFormatName="Sony-Ericsson UTZ Theme";
}

CUTZFile::~CUTZFile(void)
{
}

/*\
 * <---------- ParseXmlData ---------->
 * @m parses "theme_descriptor.xml"
 * --> I N <-- @p
 * char *pcDest - xml-data string
 * unsigned int nSize - buffer size
\*/
void CUTZFile::ParseXmlData(char *pcDest,unsigned int nSize)
{
	bool bRet=false;
	CMyString strXml;
	CMyString strElement;
	CMyString strAttribute;

	int iVers=0;
	
	strXml=pcDest;

	TRACEIT2("XML: %s\n",pcDest);
	strElement=strXml.sGetXmlElement("theme");
	if (strElement.empty())
		throw new CFormatException(CFormatException::formaterrInvalid,_T("theme element missing"));

	m_strVersion=strElement.sGetXmlElementAttribute("version");
	if (m_strVersion.empty())
		m_strVersion="1.0";
	m_strName=strElement.sGetXmlElementAttribute("title");
	m_strAuthor=strElement.sGetXmlElementAttribute("author");
	if (m_strAuthor.compare("Not defined") == 0)
		m_strAuthor="";
	m_strCompany=strElement.sGetXmlElementAttribute("copyright");
	if (m_strCompany.compare("Not defined") == 0)
		m_strCompany="";
	m_strScreenWidth=strElement.sGetXmlElementAttribute("screenWidth");
	m_strScreenHeight=strElement.sGetXmlElementAttribute("screenHeight");

	strElement=strXml.sGetXmlElement("systemSound");
	if (!strElement.empty())
	{
		m_strStdResource[resRingtone]=strElement.sGetXmlElementAttribute("id");
		/*
		strAttribute=strElement.sGetXmlElementAttribute("id");
		if (!strAttribute.compare("DefaultRing"))
		{
			m_strStdResource[resRingtone]=strXml.sGetXmlElementContent("systemSound");
		}
		*/
	}
	strElement=strXml.sGetXmlElement("wallpaper");
	if (!strElement.empty())
	{
		m_strStdResource[resDesktop]=strElement.sGetXmlElementAttribute("id");
		/*strAttribute=strElement.sGetXmlElementAttribute("id");
		if (!strAttribute.compare("FcStandby_208x189"))
		{
			m_strStdResource[resDesktop]=strXml.sGetXmlElementContent("systemSound");
		}
		*/
	}
}

void CUTZFile::Read(istream &ar)
{
	const char *password=NULL;
	char *pcDest=NULL;
	unsigned int nFileSize=0;
	char szComment[256]="";
	CZIPArchive zip;

	if(zip.bOpen(ar))
	{
		TRACEIT2("archive opened sucessfully\n");
		if (zip.bFindFile("Theme.xml"))
		{
			zip.nExtractCurrentFile(password,&pcDest,&nFileSize);
			m_strDeviceName=zip.sGetGlobalComment();
			TRACEIT2("comment: %s\n",m_strDeviceName);
			if (m_strDeviceName.empty())
				m_strDeviceName="unknown";
			if (pcDest)
			{
				ParseXmlData(pcDest,nFileSize);
				delete [] pcDest;
			}
		}
		else
			throw new CFormatException(CFormatException::formaterrInvalid,"no theme xml found");
		if (zip.bFindFile("System/ScreenSaver.gif"))
		{
			TRACEIT2("located screensaver in archive\n");
			m_strStdResource[resScreensaver]="System/ScreenSaver.gif";
		}
		if (zip.bFindFile("FcTitleBarSkin.mbm"))
		{
			TRACEIT2("located background in archive\n");
			m_strStdResource[resBackground]="FcTitleBarSkin.mbm";
		}
		for (int i=0;i < resLast;i++)
		{
			if (!m_strStdResource[i].empty())
			{
				if (zip.bFindFile(m_strStdResource[i].c_str()))
				{
					zip.nExtractCurrentFile(password,&pcDest,&nFileSize);
					if (pcDest)
					{
						poProcessFile(pcDest,nFileSize,m_strStdResource[i].c_str());
						delete [] pcDest;
					}
				}
				/*
				else
					throw new CFormatException(CFormatException::formaterrInvalid,"referenced file not found");
					*/
			}
		}
		zip.Close();
	}
	else
		throw new CFormatException(CFormatException::formaterrUnknown,_T("failed open archive file"));
}

tstring CUTZFile::sGetFormatName(void)
{
	return "Version " + m_strVersion + " : " + m_strDeviceName;
}

int CUTZFile::nGetSubFormat(void)
{
	map<tstring,int> mapDeviceName;
	mapDeviceName["unknown"]=0;
	mapDeviceName["P900"]=1;
	mapDeviceName["P910"]=2;
	mapDeviceName["M600"]=3;
	mapDeviceName["W950"]=4;
	mapDeviceName["P990"]=5;
	mapDeviceName["M600"]=6;
	mapDeviceName["W950"]=7;
	return mapDeviceName[m_strDeviceName];
}

bool CUTZFile::bMagicHead(std::istream &ar,uint32_t nSize)
{
	bool bRet=false;
	char *acCompare=new char[m_nMagicSize];
	
	ar.read(acCompare,m_nMagicSize);
	if (!memcmp(acCompare,m_pcMagic,m_nMagicSize))
	{
		CZIPArchive zip;
		if (zip.bOpen(ar))
		{
			bRet=zip.bFindFile("Theme.xml");
			zip.Close();
		}
	}
	delete [] acCompare;
	return bRet;
}
