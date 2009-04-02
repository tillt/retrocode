/*\
 * MyString
 * Copyright (C) 2002-2004, MMSGURU - written by Till Toenshoff
 * $Id: MyString.h,v 1.5 2009/02/22 20:06:35 lobotomat Exp $
\*/
#include "Basics.h"

#ifndef MyString_Included
#define MyString_Included
#include <map>
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <iostream>
#ifdef WIN32
#define LANG_OLD	LANG_ENGLISH
#undef LANG_ENGLISH
#undef LANG_GERMAN
#include "windows.h"
#undef LANG_ENGLISH 
#undef LANG_GERMAN
#define LANG_ENGLISH	LANG_OLD
#include <tchar.h>
#endif

#ifndef WIN32
typedef char char_t;
typedef char TCHAR;
typedef const char_t * LPCTSTR;
typedef const char * LPCSTR;
#define _T(txt) txt
#else
typedef _TCHAR char_t;
typedef _TCHAR TCHAR;
#endif

#ifdef WIN32
#ifdef _UNICODE
	#define tstring wstring
#else
	#define tstring string
#endif
#else
	#define tstring string
#endif

using namespace std;

#ifndef WIN32
#define GetModuleHandle(a)	
#endif

#ifdef WIN32
#ifndef RETROBASE_EXPORTS
#undef DllExport
#define DllExport 
#else
#undef DllExport
#define DllExport   __declspec( dllexport )
#endif
#else
#undef DllExport
#define DllExport 
#endif
//DllExport bool MyLoadString (unsigned int nID,char *pcDest,int nMaxLen);

class CMyString : public tstring
{
public:
	DllExport CMyString(void);
	DllExport CMyString(const char *pcSource);
	DllExport ~CMyString(void);
	DllExport CMyString(const string &source);
	DllExport CMyString(unsigned int value);
	DllExport CMyString(const CMyString &source);

	DllExport void Format(const char *pszFormat, ... );
	DllExport bool Load(unsigned int nID);
	DllExport bool LoadString(unsigned int nID);
	
	DllExport static void InitStringtable(void);

	DllExport void ToLower(void);
	DllExport void ToUpper(void);
	
	DllExport int nComparePartNoCase(const char *szCompareWith,unsigned int nLength=0);
	DllExport int nCompareRPartNoCase(const char *szCompareWith,unsigned int nLength=0);

	DllExport CMyString sGetXmlElement(const char *szName);
	DllExport CMyString sGetXmlElementAttribute(const char *szName);
	DllExport CMyString sGetXmlElementContent(const char *szName);

	DllExport CMyString sGetFirstXmlElement(const char *szName);
	DllExport CMyString sGetNextXmlElement(const char *szName);
	DllExport CMyString sGetCurrentXmlContent(void);

	DllExport CMyString sGetValuePairElement(const char *szName);

protected:
	static map<int,string> :: const_iterator iterStringResource;
	static map<int,string> mapStringResource;

	size_t m_iAfterLastMatchCh;

public:
	// a simple test function for the first time
	int test(int nTestInt, bool bTextBiik);
};
#endif
