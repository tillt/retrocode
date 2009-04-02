/*\
 * MonoFile.cpp
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
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "MonoFile.h"
#include "../include/Resource.h"

CMonoContent::CMonoContent(void)
{
	m_pcData=NULL;
	m_nMaxSize=0;
	m_nEventCount=0;
}

CMonoContent::~CMonoContent(void)
{
}

unsigned char CMonoContent::cReadHexByte(std::istream &ar)
{
	unsigned int val;
	unsigned char pcValue[3]={0,0,0};
	ar.read((char *)pcValue,2);
	sscanf((const char *)pcValue,"%02x",&val);
	return (unsigned char)val;
}

void CMonoContent::HexToBin(unsigned char *pcDest,unsigned char *pcSrc,int nSize)
{
	unsigned int val;
	char pcValue[3]={0,0,0};
	while (nSize)
	{
		memcpy(pcValue,pcSrc,2);
		sscanf((const char *)pcValue,"%02x",&val);
		*(pcDest++)=(unsigned char)val;
		pcSrc+=2;
		nSize-=2;
	};
}

bool CMonoContent::bIsText(unsigned char theChar)
{
	return (theChar >= 0x09 && theChar <= 0x7F);
}

bool CMonoContent::bIsHex(unsigned char theChar)
{
	return ((theChar >= '0' && theChar <= '9') ||
			(theChar >= 'A' && theChar <= 'F') ||
			(theChar >= 'a' && theChar <= 'f'));
}

int CMonoContent::nGetPlaytime(void)
{
	unsigned int nSize=((unsigned int) ( ((double)m_nLastQuanta/125) + 0.5) );
	if (m_nTempo)
		return (int)(((uint64_t)nSize*100000) / (8*m_nTempo));
	else
		return 0;
}

tstring CMonoContent::sGetEncodingName(int nEncoding)
{
	CMyString strText;
	strText.Load(IDS_FENCODING_BINARY+nEncoding);
	return strText;
}

int CMonoContent::nGetSMSCount(void)
{
	int nSms=m_nSize/m_nMaxSize;
	if (m_nSize%m_nMaxSize)
		nSms++;
	return nSms;
}
