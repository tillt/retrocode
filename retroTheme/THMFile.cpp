/*\
 * THMFile.cpp - Sony-Ericsson THM Theme File en/decoding
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
#include "TARArchive.h"
#include "THMFile.h"
#include "THMProperty.h"

DYNIMPPROPERTY(CTHMFile,CTHMProperty)

CTHMFile::CTHMFile(void)
{
	m_pcMagic="Theme.xml\000";
	m_nMagicSize=10;
	m_sFormatName="Sony-Ericsson THM Theme";
}

CTHMFile::~CTHMFile(void)
{
}

/*\
 * <---------- ParseXmlData ---------->
 * @m parses "theme_descriptor.xml"
 * --> I N <-- @p
 * char *pcDest - xml-data string
 * unsigned int nSize - buffer size
\*/
void CTHMFile::ParseXmlData(char *pcDest,unsigned int nSize)
{
	typedef struct tagELEMENTTABLE
	{
		const char *szElementName[2];
		tstring *pstrAttributeDestination;
	}ElementTable;

	bool bRet=false;
	CMyString strXml;
	CMyString strElement;
	CMyString strAttribute;
	int iVers=0,iElements=0,i;

	strXml=pcDest;
	TRACEIT2("XML: %s\n",pcDest);
	strElement=strXml.sGetXmlElement("Sony_Ericsson_theme");
	if (strElement.empty())
		throw new CFormatException(CFormatException::formaterrInvalid,_T("theme element missing"));
	m_strVersion=strElement.sGetXmlElementAttribute("version");
	if (m_strVersion.empty())
		m_strVersion="1.0";
	
	if (m_strVersion == "1.0" || m_strVersion == "2.0")
		iElements=0;
	else
		iElements=1;

	strElement=strXml.sGetXmlElement("Author_firstname");
	if (!strElement.empty())
		m_strFirstName=strElement.sGetXmlElementAttribute("value");
	strElement=strXml.sGetXmlElement("Author_lastname");
	if (!strElement.empty())
		m_strLastName=strElement.sGetXmlElementAttribute("value");
	strElement=strXml.sGetXmlElement("Author_organisation");
	if (!strElement.empty())
		m_strCompany=strElement.sGetXmlElementAttribute("value");
	strElement=strXml.sGetXmlElement("Author_email");
	if (!strElement.empty())
		m_strEmail=strElement.sGetXmlElementAttribute("value");

	ElementTable t[]=
	{
		{"Desktop_background_image",	"Desktop_image",		&m_strStdResource[resDesktop]		},
		{"Wallpaper_image",				"Standby_image",		&m_strStdResource[resScreensaver]	},
		{"General_background_image",	"Background_image",		&m_strStdResource[resBackground]	},
		{NULL,NULL,NULL}
	};
	for (i=0;t[i].szElementName[iElements] != NULL;i++)
	{
		strElement=strXml.sGetXmlElement(t[i].szElementName[iElements]);
		if (!strElement.empty())
			*(t[i].pstrAttributeDestination)=strElement.sGetXmlElementAttribute("Source");
	}	
	strElement=strXml.sGetXmlElement("Ring_signal");
	if (!strElement.empty())
		m_strStdResource[resRingtone]=strElement.sGetXmlElementAttribute("Source");
}

void CTHMFile::Read(istream &ar)
{
	const char *filename_to_extract=NULL;
	const char *password=NULL;
	const char *dirname=NULL;
	char *pcDest;
	unsigned int nFileSize=0;
	char szDevice[32];
	CTARArchive tar;

	pcDest=NULL;
	tar.ExtractOneFile(ar,"Theme.xml",&pcDest,&nFileSize,szDevice);
	if (pcDest)
	{
		m_strDeviceName=szDevice;
		ParseXmlData(pcDest,nFileSize);
		delete [] pcDest;
	}
	ar.seekg(0,ios_base::beg);
	for (int i=0;i < resLast;i++)
	{
		if (!m_strStdResource[i].empty())
		{
			pcDest=NULL;
			tar.ExtractOneFile(ar,m_strStdResource[i].c_str(),&pcDest,&nFileSize);
			if (pcDest)
			{
				poProcessFile(pcDest,nFileSize,m_strStdResource[i].c_str());
				delete [] pcDest;
			}
		}
	}
}

tstring CTHMFile::sGetFormatName(void)
{
	return "Version " + m_strVersion + " : " + m_strDeviceName;
}

int CTHMFile::nGetSubFormat(void)
{
	map<tstring,int> mapDeviceName;
	//version 1.0
	mapDeviceName["T68i"]=1;
	mapDeviceName["T300"]=2;
	mapDeviceName["T310"]=3;
	mapDeviceName["T230"]=4;
	mapDeviceName["T290"]=5;
	mapDeviceName["T226"]=46;
	mapDeviceName["T290i"]=47;
	//version 2.0
	mapDeviceName["T610"]=6;
	mapDeviceName["T610+AC8-T616+AC8-T618"]=48;
	mapDeviceName["T630"]=7;
	mapDeviceName["Z600"]=8;
	mapDeviceName["J210"]=9;
	mapDeviceName["Z300"]=10;
	mapDeviceName["J220"]=11;
	mapDeviceName["J230"]=12;
	mapDeviceName["T637"]=45;
	//version 3.0
	mapDeviceName["Z1010"]=13;
	mapDeviceName["K700"]=14;
	mapDeviceName["S700"]=15;
	mapDeviceName["K500"]=16;
	mapDeviceName["Z500"]=17;
	mapDeviceName["K300"]=18;
	mapDeviceName["J300"]=19;
	mapDeviceName["S710"]=43;
	mapDeviceName["F500"]=44;
	//version 3.1
	mapDeviceName["V800+ACA-Standard"]=20;
	mapDeviceName["Z800+ACA-Standard"]=21;
	//version 3.2
	mapDeviceName["V800+ACA-Organic"]=22;
	mapDeviceName["Z800+ACA-Organic"]=23;
	//version 4.0
	mapDeviceName["K750"]=24;
	mapDeviceName["W800"]=25;
	mapDeviceName["W700"]=26;
	mapDeviceName["K600"]=27;
	mapDeviceName["Z520"]=28;
	mapDeviceName["Z525"]=29;
	//version 4.1
	mapDeviceName["W900"]=36;
	mapDeviceName["Z550"]=37;
	mapDeviceName["Z530"]=38;
	mapDeviceName["W300"]=39;
	mapDeviceName["K510"]=40;
	mapDeviceName["W810"]=41;
	mapDeviceName["W600"]=42;
	//version 4.5
	mapDeviceName["K610"]=30;
	mapDeviceName["K800"]=31;
	mapDeviceName["K790"]=32;
	mapDeviceName["W850"]=33;
	mapDeviceName["Z710"]=34;
	mapDeviceName["W710"]=35;

	return mapDeviceName[m_strDeviceName];
}

tstring CTHMFile::sGetAuthor(void)
{
	tstring strRet="";

	strRet=m_strFirstName;
	if (!m_strFirstName.empty() &&  !m_strLastName.empty())
		strRet+=" ";
	strRet+=m_strLastName;
	return strRet;
}
