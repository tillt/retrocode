/*\
 * MTFFile.cpp
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
#ifndef WIN32
#include <netinet/in.h>
#else
#include <winsock2.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <string.h>
#include <map>
#include <stdlib.h>
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/FIDProperty.h"
#include "ThemeBaseContent.h"
#include "MTFFile.h"
#include "MTFProperty.h"

DYNIMPPROPERTY(CMTFFile,CMTFProperty)

CMTFFile::CMTFFile(void)
{
	m_pcMagic="MTF";
	m_nMagicSize=3;
	m_sFormatName="Motorola Theme File";
	m_nCheckSum=0;
}

CMTFFile::~CMTFFile(void)
{
}

void CMTFFile::Stream(istream &ar,char *pcIn,unsigned long nSize)
{
	ASSERT(pcIn);
	ASSERT(nSize);
	ar.read(pcIn,nSize);
	UpdateCheckSum(pcIn,nSize);
	m_nOffset+=nSize;
}

void CMTFFile::UpdateCheckSum(const char *pcIn,unsigned long nSize)
{
	while (nSize--)
		m_nCheckSum+=*(unsigned char *)(pcIn++);
}

void CMTFFile::Read(istream &ar)
{
	char cSignature[3];
	unsigned char i;
	unsigned char cFileCount;

	unsigned long int nFileSize[255];
	unsigned char cFileType[255];
	tstring strFileName[255];
	unsigned short int wCheckSum;
	char *buffer;
	CMobileProperty *pp;
	m_nOffset=0;

	Stream(ar,(char *)cSignature,3);			//MTF
	Stream(ar,(char *)&m_cVersion,1);			//Version
	TRACEIT2("version %Xh\n",m_cVersion);
	if (m_cVersion != 0x10)
		throw new CFormatException(CFormatException::formaterrInvalid,"invalid version");
	Stream(ar,(char *)&cFileCount,1);			//file-count
	TRACEIT2("file count %d\n",cFileCount);
	if (cFileCount > 0x7F)
		throw new CFormatException(CFormatException::formaterrInvalid,"invalid file count");
	for (i=0;i < cFileCount;i++)
	{
		Stream(ar,(char *)&nFileSize[i],4);		//file-size
		nFileSize[i]=ntohl(nFileSize[i]);
		TRACEIT2("file %d: file size %d\n",i,nFileSize[i]);
	}
	for (i=0;i < cFileCount;i++)
	{
		Stream(ar,(char *)&cFileType[i],1);		//file-type
		TRACEIT2("file %d type: %d\n",i,cFileType[i]);
		strFileName[i]=sReadWideString(ar);
		TRACEIT2("file %d name: %s\n",i,strFileName[i].c_str());
		switch (cFileType[i])
		{
			case 0:		m_strStdResource[resDesktop]=strFileName[i];		break;
			case 1:		m_strStdResource[resScreensaver]=strFileName[i];	break;
			case 2:		m_strStdResource[resRingtone]=strFileName[i];		break;
		}
	}
	//Stream(ar,&c,1);		//file-type
	ar.read((char *)&wCheckSum,2);				//CRC
	TRACEIT2("file checksum: %X\n",wCheckSum);
	m_nCheckSum=htons((unsigned short int)(m_nCheckSum&0xFFFF));
	TRACEIT2("calc checksum: %X\n",m_nCheckSum);
	if (cFileCount > 0x7F)
		throw new CFormatException(CFormatException::formaterrChecksum,"file header checksum");
	//Stream(ar,&c,1);		//file-type
	
	for (i=0;i < cFileCount;i++)
	{
		ASSERT(nFileSize[i]);
		buffer=new char[nFileSize[i]];
		ar.read(buffer,nFileSize[i]);
		//ExportRaw(strFileName[i].c_str(),buffer,nFileSize[i]);
		if ((pp=poProcessFile(buffer,nFileSize[i],strFileName[i].c_str())) != NULL)
			;

		delete [] buffer;
		//while(nToDo--)
		//	Stream(ar,(char *)&c,1);			//file-data
	}
	TRACEIT2("offset: %d\n",m_nOffset);
	//ar.read((char *)&wCheckSum,2);				//CRC
}

/*\
 * <---------- sReadWideString ---------->
 * @m read UCS2 string  (crummy workaround!)
 * --> I N <-- @p
 * istream &ar - input stream
 * <-- OUT --> @r
 * tstring - text read
\*/
tstring CMTFFile::sReadWideString(istream &ar)
{
	char c,s;
	tstring str;
	while(true)
	{
		Stream(ar,&s,1);
		Stream(ar,&c,1);
		if (c == 0)
			break;
		str+=c;
	}
	return str;
}

int CMTFFile::nGetSubFormat(void)
{
	int nId=0;
	switch(m_cVersion)
	{
		case 0x10:	nId=1;	break;
		case 0x11:	nId=2;	break;
		case 0x12:	nId=3;	break;
		case 0x13:	nId=4;	break;
	}
	return nId;
}

tstring CMTFFile::sGetFormatName(void)
{
	char cVersion[4];
	sprintf(cVersion,"%d.%d",m_cVersion>>4,m_cVersion&0x0F);
	return "Version " + tstring(cVersion);
}
