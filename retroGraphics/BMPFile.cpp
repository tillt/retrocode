/*\
 * BMPFile.cpp
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
#include "BMPFile.h"
#include "BMPProperty.h"

#ifdef HAVE_UNSIGNED_CHAR
typedef unsigned char U_CHAR;
#define UCH(x)	((int) (x))
#else /* !HAVE_UNSIGNED_CHAR */
#ifdef CHAR_IS_UNSIGNED
typedef char U_CHAR;
#define UCH(x)	((int) (x))
#else
typedef char U_CHAR;
#define UCH(x)	((int) (x) & 0xFF)
#endif
#endif /* HAVE_UNSIGNED_CHAR */

/* Private version of data source object */
DYNIMPPROPERTY(CBMPFile,CBMPProperty)

CBMPFile::CBMPFile(void)
{
	m_pcMagic="BM";
	m_nMagicSize=2;
	m_sFormatName="Bitmap";
}

CBMPFile::~CBMPFile(void)
{
}

void CBMPFile::Read(istream &ar) 
{	
	unsigned char bmpfileheader[14];
	unsigned char bmpinfoheader[64];
#define GET_2B(array,offset)	((unsigned int) UCH(array[offset]) + \
								(((unsigned int) UCH(array[offset+1])) << 8))
#define GET_4B(array,offset)	((signed int) UCH(array[offset]) + \
								(((signed int)UCH(array[offset+1])) << 8) + \
								(((signed int)UCH(array[offset+2])) << 16) + \
								(((signed int)UCH(array[offset+3])) << 24))
	int bfOffBits;
	int headerSize;
	unsigned int biPlanes;
	int biXPelsPerMeter,biYPelsPerMeter;
	int biClrUsed = 0;
	int mapentrysize = 0;		/* 0 indicates no colormap */

	m_nWidth=m_nHeight=0;
	m_nResolutionX=m_nResolutionY=0;
	m_nCompression=0;

	/* Read and verify the bitmap file header */
	ar.read((char *)bmpfileheader,14);
	if (GET_2B(bmpfileheader,0) != 0x4D42) /* 'BM' */
		throw new CFormatException(CFormatException::formaterrInvalid,_T("JERR_BMP_NOT"));

	bfOffBits = (int) GET_4B(bmpfileheader,10);
	/* We ignore the remaining fileheader fields */

	/* The infoheader might be 12 bytes (OS/2 1.x), 40 bytes (Windows),
	* or 64 bytes (OS/2 2.x).  Check the first 4 bytes to find out which.
	*/
	ar.read((char *)bmpinfoheader, 4);
	headerSize = (int) GET_4B(bmpinfoheader,0);
	if (headerSize < 12 || headerSize > 64)
		throw new CFormatException(CFormatException::formaterrInvalid,_T("JERR_BMP_BADHEADER"));
	ar.read((char *)bmpinfoheader+4, headerSize-4);

	switch ((int) headerSize) 
	{
		case 12:	m_nSubFormat=1<<4;		break;
		case 40:	m_nSubFormat=2<<4;		break;
		case 64:	m_nSubFormat=3<<4;		break;
	}

	switch ((int) headerSize) 
	{
		case 12:
			/* Decode OS/2 1.x header (Microsoft calls this a BITMAPCOREHEADER) */
			m_nWidth = (int) GET_2B(bmpinfoheader,4);
			m_nHeight = (int) GET_2B(bmpinfoheader,6);
			biPlanes = GET_2B(bmpinfoheader,8);
			m_nBitsPerPixel = (int) GET_2B(bmpinfoheader,10);

			switch (m_nBitsPerPixel) 
			{
				case 4:
				break;
				case 8:						/* colormapped image */
				{
					//mapentrysize = 3;		/* OS/2 uses RGBTRIPLE colormap */
					TRACEIT("JTRC_BMP_OS2_MAPPED\n");
				}
				break;
				case 24:					/* RGB image */
				{
					TRACEIT("JTRC_BMP_OS2\n");
				}
				break;
				default:
					throw new CFormatException(CFormatException::formaterrInvalid,_T("invalid bitcount per pixel"));
			}
		break;
		case 40:
		case 64:
			/* Decode Windows 3.x header (Microsoft calls this a BITMAPINFOHEADER) */
			/* or OS/2 2.x header, which has additional fields that we ignore */
			m_nWidth = GET_4B(bmpinfoheader,4);
			m_nHeight = GET_4B(bmpinfoheader,8);
			biPlanes = GET_2B(bmpinfoheader,12);
			m_nBitsPerPixel = (int) GET_2B(bmpinfoheader,14);
			m_nCompression = GET_4B(bmpinfoheader,16);
			biXPelsPerMeter = GET_4B(bmpinfoheader,24);
			biYPelsPerMeter = GET_4B(bmpinfoheader,28);
			biClrUsed = GET_4B(bmpinfoheader,32);
			/* biSizeImage, biClrImportant fields are ignored */

			switch (m_nBitsPerPixel) 
			{
				case 4:
				{
					Log2(verbLevDebug2,"JTRC_BMP_NIBBLES\n");
				}
				break;
				case 8:						/* colormapped image */
				{
					mapentrysize = 4;		/* Windows uses RGBQUAD colormap */
					//TRACEMS2(cinfo, 1, JTRC_BMP_MAPPED, (int) biWidth, (int) biHeight);
					Log2(verbLevDebug2,"JTRC_BMP_MAPPED\n");
				}
				break;
				case 24:					/* RGB image */
				{
					//TRACEMS2(cinfo, 1, JTRC_BMP, (int) biWidth, (int) biHeight);
					Log2(verbLevDebug2,"JTRC_BMP\n");
				}
				break;
				default:
				{
					Log2(verbLevDebug1,"bit per pixel %d\n",m_nBitsPerPixel);
					throw new CFormatException(CFormatException::formaterrInvalid,_T("invalid bitcount per pixel"));
				}
			}
			m_nResolutionX=m_nResolutionY=0;
			if (biXPelsPerMeter > 0 && biYPelsPerMeter > 0) 
			{
				m_nResolutionX=(unsigned short int)(biXPelsPerMeter/100); /* 100 cm per meter */
				m_nResolutionY=(unsigned short int)(biYPelsPerMeter/100);
			}
		break;
		default:
			throw new CFormatException(CFormatException::formaterrInvalid,_T("invalid header size"));
	}
	if (m_nCompression < 6)
		++m_nCompression;
	else
		m_nCompression=0;
	m_nSubFormat |=m_nCompression;
	/*
	catch (CFormatException *fe)
	{
		Log("Format Exception: ");
		Log(fe->szGetErrorMessage());
		Log("\n\n");
		delete fe;
	}
	catch (istream::failure const &e)
	{
		TRACEIT("CBMPFile::bLoad - failed to load file\n");
		Log("file read exception: %s\n",e.what());
	}
	*/
}


tstring CBMPFile::sGetFormatName(int nFormat)
{
	tstring strName;
	map<int,tstring> mapFormatName;
	mapFormatName[0x1]=tstring(_T("OS/2 1.x"));
	mapFormatName[0x2]=tstring(_T("Windows 3.x BMP"));
	mapFormatName[0x3]=tstring(_T("OS/2 2.0"));
	
	strName=mapFormatName[(nFormat&0xF0)>>4];
	strName+=", "+sGetCompressionName(nFormat&0x0F);
	return strName;
}

tstring CBMPFile::sGetCompressionName(int nCompressionId)
{
	map<int,tstring> mapCompressionName;
	mapCompressionName[0x0]=tstring(_T("unknown"));
	mapCompressionName[0x1]=tstring(_T("uncompressed"));
	mapCompressionName[0x2]=tstring(_T("RLE8"));
	mapCompressionName[0x3]=tstring(_T("RLE4"));
	mapCompressionName[0x4]=tstring(_T("bitfields"));
	mapCompressionName[0x5]=tstring(_T("JPEG"));
	mapCompressionName[0x6]=tstring(_T("PNG"));
	return mapCompressionName[nCompressionId];
}

int CBMPFile::nGetWidth()
{
	return m_nWidth;
}
int CBMPFile::nGetHeight()
{
	return m_nHeight;
}
int CBMPFile::nGetBitsPerPixel()
{
	return m_nBitsPerPixel;
}

int CBMPFile::nGetColors()
{
	return 1<<m_nBitsPerPixel;
}
