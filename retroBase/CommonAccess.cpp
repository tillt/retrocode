/*\
 * CommonAccess.cpp
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
#include <time.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#ifndef WIN32
#include <typeinfo>
#endif
#include "Basics.h"
#include "MyString.h"
#include "Integer.h"
#include "MobileContent.h"
#include "CommonAccess.h"
#include "../include/Resource.h"

/*\
 * <---------- pReadSource ---------->
 * @m read, parse and decode source content stream
 * --> I N <-- @p
 * const TCHAR *szSource - filename
 * ListOfFormats &listFormat - list of formats
 * <-- OUT --> @r
 * CMobileSampleContent * - instantiated format class pointer
\*/
void *CCommonAccess::pReadAnyFormat(const TCHAR *szSource,VectorModules *pListModules,converting_parameters *pParameters,bool bByExtension)
{
	std::ifstream ar;
	CMobileSampleContent *pm=NULL;
	uint32_t nSize;

	ar.exceptions(ios_base::badbit|ios_base::eofbit|ios_base::failbit);
	Log2(verbLevDebug2,"opening file \"%s\"...\n",szSource);
	try
	{
		//open file
		ar.open(szSource,ios_base::binary | ios_base::in);
		
		//get filesize
		ar.seekg(0L,ios_base::end);
		nSize=ar.tellg();
		ar.seekg(0L,ios_base::beg);
		
		if (bByExtension)
		{
			//identify file type by file-extension
			Log2(verbLevDebug2,"identifying file-extension...\n");
			pm=static_cast<CMobileSampleContent *>(CCommonAccess::pExtensionLoader(szSource,pListModules));
		}
		else
		{
			//identify file type by magic-bytes
			Log2(verbLevDebug2,"identifying file-magix...\n");
			pm=static_cast<CMobileSampleContent *>(CCommonAccess::pMagicHeads(ar,nSize,pListModules,szSource));
		}
		if (pm)
		{
			pm->AttachParameter(pParameters);
			pm->m_nFileSize=nSize;
			Log2(verbLevMessages,IDS_PRG_READFILE,pm->m_sFormatName.c_str());
			//read and decode the content
			pm->Read(ar);
		}
	}
	catch (CFormatException *fe)
	{
		Log2(verbLevErrors,IDS_ERR_FORMATEXCEPTION,fe->szGetErrorMessage());
		if (pm != NULL)
			pm->Release();
		pm=NULL;
		delete fe;
	}
	catch (istream::failure const &e)
	{
		Log2(verbLevErrors,IDS_ERR_FILEEXCEPTION,e.what());
		if (pm != NULL)
			pm->Release();
		pm=NULL;
	}
	if (pm == NULL)
		Log2(verbLevErrors,IDS_ERR_UNKFORMAT);
	return pm;
}

/*\ 
 * <---------- *pMagicHeads ----------> 
 * @m checks first n (usually 4) bytes of a content stream
 * --> I N <-- @p
 * istream &ar - input stream
 * int nLen - input stream size
 * ListOfFormats - reference to the list of formats used
 * <-- OUT --> @r
 * CMobileContent - pointer to instantiated content class
\*/ 
void *CCommonAccess::pMagicHeads(istream &ar,uint32_t nLen,VectorModules *pListModules,const char *szSource)
{
	CRetroModule *pModule;
	CMobileContent *pm=NULL;
	Log2(verbLevDebug2,"%d modules available\n",(int)pListModules->size());
	//go through all loaded decoder modules
	for (int i=0;i < (int)pListModules->size();i++)
	{
		//fetch the supported format of the current module
		pModule=pListModules->at(i);
		Log2(verbLevDebug3,"module pointer %X\n",pModule);
		//pModule->dump();
		Log2(verbLevDebug3,"module type %s\n",typeid(*pModule).name());
		Log2(verbLevDebug3,"query function pointer %X\n",pModule->CRetroModule::pGetCreateCodecInstance());

		//pListModules->at(i)->dump();
		uint32_t *nFormats=reinterpret_cast<uint32_t *>(pListModules->at(i)->pGetQueryLibrary()(RETROLIBQ_FORMATS));
		Log2(verbLevDebug3,"formats pointer %X\n",nFormats);
		Log2(verbLevDebug3,"module %s capable of %d formats\n",pListModules->at(i)->pcGetName(),*nFormats);
		//go through all formats supported by this module
		while(*nFormats != 0)
		{
			//instantiate the codec of this format
			pm=reinterpret_cast<CMobileSampleContent *>(pListModules->at(i)->pGetCreateCodecInstance()(*nFormats));
			ASSERT(pm);
			if (pm)
			{
				Log2(verbLevDebug2,"test using %s\n",typeid(*pm).name());
				pm->SetFormatId(*nFormats);
				if (szSource)
					pm->SetFileName(szSource);
				ar.seekg(0,ios_base::beg);
				Log2(verbLevDebug2,"magic head (bMagicHead @ %Xh)...\n",pm);
				if (pm->bMagicHead(ar,nLen))
				{
					ar.clear(ios::goodbit);
					ar.seekg(0,ios_base::beg);
					return pm;
				}
				pm->Release();
				pm=NULL;
				ar.clear(ios::goodbit);
			}
			else
			{
				Log2(verbLevErrors,"failed to instantiate CODEC for format-id: %d\n",*nFormats);
			}
			++nFormats;
		};
	}
	ar.clear(ios::goodbit);
	ar.seekg(0,ios_base::beg);
	return NULL;
}

/*\
 * <---------- sGetFileExtension ---------->
 * @m get the extension of a file name
 * --> I N <-- @p
 * const char *pszFileName - complete path or file name
 * <-- OUT --> @r
 * CMyString - returns the file-extension including the leading dot
\*/
CMyString CCommonAccess::sGetFileExtension(const char *pszFileName)
{
	size_t nExtPos;
	CMyString strFileExt=CMyString(pszFileName);
	if ((nExtPos=strFileExt.rfind('.')) != string::npos)
		strFileExt=strFileExt.substr(nExtPos+1).c_str();
	strFileExt.ToLower();
	return strFileExt;
}

/*\
 * <---------- pExtensionLoader ---------->
 * @m instantiate a codec based on the file-extension to be processed
 * --> I N <-- @p
 * const char *pszPath - file to process
 * VectorModules &listModules - list of loaded codecs
 * <-- OUT --> @r
 * void * - pointer to codec instance
\*/
void *CCommonAccess::pExtensionLoader(const char *pszPath,VectorModules *pListModules)
{
	CMobileContent *pm=NULL;
	CMyString strFileExt=sGetFileExtension(pszPath);

	for (int i=0;i < (int)pListModules->size();i++)
	{
		int *nFormats=reinterpret_cast<int *>(pListModules->at(i)->pGetQueryLibrary()(RETROLIBQ_FORMATS));
		while(*nFormats != 0)
		{
			pm=reinterpret_cast<CMobileSampleContent *>(pListModules->at(i)->pGetCreateCodecInstance()(*nFormats));
			pm->AddRef();
			Log2(verbLevDebug2,"test using %s\n",typeid(*pm).name());
			Log2(verbLevDebug2,"extension \"%s\" against \"%s\"\n",strFileExt.c_str(),pm->m_sDefaultExtension.c_str());
			pm->SetFormatId(*nFormats);
			if (!strFileExt.compare(pm->m_sDefaultExtension))
				return pm;
			for (int o=0;o < pm->m_listAltExtensions.size();o++)
			{
				Log2(verbLevDebug2,"test using alt extension %s\n",pm->m_listAltExtensions[o].c_str());
				if (!strFileExt.compare(pm->m_listAltExtensions[o]))
				{
					return pm;
				}
			}
			pm->Release();
			pm=NULL;
			++nFormats;
		};
	}
	return NULL;
}

/*\
 * <---------- bHasDisplayableSample ---------->
 * @m 
 * --> I N <-- @p
 * void *content - 
 * void *prop - 
 * <-- OUT --> @r
 * bool - 
\*/
/*
bool CCommonAccess::bHasDisplayableSample(void *content,void *prop)
{
	CMobileContent *pm=static_cast<CMobileContent *>(content);
	CMobileProperty *pp=static_cast<CMobileProperty *>(prop);
	bool bContainsSample=false;

	if (pm && prop)
	{
		switch(pm->m_nFormatId)
		{
			case CMobileContent::formatRMF:
			case CMobileContent::formatSMAF:
			case CMobileContent::formatCMX:
				pp->getProperty("prboolContainsSamples",bContainsSample);
			break;
			case CMobileContent::formatAAC:
			case CMobileContent::formatMFM:
			case CMobileContent::formatQCELP:
			case CMobileContent::formatWAV:
			case CMobileContent::formatMP3:
			case CMobileContent::formatMP4:
			case CMobileContent::format3GP:
			case CMobileContent::formatRA:
			case CMobileContent::formatWMA:
			case CMobileContent::formatAIF:
			case CMobileContent::formatAMR:
				bContainsSample=true;
			break;
		}	
	}
	return bContainsSample;
}
*/
