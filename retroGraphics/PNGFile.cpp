/*\
 * PNGFile.cpp
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
#include "PNGFile.h"
#include "../retroBase/FIDProperty.h"
#include "PNGProperty.h"
#include "png.h"

png_const_charp msg;

static png_structp png_ptr = NULL;
static png_infop info_ptr = NULL;

DYNIMPPROPERTY(CPNGFile,CPNGProperty)

CPNGFile::CPNGFile(void)
{
	m_pcMagic="\211PNG\15\12\32\12";
	m_nMagicSize=8;
	m_sFormatName="Portable Networks Graphic";
}

CPNGFile::~CPNGFile(void)
{
}

static void png_cexcept_error(png_structp png_ptr, png_const_charp msg)
{
   TRACEIT2("libpng error: %s\n", msg);
   throw new CFormatException(CFormatException::formaterrInvalid,msg);
}


static void png_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	istream *pArchive=(istream *)png_ptr->io_ptr;
	try
	{
		pArchive->read((char *)data,(unsigned int)length);
	}
	catch (istream::failure const &e)
	{
		TRACEIT2("failed to load file\n");
		png_error(png_ptr, "Read Error");
		Log2(verbLevErrors,"file read exception: %s\n",e.what());
	}
}

void CPNGFile::Read(istream &ar) 
{
	int nInterlacedType;
	int nCompressionType;
    //int i;
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,(png_error_ptr)png_cexcept_error, (png_error_ptr)NULL);
    try
    {
		if (!png_ptr)
		{
			TRACEIT2("png_create_read_struct returned invalid value\n");
			throw new CFormatException(CFormatException::formaterrUnknown);
		}
		info_ptr = png_create_info_struct(png_ptr);
		if (!info_ptr)
		{
			TRACEIT2("png_create_info_struct returned invalid value\n");
			throw new CFormatException(CFormatException::formaterrUnknown);
		}
        // initialize the png structure
        png_set_read_fn(png_ptr, (png_voidp)&ar, png_read_data);
        // read all PNG info up to image data
        png_read_info(png_ptr, info_ptr);
        // get width, height, bit-depth and color-type
        png_get_IHDR(	png_ptr, 
						info_ptr,
						(png_uint_32 *)&m_nWidth,
						(png_uint_32 *)&m_nHeight,
						&m_nBitsPerPixel, 
						&m_nSubFormat, 
						&nInterlacedType, 
						&nCompressionType,
						NULL);
    }
	catch (CFormatException *fe)
	{
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		throw fe;
	}
	m_bInterlaced=nInterlacedType != PNG_INTERLACE_NONE;
}

tstring CPNGFile::sGetFormatName(int nFormat)
{
	map<int,tstring> mapFormatName;
	mapFormatName[PNG_COLOR_TYPE_PALETTE]=tstring(_T("Paletted"));
	mapFormatName[PNG_COLOR_TYPE_GRAY]=tstring(_T("Grayscale"));
	mapFormatName[PNG_COLOR_TYPE_RGB]=tstring(_T("Colored"));
	mapFormatName[PNG_COLOR_TYPE_RGB_ALPHA]=tstring(_T("Colored+Alpha"));
	mapFormatName[PNG_COLOR_TYPE_GRAY_ALPHA]=tstring(_T("Grayscale+Alpha"));
	if (nFormat)
		return mapFormatName[nFormat-1];
	else
		return tstring(_T("Unknown"));
}

int CPNGFile::nGetWidth()
{
	return m_nWidth;
}
int CPNGFile::nGetHeight()
{
	return m_nHeight;
}
int CPNGFile::nGetBitsPerPixel()
{
	return m_nBitsPerPixel;
}

int CPNGFile::nGetColors()
{
	return 1<<m_nBitsPerPixel;
}

LPCTSTR szGetLibPngVersion(void)
{
	static const char *str = PNG_LIBPNG_VER_STRING;
	return str;
}
