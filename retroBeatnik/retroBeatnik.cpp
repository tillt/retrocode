// retroBeatnik.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
#include "../include/Resource.h"
#include "../retroBase/Basics.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MyString.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/Modules.h"
#include "RMFBasics.h"
#include "RMFFile.h"
#include "../retroBase/ResourceVersion.h"
#include "Version.h"

#ifndef WIN32
#define RETROBEATNIK_API extern "C"
#else
#define RETROBEATNIK_API __declspec(dllexport)
#endif

#ifdef WIN32
BOOL APIENTRY DllMain( HANDLE hModule, 
                       int32_t  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}
#endif

/*\ 
 * <---------- szGetRetroBeatnikVersion ----------> 
 * @m get the version information string
 * <-- OUT --> @r
 * LPCTSTR  - static string pointer
\*/ 
LPCTSTR szGetRetroBeatnikVersion(void)
{
#if   RETROBEATNIK_ALPHA_VERSION
	static LPCTSTR str = XSTR(RETROBEATNIK_MAJOR_VERSION) "." XSTR(RETROBEATNIK_MINOR_VERSION) " " "(alpha " XSTR(RETROBEATNIK_PATCH_VERSION) ", " __DATE__ " " __TIME__ ")";
#elif RETROBEATNIK_BETA_VERSION
    static LPCTSTR str = XSTR(RETROBEATNIK_MAJOR_VERSION)  "."  XSTR(RETROBEATNIK_MINOR_VERSION)  " "  "(beta "  XSTR(RETROBEATNIK_PATCH_VERSION)  ", "  __DATE__  ")";
#elif RETROBEATNIK_RELEASE_VERSION && (RETROBEATNIK_PATCH_VERSION > 0)
	static LPCTSTR str = XSTR(RETROBEATNIK_MAJOR_VERSION) "." XSTR(RETROBEATNIK_MINOR_VERSION) "." XSTR(RETROBEATNIK_PATCH_VERSION);
#else
	static LPCTSTR str = XSTR(RETROBEATNIK_MAJOR_VERSION) "." XSTR(RETROBEATNIK_MINOR_VERSION);
#endif
    return str;
}

RETROBEATNIK_API void *pCreateCodecInstance(uint32_t idFormat)
{
	LIBFORMATINIT
	LIBFORMAT(CMobileContent::formatRMF,CRMFFile);
	if ((iter=listFormat.find(idFormat)) == listFormat.end())
		return NULL;
	else
		return (void *)listFormat[idFormat]();
}

RETROBEATNIK_API void *pQueryLibrary(uint32_t idWhat)
{
	static const uint32_t anFormats[]=
	{
		CMobileContent::formatRMF,
		0
	};
	switch(idWhat)
	{
		case RETROLIBQ_VERSION:
			return (void *)szGetRetroBeatnikVersion();
		case RETROLIBQ_FORMATS:
			return (void *)&anFormats[0];
	}
	return NULL;
}
