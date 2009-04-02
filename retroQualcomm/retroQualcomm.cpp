// retroQualcomm.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
#include "../include/Resource.h"
#include "../retroBase/Basics.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MyString.h"
#include "../retroBase/MobileContent.h"
#include "QCELPFile.h"
#include "CMXFile.h"
#include "../retroBase/Modules.h"
#include "../retroBase/ResourceVersion.h"
#include "Version.h"

#ifdef WIN32
#define RETROQUALCOMM_API __declspec(dllexport)
#else
#define RETROQUALCOMM_API extern "C"
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
 * <---------- szGetRetroQualcommVersion ----------> 
 * @m 
 * <-- OUT --> @r
 * LPCTSTR - get DLL version info
\*/ 
LPCTSTR szGetRetroQualcommVersion(void)
{
#if   RETROQUALCOMM_ALPHA_VERSION
	static const char *const str = XSTR(RETROQUALCOMM_MAJOR_VERSION) "." XSTR(RETROQUALCOMM_MINOR_VERSION) " " "(alpha " XSTR(RETROQUALCOMM_PATCH_VERSION) ", " __DATE__ " " __TIME__ ")";
#elif RETROQUALCOMM_BETA_VERSION
    static LPCTSTR str = XSTR(RETROQUALCOMM_MAJOR_VERSION) "." XSTR(RETROQUALCOMM_MINOR_VERSION) " " "(beta " XSTR(RETROQUALCOMM_PATCH_VERSION) ", " __DATE__ ")";
#elif RETROQUALCOMM_RELEASE_VERSION && (RETROQUALCOMM_PATCH_VERSION > 0)
	static const char *const str = XSTR(RETROQUALCOMM_MAJOR_VERSION) "." XSTR(RETROQUALCOMM_MINOR_VERSION) "." XSTR(RETROQUALCOMM_PATCH_VERSION);
#else
	static const char *const str = XSTR(RETROQUALCOMM_MAJOR_VERSION) "." XSTR(RETROQUALCOMM_MINOR_VERSION);
#endif
	static char version[256];
#ifdef USE_QUALCOMM_LIBRARY
	sprintf(version,"%s\n\tQSCL %s",str,CQCELPFile::szGetQcelpVersion());
#else
	sprintf(version,"%s\n\t%s",str,CQCELPFile::szGetQcelpVersion());
#endif
	return version;
}

RETROQUALCOMM_API void *pCreateCodecInstance(uint32_t idFormat)
{
	LIBFORMATINIT
	LIBFORMAT(CMobileContent::formatCMX,CCMXFile);
	LIBFORMAT(CMobileContent::formatQCELP,CQCELPFile);
	if ((iter=listFormat.find(idFormat)) == listFormat.end())
		return NULL;
	else
		return (void *)listFormat[idFormat]();
}

RETROQUALCOMM_API void *pQueryLibrary(uint32_t idWhat)
{
	static const uint32_t anFormats[]=
	{
		CMobileContent::formatCMX,
		CMobileContent::formatQCELP,
		0
	};
	switch(idWhat)
	{
		case RETROLIBQ_VERSION:
			return (void *)szGetRetroQualcommVersion();
		case RETROLIBQ_FORMATS:
			return (void *)&anFormats[0];
	}
	return NULL;
}
