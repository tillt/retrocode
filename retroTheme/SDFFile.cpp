/*\
 * SDFFile.cpp
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
#include "SDFFile.h"
#include "SDFProperty.h"

DYNIMPPROPERTY(CSDFFile,CSDFProperty)

CSDFFile::CSDFFile(void) : CThemeBaseContent()
{
	m_pcMagic="PK\003\004";
	m_nMagicSize=4;
	m_sFormatName="Siemens SDF Theme";
}

CSDFFile::~CSDFFile(void)
{
}

/*\
 * <---------- ParseXmlData ---------->
 * @m parses "theme_descriptor.xml"
 * --> I N <-- @p
 * char *pcDest - xml-data string
 * unsigned int nSize - buffer size
\*/
void CSDFFile::ParseXmlData(char *pcDest,unsigned int nSize)
{
	map<tstring,int> mapMetaInfo;
	map<tstring,int> mapFileInfo;
	map<tstring,int> :: const_iterator iter;

	bool bRet=false;
	CMyString strXml;
	CMyString strElement;
	CMyString strName,strContent;

	int iVers=0;
	strXml=pcDest;

	mapMetaInfo["Title"]=resmetaName;
	mapMetaInfo["Author"]=resmetaAuthor;
	mapMetaInfo["Published"]=resmetaPublished;
	mapMetaInfo["Version"]=resmetaVersion;
	mapMetaInfo["Resolution"]=resmetaResolution;
	mapMetaInfo["Vendor"]=resmetaVendor;
	mapMetaInfo["Price"]=resmetaPrice;
	mapMetaInfo["URL"]=resmetaUrl;

	mapFileInfo["Screen saver"]=resScreensaver;
	mapFileInfo["Idle background animation"]=resBackground;
	mapFileInfo["Switch on animation"]=resBootup;
	mapFileInfo["Switch off animation"]=resShutdown;

	TRACEIT2("XML: %s\n",pcDest);
	strElement=strXml.sGetXmlElement("theme_config_info");
	if (strElement.empty())
		throw new CFormatException(CFormatException::formaterrInvalid,_T("theme element missing"));

	strElement=strXml.sGetFirstXmlElement("txt");
	while (!strElement.empty()) 
	{
		strName=strElement.sGetXmlElementAttribute("name");
		if (!strName.empty())
		{
			strName=strName.substr(0,strName.length()-1);
			strContent=strElement.sGetXmlElementAttribute("content");
			TRACEIT2("name: %s, content: %s\n",strName.c_str(),strContent.c_str());
			if ((iter=mapMetaInfo.find(strName)) != mapMetaInfo.end())
				m_strResMeta[iter->second]=strContent;
			strElement=strXml.sGetNextXmlElement("txt");
		}
	};

	strElement=strXml.sGetFirstXmlElement("res");
	while (!strElement.empty()) 
	{
		strName=strElement.sGetXmlElementAttribute("name");
		strContent=strElement.sGetXmlElementAttribute("src");
		TRACEIT2("name: %s, src: %s\n",strName.c_str(),strContent.c_str());
		if ((iter=mapFileInfo.find(strName)) != mapFileInfo.end())
			m_strStdResource[iter->second]=strContent;
		strElement=strXml.sGetNextXmlElement("res");
	};
	
	strElement=strXml.sGetXmlElement("description");
	if (!strElement.empty()) 
		m_strResMeta[resmetaDescription]=strElement.sGetXmlElementAttribute("content");

	strElement=strXml.sGetXmlElement("copyright");
	if (!strElement.empty()) 
		m_strResMeta[resmetaCopyright]=strElement.sGetXmlElementAttribute("content");
}

void CSDFFile::Read(istream &ar)
{
	const char *password=NULL;
	char *pcDest;
	unsigned int nFileSize=0;
	char szComment[256]="";
	CZIPArchive zip;

	if(zip.bOpen(ar))
	{
		TRACEIT2("archive opened sucessfully\n");
		if (zip.bFindFile("config.stc"))
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

bool CSDFFile::bMagicHead(std::istream &ar,uint32_t nSize)
{
	bool bRet=false;
	char *acCompare=new char[m_nMagicSize];
	
	ar.read(acCompare,m_nMagicSize);
	if (!memcmp(acCompare,m_pcMagic,m_nMagicSize))
	{
		CZIPArchive zip;
		if (zip.bOpen(ar))
		{
			bRet=zip.bFindFile("config.stc");
			zip.Close();
		}
	}
	delete [] acCompare;
	return bRet;
}
