/*\
 * TARArchive.cpp
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
#include <io.h>
#include <direct.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <map>
#include <stdlib.h>
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "zlib.h"
#include "unzip/unzip.h"
#include "TARArchive.h"

CTARArchive::CTARArchive(void)
{
}

CTARArchive::~CTARArchive(void)
{
}

void CTARArchive::ExtractOneFile(istream &ar,const char *szFilename,char **ppcDest,unsigned int *pnSize,char *pcOwner,bool bIsText)
{
	typedef struct
	{
		char szFilename[100];
		char szMode[8];
		char szUserID[8];
		char szGroupID[8];
		char szSize[12];
		char szTimeModified[12];
		char szCheckSum[8];

		char cTypeFlag;
		char szLinkedFilename[100];
		char szUSTAR[6];
		char szVersion[2];
		char szOwnerName[32];
		char szOwnerGroup[32];
		char szDeviceMajor[8];
		char szDeviceMinor[8];
		char szFilenamePrefid[155];
	}tarHead;
	
	tarHead head;
	unsigned long nFileSize=0;
	bool bFound=false;
	
	while(!bFound)
	{
 		ar.read((char *)&head,sizeof(tarHead));
		if (512-sizeof(tarHead))
			ar.seekg(512-sizeof(tarHead),ios_base::cur);
		if (!strlen(head.szFilename))
			break;
		TRACEIT("file-entry: %s\n",head.szFilename);
		sscanf(head.szSize,"%o",&nFileSize);
		if (nFileSize)
		{
			if (!strcmp(head.szFilename,szFilename))
				bFound=true;
			else
			{
				unsigned int nSkip=nFileSize;
				if (nFileSize % 512)
					nSkip+=512-(nFileSize % 512);
				ar.seekg(nSkip,ios_base::cur);
			}
		}
	};	
	if (bFound && nFileSize)
	{
		if (pcOwner)
			strcpy(pcOwner,head.szOwnerName);
		*pnSize=nFileSize;
		if (nFileSize && bIsText)
			*ppcDest=new char[nFileSize+1];
		else
			*ppcDest=new char[nFileSize];
		ar.read(*ppcDest,nFileSize);
		if (nFileSize && bIsText)
			*((*ppcDest)+nFileSize)=0;
	}	
}
