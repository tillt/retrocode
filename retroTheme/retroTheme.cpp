#include "stdafx.h"
#include "../retroBase/Integer.h"
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/ResourceVersion.h"
#include "../retroBase/Modules.h"

#include "../include/Resource.h"

#include "unzip/unzip.h"
#include "zlib.h"

#include "Version.h"
#include "ZIPArchive.h"

#include "SISFile.h"
#include "THMFile.h"
#include "UTZFile.h"
#include "MTFFile.h"
#include "JARFile.h"
#include "NTHFile.h"
#include "JADFile.h"
#include "SDFFile.h"
#include "THMSamsungFile.h"
//#include "ITAFile.h"

#ifdef WIN32
#ifdef RETROTHEME_EXPORTS
#define RETROTHEME_API __declspec(dllexport)
#else
#define RETROTHEME_API __declspec(dllimport)
#endif
#else
#define RETROTHEME_API extern "C"
#endif

/*\ 
 * <---------- szGetRetroPanasonicVersion ----------> 
 * @m 
 * <-- OUT --> @r
 * LPCTSTR - get DLL version info
\*/ 
LPCTSTR szGetRetroThemeVersion(void)
{
#if   RETROTHEME_ALPHA_VERSION
	static const char *const str = XSTR(RETROTHEME_MAJOR_VERSION) "." XSTR(RETROTHEME_MINOR_VERSION) " " "(alpha " XSTR(RETROTHEME_PATCH_VERSION) ", " __DATE__ " " __TIME__ ")";
#elif RETROTHEME_BETA_VERSION
    static LPCTSTR str = XSTR(RETROTHEME_MAJOR_VERSION) "." XSTR(RETROTHEME_MINOR_VERSION) " " "(beta " XSTR(RETROTHEME_PATCH_VERSION) ", " __DATE__ ")";
#elif RETROTHEME_RELEASE_VERSION && (RETROTHEME_PATCH_VERSION > 0)
	static const char *const str = XSTR(RETROTHEME_MAJOR_VERSION) "." XSTR(RETROTHEME_MINOR_VERSION) "." XSTR(RETROTHEME_PATCH_VERSION);
#else
	static const char *const str = XSTR(RETROTHEME_MAJOR_VERSION) "." XSTR(RETROTHEME_MINOR_VERSION);
#endif
	static char version[256];
	sprintf(version,"%s\n\tzlib %s",str,szGetZlibVersion());
	return version;
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


RETROTHEME_API void *pCreateCodecInstance(uint32_t idFormat)
{
	LIBFORMATINIT
	LIBFORMAT(CMobileContent::formatSIS,CSISFile);
	LIBFORMAT(CMobileContent::formatTHM,CTHMFile);
	LIBFORMAT(CMobileContent::formatTHMS,CTHMSamsungFile);
	LIBFORMAT(CMobileContent::formatUTZ,CUTZFile);
	LIBFORMAT(CMobileContent::formatNTH,CNTHFile);
	LIBFORMAT(CMobileContent::formatSDF,CSDFFile);
	LIBFORMAT(CMobileContent::formatMTF,CMTFFile);
	LIBFORMAT(CMobileContent::formatJAR,CJARFile);
	LIBFORMAT(CMobileContent::formatJAD,CJADFile);
//	LIBFORMAT(CMobileContent::formatITA,CITAFile);
	if ((iter=listFormat.find(idFormat)) == listFormat.end())
		return NULL;
	else
		return (void *)listFormat[idFormat]();
}

RETROTHEME_API void *pQueryLibrary(uint32_t idWhat)
{
	static const uint32_t anFormats[]=
	{
		CMobileContent::formatSIS,
		CMobileContent::formatTHM,
		CMobileContent::formatTHMS,
		CMobileContent::formatNTH,
		CMobileContent::formatUTZ,
		CMobileContent::formatSDF,
		CMobileContent::formatMTF,
		CMobileContent::formatJAR,
		CMobileContent::formatJAD,
//		CMobileContent::formatITA,
		0
	};
	switch(idWhat)
	{
		case RETROLIBQ_VERSION:
			return (void *)szGetRetroThemeVersion();
		case RETROLIBQ_FORMATS:
			return (void *)&anFormats[0];
	}
	return NULL;
}
