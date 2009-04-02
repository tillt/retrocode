// retroPanasonic.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
#include "../include/Resource.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MyString.h"
#include "../retroBase/MobileContent.h"
#include "AIFFile.h"
#include "../retroBase/Modules.h"
#include "../retroBase/ResourceVersion.h"
#include "Version.h"

#ifndef WIN32
#define RETROAPPLE_API extern "C"
#else
#define RETROAPPLE_API __declspec(dllexport)
#endif

/*\ 
 * <---------- szGetRetroPanasonicVersion ----------> 
 * @m 
 * <-- OUT --> @r
 * LPCTSTR - get DLL version info
\*/ 
LPCTSTR szGetRetroAppleVersion(void)
{
#if   RETROAPPLE_ALPHA_VERSION
	static const char *const str = XSTR(RETROAPPLE_MAJOR_VERSION) "." XSTR(RETROAPPLE_MINOR_VERSION) " " "(alpha " XSTR(RETROAPPLE_PATCH_VERSION) ", " __DATE__ " " __TIME__ ")";
#elif RETROAPPLE_BETA_VERSION
    static LPCTSTR str = XSTR(RETROAPPLE_MAJOR_VERSION) "." XSTR(RETROAPPLE_MINOR_VERSION) " " "(beta " XSTR(RETROAPPLE_PATCH_VERSION) ", " __DATE__ ")";
#elif RETROAPPLE_RELEASE_VERSION && (RETROAPPLE_PATCH_VERSION > 0)
	static const char *const str = XSTR(RETROAPPLE_MAJOR_VERSION) "." XSTR(RETROAPPLE_MINOR_VERSION) "." XSTR(RETROAPPLE_PATCH_VERSION);
#else
	static const char *const str = XSTR(RETROAPPLE_MAJOR_VERSION) "." XSTR(RETROAPPLE_MINOR_VERSION);
#endif
	return str;
}

#ifdef WIN32
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}
#endif


RETROAPPLE_API void *pCreateCodecInstance(uint32_t idFormat)
{
	LIBFORMATINIT
	LIBFORMAT(CMobileContent::formatAIF,CAIFFile);
	if ((iter=listFormat.find(idFormat)) == listFormat.end())
		return NULL;
	else
		return (void *)listFormat[idFormat]();
}

RETROAPPLE_API void *pQueryLibrary(uint32_t idWhat)
{
	static const uint32_t anFormats[]=
	{
		CMobileContent::formatAIF,
		0
	};
	switch(idWhat)
	{
		case RETROLIBQ_VERSION:
			return (void *)szGetRetroAppleVersion();
		case RETROLIBQ_FORMATS:
			return (void *)&anFormats[0];
	}
	return NULL;
}
