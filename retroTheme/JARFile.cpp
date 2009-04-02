/*\
 * JARFile.cpp
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
#include "JARFile.h"
#include "JARProperty.h"

DYNIMPPROPERTY(CJARFile,CJARProperty)

CJARFile::CJARFile(void)
{
	m_pcMagic="PK\003\004";
	m_nMagicSize=4;
	m_sFormatName="Java Archive Resource";
}

CJARFile::~CJARFile(void)
{
}

/*\
 * <---------- ParseManifestData ---------->
 * @m parses "theme_descriptor.xml"
 * --> I N <-- @p
 * char *pcDest - xml-data string
 * unsigned int nSize - buffer size
\*/
void CJARFile::ParseManifestData(char *pcDest,unsigned int nSize)
{
	bool bRet=false;
	CMyString strData=pcDest;
	CMyString strValue;
	
	TRACEIT2("Manifest Data: %s\n",strData.c_str());
	if (!strData.sGetValuePairElement("Manifest-Version").empty())
	{
		m_strName=strData.sGetValuePairElement("MIDlet-Name");
		m_strCompany=strData.sGetValuePairElement("MIDlet-Vendor");
		strValue=strData.sGetValuePairElement("MIDlet-Version");

		m_strCLDC=strData.sGetValuePairElement("MicroEdition-Configuration");
		m_strMIDP=strData.sGetValuePairElement("MicroEdition-Profile");

		if (!strValue.empty())
			m_strName+=" V." + strValue;
		TRACEIT2("name: %s\n",m_strName.c_str());
		TRACEIT2("copyright: %s\n",m_strCompany.c_str());
	}
	else
	{
		throw new CFormatException(CFormatException::formaterrInvalid,"no compatible manifest data found");
		TRACEIT2("not a compatible manifest file");
	}
}

void CJARFile::Read(istream &ar)
{
	const char *password=NULL;
	char *pcDest=NULL;
	unsigned int nFileSize=0;
	char szComment[256]="";
	CZIPArchive zip;

	if(zip.bOpen(ar))
	{
		TRACEIT2("archive opened sucessfully\n");
		if (zip.bFindFile("META-INF/MANIFEST.MF"))
		{
			zip.nExtractCurrentFile(password,&pcDest,&nFileSize);
			if (pcDest)
			{
				ParseManifestData(pcDest,nFileSize);
				delete [] pcDest;
			}
		}
		zip.Close();
	}
	else
		throw new CFormatException(CFormatException::formaterrUnknown,_T("failed open archive file"));
}

tstring CJARFile::sGetFormatName(int nFormatId)
{
	tstring strName[]=
	{
		"Java, no manifest",
		"Java 2 Micro Edition: MIDP 1.0",
		"Java 2 Micro Edition: MIDP 2.0",
		"Java 2 Micro Edition"
	};
	return strName[nFormatId&0x3];
}

int CJARFile::nGetSubFormat(void)
{
	int nFormatId=0;
	if (!m_strMIDP.compare("MIDP-1.0"))
		nFormatId=1;
	else if (!m_strMIDP.compare("MIDP-2.0"))
		nFormatId=2;
	else if (!m_strMIDP.empty())
		nFormatId=3;
	if (!m_strCLDC.compare("CLDC-1.0"))
		nFormatId+=4;
	else if (!m_strCLDC.compare("CLDC-1.1"))
		nFormatId+=8;
	return nFormatId;
}

bool CJARFile::bMagicHead(std::istream &ar,uint32_t nSize)
{
	bool bRet=false;
	char *acCompare=new char[m_nMagicSize];
	
	ar.read(acCompare,m_nMagicSize);
	if (!memcmp(acCompare,m_pcMagic,m_nMagicSize))
	{
		CZIPArchive zip;
		if (zip.bOpen(ar))
		{
			bRet=zip.bFindFile("META-INF/MANIFEST.MF");
			zip.Close();
		}
	}
	delete [] acCompare;
	return bRet;
}
