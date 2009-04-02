/*\
 * PngMemoryBitmap.cpp
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

#include "PngMemoryBitmap.h"
#include "png.h"

CPngMemoryBitmap::CPngMemoryBitmap(unsigned int nWidth,unsigned int nHeight,unsigned int nFramePix,MyCol *pColBack,MyCol *pColPen) : CMemoryBitmap(nWidth,nHeight,nFramePix,pColBack,pColPen) 
{
}

CPngMemoryBitmap::~CPngMemoryBitmap(void)
{
}

/*
#ifdef ENABLE_DDB_EXPORT
HBITMAP CMemoryBitmap::hGetDDB(HDC hDeviceContext)
{
	HBITMAP hbm;
	void *pDeviceBitmap;
	
	if ((hbm=CreateAccessableRGBBitmap(hDeviceContext,m_nWidth,m_nHeight,&pDeviceBitmap)) != NULL)
	{
		if (pDeviceBitmap != NULL)
		{
			CopyMemory(pDeviceBitmap,m_pcImage,m_nSize);
		}
		else
		{
			TRACEIT2("invalid bitmap pointer\n");
		}
	}
	else
	{
		TRACEIT2("couldnt generate accessable bitmap\n");
	}
	return hbm;
}
#endif
*/


/*\
 * <---------- bWritePng ---------->
 * @m write a PNG image file
 * --> I N <-- @p
 * const char *file_name - png file path
 * <-- OUT --> @r
 * bool - true=done
\*/
bool CPngMemoryBitmap::bWritePng(const char *file_name)
{
	FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;
	png_colorp palette;
	png_uint_32 k;
	bool bRet=false;

	if (m_pcImage)
	{
		if ((fp = fopen(file_name, "wb")) != NULL)
		{
			if ((png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)) != NULL)
			{
				if ((info_ptr = png_create_info_struct(png_ptr)) != NULL)
				{
					if (!setjmp(png_jmpbuf(png_ptr)))
					{
						png_init_io(png_ptr, fp);
						
						png_set_IHDR(	png_ptr, info_ptr, 
										m_nWidth, m_nHeight, 
										8, 
										PNG_COLOR_TYPE_RGB,
										PNG_INTERLACE_NONE, 
										PNG_COMPRESSION_TYPE_BASE, 
										PNG_FILTER_TYPE_BASE);

						palette = (png_colorp)png_malloc(png_ptr, PNG_MAX_PALETTE_LENGTH * png_sizeof (png_color));
						png_set_PLTE(png_ptr, info_ptr, palette, PNG_MAX_PALETTE_LENGTH);
						png_write_info(png_ptr, info_ptr);

						png_bytep *prow_pointers=new png_bytep[m_nHeight];

						if (m_nHeight > PNG_UINT_32_MAX/png_sizeof(png_bytep))
							png_error (png_ptr, "Image is too tall to process in memory");
						else
						{
							for (k = 0; k < m_nHeight; k++)
								prow_pointers[k] = m_pcImage + k*m_nWidth*sizeof(MyCol);

							png_write_rows(png_ptr, prow_pointers, m_nHeight);
							png_write_end(png_ptr, info_ptr);
							bRet=true;
						}
						png_free(png_ptr, palette);
					}
					png_destroy_write_struct(&png_ptr, &info_ptr);
				}
				else
					png_destroy_write_struct(&png_ptr,  png_infopp_NULL);
			}
			fclose(fp);
		}
	}
	else
	{
		//TRACEIT2("no bitmap in memory!\n");
	}
	return bRet;
}

#ifdef ENABLE_DDB_EXPORT
/*
HBITMAP CMemoryBitmap::CreateAccessableRGBBitmap(HDC hdc,unsigned int nWidth,unsigned int nHeight,void **pDirect)
{
	HBITMAP hb;
	BITMAPINFO bi;
	ZeroMemory (&bi,sizeof(BITMAPINFOHEADER));
	bi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth=nWidth;
	bi.bmiHeader.biHeight=nHeight;
	bi.bmiHeader.biPlanes=1;
	bi.bmiHeader.biBitCount=24;
	bi.bmiHeader.biCompression=BI_RGB;
	bi.bmiHeader.biSizeImage=0;
	bi.bmiHeader.biXPelsPerMeter=0;
	bi.bmiHeader.biYPelsPerMeter=0;
	bi.bmiHeader.biClrUsed=0;
	bi.bmiHeader.biClrImportant=0;
	hb=CreateDIBSection (	hdc,					//handle to device context
							&bi,					//pointer to structure containing bitmap size, format, and color data
							DIB_RGB_COLORS,			//color data type indicator: RGB values or palette indices
							pDirect,				//pointer to variable to receive a pointer to the bitmap's bit values
							NULL,					//optional handle to a file mapping object
							0		);				//offset to the bitmap bit values within the file mapping object
	GdiFlush ();
	return hb;
}
*/
#endif
