/*\
 * Basics.h
 * Copyright (C) 2004-2008, MMSGURU - written by Till Toenshoff
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
#ifndef BASICS_HEADER
#define BASICS_HEADER
#include <fstream>
#include "Platform.h"
#include "wruntime_error.h"
#include "MyString.h"
#include "Integer.h"

#include <iostream>
#include <vector>

#ifdef ANSI
DllExport void MyTrace(const char *pszFormat, ... );
#else
DllExport void MyTrace(const char *pszFormat, va_list );
#endif

#define Log2 CLogVerbosive(__FUNCTION__,__FILE__,__LINE__)
#define LogLineSynth CLogVerbosive(__FUNCTION__,__FILE__,__LINE__,TRUE)

using namespace std;

#ifdef _DEBUG

#define TRACEIT MyTrace
#define TRACEIT2 MyTrace("%s - ",__FUNCTION__); MyTrace

#ifdef _UNICODE
#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)

#define __TFILE__		WIDEN(__FILE__)
#define __TFUNCTION__	WIDEN(__FILE__)
#else
#define __TFILE__		(__FILE__)
#define __TFUNCTION__	__FILE__
#endif

#ifndef ASSERT
#define ASSERT(c) if(!(c))	throw new assert_error(__TFILE__,__LINE__,__TFUNCTION__,_T("Debug Assertion"))
#endif

#else

#define TRACEIT 
#define TRACEIT2 

#ifndef ASSERT
#define ASSERT(c)	
#endif
#endif

#ifdef WIN32
DllExport const char *szGetDllPath(void);
DllExport void SetDllPath(const char *pszPath);
#endif

DllExport unsigned int nMakeID4(const char *cID4);
DllExport char *pcSplitID4(uint32_t nTag,char *pcDest);

DllExport void hexdump (const char *pcPrefix,unsigned char *pcData,uint32_t nLen);
DllExport int32_t round(uint64_t nNom, int32_t nDiv);
DllExport void SetVerbosity(int nLevel);

DllExport enum verbosityLevels {	verbLevNone=0,
									verbLevErrors=1,
									verbLevWarnings=2,
									//verbLevMessages=verbLevWarnings,
									verbLevMessages,
									verbLevDebug1,
									verbLevDebug2,
									verbLevDebug3};

DllExport LPCTSTR szGetRetroBaseVersion(void);


class CLogVerbosive
{
public:
	DllExport CLogVerbosive(const char *pszFunctionName,const char *pszFileName,const int nLineNumber,bool autoLinebreak=false) : m_pszFunctionName(pszFunctionName),m_pszFileName(pszFileName),m_nLineNumber(nLineNumber),m_bAutoLinebreak(autoLinebreak) {};

#ifdef WIN32
#ifdef ANSI
	DllExport void __cdecl operator()(int nLevel, const char *pszFmt, ...);
	DllExport void __cdecl operator()(int nLevel, int nResourceId, ...);
#else
	DllExport void __cdecl operator()(int nLevel, const char *pszFmt, va_list );
	DllExport void __cdecl operator()(int nLevel, int nResourceId, va_list );
#endif
#else
	void operator()(int nLevel, const char *pszFmt, ... );
	void operator()(int nLevel, int nResourceId, ... );
#endif
private:
	const char *m_pszFunctionName;
	const char *m_pszFileName;
	const int m_nLineNumber;
	const bool m_bAutoLinebreak;
	static unsigned int m_nCursorCol;
};
#ifndef min
#define min(a,b) (((a)<(b))?(a):(b)) 
#endif
#ifndef max
#define max(a,b) (((a)>(b))?(a):(b))
#endif
#define PAGE_COLUMNS	79

#endif
