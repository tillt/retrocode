/*\
 * PacketCollection.cpp
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
#include "MyString.h"
#include "Integer.h"
#include "PacketCollection.h"

CBufferEntry::CBufferEntry(unsigned char *pData,uint32_t nSize) 
{
	m_pData=new unsigned char [nSize];
	if (m_pData)
	{
		CopyMemory(m_pData,pData,nSize);
		m_nSize=nSize;
	}
}

CBufferEntry::~CBufferEntry(void)
{
	if (m_pData)
		delete [] m_pData;
}

CBufferCollector::CBufferCollector(void) : m_nTotalSize(0)
{
}

CBufferCollector::~CBufferCollector(void)
{
	unsigned int i;
	for (i=0;i < (unsigned int)m_Packets.size();i++)
	{
		if (m_Packets[i] != NULL)
			delete m_Packets[i];
	}
	if (m_Packets.size())
		m_Packets.erase(m_Packets.begin(), m_Packets.end());
}

void CBufferCollector::CreateCopyPacket(unsigned char *pSource,uint32_t nSize)
{
	CBufferEntry *pPacket=new CBufferEntry(pSource,nSize);
	m_nTotalSize+=nSize;
	m_Packets.push_back(pPacket);
}

uint32_t CBufferCollector::nCopyLinear(unsigned char *pDest,uint32_t nSize)
{
	int i=0;
	uint32_t nCopied=0;
	uint32_t nToCopy=0;
	if (nSize <= m_nTotalSize)
	{
		while (nSize)
		{
			nToCopy=min(m_Packets[i]->nGetSize(),nSize);
			CopyMemory(pDest,m_Packets[i]->pGetData(),nToCopy);
			pDest+=nToCopy;
			nSize-=nToCopy;
			nCopied+=nToCopy;
			i++;
		};
	}
	return nCopied;
}
