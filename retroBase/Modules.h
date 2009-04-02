#ifndef MYMODULESincluded
#define MYMODULESincluded
#include "Basics.h"
#include <vector>
#include <stdio.h>
#include <string.h>

#undef RETROBASE_API
#ifdef RETROBASE_EXPORTS
#define RETROBASE_API __declspec(dllexport)
#else
#define RETROBASE_API 
#endif

#define RETROLIBQ_VERSION	1
#define RETROLIBQ_FORMATS	2

class CRetroModule;

typedef vector<CRetroModule *> VectorModules; 
typedef vector<CRetroModule *>::iterator IteratorModules;


class CRetroModule 
{ 
public:
	CRetroModule(const char *szName,const char *szPath);
	~CRetroModule() {};

	void *(*pGetQueryLibrary(void))(uint32_t) { return m_pQueryLibrary; };
	void SetQueryLibrary(void *(*pQL)(uint32_t)) { m_pQueryLibrary=pQL; };

	void *(*pGetCreateCodecInstance(void))(uint32_t) { return m_pCreateCodecInstance; };
	void SetCreateCodecInstance(void *(*pCC)(uint32_t)) { m_pCreateCodecInstance=pCC; };

	void SetModuleList(VectorModules *pListModules) { m_pListModules=pListModules; };
	
	const char *pcGetName(void) { return m_szName; };
	const char *pcGetPath(void)	{ return m_szPath; };

	void *pGetLibrary(void) { return m_pLibrary; };
	void SetLibrary(void *pLib) { m_pLibrary=pLib; };

	void dump(void);

protected:
	void *m_pLibrary;
	void *(*m_pQueryLibrary)(uint32_t idWhat);
	void *(*m_pCreateCodecInstance)(uint32_t idFormat);

	VectorModules *m_pListModules;

	char m_szName[_MAX_PATH];
	char m_szPath[_MAX_PATH];
};

void RemoveLibrary(VectorModules *pListModules,int &i);

#ifndef WIN32
RETROBASE_API void _makepath(char *pszPath, const char *pszDrive, const char *pszDir, const char *pszFname, const char *pszExt);
RETROBASE_API void _splitpath(const char *pszPath, char *pszDrive, char *pszDir, char *pszFname, char *pszExt);
#endif

RETROBASE_API void LocateLibraries(const char *szFolder,VectorModules *pListModules);
RETROBASE_API void InitLibraries(VectorModules *pListModules);
RETROBASE_API void DeinitLibraries(VectorModules *pListModules);
#endif
