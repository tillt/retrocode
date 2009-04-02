/*\
 * MyString.cpp
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
#include <algorithm>
#include <cctype>
#include <map>
#include "../include/Resource.h"
#include "MyString.h"

map<int,string> :: const_iterator CMyString::iterStringResource;
map<int,string> CMyString::mapStringResource;


CMyString::CMyString(void)
{
}

CMyString::CMyString(const string &source)
{
	this->assign(source.c_str());
}

CMyString::CMyString(const CMyString &source)
{
	this->assign(source.c_str());
}


CMyString::CMyString(const char *pcSource)
{
	ASSERT(pcSource);
	this->assign(pcSource);
}

CMyString::CMyString(unsigned int source)
{
	unsigned int n=source;
	unsigned int nVal;
	TCHAR buffer[5];
	string str;
	while (n)
	{
		nVal=n%1000;
		if (n > 999)
		{
			n=(n-nVal)/1000;
			_stprintf(buffer,_T(".%03d"),nVal);
		}
		else
		{
			n-=nVal;
			_stprintf(buffer,_T("%d"),nVal);
		}
		str=string((char *)buffer)+str;
	};
	this->assign(str);
}

CMyString::~CMyString(void)
{
}

void CMyString::Format(const char *pszFormat, ... )
{
	TCHAR szOut[32000];
	va_list argList;

	va_start(argList,pszFormat);
	_vstprintf (szOut,pszFormat,argList);
	va_end( argList );
	this->assign(szOut);
}

bool CMyString::LoadString(unsigned int nID)
{
	return Load(nID);
}

bool CMyString::Load(unsigned int nID)
{
	bool bRet=false;
	this->assign(_T(""));
	if ((iterStringResource=mapStringResource.find(nID)) != mapStringResource.end())
	{
		this->assign(iterStringResource->second.c_str());
		bRet=true;
	}
	else
	{
		Log2(verbLevWarnings,"Resource with the ID %d could not be loaded\n",nID);
	}
	return bRet;
}

/*
#ifdef WIN32
#ifndef UNICODE
void CMyString::ToNarrowString( const wchar_t* pStr , int len )
{
    // figure out how many narrow characters we are going to get 
    int nChars = WideCharToMultiByte( CP_ACP , 0 , pStr , len , NULL , 0 , NULL , NULL ) ; 
    if ( len == -1 )
        --nChars ; 
    if ( nChars == 0 )
        this->assign(_T(""));

    // convert the wide string to a narrow string
    // nb: slightly naughty to write directly into the string like this
	this->resize(nChars);
    WideCharToMultiByte( CP_ACP , 0 , pStr , len , const_cast<char*>(this->c_str()) , nChars , NULL , NULL ); 
}
#endif
#endif
*/
void CMyString::InitStringtable(void)
{
#define MAPSTRING(a,b) mapStringResource[a]=_T(b); 
#include "MyLanguage.h"
}

void CMyString::ToLower(void)
{
	transform(this->begin(),this->end(),this->begin(),(int(*)(int))tolower);
}

void CMyString::ToUpper(void)
{
	transform(this->begin(),this->end(),this->begin(),(int(*)(int))toupper);
}

int CMyString::nComparePartNoCase(const char *szCompareWith,unsigned int nLength)
{
	CMyString strMe=*this;
	CMyString strCompareWith=szCompareWith;
	if (nLength == 0)
		nLength=(unsigned int)strCompareWith.length();
	else
		strCompareWith=strCompareWith.substr(0,nLength).c_str();
	strCompareWith.ToLower();
	strMe=strMe.substr(0,nLength).c_str();
	strMe.ToLower();
	return strMe.compare(strCompareWith);
}

int CMyString::nCompareRPartNoCase(const char *szCompareWith,unsigned int nLength)
{
	CMyString strMe=*this;
	CMyString strCompareWith=szCompareWith;
	if (nLength == 0)
		nLength=(unsigned int)strCompareWith.length();
	strCompareWith=strCompareWith.substr(0,nLength).c_str();
	strCompareWith.ToLower();

	strMe=strMe.substr(strMe.length()-nLength).c_str();
	strMe.ToLower();
	return strMe.compare(strCompareWith);
}

// a simple test function for the first time
int CMyString::test(int nTestInt, bool bTextBiik)
{
	return 0;
}

/*\
 * <---------- sGetXmlElementAttribute ---------->
 * @m get an attribute value from a xml element
 * --> I N <-- @p
 * tstring &strXml - xml source element data
 * const char *szName - attribute name
 * <-- OUT --> @r
 * tstring - attribute value
\*/
CMyString CMyString::sGetXmlElementAttribute(const char *szName)
{
	basic_string <char>::size_type indexCh;
	tstring strName=szName;
	tstring strValue="";
	strName+="=\"";
	//found 'ATTRIBUTE="' within this string?
	if ((indexCh=this->find(strName)) != string::npos)
	{	//yes->remember the index
		indexCh+=strName.length();
		//repeat until we found a closing quote
		while(indexCh < size() && this->at(indexCh) != '\"')
		{
			strValue+=this->at(indexCh);
			++indexCh;
		};
		TRACEIT2("attribute : %s\n\n",strValue.c_str());
	}
	return strValue;
}

CMyString CMyString::sGetXmlElementContent(const char *szName)
{
	string strElement;
	string strXml;
	string strContent;
	unsigned int indexCh=0;
	strElement=sGetXmlElement(szName);
	if (!strElement.empty())
	{
		strXml=substr(m_iAfterLastMatchCh);
		while (indexCh < strXml.size() && strXml.at(indexCh) != '>')
			++indexCh;
		if (++indexCh < strXml.size())
		{
			strXml=strXml.substr(indexCh);
			indexCh=0;
			while (indexCh+1 < strXml.size() && !(strXml.at(indexCh) == '<' && strXml.at(indexCh+1) == '/'))
			{
				strContent+=strXml.at(indexCh);
				++indexCh;
			};
		}
	}
	return strContent;
}

CMyString CMyString::sGetCurrentXmlContent(void)
{
	string strElement;
	string strXml;
	string strContent;
	signed int nOpenBraces=0;
	unsigned int indexCh=0;
	if (m_iAfterLastMatchCh != string::npos)
	{
		strXml=substr(m_iAfterLastMatchCh);
		while (indexCh < strXml.size() && strXml.at(indexCh) != '>')
			++indexCh;
		if (++indexCh < strXml.size())
		{
			strXml=strXml.substr(indexCh);
			indexCh=0;
			while (indexCh+1 < strXml.size())
			{
				switch(strXml.at(indexCh))
				{
					case '/':
						if (strXml.at(indexCh+1) == '>')
						{
							if (nOpenBraces > 0)
								--nOpenBraces;
						}
					break;
					case '<':
						if (strXml.at(indexCh+1) == '/')
						{
							if (nOpenBraces == 0)
								return strContent;
							else
								--nOpenBraces;
						}
						else
							++nOpenBraces;
					break;
				}
				strContent+=strXml.at(indexCh);
				++indexCh;
			};
		}
	}
	return strContent;
}



/*\
 * <---------- sGetXmlElement ---------->
 * @m retrieve one specific element (opener)
 * --> I N <-- @p
 * tstring &strXml - xml source data
 * const char *szName - element name
 * <-- OUT --> @r
 * tstring - element string
\*/
CMyString CMyString::sGetXmlElement(const char *szName)
{
	return sGetFirstXmlElement(szName);
}

CMyString CMyString::sGetNextXmlElement(const char *szName)
{
	basic_string <char>::size_type indexCh;
	tstring strName="<";
	string strElement="";

	strName+=szName;
	if (m_iAfterLastMatchCh != string::npos && (indexCh=this->find(strName,m_iAfterLastMatchCh)) != string::npos)
	{
		m_iAfterLastMatchCh=indexCh+1;
		while(this->at(indexCh) != '>')
		{
			strElement+=this->at(indexCh);
			++indexCh;
		};
		if (this->at(indexCh) == '>' && !strElement.empty())
			strElement+=this->at(indexCh);
		TRACEIT2("element : %s\n\n",strElement.c_str());
	}
	else
		m_iAfterLastMatchCh=string::npos;
	return strElement;
}

CMyString CMyString::sGetFirstXmlElement(const char *szName)
{
	m_iAfterLastMatchCh=0;
	return sGetNextXmlElement(szName);
}

CMyString CMyString::sGetValuePairElement(const char *szName)
{
	basic_string <char>::size_type indexCh;
	tstring strName;
	string strValue="";

	strName=szName;
	strName+=":";
	if ((indexCh=this->find(strName)) != string::npos)
	{
		indexCh+=strName.length();
		while(this->at(indexCh) == ' ' && indexCh < this->length() && this->at(indexCh) != 0x0D)
			++indexCh;
		while(indexCh < this->length() && this->at(indexCh) != 0x0D)
			strValue+=this->at(indexCh++);
		TRACEIT2("value : %s\n",strValue.c_str());
	}
	return strValue;
}
