/*\
 * ZIPArchive.cpp
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
#include "unzip/unzip.h"
#include "zlib.h"
#include "ZIPArchive.h"

#define WRITEBUFFERSIZE (8192)
#define CASESENSITIVITY (1)

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE (0xFFFFFFFF)
#endif

#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif

voidpf ZCALLBACK win32_open_file_func OF((voidpf opaque,const char* filename,int mode));
uLong ZCALLBACK win32_read_file_func OF((voidpf opaque,voidpf stream,void* buf,uLong size));
uLong ZCALLBACK win32_write_file_func OF((voidpf opaque,voidpf stream,const void* buf,uLong size));
long ZCALLBACK win32_tell_file_func OF((voidpf opaque,voidpf stream));
long ZCALLBACK win32_seek_file_func OF((voidpf opaque,voidpf stream,uLong offset,int origin));
int ZCALLBACK win32_close_file_func OF((voidpf opaque,voidpf stream));
int ZCALLBACK win32_error_file_func OF((voidpf opaque,voidpf stream));

typedef struct
{
    istream *pAr;
    int error;
} WIN32FILE_IOWIN;

voidpf ZCALLBACK win32_open_file_func (voidpf opaque, const char *filename, int mode)
{
	WIN32FILE_IOWIN w32fiow;
	w32fiow.error = 0;
	w32fiow.pAr = (istream *)filename;
	voidpf ret=NULL;

	ret = malloc(sizeof(WIN32FILE_IOWIN));

	*((WIN32FILE_IOWIN *)ret) = w32fiow;
	return ret;
}

uLong ZCALLBACK win32_read_file_func (voidpf opaque, voidpf stream, void* buf, uLong size)
{
	uLong ret=0;
	istream *pArchive=NULL;

	if (stream!=NULL)
		pArchive = ((WIN32FILE_IOWIN*)stream) -> pAr;
	if (pArchive != NULL)
	{
		try
		{
			pArchive->read((char *)buf,(unsigned int)size);
			ret=size;
		}
		catch (istream::failure const &e)
		{
			Log2(verbLevErrors,"file read exception: %s\n",e.what());
            ((WIN32FILE_IOWIN*)stream) -> error=(int)0;
			ret=pArchive->gcount();
		}
	}
	return ret;
}

uLong ZCALLBACK win32_write_file_func (voidpf opaque, voidpf stream, const void* buf, uLong size)
{
    return 0;
}

long ZCALLBACK win32_tell_file_func (voidpf opaque, voidpf stream)
{
    long ret=-1;
    istream *pArchive = NULL;
    if (stream!=NULL)
		pArchive = ((WIN32FILE_IOWIN*)stream) -> pAr;
    if (pArchive != NULL)
    {
		try
		{
			ret=(long)pArchive->tellg();
		}
		catch (istream::failure const &e)
		{
			Log2(verbLevErrors,"file exception: %s\n",e.what());
			ret=-1;
		}
    }
    return ret;
}

long ZCALLBACK win32_seek_file_func (voidpf opaque, voidpf stream, uLong offset, int origin)
{
	ios_base::seekdir dwMoveMethod=ios_base::cur;
    istream *pArchive = NULL;

    long ret=-1;
	if (stream!=NULL)
		pArchive = ((WIN32FILE_IOWIN*)stream) -> pAr;

    switch (origin)
    {
		case ZLIB_FILEFUNC_SEEK_CUR :
			dwMoveMethod = ios_base::cur;
		break;
		case ZLIB_FILEFUNC_SEEK_END :
			dwMoveMethod = ios_base::end;
		break;
		case ZLIB_FILEFUNC_SEEK_SET :
			dwMoveMethod = ios_base::beg;
		break;
		default: 
			return -1;
    }
    if (pArchive != NULL)
    {
		try
		{
			pArchive->seekg(offset,dwMoveMethod);
			ret=0;
		}
		catch (istream::failure const &e)
		{
			Log2(verbLevErrors,"file exception: %s\n",e.what());
			ret=-1;
		}
    }
    return ret;
}

int ZCALLBACK win32_close_file_func (voidpf opaque, voidpf stream)
{
	int ret=-1;
    if (stream != NULL)
    {
        istream *pArchive;
        pArchive = ((WIN32FILE_IOWIN*)stream) -> pAr;
        if (pArchive != NULL)
        {
            ret=0;
        }
        free(stream);
    }
    return ret;
}

int ZCALLBACK win32_error_file_func (voidpf opaque, voidpf stream)
{
    int ret=-1;
    if (stream!=NULL)
    {
        ret = ((WIN32FILE_IOWIN*)stream) -> error;
    }
    return ret;
}

CZIPArchive::CZIPArchive(void)
{
}

CZIPArchive::~CZIPArchive(void)
{
}

void CZIPArchive::InitFileXS(zlib_filefunc_def *pzlib_filefunc_def)
{
    pzlib_filefunc_def->zopen_file = win32_open_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell_file = win32_tell_file_func;
    pzlib_filefunc_def->zseek_file = win32_seek_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque=NULL;
}

int CZIPArchive::nExtractCurrentFile(const char* password,char **ppcDest,unsigned int *pnSize,char *szComment,int nCommentBufferSize,bool bIsText)
{
	char filename_inzip[256];
	char* filename_withoutpath;
	char* p;
	int err=UNZ_OK;
	void* buf;
	uInt size_buf;
	char *pcWrite;

	unz_file_info file_info;
	uLong ratio=0;
	err = unzGetCurrentFileInfo(m_uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,szComment,nCommentBufferSize);

	if (err!=UNZ_OK)
	{
		Log2(verbLevErrors,"error %d with zipfile in unzGetCurrentFileInfo\n",err);
		return err;
	}

	size_buf = WRITEBUFFERSIZE;
	buf = (void*)malloc(size_buf);
	if (buf==NULL)
	{
		Log2(verbLevErrors,"Error allocating memory\n");
		return UNZ_INTERNALERROR;
	}

	p = filename_withoutpath = filename_inzip;
	while ((*p) != '\0')
	{
		if (((*p)=='/') || ((*p)=='\\'))
			filename_withoutpath = p+1;
		p++;
	}

	const char *write_filename;
	int skip=0;

	write_filename = filename_withoutpath;

	err = unzOpenCurrentFilePassword(m_uf,password);

	if (err != UNZ_OK)
	{
		Log2(verbLevErrors,"error %d with zipfile in unzOpenCurrentFilePassword\n",err);
	}
	if (err == UNZ_OK)
	{
		*pnSize=file_info.uncompressed_size;
		if (file_info.uncompressed_size)
		{
			if (bIsText)
				*ppcDest=new char[file_info.uncompressed_size+1];
			else
				*ppcDest=new char[file_info.uncompressed_size];
		}
	}

	if (*ppcDest != NULL)
	{
		pcWrite=*ppcDest;
		TRACEIT2("extracting: %s\n",write_filename);
		do
		{
			err = unzReadCurrentFile(m_uf,buf,size_buf);
			if (err<0)
			{
				Log2(verbLevErrors,"error %d with zipfile in unzReadCurrentFile\n",err);
				break;
			}
			if (err > 0)
			{
				memcpy(pcWrite,buf,err);
				pcWrite+=err;
			}
		}while (err > 0);
	}

	if (err == UNZ_OK)
	{
		err = unzCloseCurrentFile (m_uf);
		if (err != UNZ_OK)
		{
			Log2(verbLevErrors,"error %d with zipfile in unzCloseCurrentFile\n",err);
		}
	}
	else
		unzCloseCurrentFile(m_uf); /* don't lose the error */
	free(buf);
	if (bIsText && pcWrite)
		*pcWrite=0;
	return err;
}

bool CZIPArchive::bFindFile(const char *filename)
{
	bool bRet=true;
	if (unzLocateFile(m_uf,filename,CASESENSITIVITY) != UNZ_OK)
	{
		TRACEIT2("file %s not found in the zipfile\n",filename);
		bRet=false;
	}
	return bRet;
}

bool CZIPArchive::bOpen(istream &ar)
{
	zlib_filefunc_def ffunc;
	InitFileXS(&ffunc);
	return (m_uf = unzOpen2((const char *)&ar,&ffunc)) != NULL;
}

void CZIPArchive::Close(void)
{
	unzCloseCurrentFile(m_uf);
}

tstring CZIPArchive::sGetGlobalComment(void)
{
	char szComment[1024];
	unzGetGlobalComment (m_uf,szComment,1023);
	return tstring(szComment);
}

/*\ 
 * <---------- ----------> 
 * @m get displayable version information
 * <-- OUT --> @r
 * LPCTSTR - get DLL version info
\*/ 
LPCTSTR szGetZlibVersion(void)
{
	static const char *str = ZLIB_VERSION;
	return str;
}
