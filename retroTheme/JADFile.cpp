/*\
 * JADFile.cpp
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
#include "JADFile.h"
#include "JADProperty.h"

DYNIMPPROPERTY(CJADFile,CJADProperty)

CJADFile::CJADFile(void)
{
	m_sFormatName="Java Archive Descriptor";
}

CJADFile::~CJADFile(void)
{
}

void CJADFile::ParseData(char *pcDest,unsigned int nSize)
{
	bool bRet=false;
	CMyString strData;
	CMyString strValue;

	strData=pcDest;

	TRACEIT2("Data: %s\n",strData.c_str());
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

void CJADFile::Read(istream &ar) throw()
{
	char *pcDest=NULL;
	if (m_nFileSize)
	{
		pcDest=new char[m_nFileSize+1];
		ar.read(pcDest,m_nFileSize);
		pcDest[m_nFileSize]=0;
		ParseData(pcDest,m_nFileSize);
		delete [] pcDest;
	}
}

tstring CJADFile::sGetFormatName(int nFormatId)
{
	tstring strName[]=
	{
		"Java",
		"Java 2 Micro Edition: MIDP 1.0",
		"Java 2 Micro Edition: MIDP 2.0",
		"Java 2 Micro Edition"
	};
	return strName[nFormatId&0x3];
}

int CJADFile::nGetSubFormat(void)
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

bool CJADFile::bMagicHead(std::istream &ar,uint32_t nSize)
{
	bool bRet=false;
	char *pcCompare;
	CMyString strCompare;
	string str;
	if (nSize)
	{
		pcCompare=new char[nSize+1];
		ar.read(pcCompare,nSize);
		pcCompare[nSize]=0;
		strCompare=pcCompare;
		TRACEIT2("Data: %s\n",strCompare.c_str());
		bRet=!strCompare.sGetValuePairElement("MIDlet-1").empty();
		delete [] pcCompare;
	}
	return bRet;
}
