// retroYamaha.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
#include "../include/Resource.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MyString.h"
#include "../retroBase/MobileContent.h"
#include "SMAFDecoder.h"
#include "SMAFFile.h"
#include "../retroBase/Modules.h"
#include "../retroBase/ResourceVersion.h"
#include "Version.h"

#ifdef WIN32
#define RETROYAMAHA_API __declspec(dllexport)
#else
#define RETROYAMAHA_API extern "C"
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


LPCTSTR szGetRetroYamahaVersion(void)
{
#if   RETROYAMAHA_ALPHA_VERSION
	static const char *const str = XSTR(RETROYAMAHA_MAJOR_VERSION) "." XSTR(RETROYAMAHA_MINOR_VERSION) " " "(alpha " XSTR(RETROYAMAHA_PATCH_VERSION) ", " __DATE__ " " __TIME__ ")";
#elif RETROYAMAHA_BETA_VERSION
    static LPCTSTR str = XSTR(RETROYAMAHA_MAJOR_VERSION) "." XSTR(RETROYAMAHA_MINOR_VERSION) " " "(beta " XSTR(RETROYAMAHA_PATCH_VERSION) ", " __DATE__ ")";
#elif RETROYAMAHA_RELEASE_VERSION && (RETROYAMAHA_PATCH_VERSION > 0)
	static const char *const str = XSTR(RETROYAMAHA_MAJOR_VERSION) "." XSTR(RETROYAMAHA_MINOR_VERSION) "." XSTR(RETROYAMAHA_PATCH_VERSION);
#else
	static const char *const str = XSTR(RETROYAMAHA_MAJOR_VERSION) "." XSTR(RETROYAMAHA_MINOR_VERSION);
#endif
    return str;
}

RETROYAMAHA_API void *pCreateCodecInstance(int idFormat)
{
	LIBFORMATINIT
	LIBFORMAT(CMobileContent::formatSMAF,CSMAFFile);
	if ((iter=listFormat.find(idFormat)) == listFormat.end())
		return NULL;
	else
		return (void *)listFormat[idFormat]();
}

RETROYAMAHA_API void *pQueryLibrary(int idWhat)
{
	static const int anFormats[]=
	{
		CMobileContent::formatSMAF,
		0
	};
	switch(idWhat)
	{
		case RETROLIBQ_VERSION:
			return (void *)szGetRetroYamahaVersion();
		case RETROLIBQ_FORMATS:
			return (void *)&anFormats[0];
	}
	return NULL;
}
