/*\
 * MSEQFile.cpp
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
#include <map>
#include <iostream>
#include <fstream>
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/FIDProperty.h"
#include "MonoFile.h"
#include "MSEQFile.h"
#include "MSEQProperty.h"

DYNIMPPROPERTY(CMSEQFile,CMSEQProperty)

CMSEQFile::CMSEQFile(void)
{
	m_nMagicSize=4;
	m_pcMagic=".SEQ";
	m_sFormatName="Multimedia Sequential Event Quantification Format";
	//ZeroMemory(&m_Header,sizeof(m_Header));
}

CMSEQFile::~CMSEQFile(void)
{
}

void CMSEQFile::Read(istream &ar)
{
	ReadHeader(ar,&m_Header);
}

void CMSEQFile::ReadHeader(istream &ar, MSEQHeader *pMseq) 
{ 
   //Read the first 4 bytes, which will hold 'RIFF' 
	unsigned char cIn;
	//magic
	ar.read(pMseq->sID,sizeof(pMseq->sID));
	//some status flags
	ar >> cIn;
	TRACEIT("class: %d\n",cIn&0x0F);
	TRACEIT("EMS: %d\n",(cIn&0x10)>>4);
	TRACEIT("TMODE: %d\n",(cIn&0xC0)>>6);

	pMseq->nClass=cIn&0x0F;
	pMseq->bEms=((cIn&0x10)>>4) != 0;
	pMseq->nTmode=((cIn&0xC0)>>6) != 0;			//0 multiprocessor
												//1 streaming
												//2 packet
												//3 reserved

	//filesize LSB
	ar >> cIn;
	pMseq->nFilesize=(unsigned int)cIn;
	//filesize MSB
	ar >> cIn;
	pMseq->nFilesize|=(unsigned int)cIn<<8;
	ar >> cIn;
	pMseq->nFilesize|=(unsigned int)cIn<<16;

	//copy status
	ar >> cIn;
	TRACEIT("copy status: %d\n",cIn&0x03);
	pMseq->nCopyStatus=cIn&0x03;				//0 global copyprotect
												//1 global copyprotect after copy countdown
												//2 see tracks copy status
												//3 global unprotected

	TRACEIT("copy countdown: %d\n",(cIn&0xFC)>>2);
	pMseq->nCopyCountdown=(cIn&0xFC)>>2;		//64-0

	//resend status
	ar >> cIn;
	TRACEIT("resend status: %d\n",cIn&0x03);
	pMseq->nResendStatus=cIn&0x03;

	TRACEIT("resend countdown: %d\n",(cIn&0xFC)>>2);
	pMseq->nResendCountdown=(cIn&0xFC)>>2;		//64-0
	
	//loop def 1
	ar >> cIn;
	TRACEIT("loop def#1: %d\n",cIn&0x03);
	pMseq->nLoopDef1=cIn&0x03;			//loop define part 1
										//0 loop not defined
										//1 loop defined
										//2 reserved
										//3 infinite loop (without regarding NBRepeat
	pMseq->nLoopPoint1=(cIn&0xE0)>>5;
	ar >> cIn;
	pMseq->nLoopPoint1|=((unsigned int)cIn)<<3;

	ar >> cIn;
	pMseq->nNBRepeat1=cIn&0x1F;			//number of loops for part 1
	pMseq->nStartPoint1=(cIn&0xE0)>>5;	//start of loop 1
	ar >> cIn;
	pMseq->nStartPoint1|=((unsigned int)cIn)<<3;

	//loop def 2
	ar >> cIn;
	TRACEIT("loop def#2: %d\n",cIn&0x03);
	pMseq->nLoopDef2=cIn&0x03;			//loop define part 1
										//0 loop not defined
										//1 loop defined
										//2 reserved
										//3 infinite loop (without regarding NBRepeat
	pMseq->nLoopPoint2=(cIn&0xE0)>>5;
	ar >> cIn;
	pMseq->nLoopPoint2|=((unsigned int)cIn)<<3;

	ar >> cIn;
	pMseq->nNBRepeat2=cIn&0x1F;			//number of loops for part 1
	pMseq->nStartPoint2=(cIn&0xE0)>>5;	//start of loop 1
	ar >> cIn;
	pMseq->nStartPoint2|=((unsigned int)cIn)<<3;

	ar >> cIn;
	TRACEIT("tim: %02X\n",cIn);
	pMseq->nTimebase=(unsigned int)cIn;
	ar >> cIn;
	TRACEIT("tim: %02X\n",cIn);
	pMseq->nTimebase|=((unsigned int)cIn)<<8;
	ar >> cIn;
	TRACEIT("tim: %02X\n",cIn);
	pMseq->nTimebase|=((unsigned int)cIn)<<16;
	TRACEIT("timebase: %d\n",pMseq->nTimebase);

	//reserved
	ar >> cIn;

	//reserved
	ar >> cIn;
	//a track = a media type (in Tmode 0)
	TRACEIT("track count: %d\n",cIn);
	pMseq->nTracks=cIn;					//
}

int CMSEQFile::nGetTrackCount(void)
{
	return m_Header.nTracks;
}

int CMSEQFile::nGetCopyStatus(void)
{
	return m_Header.nCopyStatus;
}

int CMSEQFile::nGetTmode(void)
{
	return m_Header.nTmode;
}

int CMSEQFile::nGetTimebase(void)
{
	return m_Header.nTimebase;
}

int CMSEQFile::nGetLoopCount(void)
{
	int nLoop=0;
	switch(m_Header.nLoopDef1)
	{
		case 1:
			if (nLoop < 0xFFFFFFFF)
				nLoop+=m_Header.nNBRepeat1;
		break;
		case 3:
			nLoop=0xFFFFFFFF;
		break;
	}
	switch(m_Header.nLoopDef2)
	{
		case 1:
			if (nLoop < 0xFFFFFFFF)
				nLoop+=m_Header.nNBRepeat2;
		break;
		case 3:
			nLoop=0xFFFFFFFF;
		break;
	}
	return nLoop;
}
