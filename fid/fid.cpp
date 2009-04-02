/*\
 * fid.cpp
 * Copyright (C) 2004-2009, MMSGURU - written by Till Toenshoff
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
#include <strstream>
#include <map>
#include <vector>
#ifndef WIN32
#include <sys/types.h>
#include <dlfcn.h>
#else
#include <windows.h>
#include <crtdbg.h>
#endif
#include "../include/Resource.h"
#include "../retroBase/ResourceVersion.h"
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/FIDProperty.h"
#include "../retroBase/CommonAccess.h"
#include "fid.h"
#include "XMFFile.h"
#include "MFIFile.h"

static const char *szAppName="RetroFID(tm)";
static const char *szAppCopyright="Copyright (C) 2004-2005, Retro Ringtones LLC, Copyright (C) 2006-2009, MMSGURU";

#ifdef INCLUDE_PIXRENDER
#include "png/png.h"
void RenderWaveDisplay(const char *pcPicPath,CMobileContent *pm,CMobileProperty *pp,unsigned int nWidth,unsigned int nHeight)
{
	TRACEIT2("render wave display...\n");
	if (CCommonAccess::bHasDisplayableSample(pm,pp))
	{
		MyCol colBack={0x00,0x00,0x00};			//background default (used for Erase,if used)
		MyCol colMax={0xE0,0xFF,0xA0};			//peak point color
		MyCol colForeLight={0x7D,0xFF,0x00};			//normal foreground color
		MyCol colForeDark={0x1D,0x72,0x00};			//normal foreground color
		MyCol colForeLightCenter={0x9D,0xFF,0x20};			
		MyCol colFadeFrom={0x10,0x10,0x10};		//background fade from
		MyCol colFadeTo={0xFF,0xFF,0xFF};		//background fade to

		double dr,dg,db;
		double r,g,b;
		
		MyCol *pColFore=new MyCol[nHeight];
		MyCol *p=pColFore;
		MyCol *q=pColFore+(nHeight-1);

		r=colForeDark.cRed;			
		g=colForeDark.cGreen;			
		b=colForeDark.cBlue;			
		dr=((double)colForeLight.cRed-r)/(nHeight/2);
		dg=((double)colForeLight.cGreen-g)/(nHeight/2);
		db=((double)colForeLight.cBlue-b)/(nHeight/2);
		for (unsigned int i=0;i < nHeight/2;i++)
		{
			q->cRed=p->cRed=(unsigned char)min(r,255.0);
			q->cGreen=p->cGreen=(unsigned char)min(g,255.0);
			q->cBlue=p->cBlue=(unsigned char)min(b,255.0);
			p++;
			q--;
			r+=dr;
			g+=dg;
			b+=db; 
		}
		if (nHeight%2)
			memcpy(pColFore+(nHeight/2),&colForeLightCenter,sizeof(MyCol));

		unsigned int nFramePix=nHeight/10;
		
		CMobileSampleContent *ps=dynamic_cast<CMobileSampleContent *>(pm);							
		CPngMemoryBitmap bm(nWidth-(nFramePix*2),nHeight-(nFramePix*2),nFramePix,&colBack,&colForeDark);
		ps->bRenderWaveDisplay(&bm, &colFadeFrom, &colFadeTo, pColFore, &colMax, nHeight/3,0, 0.7, 0.1);
		bm.bWritePng(pcPicPath);
	}
}
#endif

bool bProcessFile(VectorModules *pListModules,char *pcPath,bool bDetails=false,bool bPicture=false,unsigned int nWidth=320,unsigned int nHeight=160,const char *pcPicPath=NULL)
{
	unsigned long nFileSize=0;
	ifstream ar;
	bool bFailed=false;
	CMobileContent *pm=NULL;
	CFIDProperty *pp=NULL;
	CMyString str;
	CMyString strError;

	ar.exceptions(ios_base::badbit|ios_base::eofbit|ios_base::failbit);
	try
	{
		//open file
		TRACEIT2("opening file \"%s\"...\n",pcPath);
		ar.open(pcPath,ios_base::binary | ios_base::in);
		
		//get filesize
		ar.seekg(0L,ios_base::end);
		nFileSize=ar.tellg();
		ar.seekg(0L,ios_base::beg);
		  
		//identify file type by magic-bytes
		TRACEIT2("identifying file-magix: %s...\n",pcPath);
		if ((pm=(CMobileContent *)CCommonAccess::pMagicHeads(ar,nFileSize,pListModules)) != NULL)
		{
			TRACEIT2("mobile content: %s\n",typeid(*pm).name());
			pp=(CFIDProperty *)pm->PropertyCreate();
			ASSERT(pp);
			if (pp)
			{
				if (bDetails)
				{
					TRACEIT2("Format: %s\n",pm->m_sFormatName.c_str());
					TRACEIT2("checking details...\n");
					ar.seekg(0L,ios_base::beg);
					pm->m_nFileSize=nFileSize;
					//introduce the mobile content object to the list of decoder and validator plugins
					//needed for embedded content decoding
					pm->AttachModules(pListModules);
					//parse / decode the content object
					pm->Read(ar);
					//initialize the properties object with all details from the mobile object
					TRACEIT2("init from content file: %s...\n",pcPath);
					pp->InitFromContent(pcPath,nFileSize,pm);
	
#ifdef _DEBUG
					pp->dump();				
#endif						
					//render the xml output
					TRACEIT2("write XML...\n");
					pp->writeXMLhead(cout);
					pp->writeXML(cout);
					pp->writeXMLtail(cout);
#ifdef INCLUDE_PIXRENDER
					if (bPicture)
						RenderWaveDisplay(pcPicPath,pm,pp,nWidth,nHeight);
#endif
				}
				else
				{
					pp->CMobileProperty::InitFromContent(pcPath,nFileSize,pm);
					pp->writeXMLhead(cout);
					pp->CMobileProperty::writeXML(cout);
					pp->writeXMLtail(cout);
				}
			}
		}
		else
		{
			TRACEIT2("magix failed\n");
			strError="unknown file format";
		}
	}
	catch (istream::failure const &e)
	{
		TRACEIT2("File Access Exception\n\t%s\n",e.what());
		strError=e.what();
		bFailed=true;
	}
	catch (CFormatException *fe)
	{
		Log2(verbLevErrors,IDS_ERR_FORMATEXCEPTION,fe->szGetErrorMessage());
		if (pm)
			pm->SetLastError(fe->szGetErrorMessage());
		strError=fe->szGetErrorMessage();
		bFailed=true;
		delete fe;
	}
	if (pp)
		pp->Release();
	if (pm == NULL || bFailed)
	{
		TRACEIT2("Format not identified\n");
		//create a fake property for simple error feedback reasons
		CMobileProperty *pp=new CMobileProperty();
		ASSERT(pp);
		if(pp)
		{
			//
			pp->InitFromContent(pcPath,nFileSize,NULL);
			pp->writeXMLhead(cout);
			pp->writeXML(cout);
			if (!strError.empty())
				cout << "\t<Error>" << strError << "</Error>\n";
			pp->writeXMLtail(cout);
			delete pp;
		}
	}
	if (pm)
		pm->Release();
	return true;
}

/*\
 * <---------- szGetRetroFIDVersion ----------> 
 * @m get the application release version
 * <-- OUT --> @r
 * const char * - pointer to version string
\*/
LPCTSTR szGetRetroFIDVersion(void)
{
#if   FID_ALPHA_VERSION
	static const char *const str = XSTR(FID_MAJOR_VERSION) "." XSTR(FID_MINOR_VERSION) " " "(alpha " XSTR(FID_PATCH_VERSION) ", " __DATE__ " " __TIME__ ")";
#elif FID_BETA_VERSION
    static LPCTSTR str = XSTR(FID_MAJOR_VERSION) "." XSTR(FID_MINOR_VERSION) " " "(beta " XSTR(FID_PATCH_VERSION) ", " __DATE__ ")";
#elif FID_RELEASE_VERSION && (FID_PATCH_VERSION > 0)
	static const char *const str = XSTR(FID_MAJOR_VERSION) "." XSTR(FID_MINOR_VERSION) "." XSTR(FID_PATCH_VERSION);
#else
	static const char *const str = XSTR(FID_MAJOR_VERSION) "." XSTR(FID_MINOR_VERSION);
#endif
    return str;
}

tstring strAppDescriptionHeader(void)
{
	tstring str="";
	str+=_T("------------------------------------------------------------------------------\n");
	str+=_T("DESCRIPTION:\n");
	str+=_T("RetroFID(tm) reads mobile content files and identifies their content,\n");
	str+=_T("returning the mobile content format and meta-data as xml data.\n");
	str+=_T("\n");
	return str;
}

/*\
 * <---------- strAppOutputHeader ----------> 
 * @m get the application console ouput header
 * <-- OUT --> @r
 * tstring - displayable text
\*/
tstring strAppOutputHeader(void)
{
	extern const char *szAppName;
	extern const char *szAppCopyright;
	
 	tstring str="";
	str+=_T("------------------------------------------------------------------------------\n");
	str+=tstring(szAppName) + tstring(_T(" "));
	str+=tstring(szGetRetroFIDVersion()) + tstring(_T(" "));
#ifdef WIN32
	str+=tstring(_T("Win32"));
#else
#ifdef DARWIN
	str+=tstring(_T("Darwin"));
#else
	str+=tstr		ing(_T("generic unix"));
#endif
#endif
	str+=tstring(_T("\n"));
	str+=tstring(szAppCopyright) + tstring(_T("\n"));
	str+=_T("$Id: fid.cpp,v 1.8 2009/03/20 21:44:32 lobotomat Exp $\n");
	str+=_T("------------------------------------------------------------------------------\n");
	str+=_T("This program is free software: you can redistribute it and/or modify\n");
	str+=_T("it under the terms of the GNU General Public License as published by\n");
	str+=_T("the Free Software Foundation, either version 3 of the License, or\n");
	str+=_T("(at your option) any later version.\n");
	str+=_T("\n");
	str+=_T("This program is distributed in the hope that it will be useful,\n");
	str+=_T("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
	str+=_T("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
	str+=_T("GNU General Public License for more details.\n");
	str+=_T("\n");
	str+=_T("You should have received a copy of the GNU General Public License\n");
	str+=_T("along with this program.  If not, see <http://www.gnu.org/licenses/>.\n");
	str+=_T("\n");
	return str;
}

void SetupDebugging(void)
{
#ifdef WIN32
	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE );
	_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE );
	_CrtSetReportFile( _CRT_ERROR, _CRTDBG_FILE_STDOUT );
	_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE );
	_CrtSetReportFile( _CRT_ASSERT, _CRTDBG_FILE_STDOUT );

	int tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    tmpDbgFlag |= _CRTDBG_DELAY_FREE_MEM_DF;
    tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
    _CrtSetDbgFlag(tmpDbgFlag);
#endif
#ifdef _DEBUG
	SetVerbosity(5);
#else
	SetVerbosity(verbLevMessages);
#endif
}

#ifdef WIN32
int _tmain(int argc, _TCHAR **argv)
#else
int main(int argc, char **argv)
#endif
{
	char sCodecDir[_MAX_PATH]=RETROCODEC_LIB_PATH;
	char sValidatorDir[_MAX_PATH]=RETROVALIDATOR_LIB_PATH;

	bool bHelp=false;
	bool bDetailedMode=false;
#ifdef INCLUDE_PIXRENDER
	bool bPictureMode=false;
	const char *pcPicPath="fid_output.png";
	unsigned int nWidth=640;
	unsigned int nHeight=300;
#endif
	VectorModules listModules;

	SetupDebugging();

	CMyString::InitStringtable();

	if (argc >= 2)
	{
		int i;
		for (i=1;i < argc;i++)
		{
			if (*argv[i] == '-')
			{
				if (!_stricmp(argv[i]+1,"-codecdir") || !_stricmp(argv[i]+1,"cod"))
				{
					strcpy(sCodecDir,argv[i+1]);
					++i;
				}
				else if (!_stricmp(argv[i]+1,"-validatordir") || !_stricmp(argv[i]+1,"vad"))
				{
					strcpy(sValidatorDir,argv[i+1]);
					++i;
				}
			}
		}
	}

	LocateLibraries(sCodecDir,&listModules);
	LocateLibraries(sValidatorDir,&listModules);
	InitLibraries(&listModules);
	
	if (argc >= 2)
	{
		int i;
		for (i=1;i < argc;i++)
		{
			if (*argv[i] == '-')
			{
				if (!_stricmp(argv[i]+1,"-details") || !_stricmp(argv[i]+1,"d"))
					bDetailedMode=true;
				else if ((!_stricmp(argv[i]+1,"-output") || !_stricmp(argv[i]+1,"o")) && argc > i+1)
				{
					SetVerbosity(atoi(argv[i+1]));
					++i;
				}
#ifdef INCLUDE_PIXRENDER
				else if (!_stricmp(argv[i]+1,"-picture") || !_stricmp(argv[i]+1,"p"))
				{
					if (argc > i+3 && argv[i+1][0] != '-' && argv[i+2][0] != '-' && argv[i+3][0] != '-')
					{
						nWidth=atoi(argv[i+1]);
						nHeight=atoi(argv[i+2]);
						pcPicPath=argv[i+3];
						bPictureMode=true;
						++i;
					}
					else
						bHelp=true;
				}
#endif
				else if (!_stricmp(argv[i]+1,"-version") || !_stricmp(argv[i]+1,"v"))
				{
					Log2(verbLevMessages,strAppOutputHeader().c_str());
					Log2(verbLevMessages,"--Components--\n");
					Log2(verbLevMessages,"retroBase %s\n",szGetRetroBaseVersion());
					for(unsigned int i=0;i < listModules.size();i++)
						Log2(verbLevMessages,"%s %s\n",listModules.at(i)->pcGetName(),reinterpret_cast<LPCTSTR>(listModules[i]->pGetQueryLibrary()(RETROLIBQ_VERSION)));
				}
				else if (!_stricmp(argv[i]+1,"-help") || !_stricmp(argv[i]+1,"h") || !_stricmp(argv[i]+1,"?"))
					bHelp=true;			
			}
		}
		if (!bHelp)
		{
			for (i=1;i < argc;i++)
			{
				if (*argv[i] != '-')
				{
					try
					{
						#ifdef INCLUDE_PIXRENDER
							bProcessFile(&listModules,argv[i],bDetailedMode,bPictureMode,nWidth,nHeight,pcPicPath);
						#else
							bProcessFile(&listModules,argv[i],bDetailedMode);
						#endif
					}
					catch (assert_error *ae)
					{
						Log2(verbLevErrors,"Error: %s\n",ae->errorMsg().c_str());
						Log2(verbLevErrors,"Filename: %s\n",ae->fileName().c_str());
						Log2(verbLevErrors,"Line: %d\n",ae->nLineNumber());
						delete ae;
					}
				}
				else if (!_stricmp(argv[i]+1,"-codecdir") || !_stricmp(argv[i]+1,"cod"))
					++i;
				else if (!_stricmp(argv[i]+1,"-validatordir") || !_stricmp(argv[i]+1,"vad"))
					++i;
				#ifdef INCLUDE_PIXRENDER
					else if (!_stricmp(argv[i]+1,"-picture") || !_stricmp(argv[i]+1,"p"))
						i+=3;
				#endif
				else if (!_stricmp(argv[i]+1,"-output") || !_stricmp(argv[i]+1,"o"))
					i+=1;
			}
		}
	}
	else
		bHelp=true;
	if(bHelp)
	{
		Log2(verbLevMessages,"------------------------------------------------------------------------------\n");
		Log2(verbLevMessages,strAppOutputHeader().c_str());
		Log2(verbLevMessages,strAppDescriptionHeader().c_str());
		#ifdef INCLUDE_PIXRENDER
			Log2(verbLevMessages,"\t\tfid PATH [--details|--version|-d|-v] [--output|-o VERBOSITY LEVEL] [--picture|-p PNG_WIDTH PNG_HEIGHT PNG_PATH] \n");
		#else
			Log2(verbLevMessages,"\t\tfid PATH [--details|--version|-d|-v] [--output|-o VERBOSITY LEVEL]\n");
		#endif
		Log2(verbLevMessages,"------------------------------------------------------------------------------\n");
		Log2(verbLevMessages,"SHORT\tLONG            (DEFAULT)       DESCRIPTION\n");
		Log2(verbLevMessages,"cod\tcodecdir        %s          path to codecs folder\n",RETROCODEC_LIB_PATH);
		Log2(verbLevMessages,"vad\tvalidatordir    %s      path to validators folder\n",RETROVALIDATOR_LIB_PATH);
		Log2(verbLevMessages,"v\tversion                         display application version information\n");
		Log2(verbLevMessages,"d\tdetails                         display detailed content information\n");
		#ifdef INCLUDE_PIXRENDER
			Log2(verbLevMessages,"p\tpicture                         render a wave display of sample sound\n");
		#endif
		Log2(verbLevMessages,"o\toutput                          output verbosity\n");
		Log2(verbLevMessages,"h\thelp				show this quick help\n");
		Log2(verbLevMessages,"------------------------------------------------------------------------------\n");
		Log2(verbLevMessages,"EXAMPLE:\n");
		Log2(verbLevMessages,"\t\tfid -d test.mid\n");
	}
	DeinitLibraries(&listModules);
	return 0;
}
