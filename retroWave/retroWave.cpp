// retroWave.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
#include "../include/Resource.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MyString.h"
#include "../retroBase/Adpcm.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/Modules.h"
#include "../retroBase/ResourceVersion.h"
#include "WaveFile.h"
#include "RAWFile.h"
#include "AVIFile.h"
#include "Version.h"


#ifdef WIN32
#define RETROWAVE_API __declspec(dllexport)
#else
#define RETROWAVE_API extern "C"
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
 * <---------- szGetRetroWaveVersion ----------> 
 * @m get displayable version information
 * <-- OUT --> @r
 * LPCTSTR - get DLL version info
\*/ 
LPCTSTR szGetRetroWaveVersion(void)
{
#if   RETROWAVE_ALPHA_VERSION
	static const char *const str = XSTR(RETROWAVE_MAJOR_VERSION) "." XSTR(RETROWAVE_MINOR_VERSION) " " "(alpha " XSTR(RETROWAVE_PATCH_VERSION) ", " __DATE__ " " __TIME__ ")";
#elif RETROWAVE_BETA_VERSION
    static LPCTSTR str = XSTR(RETROWAVE_MAJOR_VERSION) "." XSTR(RETROWAVE_MINOR_VERSION) " " "(beta " XSTR(RETROWAVE_PATCH_VERSION) ", " __DATE__ ")";
#elif RETROWAVE_RELEASE_VERSION && (RETROWAVE_PATCH_VERSION > 0)
	static const char *const str = XSTR(RETROWAVE_MAJOR_VERSION) "." XSTR(RETROWAVE_MINOR_VERSION) "." XSTR(RETROWAVE_PATCH_VERSION);
#else
	static const char *const str = XSTR(RETROWAVE_MAJOR_VERSION) "." XSTR(RETROWAVE_MINOR_VERSION);
#endif
	return str;
}

RETROWAVE_API void *pCreateCodecInstance(uint32_t idFormat)
{
	LIBFORMATINIT
	LIBFORMAT(CMobileContent::formatWAV,CWaveFile);
	LIBFORMAT(CMobileContent::formatAVI,CAVIFile);
	LIBFORMAT(CMobileContent::formatRAW,CRAWFile);
	if ((iter=listFormat.find(idFormat)) == listFormat.end())
		return NULL;
	else
		return (void *)listFormat[idFormat]();
}

RETROWAVE_API void *pQueryLibrary(uint32_t idWhat)
{
	static const uint32_t anFormats[]=
	{
		CMobileContent::formatAVI,
		CMobileContent::formatWAV,
		CMobileContent::formatRAW,
		0
	};
	switch(idWhat)
	{
		case RETROLIBQ_VERSION:
			return (void *)szGetRetroWaveVersion();
		case RETROLIBQ_FORMATS:
			return (void *)&anFormats[0];
	}
	return NULL;
}
