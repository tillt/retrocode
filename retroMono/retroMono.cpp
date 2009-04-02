#include "stdafx.h"
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/FIDProperty.h"
#include "../retroBase/ResourceVersion.h"
#include "../retroBase/Modules.h"
#include "MonoFile.h"
#include "EMSFile.h"
#include "MSEQFile.h"
#include "NokiaFile.h"
#include "MotorolaFile.h"
#include "SagemFile.h"
#include "../include/Resource.h"
#include "Version.h"

#ifdef WIN32
#ifdef RETROMONO_EXPORTS
#define RETROMONO_API __declspec(dllexport)
#else
#define RETROMONO_API __declspec(dllimport)
#endif
#else
#define RETROMONO_API extern "C"
#endif

/*\ 
 * <---------- szGetRetroPanasonicVersion ----------> 
 * @m 
 * <-- OUT --> @r
 * LPCTSTR - get DLL version info
\*/ 
LPCTSTR szGetRetroMonoVersion(void)
{
#if   RETROMONO_ALPHA_VERSION
	static const char *const str = XSTR(RETROMONO_MAJOR_VERSION) "." XSTR(RETROMONO_MINOR_VERSION) " " "(alpha " XSTR(RETROMONO_PATCH_VERSION) ", " __DATE__ " " __TIME__ ")";
#elif RETROMONO_BETA_VERSION
    static LPCTSTR str = XSTR(RETROMONO_MAJOR_VERSION) "." XSTR(RETROMONO_MINOR_VERSION) " " "(beta " XSTR(RETROMONO_PATCH_VERSION) ", " __DATE__ ")";
#elif RETROMONO_RELEASE_VERSION && (RETROMONO_PATCH_VERSION > 0)
	static const char *const str = XSTR(RETROMONO_MAJOR_VERSION) "." XSTR(RETROMONO_MINOR_VERSION) "." XSTR(RETROMONO_PATCH_VERSION);
#else
	static const char *const str = XSTR(RETROMONO_MAJOR_VERSION) "." XSTR(RETROMONO_MINOR_VERSION);
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

RETROMONO_API void *pCreateCodecInstance(uint32_t idFormat)
{
	LIBFORMATINIT
	LIBFORMAT(CMobileContent::formatEMS,CEMSFile);
	LIBFORMAT(CMobileContent::formatMotorola,CMotorolaFile);
	LIBFORMAT(CMobileContent::formatNokia,CNokiaFile);
	LIBFORMAT(CMobileContent::formatMSEQ,CMSEQFile);
	LIBFORMAT(CMobileContent::formatSagem,CSagemFile);
	if ((iter=listFormat.find(idFormat)) == listFormat.end())
		return NULL;
	else
		return (void *)listFormat[idFormat]();
}

RETROMONO_API void *pQueryLibrary(uint32_t idWhat)
{
	static const uint32_t anFormats[]=
	{
		CMobileContent::formatEMS,
		CMobileContent::formatMSEQ,
		CMobileContent::formatNokia,
		CMobileContent::formatSagem,
		CMobileContent::formatMotorola,
		0
	};
	switch(idWhat)
	{
		case RETROLIBQ_VERSION:
			return (void *)szGetRetroMonoVersion();
		case RETROLIBQ_FORMATS:
			return (void *)&anFormats[0];
	}
	return NULL;
}
