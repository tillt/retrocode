/*\
 * Basics.cpp
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
#include "stdafx.h"
#include <stdlib.h>
#include "Basics.h"
#include "MyString.h"
#ifdef WIN32
#include "Winsock2.h"
#else
#include <netinet/in.h>
#endif
#ifdef ANSI
#include <stdarg.h>
#else                   
#include <varargs.h>
#endif
#include <string.h>
#include <string>
#include <iostream>
#include "../include/Resource.h"
#include "Endian.h"
#include "ResourceVersion.h"
#include "Version.h"

int gnVerbosity=0;
unsigned int CLogVerbosive::m_nCursorCol=0;

unsigned int nMakeID4(const char *cID4)
{
	return  ntohl(MAKEFOURCC(cID4[0],cID4[1],cID4[2],cID4[3]));
}

/*\
 * <---------- *pcSplitID4 ----------> 
 * @m convert a fourcc into a readable string
 * --> I N <-- @p
 * uint32_tnTag - identifier
 * char *pcDest       - char array 
 * <-- OUT --> @r
 * pcDest
\*/
char *pcSplitID4(uint32_t nTag,char *pcDest)
{
	nTag=htonl(nTag);
	pcDest[3]=(unsigned char)(nTag&0x000000FF);
	pcDest[2]=(unsigned char)((nTag&0x0000FF00)>>8);
	pcDest[1]=(unsigned char)((nTag&0x00FF0000)>>16);
	pcDest[0]=(unsigned char)((nTag&0xFF000000)>>24);
	return pcDest;
}

/*\
 *<------------ MyTrace ------------>
 @m display printf-variable string output in stderr
 *--> I N <-- @p
 * const char *pszFormat - string
\*/
#ifdef ANSI
void MyTrace(const char *pszFormat, ... )
#else
void MyTrace(const char *pszFormat, va_list )
#endif
{
	char szOut[16384];
	va_list argList;
#ifdef ANSI
	va_start(argList,pszFormat);
#else
	va_start(argList);
#endif
	vsprintf (szOut,pszFormat,argList);
	va_end( argList );

	string strOut;
	strOut=szOut;
	fprintf (stderr,strOut.c_str());
}

int32_t round(uint64_t nNom, int32_t nDiv)
{
	int32_t nRet=(int32_t)(nNom/nDiv);
	if ((nNom%nDiv) > (nDiv>>1))
		++nRet;
	return nRet;
}

/*\
 * <---------- hexdump ----------> 
 * @m display a memory content using hexadecimal values
 * --> I N <-- @p
 * const char *pcPrefix  - line prefix
 * unsigned char *pcData - data
 * uint32_tnLen    - size
\*/
void hexdump (const char *pcPrefix,unsigned char *pcData,uint32_t nLen)
{
	int nOffset=0;

	if (pcData)
	{
		while (nLen)
		{
			if (nOffset % 16 == 0)
			{
				if (nOffset > 0)
				{
					TRACEIT("\n");
				}
				if (pcPrefix)
				{
					TRACEIT(pcPrefix);
				}
				TRACEIT("%08X: ",nOffset);
			}
			else
			{
				if (nOffset > 0)
				{
					TRACEIT(" ");
				}
				if (nOffset % 4 == 0)
				{
					TRACEIT(" ");
				}
			}
			TRACEIT("%02X",*(pcData++));
			++nOffset;
			--nLen;
		};
		if (nOffset > 0)
			TRACEIT("\n");
	}
}

void SetVerbosity(int nLevel) 
{
	extern int gnVerbosity; 
	gnVerbosity=nLevel;
}


#ifdef WIN32
#ifdef ANSI
void __cdecl CLogVerbosive::operator()(int nLevel, int nResourceId, ...)
#else
void __cdecl CLogVerbosive::operator()(int nLevel, int nResourceId, va_list )
#endif
#else
void CLogVerbosive::operator()(int nLevel, int nResourceId, ...)
#endif
{
	va_list argList;
	CMyString strFormat;
	char szOut[16384];
	char szFormat[16384];
	char szPrefix[16384];
	
	bool bResRet=strFormat.Load(nResourceId);
	ASSERT(bResRet);
	strcpy(szFormat,strFormat.c_str());
	
	if (nLevel <= gnVerbosity)
	{
#ifdef ANSI
		va_start(argList,nResourceId);
#else
		va_start(argList);
#endif
		vsprintf (szOut,szFormat,argList);
		va_end( argList );
		tstring strOut,strPrefix;
		strOut.assign(szOut);
		if (nLevel >= verbLevDebug2)
		{
			strPrefix=tstring(m_pszFileName)+tstring(_T(" - "))+tstring(m_pszFunctionName)+tstring(_T(" : "));
			sprintf(szPrefix," Line: %d ",m_nLineNumber);
			strOut=strPrefix + tstring(szPrefix) + strOut;
		}
		else
		{
			switch(nLevel)
			{
				case verbLevErrors:		strOut=tstring("ERROR: ")+strOut;		break;
				case verbLevWarnings:	strOut=tstring("WARNING: ")+strOut;		break;
			}
		}
		if(m_bAutoLinebreak && m_nCursorCol >= PAGE_COLUMNS)
		{
			m_nCursorCol=0;
			strOut+="\n";
		}
		else
			m_nCursorCol+=(unsigned int)strOut.size();
		fprintf (stdout,strOut.c_str());
	}
}

#ifdef WIN32
#ifdef ANSI
void __cdecl CLogVerbosive::operator()(int nLevel, const char *pszFmt, ...)
#else
void __cdecl CLogVerbosive::operator()(int nLevel, const char *pszFmt, va_list )
#endif
#else
void CLogVerbosive::operator()(int nLevel, const char *pszFmt, ...)
#endif
{
	va_list argList;
	extern int gnVerbosity;
	char szOut[16384];
	char szPrefix[16384];
	
	if (nLevel <= gnVerbosity)
	{
#ifdef ANSI
		va_start(argList,pszFmt);
#else
		va_start(argList);
#endif
		vsprintf (szOut,pszFmt,argList);
		va_end( argList );
		tstring strOut,strPrefix;
		strOut.assign(szOut);
		if (nLevel >= verbLevDebug2)
		{
			strPrefix=tstring(m_pszFileName)+tstring(_T(" - "))+tstring(m_pszFunctionName)+tstring(_T(" : "));
			sprintf(szPrefix," Line: %d ",m_nLineNumber);
			strOut=strPrefix + tstring(szPrefix) + strOut;
		}
		else
		{
			switch(nLevel)
			{
				case verbLevErrors:		strOut=tstring("ERROR: ")+strOut;		break;
				case verbLevWarnings:	strOut=tstring("WARNING: ")+strOut;		break;
			}
		}
		if(m_bAutoLinebreak && m_nCursorCol >= PAGE_COLUMNS)
		{
			m_nCursorCol=0;
			strOut+="\n";
		}
		else
			m_nCursorCol+=(unsigned int)strOut.size();
		fprintf (stdout,strOut.c_str());
	}
}

LPCTSTR szGetRetroBaseVersion(void)
{
#if   RETROBASE_ALPHA_VERSION
	static const char *const str = XSTR(RETROBASE_MAJOR_VERSION) "." XSTR(RETROBASE_MINOR_VERSION) " " "(alpha " XSTR(RETROBASE_PATCH_VERSION) ", " __DATE__ " " __TIME__ ")";
#elif RETROBASE_BETA_VERSION
    static LPCTSTR str = XSTR(RETROBASE_MAJOR_VERSION) "." XSTR(RETROBASE_MINOR_VERSION) " " "(beta " XSTR(RETROBASE_PATCH_VERSION) ", " __DATE__ ")";
#elif RETROBASE_RELEASE_VERSION && (RETROBASE_PATCH_VERSION > 0)
	static const char *const str = XSTR(RETROBASE_MAJOR_VERSION) "." XSTR(RETROBASE_MINOR_VERSION) "." XSTR(RETROBASE_PATCH_VERSION);
#else
	static const char *const str = XSTR(RETROBASE_MAJOR_VERSION) "." XSTR(RETROBASE_MINOR_VERSION);
#endif
	return str;
}
