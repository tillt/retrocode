#ifndef COMMONincluded
#define COMMONincluded
#include "Modules.h"
#ifndef RETROBASE_EXPORTS
#undef DllExport
#define DllExport 
#else
#undef DllExport
#define DllExport   __declspec( dllexport )
#endif

class CCommonAccess
{
public:
	CCommonAccess(){};
	~CCommonAccess(){};

	DllExport static void *pReadAnyFormat(const TCHAR *szSource,VectorModules *pListModules,converting_parameters *pParameters,bool bByExtension=false);
	DllExport static void *pMagicHeads(istream &ar,uint32_t nLen,VectorModules *pListModules,const char *szSource=NULL);
	DllExport static void *pExtensionLoader(const char *szSource,VectorModules *pListModules);

	DllExport static void Init(void) {CMyString::InitStringtable();};

	DllExport static CMyString sGetFileExtension(const char *pszFileName);
};

#endif
