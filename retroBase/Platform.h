#ifndef PLATFORM_HEADERincluded
#define PLATFORM_HEADERincluded
#include <stdlib.h>
#include <memory.h>
#include "Integer.h"

#ifdef __APPLE__
#define DARWIN
#endif

#ifdef _ANSI_STDARG_H_
#define ANSI
#endif

#ifdef WIN32
#define ANSI
#endif

#ifdef WIN32
#undef LANG_ENGLISH
#include "windows.h"
#endif

#ifndef _MAX_PATH
#ifndef PATH_MAX
#define _MAX_PATH	255
#endif
#endif

#ifndef PATH_MAX
#define PATH_MAX	(_MAX_PATH)
#endif

#ifndef _MAX_PATH
#define _MAX_PATH	(PATH_MAX)
#endif
#ifndef _MAX_DRIVE
#define _MAX_DRIVE	3
#endif
#ifndef _MAX_DIR
#define _MAX_DIR	(PATH_MAX-4)
#endif
#ifndef _MAX_FNAME 
#define _MAX_FNAME	(PATH_MAX-4)
#endif
#ifndef _MAX_EXT
#define _MAX_EXT	(PATH_MAX-4)
#endif

#ifdef WIN32
#ifndef RETROBASE_EXPORTS
#undef DllExport
#define DllExport 
#else
#undef DllExport
#define DllExport   __declspec( dllexport )
#endif
#else
#undef DllExport
#define DllExport
#endif

#ifndef min
#define min(a, b)  (((a) < (b)) ? (a) : (b)) 
#endif

#ifndef WIN32
#define stricmp strcasecmp
#endif

//typedef unsigned char UBYTE;

#ifndef WIN32
typedef char BYTE;
typedef bool BOOL;
typedef void * LPVOID;

#undef TRUE
#define TRUE true
#undef FALSE
#define FALSE false
#endif

#ifndef WIN32 
#define CopyMemory memcpy
#define ZeroMemory(a,b)	memset(a,0,b)
#else
#ifdef _USRDLL
#include "winsock2.h"
#endif
#endif

#ifndef WIN32
#ifndef _itoa
#define _itoa(number,string,radix)	sprintf(string,"%d",number)
#define _stprintf	sprintf
#define _vstprintf	vsprintf
#define _stricmp stricmp
#endif
#endif

#ifndef TOFOURCC
#undef MAKEFOURCC
#undef FOURCC
//convert number to big endian if needed
#define TOFOURCC(a) htonl(a)
//note, this is a big-endian four-cc
#define MAKEFOURCC(a,b,c,d) ( ((uint32_t)d) | (((uint32_t)c)<< 8) | (((uint32_t)b)<<16) | (((uint32_t)a)<<24)  )
#endif

#endif
