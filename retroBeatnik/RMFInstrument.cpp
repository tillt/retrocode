/*\
 * RMFInstrument.cpp
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
#include <winsock2.h>
#endif
#include "../include/Resource.h"
#include "../retroBase/Basics.h"
#include "../retroBase/MyString.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MobileContent.h"
#include "RMFBasics.h"
#include "RMFInstrument.h"

using namespace std;

CRMFInstrument::CRMFInstrument(const TCHAR *pszName) : m_sName(_T("unknown"))
{
	if (pszName != NULL)
		m_sName=pszName;
}

CRMFInstrument::~CRMFInstrument()
{
}

void CRMFInstrument::Serialize(istream &ar)
{
	unsigned char nNameLen;
	TCHAR sBuffer[256];
	char pcBuffer[5];
	pcBuffer[4]=0;
	tstring s;
	unsigned int nPatch;
	ar.read((char *)&nPatch,4);
	nPatch=ntohl(nPatch);
	Log2(verbLevDebug1,IDS_PRG_PATCHNO,nPatch);
	ar.read((char *)&nNameLen,1);
	if (nNameLen > 0)
	{
		ar.read(sBuffer,nNameLen);
		sBuffer[nNameLen]=0;
		m_sName.assign((const TCHAR *)sBuffer);
	}
	Log2(verbLevDebug3,"name: \"%s\"\n",m_sName.c_str());

	uint32_t nBlockSize;
	unsigned char *pcInst,*pc;

	ar.read((char *)&nBlockSize,4);
	nBlockSize=ntohl(nBlockSize);
	Log2(verbLevDebug3,"inst block size: %d\n",nBlockSize);

	pcInst=new unsigned char[nBlockSize];
	ar.read((char *)pcInst,nBlockSize);

	hexdump("INST header: ",pcInst,min(43,nBlockSize));

	if (nBlockSize >= 43)
	{
		pc=pcInst+43;
		nBlockSize-=43;
		
		while (nBlockSize)
		{
			unsigned int nTag;
			memcpy(&nTag,pc,4);
			pc+=4;
			nBlockSize-=4;
			switch(TOFOURCC(nTag))
			{
				case MAKEFOURCC('A','D','S','R'):
					hexdump("ADSR: ",pc,9);
					pc+=9;
					nBlockSize-=9;
				break;
				case MAKEFOURCC('L','I','N','E'):
					hexdump("LINE: ",pc,8);
					pc+=8;
					nBlockSize-=8;
				break;
				case MAKEFOURCC('S','U','S','T'):
					hexdump("SUST: ",pc,8);
					pc+=8;
					nBlockSize-=8;
				break;
				case MAKEFOURCC('L','A','S','T'):
					if (nBlockSize != 0)
					{
						Log2(verbLevErrors,"block is too large\n");
						throw new CFormatException(CFormatException::formaterrInvalid);
					}
				break;
				default:
					Log2(verbLevWarnings,"unknown inst subtag \"%s\"\n",pcSplitID4(nTag,pcBuffer));
					nBlockSize=0;
			}
		};
	}
	delete pcInst;
}

/*\
 * <---------- Serialize ---------->
 * @m 
 * --> I N <-- @p
 * ostream &ar - 
\*/
void CRMFInstrument::Serialize(ostream &ar)
{
}

/*\
 * <---------- nRender ---------->
 * @m render an instrument to a destination buffer
 * --> I N <-- @p
 * rmfCACH *pCache - internal caching structure
 * unsigned char *pDest - pointer to destination memory or NULL
 * <-- OUT --> @r
 * uint32_t- size of rendered data
\*/
uint32_t CRMFInstrument::nRender(rmfCACH *pCache,unsigned char *pDest)
{
	uint32_t nUsed=0;
	unsigned char data[96] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0xCA, 0x08, 0x00, 0x00, 0x3C, 0x01, 0x90, 0x00, 0x01, 0x00, 0x7F, 
		0x00, 0x00, 0x00, 0x3C, 0x01, 0x90, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x41, 0x44, 0x53, 0x52, 0x04, 
		0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4C, 0x49, 0x4E, 0x45, 0x00, 0x00, 0x10, 0x00, 
		0x00, 0x00, 0xEA, 0x60, 0x4C, 0x49, 0x4E, 0x45, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x53, 0x55, 0x53, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEA, 0x60, 0x4C, 0x41, 0x53, 0x54
	};
	int nLen=(int)m_sName.length();

	if (pDest)
	{
		ASSERT(pCache);
		memcpy(pDest,"INST",4);
		memcpy(pCache->sTag,"INST",4);
		pDest+=4;
	}
	nUsed+=4;
	if (pDest)
	{
		pCache->nFirstValue=0x200;
		pDest=CRMFBasics::pRenderInteger(pDest,pCache->nFirstValue);
	}
	nUsed+=4;
	if (pDest)
	{
		*(pDest++)=(unsigned char)nLen;
		pCache->nNameOffset=nUsed;
		memcpy(pDest,(unsigned char *)m_sName.c_str(),nLen);
		pDest+=nLen;
	}
	nUsed+=nLen+1;
	if (pDest)
	{
		pCache->nDataSize=96;
		pDest=CRMFBasics::pRenderInteger(pDest,pCache->nDataSize);
	}
	nUsed+=4;
	if (pDest)
	{
		pCache->nDataOffset=nUsed;
		memcpy(pDest,data,96);
		pDest+=96;
	}
	nUsed+=96;
	return nUsed;
}
