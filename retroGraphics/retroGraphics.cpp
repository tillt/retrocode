#include "stdafx.h"
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/FIDProperty.h"
#include "../retroBase/ResourceVersion.h"
#include "../retroGraphics/GIFFile.h"
#include "../retroGraphics/JPGFile.h"
#include "../retroGraphics/PNGFile.h"
#include "../retroGraphics/BMPFile.h"
#include "../retroGraphics/MBMFile.h"
#include "../include/Resource.h"
#include "Version.h"
#include "../retroBase/Modules.h"

#ifdef WIN32
#ifdef RETROGRAPHICS_EXPORTS
#define RETROGRAPHICS_API __declspec(dllexport)
#else
#define RETROGRAPHICS_API __declspec(dllimport)
#endif
#else
#define RETROGRAPHICS_API extern "C"
#endif

/*\ 
 * <---------- szGetRetroPanasonicVersion ----------> 
 * @m 
 * <-- OUT --> @r
 * LPCTSTR - get DLL version info
\*/ 
LPCTSTR szGetRetroGraphicsVersion(void)
{
#if   RETROGRAPHICS_ALPHA_VERSION
	static const char *const str = XSTR(RETROGRAPHICS_MAJOR_VERSION) "." XSTR(RETROGRAPHICS_MINOR_VERSION) " " "(alpha " XSTR(RETROGRAPHICS_PATCH_VERSION) ", " __DATE__ " " __TIME__ ")";
#elif RETROGRAPHICS_BETA_VERSION
    static LPCTSTR str = XSTR(RETROGRAPHICS_MAJOR_VERSION) "." XSTR(RETROGRAPHICS_MINOR_VERSION) " " "(beta " XSTR(RETROGRAPHICS_PATCH_VERSION) ", " __DATE__ ")";
#elif RETROGRAPHICS_RELEASE_VERSION && (RETROGRAPHICS_PATCH_VERSION > 0)
	static const char *const str = XSTR(RETROGRAPHICS_MAJOR_VERSION) "." XSTR(RETROGRAPHICS_MINOR_VERSION) "." XSTR(RETROGRAPHICS_PATCH_VERSION);
#else
	static const char *const str = XSTR(RETROGRAPHICS_MAJOR_VERSION) "." XSTR(RETROGRAPHICS_MINOR_VERSION);
#endif
	static char version[256];
	sprintf(version,"%s\n\tlibJPEG %s\n\tlibPNG %s\n\tgifLib%s",str,szGetLibJpegVersion(),szGetLibPngVersion(),szGetLibGifVersion());
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

RETROGRAPHICS_API void *pCreateCodecInstance(uint32_t idFormat)
{
	LIBFORMATINIT
	LIBFORMAT(CMobileContent::formatPNG,CPNGFile);
	LIBFORMAT(CMobileContent::formatJPEG,CJPGFile);
	LIBFORMAT(CMobileContent::formatBMP,CBMPFile);
	LIBFORMAT(CMobileContent::formatGIF,CGIFFile);
	LIBFORMAT(CMobileContent::formatMBM,CMBMFile);
	if ((iter=listFormat.find(idFormat)) == listFormat.end())
		return NULL;
	else
		return (void *)listFormat[idFormat]();
}

RETROGRAPHICS_API void *pQueryLibrary(uint32_t idWhat)
{
	static const uint32_t anFormats[]=
	{
		CMobileContent::formatGIF,
		CMobileContent::formatPNG,
		CMobileContent::formatJPEG,
		CMobileContent::formatBMP,
		CMobileContent::formatMBM,
		0
	};
	switch(idWhat)
	{
		case RETROLIBQ_VERSION:
			return (void *)szGetRetroGraphicsVersion();
		case RETROLIBQ_FORMATS:
			return (void *)&anFormats[0];
	}
	return NULL;
}
