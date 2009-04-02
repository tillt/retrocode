/*\
 * Modules.cpp
 * Copyright (C) 2004-2007, MMSGURU - written by Till Toenshoff
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
\*/
#include "stdafx.h"
#ifdef WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>
#endif
#include <time.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include "Modules.h"


#ifndef WIN32
/*\
 * <---------- _makepath ----------> 
 * @m melt the components of a filesystem path together
 * --> I N <-- @p
 * char *pszPath        - ouput path pointer
 * const char *pszDrive - input drive string (or NULL)
 * const char *pszDir   - input folder string (or NULL)
 * const char *pszFname - input filename string (or NULL)
 * const char *pszExt   - input file-extension string including the leading dot (or NULL)
\*/
DllExport void _makepath(char *pszPath, const char *pszDrive, const char *pszDir, const char *pszFname, const char *pszExt) 
{
	ASSERT(pszPath);
	strcpy(pszPath, "");
	if (pszDir != NULL)
		strcpy (pszPath,pszDir);
	if (strlen(pszPath) && pszPath[strlen(pszPath)-1] != '/')
		strcat(pszPath,"/");
	if (pszFname != NULL)
		strcat (pszPath,pszFname);
	if (pszExt != NULL)
		strcat(pszPath,pszExt);
}

/*\
 * <---------- _splitpath ----------> 
 * @m devide a given filesystem path into its components
 * mostly an emulation of that same DOS/windows function
 * --> I N <-- @p
 * const char *pszPath - input path string
 * char *pszDrive      - output drive string (or NULL)
 * char *pszDir        - output folder string (or NULL)
 * char *pszFname      - output filename string (or NULL)
 * char *pszExt        - output file-extension string including the leading dot (or NULL)
\*/
DllExport void _splitpath(const char *pszPath, char *pszDrive, char *pszDir, char *pszFname, char *pszExt) 
{   
    const char *pszLastslash = NULL; 
    const char *pszLastdot = NULL; 
    const char *pszAt;
    const char *pszBegin;

    if(pszDrive != NULL)
        strcpy(pszDrive, "");								//unix doesnt use "drives" the way windows does, always empty
    for(pszAt = pszPath; *pszAt != 0; pszAt++) 
    { 
        if(*pszAt == '/') 
            pszLastslash = pszAt; 
    }  
    if(pszDir) 
    { 
        if(pszLastslash == NULL) 
            strcpy(pszDir, ""); 
        else
        {
            strncpy(pszDir, pszPath, (pszLastslash - pszPath) + 1); 
            pszDir[(pszLastslash-pszPath) + 1] = 0;
        }
    }  
    pszBegin = (pszLastslash != NULL) ? pszLastslash+1 : pszPath; 
    for(pszAt = pszBegin; *pszAt != 0; pszAt++) 
    { 
        if(*pszAt == '.') 
            pszLastdot = pszAt; 
    }   
    if(pszLastdot == NULL) 
    { 
        if(pszFname != NULL) 
            strcpy(pszFname, pszBegin); 
        if(pszExt != NULL) 
            strcpy(pszExt, ""); 
    } 
    else 
    { 
        if(pszFname != NULL) 
        {
            strncpy(pszFname, pszBegin, pszLastdot - pszBegin); 
            pszFname[pszLastdot-pszBegin] = 0;
        }
        if(pszExt != NULL) 
            strcpy(pszExt, pszLastdot); 
    }
} 
#endif


/*\
 * <---------- LocateLibraries ----------> 
 * @m locate dymamic libraries at the given path
 * --> I N <-- @p
 * const char *szFolder       - path to search
 * VectorModules &listModules - reference to list of libraries
\*/
void LocateLibraries(const char *szFolder,VectorModules *pListModules)
{
	#ifdef WIN32
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	#else
	DIR *FindFileData;
	struct dirent *hFind;
	#endif	
	char path[_MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	
	string strSource=szFolder;

	Log2(verbLevDebug1,"Locating libs in \"%s\"\n",szFolder);
	
	#ifdef WIN32
	strSource+="\\*.dll";
	#endif

	#ifdef WIN32
	hFind = FindFirstFile(strSource.c_str(),&FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) 
	#else
	FindFileData = opendir (szFolder);
	if (FindFileData == NULL )
	{
		Log2(verbLevMessages,"Unable to enter \"%s\"\n",szFolder);
		return;
	}
	hFind = readdir(FindFileData);
	if (hFind == NULL)
	#endif
	{
		Log2(verbLevMessages,"No libraries found at \"%s\"\n",szFolder);
	}
	else 
	{
		do
		{
#ifdef WIN32
			_splitpath(FindFileData.cFileName,drive,dir,fname,ext);
#else
#ifdef DARWIN
			if (strstr(hFind->d_name,".dylib"))
#else
			if (strstr(hFind->d_name,".so"))
#endif
			{
				_splitpath(hFind->d_name,drive,dir,fname,ext);
#endif
				_makepath(path,drive,szFolder,fname,ext);
				Log2(verbLevDebug1,"Located library \"%s\" at %s\n",fname,path);
				pListModules->push_back(new CRetroModule(fname,path));	
#ifndef WIN32
			}
#endif
		
#ifdef WIN32
		}while(FindNextFile(hFind, &FindFileData) != 0);
#else
		}while((hFind = readdir(FindFileData)) != NULL);
#endif
				
		#ifdef WIN32
		FindClose(hFind);
		#else
		closedir(FindFileData);
		#endif
	}
}

void DeinitLibraries(VectorModules *pListModules)
{
	for (int i=0;i < pListModules->size();i++)
	{
		#ifdef WIN32
		FreeLibrary((HMODULE)pListModules->at(i)->pGetLibrary());
		#else
		dlclose(pListModules->at(i)->pGetLibrary());
		#endif
		delete pListModules->at(i);
	};
	pListModules->clear();
}

/*\
 * <---------- RemoveLibrary ----------> 
 * @m removes a single library from the list
 * --> I N <-- @p
 * VectorModules &listModules - reference to list of libraries
 * int &i                     - reference to index of the library to remove
\*/
void RemoveLibrary(VectorModules *pListModules,int &i)
{
	uint32_t a;
	VectorModules replist;
	for (a=0;a < i;a++)
		replist.push_back(pListModules->at(a));
	++a;
	for (;a < (int)pListModules->size();a++)
		replist.push_back(pListModules->at(a));			
	*pListModules=replist;
	if (i)
		--i;
}

/*\
 * <---------- InitLibraries ----------> 
 * @m loads all libraries from the list and tries to initialize them - 
 * libraries that fail while initializing are removed from the list.
 * --> I N <-- @p
 * VectorModules &listModules - reference to list of libraries
\*/
void InitLibraries(VectorModules *pListModules)
{
	int i;
	void *retroLib;

	for (i=0;i < (int)pListModules->size();i++)
	{
		#ifdef WIN32
		retroLib = LoadLibrary(pListModules->at(i)->pcGetPath());
		#else
		retroLib = dlopen (pListModules->at(i)->pcGetPath(), RTLD_LAZY);
		#endif

		if (!retroLib)
		{	
			#ifdef WIN32
			LPVOID lpMsgBuf;			
			uint32_t dw = GetLastError(); 
			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				dw,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR) &lpMsgBuf,
				0, NULL );
			Log2(verbLevErrors,"unable to load %s: %s\n",pListModules->at(i)->pcGetName(),(LPTSTR)lpMsgBuf);
			LocalFree(lpMsgBuf);
			#else
			Log2(verbLevErrors,"unable to load %s: %s\n",pListModules->at(i)->pcGetName(),dlerror());
			#endif
			
			TRACEIT2("unable to load %s dll\n",pListModules->at(i)->pcGetName());
			
			RemoveLibrary(pListModules,i);	//library seems to be invalid, remove it from the list
		}
		else
		{
			TRACEIT2("library %s loaded\n",pListModules->at(i)->pcGetName());
			Log2(verbLevDebug3,"module location %X\n",pListModules->at(i));
			pListModules->at(i)->SetLibrary(retroLib);
			#ifdef WIN32
			void * (*cq)(uint32_t) = (void * (*) (uint32_t))(GetProcAddress ((HMODULE)retroLib, "pQueryLibrary"));
			#else
			void * (*cq)(uint32_t) = (void * (*) (uint32_t))(dlsym (retroLib, "pQueryLibrary"));
			#endif
			if (!cq)
			{
#ifdef WIN32
				TRACEIT2("unable to access %s class query\n",pListModules->at(i)->pcGetName());
#else
				TRACEIT2("unable to access %s class query: %s\n",pListModules->at(i)->pcGetName(),dlerror());
#endif
				Log2(verbLevErrors,"unable to access %s class query\n",pListModules->at(i)->pcGetName());
				#ifdef WIN32
				FreeLibrary((HMODULE)retroLib);
				#else
				dlclose(retroLib);
				#endif
				RemoveLibrary(pListModules,i);	//library seems to be invalid, remove it from the list
			}
			else
			{
				pListModules->at(i)->SetQueryLibrary(cq);

				#ifdef WIN32
				void * (*cf)(uint32_t) = (void * (*) (uint32_t))(GetProcAddress ((HMODULE)retroLib, "pCreateCodecInstance"));
				#else
				void * (*cf)(uint32_t) = (void * (*) (uint32_t))(dlsym (retroLib, "pCreateCodecInstance"));
				#endif
				if (!cf)
				{
					TRACEIT2("unable to access %s class factory\n",pListModules->at(i)->pcGetName());
					Log2(verbLevErrors,"unable to access %s class factory\n",pListModules->at(i)->pcGetName());
					#ifdef WIN32
					FreeLibrary((HMODULE)retroLib);
					#else
					dlclose(retroLib);
					#endif
					RemoveLibrary(pListModules,i);	//library seems to be invalid, remove it from the list
				}
				else
				{
					pListModules->at(i)->SetCreateCodecInstance(cf);
					Log2(verbLevDebug1,"Library \"%s\" validated\n",pListModules->at(i)->pcGetName());
					//pListModules->at(i)->dump();
				}
			}
		}
	}
}

CRetroModule::CRetroModule(const char *szName,const char *szPath) : m_pLibrary(NULL),m_pQueryLibrary(NULL),m_pCreateCodecInstance(NULL),m_pListModules(NULL)
{ 
	Log2(verbLevDebug3,"module %s constructed\n",szName);
	strncpy(m_szName,szName,_MAX_PATH-1); 
	m_szName[_MAX_PATH-1]=0;
	strncpy(m_szPath,szPath,_MAX_PATH-1); 
	m_szName[_MAX_PATH-1]=0;
}

void CRetroModule::dump(void) 
{ 
	Log2(verbLevDebug3,"m_szName = %X\n",m_szName);
	Log2(verbLevDebug3,"m_szPath = %X\n",m_szPath);
	Log2(verbLevDebug3,"m_pQueryLibrary = %X\n",m_pQueryLibrary);
	Log2(verbLevDebug3,"m_pCreateCodecInstance = %X\n",m_pCreateCodecInstance);
	Log2(verbLevDebug3,"m_pLibrary = %X\n",m_pLibrary);
}
