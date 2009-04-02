/*\
 * GIFFile.cpp
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
#define HAVE_VARARGS_H
#define _HAVE_IO_H
#define _OPEN_BINARY
#include "gif_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <map>
#include "../retroBase/Basics.h"
#include "../retroBase/FIDProperty.h"
#include "../retroBase/MobileContent.h"
#include "GIFFile.h"
#include "GIFProperty.h"

#define APPLICATION_EXT_NETSCAPELOOP	0x01	

typedef struct Gif89GraphicControlExt 
{
	unsigned char Size;
	unsigned char PackedFields;
	unsigned short int Delay;
	unsigned char TransparentColorIndex;
} Gif89GraphicControlExt; 

int gnPlayTime=0;
int gnAvgInterframeDelay=0;


DYNIMPPROPERTY(CGIFFile,CGIFProperty)

CGIFFile::CGIFFile(void)
{
	//ZeroMemory(&m_Header,sizeof(m_Header));
	m_pcMagic="GIFVER";
	m_nMagicSize=6;
	m_sFormatName="Graphics Interchange Format";
}

CGIFFile::~CGIFFile(void)
{
}

int CGIFFile::gifRead (void *GifFile,void *cIn, int nLen)
{
	istream *pArchive=(istream *)((GifFileType *)GifFile)->UserData;
	pArchive->read((char *)cIn,nLen);
	return nLen;
}

void CGIFFile::Read(istream &ar)
{
    int	i, Size, Width=0, Height=0, ExtCode,ExtAppCode;
	GifRecordType RecordType=IMAGE_DESC_RECORD_TYPE;
    GifByteType  *Extension;
    char         *Comment, *AppBlock;
    GifRowType    RowBuffer;
    GifFileType  *GifFile;
	Gif89GraphicControlExt *Ext;

	ExtAppCode=0;

	m_bTransparent=false;
	m_bInterlaced=false;
	m_nLoopCount=0;

	if ((GifFile = DGifOpen(&ar,(int (*)(GifFileType *,GifByteType *,int)) gifRead)) == NULL) 
		throw new CFormatException(CFormatException::formaterrInvalid,sGifError().c_str());
	Comment = (char*) malloc(1);
	AppBlock = (char*) malloc(1);

	strcpy(Comment, "\0");
	strcpy(AppBlock, "\0");

	TRACEIT2("\tScreen Size - Width = %d, Height = %d.\n",GifFile->SWidth, GifFile->SHeight);
	TRACEIT2("\tColorResolution = %d, BitsPerPixel = %d, BackGround = %d.\n",
		GifFile->SColorResolution,
		GifFile->SColorMap->BitsPerPixel,
		GifFile->SBackGroundColor);
	if (GifFile->SColorMap)
	{
		TRACEIT2("\tHas Global Color Map.\n\n");
	}
	else
	{
		TRACEIT2("\tNo Global Color Map.\n\n"); 
	}
	/* Allocate memory or one row which will be used as trash during reading image*/
	Size = GifFile->SWidth * sizeof(GifPixelType);			//Size in bytes one row
	if ((RowBuffer = (GifRowType) malloc(Size)) == NULL)	//First row
		throw new CFormatException(CFormatException::formaterrInvalid,_T("failed to allocate row buffer"));

	/* Scan the content of the GIF file and load the image(s) in: */
	while (RecordType != TERMINATE_RECORD_TYPE)
	{
		TRACEIT2("checking records...\n");
		if (DGifGetRecordType(GifFile, &RecordType) == GIF_ERROR) 
			throw new CFormatException(CFormatException::formaterrInvalid,sGifError().c_str());
		switch (RecordType) 
		{
			case IMAGE_DESC_RECORD_TYPE:
				if (DGifGetImageDesc(GifFile) == GIF_ERROR) 
					throw new CFormatException(CFormatException::formaterrInvalid,sGifError().c_str());
				Width = GifFile->Image.Width;
				Height = GifFile->Image.Height;
				if (GifFile->Image.Left + GifFile->Image.Width > GifFile->SWidth ||
				GifFile->Image.Top + GifFile->Image.Height > GifFile->SHeight) 
					throw new CFormatException(CFormatException::formaterrInvalid,sGifError().c_str());
				//We don't care about Interlaced as image is not interesting, read sequentialy
				for (i = 0; i < Height; i++) 
				{
					if (DGifGetLine(GifFile, &RowBuffer[0], Width) == GIF_ERROR) 
						throw new CFormatException(CFormatException::formaterrInvalid,sGifError().c_str());
				}
			break;
			case EXTENSION_RECORD_TYPE:
				/* Skip any extension blocks in file except comments: */
				if (DGifGetExtension(GifFile, &ExtCode, &Extension) == GIF_ERROR) 
					throw new CFormatException(CFormatException::formaterrInvalid,sGifError().c_str());
				while (Extension != NULL) 
				{
					switch (ExtCode)
					{
						case COMMENT_EXT_FUNC_CODE:
							Extension[Extension[0]+1] = '\000';   //Convert gif's pascal-like string
							Comment = (char*) realloc(Comment, strlen(Comment) + Extension[0] + 1);
							strcat(Comment, (char*)Extension+1);
							TRACEIT2("GIF89 comment: %s\n",Comment);
						break;
						case PLAINTEXT_EXT_FUNC_CODE:
							TRACEIT2("GIF89 plaintext");
						break;
						case GRAPHICS_EXT_FUNC_CODE:
							Ext=(Gif89GraphicControlExt *)Extension;
							gnPlayTime+=Ext->Delay*10;
							if (GifFile->ImageCount < 2)
								gnAvgInterframeDelay=Ext->Delay*10;
							else
								gnAvgInterframeDelay=(gnAvgInterframeDelay+(Ext->Delay*10))/2;
							if ((Ext->PackedFields&0x01) == 0x01)
							{
								TRACEIT2("transparency valid!\n");
								m_bTransparent=true;
							}
							TRACEIT2("Frame Delay: %dms\n",Ext->Delay*10);
						break;
						case APPLICATION_EXT_FUNC_CODE:
							TRACEIT2("GIF89 application block - size:%d\n",Extension[0]);
							if (Extension[0] >= 11)
							{
								if (!memcmp(&Extension[1],"NETSCAPE2.0",11))
								{
									ExtAppCode=APPLICATION_EXT_NETSCAPELOOP;
									TRACEIT2("NETSCAPE2.0 loop extension\n");
								}
							}
							else
							{
								if (ExtAppCode == APPLICATION_EXT_NETSCAPELOOP && Extension[0] >= 3)
								{
									m_nLoopCount=Extension[2] == 0 ? -1 : Extension[2];
									TRACEIT2("loop value: %d\n",m_nLoopCount);
								}
							}
						break;
						default:
							TRACEIT2("Extension record of unknown type\n");
					} 
					if (DGifGetExtensionNext(GifFile, &Extension) == GIF_ERROR) 
						throw new CFormatException(CFormatException::formaterrInvalid,sGifError().c_str());
				}
			break;
			case TERMINATE_RECORD_TYPE:
			break;
			default:
				TRACEIT2("unhandled record type: %d\n",RecordType);
		}
	}; 
	m_sComment=Comment;
	m_nPlaytime=gnPlayTime;
	m_nBitsPerPixel=GifFile->SColorResolution;
	m_nColors=GifFile->SColorMap->ColorCount;
	m_nAverageInterFrameDelay=gnAvgInterframeDelay;
	m_nFrames=GifFile->ImageCount;
	m_nWidth=GifFile->SWidth;
	m_nHeight=GifFile->SHeight;
	m_bInterlaced=GifFile->Image.Interlace != 0;

	if (DGifCloseFile(GifFile) == GIF_ERROR) 
		throw new CFormatException(CFormatException::formaterrInvalid,sGifError().c_str());
}


tstring CGIFFile::sGetFormatName(int nFormat)
{
	map<int,tstring> mapFormatName;
	mapFormatName[0x1]=tstring(_T("GIF87"));
	mapFormatName[0x2]=tstring(_T("GIF89"));
	return mapFormatName[nFormat];
}


bool CGIFFile::bMagicHead(std::istream &ar,uint32_t nSize)
{
	bool bRet=false;
	unsigned char acCompare[7];

	char *pcMagic[2]=
	{
		"GIF87a",
		"GIF89a"
	};
	if (m_nMagicSize == 0 || (int)nSize < 6)
		return false;
	
	ar.read((char *)acCompare,m_nMagicSize);
	if (!memcmp(acCompare,pcMagic[0],6))
	{
		bRet=true;
		m_nFormatVersion=1;
	}
	if (!memcmp(acCompare,pcMagic[1],6))
	{
		bRet=true;
		m_nFormatVersion=2;
	}
	
	return bRet;
}

int CGIFFile::nGetSubFormat()
{
	return m_nFormatVersion;
}

int CGIFFile::nGetWidth()
{
	return m_nWidth;
}

int CGIFFile::nGetPlaytime()
{
	int nPlaytime=m_nPlaytime/1000;
	if (m_nPlaytime%1000)
		nPlaytime++;
	return nPlaytime;
}

int CGIFFile::nGetHeight()
{
	return m_nHeight;
}
int CGIFFile::nGetBitsPerPixel()
{
	return m_nBitsPerPixel;
}
int CGIFFile::nGetColors()
{
	return m_nColors;
}
int CGIFFile::nGetAverageInterFrameDelay()
{
	return m_nAverageInterFrameDelay;
}
int CGIFFile::nGetFrameCount()
{
	return m_nFrames;
}

tstring CGIFFile::sGifError(void) 
{
	int _GifError=GifLastError();
	TCHAR *Err;
    switch (_GifError) 
	{
		case E_GIF_ERR_OPEN_FAILED:
			Err = _T("Failed to open given file");
		break;
		case E_GIF_ERR_WRITE_FAILED:
			Err = _T("Failed to Write to given file");
		break;
		case E_GIF_ERR_HAS_SCRN_DSCR:
			Err = _T("Screen Descriptor already been set");
		break;
		case E_GIF_ERR_HAS_IMAG_DSCR:
			Err = _T("Image Descriptor is still active");
		break;
		case E_GIF_ERR_NO_COLOR_MAP:
			Err = _T("Neither Global Nor Local color map");
		break;
		case E_GIF_ERR_DATA_TOO_BIG:
			Err = _T("#Pixels bigger than Width * Height");
		break;
		case E_GIF_ERR_NOT_ENOUGH_MEM:
			Err = _T("Fail to allocate required memory");
		break;
		case E_GIF_ERR_DISK_IS_FULL:
			Err = _T("Write failed (disk full?)");
		break;
		case E_GIF_ERR_CLOSE_FAILED:
			Err = _T("Failed to close given file");
		break;
		case E_GIF_ERR_NOT_WRITEABLE:
			Err = _T("Given file was not opened for write");
		break;
		case D_GIF_ERR_OPEN_FAILED:
			Err = _T("Failed to open given file");
		break;
		case D_GIF_ERR_READ_FAILED:
			Err = _T("Failed to Read from given file");
		break;
		case D_GIF_ERR_NOT_GIF_FILE:
			Err = _T("Given file is NOT GIF file");
		break;
		case D_GIF_ERR_NO_SCRN_DSCR:
			Err = _T("No Screen Descriptor detected");
		break;
		case D_GIF_ERR_NO_IMAG_DSCR:
			Err = _T("No Image Descriptor detected");
		break;
		case D_GIF_ERR_NO_COLOR_MAP:
			Err = _T("Neither Global Nor Local color map");
		break;
		case D_GIF_ERR_WRONG_RECORD:
			Err = _T("Wrong record type detected");
		break;
		case D_GIF_ERR_DATA_TOO_BIG:
			Err = _T("#Pixels bigger than Width * Height");
		break;
		case D_GIF_ERR_NOT_ENOUGH_MEM:
			Err = _T("Fail to allocate required memory");
		break;
		case D_GIF_ERR_CLOSE_FAILED:
			Err = _T("Failed to close given file");
		break;
		case D_GIF_ERR_NOT_READABLE:
			Err = _T("Given file was not opened for read");
		break;
		case D_GIF_ERR_IMAGE_DEFECT:
			Err = _T("Image is defective, decoding aborted");
		break;
		case D_GIF_ERR_EOF_TOO_SOON:
			Err = _T("Image EOF detected, before image complete");
		break;
		default:
			Err = NULL;
    }
    if (Err != NULL)
	{
        Log2(verbLevErrors,"\nGIF-LIB error: %s.\n", Err);
	}
    else
	{
        Log2(verbLevErrors,"\nGIF-LIB undefined error %d.\n", _GifError);
		Err="undefined error";
	}
	return tstring(Err);
}

LPCTSTR szGetLibGifVersion(void)
{
	static const char *str = GIF_LIB_VERSION;
	return str;
}
