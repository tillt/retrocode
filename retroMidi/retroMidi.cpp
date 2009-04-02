// retroG711.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
#include "../include/Resource.h"
#include "../retroBase/Basics.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MyString.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/ResourceVersion.h"
#include "../retroBase/MIDIFile.h"
#include "../retroBase/MIDIFileWriter.h"
#include "../retroBase/Modules.h"
#include "MIDIFileDoc.h"
#include "Version.h"

#ifdef WIN32
#ifdef RETROMIDI_EXPORTS
#define RETROMIDI_API __declspec(dllexport)
#else
#define RETROMIDI_API __declspec(dllimport)
#endif
#else
#define RETROMIDI_API extern "C"
#endif

/*\ 
 * <---------- szGetRetroPanasonicVersion ----------> 
 * @m 
 * <-- OUT --> @r
 * LPCTSTR - get DLL version info
\*/ 
LPCTSTR szGetRetroMidiVersion(void)
{
#if   RETROMIDI_ALPHA_VERSION
	static const char *const str = XSTR(RETROMIDI_MAJOR_VERSION) "." XSTR(RETROMIDI_MINOR_VERSION) " " "(alpha " XSTR(RETROMIDI_PATCH_VERSION) ", " __DATE__ " " __TIME__ ")";
#elif RETROMIDI_BETA_VERSION
    static LPCTSTR str = XSTR(RETROMIDI_MAJOR_VERSION) "." XSTR(RETROMIDI_MINOR_VERSION) " " "(beta " XSTR(RETROMIDI_PATCH_VERSION) ", " __DATE__ ")";
#elif RETROMIDI_RELEASE_VERSION && (RETROMIDI_PATCH_VERSION > 0)
	static const char *const str = XSTR(RETROMIDI_MAJOR_VERSION) "." XSTR(RETROMIDI_MINOR_VERSION) "." XSTR(RETROMIDI_PATCH_VERSION);
#else
	static const char *const str = XSTR(RETROMIDI_MAJOR_VERSION) "." XSTR(RETROMIDI_MINOR_VERSION);
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

RETROMIDI_API void *pCreateCodecInstance(uint32_t idFormat)
{
	LIBFORMATINIT
	LIBFORMAT(CMobileContent::formatMIDI,CMIDIFileDoc);

	if ((iter=listFormat.find(idFormat)) == listFormat.end())
		return NULL;
	else
		return (void *)listFormat[idFormat]();
}

RETROMIDI_API void *pQueryLibrary(uint32_t idWhat)
{
	static const uint32_t anFormats[]=
	{
		CMobileContent::formatMIDI,
		0
	};
	switch(idWhat)
	{
		case RETROLIBQ_VERSION:
			return (void *)szGetRetroMidiVersion();
		case RETROLIBQ_FORMATS:
			return (void *)&anFormats[0];
	}
	return NULL;
}
