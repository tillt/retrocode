/*\
 * Endian.cpp
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
#include <Winsock2.h>
#else
#include <netinet/in.h>
#endif
#include "Integer.h"
#include "Endian.h"

Endian::Endian(void)
{
}

/*\ 
 * <---------- Endian::init ----------> 
 * @m initialize endian handling
\*/ 
void Endian::init(void)
{
	char acTest[2] = {0x01,0x00};
	if(*(int16_t *)&acTest[0] == 1)		//are we runnning on a little endian host?
	{										//nope->...
		bBigEndianSystem = false;			
		wToBig = wSwap;
		wToLittle = wNoSwap;
		lToBig = lSwap;
		lToLittle = lNoSwap;
		fToBig = fSwap;
		fToLittle = fNoSwap;
	}
	else
	{										//yes->...
		bBigEndianSystem = true;
		wToBig = wNoSwap;
		wToLittle = wSwap;
		lToBig = lNoSwap;
		lToLittle = lSwap;
		fToBig = fSwap;
		fToLittle = fNoSwap;
	}
}

/*\ 
 * <---------- Endian::bIsBigEndian ----------> 
 * @m tells you if the host is a big-endian machine
 * NOTE: we dont deal with middle-endian
 * <-- OUT --> @p
 * bool - true=big endian, false=little endian
\*/ 
bool Endian::bIsBigEndian(void)
{
	return bBigEndianSystem;
}

/*\ 
 * <---------- Endian::HostFromBigShortArray ----------> 
 * @m converts big endian data block into host endian data format
 * --> I N <-- @p
 * unsigned short int *pcDest - destination pointer
 * unsigned short int *pcSource - source pointer
 * uint32_tnSize - in bytes
\*/ 
void Endian::HostFromBigShortArray(uint16_t *pcDest,uint16_t *pcSource,uint32_t nSize)
{
	uint16_t *pcLimit=pcSource+(nSize/2);
	while (pcSource < pcLimit)
	{
		*pcDest=ntohs(*pcSource);
		++pcSource;
		++pcDest;
	};
}

/*\ 
 * <---------- Endian::BigFromHostShortArray ----------> 
 * @m converts host endian data block into big endian data format
 * --> I N <-- @p
 * unsigned short int *pcDest - destination pointer
 * unsigned short int *pcSource - source pointer
 * uint32_tnSize - in bytes
\*/ 
void Endian::BigFromHostShortArray(uint16_t *pcDest,uint16_t *pcSource,uint32_t nSize)
{
	uint16_t *pcLimit=pcSource+(nSize/2);
	while (pcSource < pcLimit)
	{
		*pcDest=htons(*pcSource);
		++pcSource;
		++pcDest;
	};
}

/*\ 
 * <---------- Endian::HostFromLittleShortArray ----------> 
 * @m converts little endian data block into host data format
 * --> I N <-- @p
 * unsigned short int *pcDest - destination pointer
 * unsigned short int *pcSource - source pointer
 * uint32_tnSize - in bytes
\*/ 
void Endian::HostFromLittleShortArray(uint16_t *pcDest,uint16_t *pcSource,uint32_t nSize)
{
	uint16_t *pcLimit=pcSource+(nSize/2);
	while (pcSource < pcLimit)
	{
		*pcDest=wFromLittle(*pcSource);
		++pcSource;
		++pcDest;
	};
}

int16_t Endian::wSwap(int16_t s)
{
	return wSwap(&s);
}

int16_t Endian::wSwap(int16_t *s)
{
	int16_t  ret=0;
	unsigned char *r=(unsigned char *)&ret;
	unsigned char *b=(unsigned char *)s;
	r[0]=b[1];
	r[1]=b[0];	
	return ret;
}

int16_t Endian::wNoSwap(int16_t s)
{
	return s;
}

int32_t Endian::lSwap (int32_t i)
{
	return lSwap(&i);
}

int32_t Endian::lSwap (int32_t *i)
{
	int32_t ret=0;
	unsigned char *r=(unsigned char *)&ret;
	unsigned char *b=(unsigned char *)i;
	r[0]=b[3];
	r[1]=b[2];
	r[2]=b[1];
	r[3]=b[0];
	return ret;
}

int32_t Endian::lNoSwap(int32_t i)
{
	return i;
}

float Endian::fSwap(float f)
{
	union
	{
		float f;
		unsigned char b[4];
	} valueA, valueB;
	valueA.f = f;
	valueB.b[0] = valueA.b[3];
	valueB.b[1] = valueA.b[2];
	valueB.b[2] = valueA.b[1];
	valueB.b[3] = valueA.b[0];
	return valueB.f;
}

float Endian::fNoSwap(float f)
{
	return f;
}
