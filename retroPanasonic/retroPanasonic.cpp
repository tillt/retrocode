// retroPanasonic.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MyString.h"
#include "../retroBase/MobileContent.h"
#include "../retroPanasonic/MFMFile.h"
#include "../retroPanasonic/VOXFile.h"
#include "../include/Resource.h"
#include "../retroBase/Modules.h"
#include "../retroBase/ResourceVersion.h"
#include "Version.h"

#ifdef WIN32
#define RETROPANASONIC_API __declspec(dllexport)
#else
#define RETROPANASONIC_API extern "C"
#endif

/*\ 
 * <---------- szGetRetroPanasonicVersion ----------> 
 * @m 
 * <-- OUT --> @r
 * LPCTSTR - get DLL version info
\*/ 
LPCTSTR szGetRetroPanasonicVersion(void)
{
#if   RETROPANASONIC_ALPHA_VERSION
	static const char *const str = XSTR(RETROPANASONIC_MAJOR_VERSION) "." XSTR(RETROPANASONIC_MINOR_VERSION) " " "(alpha " XSTR(RETROPANASONIC_PATCH_VERSION) ", " __DATE__ " " __TIME__ ")";
#elif RETROPANASONIC_BETA_VERSION
    static LPCTSTR str = XSTR(RETROPANASONIC_MAJOR_VERSION) "." XSTR(RETROPANASONIC_MINOR_VERSION) " " "(beta " XSTR(RETROPANASONIC_PATCH_VERSION) ", " __DATE__ ")";
#elif RETROPANASONIC_RELEASE_VERSION && (RETROPANASONIC_PATCH_VERSION > 0)
	static const char *const str = XSTR(RETROPANASONIC_MAJOR_VERSION) "." XSTR(RETROPANASONIC_MINOR_VERSION) "." XSTR(RETROPANASONIC_PATCH_VERSION);
#else
	static const char *const str = XSTR(RETROPANASONIC_MAJOR_VERSION) "." XSTR(RETROPANASONIC_MINOR_VERSION);
#endif
	return str;
}

#ifdef WIN32
BOOL APIENTRY DllMain( HANDLE hModule, 
                       int32_t  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}
#endif

RETROPANASONIC_API void *pCreateCodecInstance(uint32_t idFormat)
{
	LIBFORMATINIT
	LIBFORMAT(CMobileContent::formatMFM,CMFMFile);
	LIBFORMAT(CMobileContent::formatVOX,CVOXFile);

	if ((iter=listFormat.find(idFormat)) == listFormat.end())
		return NULL;
	else
		return (void *)listFormat[idFormat]();
}

RETROPANASONIC_API void *pQueryLibrary(uint32_t idWhat)
{
	static const uint32_t anFormats[]=
	{
		CMobileContent::formatMFM,
		CMobileContent::formatVOX,
		0
	};
	switch(idWhat)
	{
		case RETROLIBQ_VERSION:
			return (void *)szGetRetroPanasonicVersion();
		case RETROLIBQ_FORMATS:
			return (void *)&anFormats[0];
	}
	return NULL;
}
