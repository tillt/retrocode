/*\
 * SMAFDecoder.cpp
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
#ifndef WIN32
#include <netinet/in.h>
#else
#include <WinSock2.h>
#endif
#include "../include/Resource.h"
#include "../retroBase/Basics.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MyString.h"
#include "../retroBase/MobileContent.h"
#include "SMAFDecoder.h"

CSMAFDecoder::CSMAFDecoder()
{
}

CSMAFDecoder::~CSMAFDecoder()
{
}

void CSMAFDecoder::DecodeChunk(unsigned char **pcBuffer,unsigned char *pcLimit,tstring &sIdentifier,unsigned int *pnSize,unsigned char *pcChunkAttribute)
{
	int i;

	if (*pcBuffer+4 >= pcLimit)
	{
		TRACEIT("CSMAFDecoder::DecodeChunk - truncated chunk - id missing/incomplete\n");
		throw new CFormatException(CFormatException::formaterrTruncated,"id missing/incomplete");
	}

	sIdentifier.clear();
	for (i=0;i < 4;i++)
	{
		if (**pcBuffer == '#')
			break;
		if (**pcBuffer < ' ' || **pcBuffer > 'z')
			break;
		sIdentifier+=**pcBuffer;
		*pcBuffer+=1;
	}
	while (i++ < 4)
	{
		*pcChunkAttribute=**pcBuffer;
		*pcBuffer+=1;
	};

	if (*pcBuffer+4 > pcLimit)
	{
		TRACEIT("CSMAFDecoder::DecodeChunk - truncated chunk - size missing/incomplete\n");
		throw new CFormatException(CFormatException::formaterrTruncated,"size missing/incomplete");
	}

	//*pnSize=ntohl(*(unsigned int *)*pcBuffer);
	memcpy(pnSize,*pcBuffer,4);
	*pnSize=ntohl(*pnSize);
	*pcBuffer+=4;
	if (*pcBuffer+*pnSize > pcLimit)
	{
		TRACEIT("CSMAFDecoder::DecodeChunk - chunk size (%d) exceeds file by %d byte\n",*pnSize,(*pcBuffer+*pnSize)-pcLimit);
		throw new CFormatException(CFormatException::formaterrInvalidChunkSize,"chunk size exceeds file size");
	}
}
