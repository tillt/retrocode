// retroMpeg.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
#include "../include/Resource.h"
#include "../retroBase/Basics.h"
#include "../retroBase/Platform.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MyString.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/Endian.h"
#include "../retroBase/PacketCollection.h"
#include "../retroBase/Modules.h"
#include "../retroBase/ResourceVersion.h"

#ifdef __cplusplus
extern "C"{
#endif 
// "FFMPEG avformat: " FFMPEG_INC_AVFORMAT " \n"
#include "ffmpeg/avformat.h"
#include "ffmpeg/avcodec.h"
#ifdef __cplusplus
}
#endif
#include "MP3File.h"
#include "FFMPEGFile.h"
#include "MP4File.h"
#include "AMRFile.h"
#include "SWFFile.h"
#include "WMAFile.h"
#include "OGGFile.h"
#include "RAFile.h"
#include "AACFile.h"
#include "Mp4Export.h"
#include "Version.h"

#ifdef WIN32
#define RETROMPEG_API __declspec(dllexport)
#else
#define RETROMPEG_API extern "C"
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

LPCTSTR szGetRetroMpegVersion(void)
{
	static char szReturn[1024];

#if   RETROMPEG_ALPHA_VERSION
	static const char *const str = XSTR(RETROMPEG_MAJOR_VERSION) "." XSTR(RETROMPEG_MINOR_VERSION) " " "(alpha " XSTR(RETROMPEG_PATCH_VERSION) ", " __DATE__ " " __TIME__ ")";
#elif RETROMPEG_BETA_VERSION
    static LPCTSTR str = XSTR(RETROMPEG_MAJOR_VERSION) "." XSTR(RETROMPEG_MINOR_VERSION) " " "(beta " XSTR(RETROMPEG_PATCH_VERSION) ", " __DATE__ ")";
#elif RETROMPEG_RELEASE_VERSION && (RETROMPEG_PATCH_VERSION > 0)
	static const char *const str = XSTR(RETROMPEG_MAJOR_VERSION) "." XSTR(RETROMPEG_MINOR_VERSION) "." XSTR(RETROMPEG_PATCH_VERSION);
#else
	static const char *const str = XSTR(RETROMPEG_MAJOR_VERSION) "." XSTR(RETROMPEG_MINOR_VERSION);
#endif
	strcpy(szReturn,str);
	strcat(szReturn,"\n\t");
	strcat(szReturn,szGetLameVersion());
	strcat(szReturn,"\n\t");
	strcat(szReturn,szGetMadVersion());
	strcat(szReturn,"\n\t");
	strcat(szReturn,szGetFAADVersion());
	strcat(szReturn,"\n\t");
	strcat(szReturn,szGetMPEG4IPVersion());
	strcat(szReturn,"\n\t");
	strcat(szReturn,szGetFAACVersion());
	strcat(szReturn,"\n\t");
	strcat(szReturn,szGetFFMPEGVersion());
	return (LPCTSTR)szReturn;
}

RETROMPEG_API void *pCreateCodecInstance(int idFormat)
{
	LIBFORMATINIT
	LIBFORMAT(CMobileContent::formatAAC,CAACFile);
	LIBFORMAT(CMobileContent::formatMP3,CMP3File);
	LIBFORMAT(CMobileContent::formatMP4,CMP4File);
	LIBFORMAT(CMobileContent::format3GP,C3GPPFile);
//	LIBFORMAT(CMobileContent::format3G2,C3GP2File);
	LIBFORMAT(CMobileContent::formatAMR,CAMRFile);
	LIBFORMAT(CMobileContent::formatAWB,CAWBFile);
	LIBFORMAT(CMobileContent::formatOGG,COGGFile);
	LIBFORMAT(CMobileContent::formatWMA,CWMAFile);
	LIBFORMAT(CMobileContent::formatSWF,CSWFFile);
	LIBFORMAT(CMobileContent::formatRA,CRAFile);
	if ((iter=listFormat.find(idFormat)) == listFormat.end())
		return NULL;
	else
		return (void *)listFormat[idFormat]();
}

RETROMPEG_API void *pQueryLibrary(int idWhat)
{
	static const int anFormats[]=
	{
		CMobileContent::formatAAC,
		CMobileContent::formatAMR,
		CMobileContent::formatAWB,
		CMobileContent::formatMP3,
		CMobileContent::formatMP4,
		CMobileContent::format3GP,
//		CMobileContent::format3G2
		CMobileContent::formatOGG,
		CMobileContent::formatRA,
		CMobileContent::formatWMA,
		CMobileContent::formatSWF,
		0
	};
	switch(idWhat)
	{
		case RETROLIBQ_VERSION:
			return (void *)szGetRetroMpegVersion();
		case RETROLIBQ_FORMATS:
			return (void *)&anFormats[0];
	}
	return NULL;
}
