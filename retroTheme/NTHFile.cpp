/*\
 * NTHFile.cpp
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
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/FIDProperty.h"
#include "ThemeBaseContent.h"
#include "unzip/unzip.h"
#include "zlib.h"
#include "ZIPArchive.h"
#include "NTHFile.h"
#include "NTHProperty.h"
#include "resource.h"

DYNIMPPROPERTY(CNTHFile,CNTHProperty)

CNTHFile::CNTHFile(void)
{
	m_pcMagic="PK\003\004";
	m_nMagicSize=4;
	m_sFormatName="Nokia Series40 Theme";
}

CNTHFile::~CNTHFile(void)
{
}

/*\
 * <---------- ParseXmlData ---------->
 * @m parses "theme_descriptor.xml"
 * --> I N <-- @p
 * char *pcDest - xml-data string
 * unsigned int nSize - buffer size
\*/
void CNTHFile::ParseXmlData(char *pcDest,unsigned int nSize)
{
	typedef struct tagELEMENTTABLE
	{
		bool bMandatory;
		const char *szElementName;
		const char *szAttributeName[4];
		tstring *pstrAttributeDestination[4];
	}ElementTable;

	ElementTable t[]=
	{
		{	
			true,
			"theme",
			{"version", "name", NULL},
			{&m_strVersion, &m_strName, NULL}
		},
		{	
			false,
			"background",
			{"src", "main_display_graphics", "main_default_bg",NULL},
			{&m_strStdResource[resBackground], &m_strStdResource[resBackground], &m_strStdResource[resBackground],NULL}
		},
		{
			false,
			"wallpaper",
			{"src", "main_display_graphics", NULL},
			{&m_strStdResource[resDesktop], &m_strStdResource[resDesktop], NULL}
		},
		{
			false,
			"screensaver",
			{"src", "main_display_graphics", NULL},
			{&m_strStdResource[resScreensaver], &m_strStdResource[resScreensaver], NULL}
		},
		{	
			false,
			"ringtone",
			{"ringtone","tones","src",NULL},
			{&m_strStdResource[resRingtone], &m_strStdResource[resRingtone], &m_strStdResource[resRingtone],NULL}
		},
		{
			false,
			NULL,
			{NULL, NULL},
			{NULL,	NULL}
		}
	};
	int i,a;
	bool bRet=false;
	CMyString strXml;
	CMyString strElement;
	CMyString strAttribute;

	int iVers=0;

	strXml=pcDest;
	for (i=0;t[i].szElementName != NULL;i++)
	{
		strElement=strXml.sGetXmlElement(t[i].szElementName);
		if (strElement.empty() && t[i].bMandatory)
		{
			tstring strDescription=tstring(t[i].szElementName) + tstring(_T(" element missing"));
			throw new CFormatException(CFormatException::formaterrInvalid,strDescription.c_str());
		}
		if (!strElement.empty())
		{
			for (a=0;t[i].szAttributeName[a] != NULL;a++)
			{
				strAttribute=strElement.sGetXmlElementAttribute(t[i].szAttributeName[a]);
				if (!strAttribute.empty())
					*(t[i].pstrAttributeDestination[a])=strAttribute.c_str();
			}
			for (a=0;t[i].szAttributeName[a] != NULL;a++)
			{
				if (t[i].pstrAttributeDestination[a]->empty())
				{
					tstring strDescription=tstring(t[i].szElementName) + tstring(_T(" ")) + tstring(t[i].szAttributeName[a]) +  tstring(_T(" attribute invalid"));
					throw new CFormatException(CFormatException::formaterrInvalid,strDescription.c_str());
				}
			}
		}
	}
}

void CNTHFile::Read(istream &ar)
{
	const char *password=NULL;
	char *pcDest=NULL;
	unsigned int nFileSize=0;
	CZIPArchive zip;

	if(zip.bOpen(ar))
	{
		TRACEIT2("archive opened sucessfully\n");
		if (zip.bFindFile("theme_descriptor.xml"))
		{
			zip.nExtractCurrentFile(password,&pcDest,&nFileSize);
			if (pcDest)
			{
				ParseXmlData(pcDest,nFileSize);
				delete [] pcDest;
			}
		}
		for (int i=0;i < resLast;i++)
		{
			if (!m_strStdResource[i].empty())
			{
				if (zip.bFindFile(m_strStdResource[i].c_str()))
				{
					pcDest=NULL;
					zip.nExtractCurrentFile(password,&pcDest,&nFileSize);
					if (pcDest)
					{
						poProcessFile(pcDest,nFileSize,m_strStdResource[i].c_str());
						delete [] pcDest;
					}
				}
				else
					throw new CFormatException(CFormatException::formaterrInvalid,"referenced file not found");
			}
		}
		zip.Close();
	}
	else
		throw new CFormatException(CFormatException::formaterrUnknown,_T("failed open archive file"));
}

tstring CNTHFile::sGetFormatName(int nFormat)
{
	map<int,tstring> mapFormatName;
	mapFormatName[1]=tstring(_T("Version 1.0"));
	mapFormatName[2]=tstring(_T("Version 1.1"));
	mapFormatName[3]=tstring(_T("Version 2.0"));
	mapFormatName[4]=tstring(_T("Version > 2.0"));
	return mapFormatName[nFormat];
}

int CNTHFile::nGetSubFormat(const char *szVersion)
{
	tstring strVersion=szVersion;
	int nVersion=0;
	if (strVersion.compare("1.0") == 0)
		nVersion=1;
	else if (strVersion.compare("1.1") == 0)
		nVersion=2;
	else if (strVersion.compare("2.0") == 0)
		nVersion=3;
	else
		nVersion=4;
	return nVersion;
}

bool CNTHFile::bMagicHead(std::istream &ar,uint32_t nSize)
{
	bool bRet=false;
	char *acCompare=new char[m_nMagicSize];
	
	ar.read(acCompare,m_nMagicSize);
	if (!memcmp(acCompare,m_pcMagic,m_nMagicSize))
	{
		CZIPArchive zip;
		if (zip.bOpen(ar))
		{
			bRet=zip.bFindFile("theme_descriptor.xml");
			zip.Close();
		}
	}
	delete [] acCompare;
	return bRet;
}
