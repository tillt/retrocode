#ifndef COMPRESSEDIOncluded
#define COMPRESSEDIOncluded
#include "Basics.h"

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
#define DllExport	extern "C"
#endif

DllExport int nDecompressZLIB(unsigned char *pcSource,unsigned int nSrcSize,unsigned char *pcDest,unsigned int nDstSize);
#endif
