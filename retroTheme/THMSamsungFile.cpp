/*\
 * THMSamsungFile.cpp
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
#include "THMSamsungFile.h"
#include "THMSamsungProperty.h"

DYNIMPPROPERTY(CTHMSamsungFile,CTHMSamsungProperty)

CTHMSamsungFile::CTHMSamsungFile(void)
{
	m_pcMagic="PK\003\004";
	m_nMagicSize=4;
	m_sFormatName="Samsung THM Theme";
}

CTHMSamsungFile::~CTHMSamsungFile(void)
{
}

/*\
 * <---------- ParseXmlData ---------->
 * @m parses "theme_descriptor.xml"
 * --> I N <-- @p
 * char *pcDest - xml-data string
 * unsigned int nSize - buffer size
\*/
void CTHMSamsungFile::ParseXmlData(char *pcDest,unsigned int nSize)
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

	strElement=strXml.sGetXmlElement("info");
	if (!strElement.empty())
	{
		m_strName=strElement.sGetXmlElementAttribute("name");
		m_strProfile=strElement.sGetXmlElementAttribute("profile");
	}

	strElement=strXml.sGetXmlElement("wallpaper");
	if (!strElement.empty())
		m_strStdResource[resDesktop]=strElement.sGetXmlElementAttribute("idle");
	strElement=strXml.sGetXmlElement("background");
	if (!strElement.empty())
		m_strStdResource[resBackground]=strElement.sGetXmlElementAttribute("mainmenu");
	strElement=strXml.sGetXmlElement("ringtone");
	if (!strElement.empty())
		m_strStdResource[resRingtone]=strElement.sGetXmlElementAttribute("voice_call");
	strElement=strXml.sGetXmlElement("power_on");
	if (!strElement.empty())
		m_strStdResource[resBootup]=strElement.sGetXmlElementAttribute("image");
	strElement=strXml.sGetXmlElement("power_off");
	if (!strElement.empty())
		m_strStdResource[resShutdown]=strElement.sGetXmlElementAttribute("image");
}

void CTHMSamsungFile::Read(istream &ar)
{
	const char *password=NULL;
	char *pcDest;
	unsigned int nFileSize=0;
	char szComment[256]="";
	CZIPArchive zip;

	if(zip.bOpen(ar))
	{
		TRACEIT2("archive opened sucessfully\n");
		if (zip.bFindFile("theme.xml"))
		{
			pcDest=NULL;
			zip.nExtractCurrentFile(password,&pcDest,&nFileSize);
			if (pcDest)
			{
				ParseXmlData(pcDest,nFileSize);
				delete [] pcDest;
			}
		}
		else
			throw new CFormatException(CFormatException::formaterrInvalid,"no theme xml found");
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

tstring CTHMSamsungFile::sGetFormatName(void)
{
	return "Version " + m_strVersion + " : " + sGetDeviceName();
}

bool CTHMSamsungFile::bMagicHead(std::istream &ar,uint32_t nSize)
{
	bool bRet=false;
	char *acCompare=new char[m_nMagicSize];
	
	ar.read(acCompare,m_nMagicSize);
	if (!memcmp(acCompare,m_pcMagic,m_nMagicSize))
	{
		CZIPArchive zip;
		if (zip.bOpen(ar))
		{
			bRet=zip.bFindFile("theme.xml");
			zip.Close();
		}
	}
	delete [] acCompare;
	return bRet;
}

int CTHMSamsungFile::nGetSubFormat(void)
{
	map<tstring,int> mapDeviceName;
	mapDeviceName["Samsung ZV10"]=1;
	mapDeviceName["Shalimar"]=2;
	mapDeviceName["Supreme(Z520V)"]=3;
	return mapDeviceName[m_strProfile];
}

tstring CTHMSamsungFile::sGetDeviceName(void)
{
	map<tstring,tstring> mapDeviceName;
	mapDeviceName["Samsung ZV10"]="ZV10";
	mapDeviceName["Shalimar"]="Z500";
	mapDeviceName["Supreme(Z520V)"]="ZV50";
	return mapDeviceName[m_strProfile];
}
