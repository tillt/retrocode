/*\
 * ThemeBaseContent.cpp
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
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <map>
#include <stdlib.h>

#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/FIDProperty.h"
#include "../retroBase/CommonAccess.h"
#include "../retroBase/MIDIFile.h"
#include "../retroBase/MIDIFileWriter.h"
#include "../retroBase/Midispec.h"

#include "../retroBeatnik/RMFBasics.h"
#include "../retroBeatnik/RMFFile.h"

#include "../retroYamaha/SMAFDecoder.h"
#include "../retroYamaha/SMAFFile.h"

#include "../retroMpeg/AMRFile.h"
#include "../retroMpeg/MP3File.h"

#include "../retroQualcomm/QCELPFile.h"
#include "../retroQualcomm/CMXFile.h"

#include "../retroMono/MonoFile.h"
#include "../retroMono/EMSFile.h"

#include "../retroMidi/MIDIFileDoc.h"

#include "../retroGraphics/GIFFile.h"
#include "../retroGraphics/JPGFile.h"
#include "../retroGraphics/PNGFile.h"
#include "../retroGraphics/MBMFile.h"

#include "unzip/unzip.h"
#include "zlib.h"
#include "ZIPArchive.h"
#include "ThemeBaseContent.h"

#include "resource.h"

CThemeBaseContent::CThemeBaseContent(void)
{
	m_nWidth=0;
	m_nHeight=0;
	m_nColors=0;
}

CThemeBaseContent::~CThemeBaseContent(void)
{
}


/*\
 * <---------- ExportRaw ---------->
 * @m quick and dirty sequence file export
 * --> I N <-- @p
 * const char *pcpath - destination file path
\*/
void CThemeBaseContent::ExportRaw(const char *pcpath,char *pcOut,unsigned long nSize)
{
	FILE *fp;
	if ((fp=fopen(pcpath,"wb")) != NULL)
	{
		fwrite(pcOut,nSize,1,fp);
		fclose(fp);
	}
	else
	{
		Log2(verbLevErrors,"couldnt create output file \"%s\"\n",pcpath);
		throw new CFormatException(CFormatException::formaterrUnknown,"couldnt create output file");
	}
}

CMobileProperty *CThemeBaseContent::poProcessFile(char *pcIn,unsigned long nFileSize,const char *pcPath=NULL)
{
	CMobileProperty *pp=NULL;
	uint32_t nWidth=0,nHeight=0,nColors=0;
	string strFormat,strId;
	if ((pp=poParseFromMemory(pcIn,nFileSize,pcPath)) != NULL)
	{
		pp->getPropertyWithAttribute("prstrFormat",strFormat,"id",strId);
		switch(atoi(strId.c_str()))
		{
			case formatGIF:
			case formatJPEG:
			case formatPNG:
			case formatMBM:
			case formatBMP:
				pp->getProperty("prnumWidth",nWidth);
				pp->getProperty("prnumHeight",nHeight);
				pp->getProperty("prnumColors",nColors);
				if ((unsigned long int)nWidth*nHeight > m_nWidth*m_nHeight || nColors > m_nColors)
				{
					m_nWidth=nWidth;
					m_nHeight=nHeight;
					m_nColors=nColors;
				}
			break;
			default:
				Log2(verbLevDebug3,"unexpected content format parsed: %d\n",atoi(strId.c_str()));
		}
	}
	else
	{		
		Log2(verbLevDebug3,"failed to parse from memory\n");
	}
	return pp;
}

CMobileProperty *CThemeBaseContent::poParseFromMemory(char *pcIn,unsigned long nFileSize,const char *pcPath=NULL)
{
	CMobileContent *pm=NULL;
	CMyString str;
	CMyString strError;
	stringstream ar;
	CMobileProperty *pp=NULL;

	ASSERT(m_pListModules);

	ar.exceptions(ios_base::badbit|ios_base::eofbit|ios_base::failbit);
	ar.write((const char *)pcIn,nFileSize);
	ar.seekg(0,ios_base::beg);
	
	try
	{
		//identify file type by magic-bytes
		TRACEIT("identifying file-magix...\n");
		if ((pm=(CMobileContent *)CCommonAccess::pMagicHeads(ar,nFileSize,(VectorModules *)m_pListModules)) != NULL)
		{
			TRACEIT2("mobile content: %s\n",typeid(*pm).name());
			pp=(CMobileProperty *)pm->PropertyCreate();
			ASSERT(pp);
			if (pp)
			{
				TRACEIT2("Format: %s\n",pm->m_sFormatName.c_str());
				TRACEIT2("checking details...\n");
				ar.seekg(0L,ios_base::beg);
				pm->m_nFileSize=nFileSize;
				pm->Read(ar);
				TRACEIT2("init from content...\n");
				pp->InitFromContent(pcPath,nFileSize,pm);
			}
		}
		else
		{
			TRACEIT("magix failed\n");
			strError="unknown theme archived file format";
		}
	}
	catch (istream::failure const &e)
	{
		TRACEIT("File Access Exception\n\t%s\n",e.what());
		strError=e.what();
	}
	catch (CFormatException *fe)
	{
		//Log2(verbLevErrors,IDS_ERR_FORMATEXCEPTION,fe->szGetErrorMessage());
		if (pm)
			pm->SetLastError(fe->szGetErrorMessage());
		strError=fe->szGetErrorMessage();
		delete fe;
	}	
	return pp;
}
