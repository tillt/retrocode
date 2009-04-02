/*\
 * RMFBasics.cpp
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
#include <map>
#ifndef WIN32
#include <netinet/in.h>
#else
#include <winsock2.h>
#endif
#include "../retroBase/Basics.h"
#include "RMFBasics.h"

/*\
 * <---------- DecryptBinary ---------->
 * @m decrypt binary data
 * --> I N <-- @p
 * unsigned char *pcIn - input data
 * unsigned char *pcOut - output buffer
 * int nLen - data size
\*/
void CRMFBasics::DecryptBinary(unsigned char *pcIn,unsigned char *pcOut,int nLen)
{
	uint32_t nSeed=cryptionSeed;
	unsigned char cInByte=0x00;
	unsigned char cOutByte=0x00;
	uint32_t nOffset=0;
	while (nLen--)
	{
		cInByte=*(pcIn+nOffset);
		cOutByte=(unsigned char)(((nSeed >> 8)&0x000000FF)^cInByte);
		*(pcOut+nOffset)=cOutByte;
		nSeed=((cInByte+nSeed) * cryptionMultiplier) + cryptionOffset;		
		++nOffset;
	};
}

/*\
 * <---------- EncryptBinary ---------->
 * @m encrypt binary data
 * --> I N <-- @p
 * unsigned char *pcIn - pointer to input buffer
 * unsigned char *pcOut - pointer to output buffer
 * int nLen - size of the input buffer
\*/
void CRMFBasics::EncryptBinary(unsigned char *pcIn,unsigned char *pcOut,int nLen)
{
	uint32_t nSeed=cryptionSeed;
	unsigned char cInByte=0x00;
	unsigned char cOutByte=0x00;
	uint32_t nOffset=0;
	while (nLen--)
	{
		cInByte=*(pcIn+nOffset);
		cOutByte=(unsigned char)(((nSeed >> 8)&0x000000FF)^cInByte);
		*(pcOut+nOffset)=cOutByte;
		nSeed=((cOutByte+nSeed) * cryptionMultiplier) + cryptionOffset;		
		++nOffset;
	};
}

/*\
 * <---------- nEncryptText ---------->
 * @m encrypt text
 * --> I N <-- @p
 * const unsigned char *pcIn - 
 * unsigned char *pcOut - 
 * uint32_tnMaxLen - 
 * <-- OUT --> @r
 * int - number of bytes used for storing the result
\*/
int CRMFBasics::nEncryptText(const unsigned char *pcIn,unsigned char *pcOut,uint32_t nMaxLen)
{
	uint32_t nSeed=cryptionSeed;
	unsigned char cInByte=0x00;
	unsigned char cOutByte=0x00;
	uint32_t nOffset=0;
	do
	{
		cInByte=*(pcIn+nOffset);
		cOutByte=(unsigned char)(((nSeed >> 8)&0x000000FF)^cInByte);
		*(pcOut+nOffset)=cOutByte;
		nSeed=((cOutByte+nSeed) * cryptionMultiplier) + cryptionOffset;		
		++nOffset;
	}while (cInByte != 0x00 && nOffset < nMaxLen);
	return nOffset;
}

/*\
 * <---------- nDecryptText ---------->
 * @m decrypt text
 * --> I N <-- @p
 * const unsigned char *pcIn - 
 * unsigned char *pcOut - 
 * uint32_tnMaxLen - 
 * <-- OUT --> @r
 * int - number of bytes used for storing the source data
\*/
int CRMFBasics::nDecryptText(const unsigned char *pcIn,unsigned char *pcOut,uint32_t nMaxLen)
{
	uint32_t nSeed=cryptionSeed;
	unsigned char cInByte=0x00;
	unsigned char cOutByte=0x00;
	uint32_t nOffset=0;
	do
	{
		cInByte=*(pcIn+nOffset);
		cOutByte=(unsigned char)(((nSeed >> 8)&0x000000FF)^cInByte);
		*(pcOut+nOffset)=cOutByte;
		nSeed=((cInByte+nSeed) * cryptionMultiplier) + cryptionOffset;		
		++nOffset;
	}while (cOutByte != 0x00 && nOffset < nMaxLen);
	return nOffset;
}

/*\
 * <---------- bReadTag ---------->
 * @m reads a complete RMF tag 
 * --> I N <-- @p
 * istream &ar - input stream
 * uint32_t*pnTag - destination tag pointer
 * uint32_t*pnNextOffset - destination next offset pointer
 * <-- OUT --> @r
 * bool - TRUE=ok
\*/
bool CRMFBasics::bReadTag (istream &ar, uint32_t *pnTag, uint32_t *pnNextOffset)
{
	char pcBuffer[4];
	ar.read(pcBuffer,4);
	*pnNextOffset=ntohl(*(uint32_t*)pcBuffer);
	ar.read(pcBuffer,4);
	*pnTag=nMakeID4(pcBuffer);
	return true;
}

/*\
 * <---------- pcRenderINT ---------->
 * @m render 4 byte integer into a memory buffer
 * --> I N <-- @p
 * unsigned char *pcDest - pointer to destination buffer
 * uint32_tnValue - integer value to render
 * <-- OUT --> @r
 * unsigned char * - pointing after the end of the output string
\*/
unsigned char *CRMFBasics::pRenderInteger(unsigned char *pcDest,uint32_t nValue)
{
	uint32_t nRMFValue=htonl(nValue);
	memcpy(pcDest,&nRMFValue,4);
	pcDest+=4;
	return pcDest;
}
