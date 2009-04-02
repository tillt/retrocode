/*\
 * MBMFile.cpp
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
#endif
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <map>
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/FIDProperty.h"
#include "MBMFile.h"
#include "MBMProperty.h"

DYNIMPPROPERTY(CMBMFile,CMBMProperty)

CMBMFile::CMBMFile(void)
{
	m_pcMagic="\67\00\00\20\102\00\00\20";
	m_nMagicSize=8;
	m_sFormatName="Epoc Multi-Bitmap File";
}

CMBMFile::~CMBMFile(void)
{
}

void CMBMFile::Read(istream &ar) 
{
	ar.read((char *)&m_fileHeader,sizeof(m_fileHeader));
	ar.read((char *)&m_imgHeader,sizeof(m_imgHeader));
	TRACEIT2("width %d\n",m_imgHeader.nWidth);
	TRACEIT2("height %d\n",m_imgHeader.nHeight);
	TRACEIT2("bits per pixel %d\n",m_imgHeader.nBitsPerPixel);
	TRACEIT2("compression %d\n",m_imgHeader.nCompression);
	TRACEIT2("color %d\n",m_imgHeader.nImageIsColor);
	ar.seekg(m_fileHeader.nTrailerOffset,ios_base::beg);
	ar.read((char *)&m_nFileCount,4);
	TRACEIT2("bitmaps %d\n",m_nFileCount);
}

tstring CMBMFile::sGetFormatName(int nFormat)
{
	map<int,tstring> mapFormatName;
	mapFormatName[0x00]=tstring(_T("Uncompressed"));
	mapFormatName[0x01]=tstring(_T("8Bit RLE"));
	mapFormatName[0x02]=tstring(_T("12Bit RLE"));
	mapFormatName[0x03]=tstring(_T("16Bit RLE"));
	mapFormatName[0x04]=tstring(_T("24Bit RLE"));
	if(nFormat > 0 && nFormat < 6)
		return mapFormatName[nFormat-1];
	else
		return "Unknown";
}

int CMBMFile::nGetWidth()
{
	return m_imgHeader.nWidth;
}
int CMBMFile::nGetHeight()
{
	return m_imgHeader.nHeight;
}
int CMBMFile::nGetBitsPerPixel()
{
	return m_imgHeader.nBitsPerPixel;
}
int CMBMFile::nGetColors()
{
	return 1<<m_imgHeader.nBitsPerPixel;
}

int CMBMFile::nGetSubFormat()
{
	return m_imgHeader.nCompression+1;
}
