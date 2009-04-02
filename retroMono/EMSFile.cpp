/*\
 * EMSFile.cpp
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
#include "../include/Resource.h"
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/FIDProperty.h"
#include "MonoFile.h"
#include "EMSFile.h"
#include "EMSProperty.h"

DYNIMPPROPERTY(CEMSFile,CEMSProperty)

CEMSFile::CEMSFile(void)
{
	m_pcData=NULL;
	m_nMagicSize=6;
	m_pcMagic="MELODY:";
	m_sFormatName="EMS";
	m_nMaxSize=140;
	m_bEMSHead=false;
	m_bIMelodyHead=false;
	m_bUsingTempo=false;
	m_bUsingName=false;
}

CEMSFile::~CEMSFile(void)
{
}

void CEMSFile::Read(istream &ar)
{
	bool bRet=false;

	if (m_pcData)
		delete [] m_pcData;
	ar.seekg(0,ios_base::end);
	m_nFileSize=(int)ar.tellg();
	ar.seekg(0,ios_base::beg);
	if (m_nFileSize)
	{
		m_pcData=new unsigned char[m_nFileSize];
		if (m_pcData)
		{
			m_nSize=m_nFileSize;
			ar.read((char *)m_pcData,m_nFileSize);
			if (m_nEncoding == encodeHex)
			{
				unsigned char *pcBinary=new unsigned char[m_nFileSize/2];
				HexToBin(pcBinary,m_pcData,m_nFileSize);
				delete [] m_pcData;
				m_pcData=pcBinary;
				m_nSize=m_nFileSize/2;
			}
			Decode();
			bRet=true;
		}
	}
}

bool CEMSFile::bMagicHead(std::istream &ar,uint32_t nSize)
{
	bool bRet=false;
	unsigned char theChar[4];
	unsigned char pcValue[3];
	unsigned char pcHead[5];
	unsigned char pcCheck[16];
	unsigned int val;
	pcValue[2]=0;
	if (m_nMagicSize == 0 || (int)nSize < m_nMagicSize)
		return false;
	try 
	{
		ar.read((char *)theChar,4);
		if (bIsHex(theChar[0]) && 
			bIsHex(theChar[1]) && 
			bIsHex(theChar[2]) && 
			bIsHex(theChar[3]))
		{
			m_nEncoding=encodeHex;
			memcpy(pcValue,theChar,2);
			sscanf((const char *)pcValue,"%02x",&val);
			pcHead[0]=(unsigned char)val;
			memcpy(pcValue,theChar+2,2);
			sscanf((const char *)pcValue,"%02x",&val);
			pcHead[1]=(unsigned char)val;
			pcHead[2]=cReadHexByte(ar);
			pcHead[3]=cReadHexByte(ar);
		}
		else
		{
			m_nEncoding=encodeText;
			memcpy(pcHead,theChar,4);
		}	
		int i=0;
		if (pcHead[1] == 0x0C && pcHead[3] == 0x00)
			m_bEMSHead=true;
		else
		{
			i=4;
			memcpy(pcCheck,pcHead,i);
		}
		if (m_nEncoding == encodeHex)
		{
			pcCheck[i++]=cReadHexByte(ar);
			pcCheck[i++]=cReadHexByte(ar);
			pcCheck[i++]=cReadHexByte(ar);
			pcCheck[i++]=cReadHexByte(ar);
		}
		else
		{
			ar.read((char *)pcCheck+i,4);
		}
		if (memcmp(pcCheck,"BEGI",4) == 0)
		{
			m_bIMelodyHead=true;
			bRet=true;
		}
		else
		{
			if (memcmp(pcCheck,"BEAT",4) == 0)
				bRet=true;
			else
			{
				if (memcmp(pcCheck,"MELO",4) == 0)
					bRet=true;
			}
		}
	}
	catch(istream::failure const &e)
	{
		TRACEIT2("catching file exception in bMagicHead\n");
		bRet=false;
	}
	return bRet;
}

bool CEMSFile::Decode(void)
{
	bool bFailed=false;
	#undef HEAD_BIGSIZE
	#define HEAD_BIGSIZE		45

	#undef HEAD_SIZE
	#define HEAD_SIZE			7

	#undef TAIL_BIGSIZE
	#define TAIL_BIGSIZE		13


	unsigned char cHeadBig[HEAD_BIGSIZE] = {	'B','E','G','I','N',':','I','M','E','L','O','D','Y',0x0d,0x0a,
										'V','E','R','S','I','O','N',':','1','.','0',0x0d,0x0a,
										'F','O','R','M','A','T',':','C','L','A','S','S','1','.','0',0x0d,0x0a};
	unsigned char cHead[HEAD_SIZE] = {	'M','E','L','O','D','Y',':'};
	unsigned char cTailBig[TAIL_BIGSIZE] = {	'E','N','D',':','I','M','E','L','O','D','Y',0x0d,0x0a	};

	unsigned int nDura[]={32,16,8,4,2,1};
	const unsigned char cTransLength[]={0x2D,0x2A,0x27,0x24,0x21,0x00};

	int nGanzeNote=2004;

	int iByte,iBit,iScale,iNote;
	unsigned char *pData;
	int nNote,nLength;
	bool bBigHead=false;

	m_nLastQuanta=0;
	int iStyle=0;
	int temp;

	iByte=0;
	iBit=7;
	iScale=0;

	pData=m_pcData;

	//check for SMS header
	if (m_nSize > 4)
	{
		if (pData[1] == 0x0C && pData[3] == 0x00)
			iByte+=4;
	}

	//check for big header
	if (m_nSize > HEAD_BIGSIZE)
	{
		if (!memcmp(pData+iByte,cHeadBig,HEAD_BIGSIZE))
		{	//found it -> skip it
			iByte+=HEAD_BIGSIZE;
			bBigHead=true;
		}
	}
	m_nTempo=120;
	//check for name tag
	if (iByte+6 < m_nSize)
	{
		if (!memcmp(pData+iByte,"NAME:",5))
		{
			m_bUsingName=true;
			//skip "name:"
			iByte+=5;
			//get name
			m_strName.empty();
			while (pData[iByte] != 0x0D)
				m_strName+=pData[iByte++];
			TRACEIT2("song name: %s\n",m_strName.c_str());
			//skip CR LF
			iByte+=2;
		}
	}
	//check for beat tag
	if (iByte+6 < m_nSize)
	{
		if (!memcmp(pData+iByte,"BEAT:",5))
		{
			//skip "beat:"
			iByte+=5;
			sscanf((const char *)pData+iByte,"%d",&m_nTempo);
			m_bUsingTempo=true;
			if (m_nTempo == 0)
			{
				bFailed=true;
				TRACEIT2("invalid tempo found\n");
				throw new CFormatException(CFormatException::formaterrInvalid);
			}
			while (*(pData+iByte) != 0x0D)
				++iByte;
			//skip CR LF
			iByte+=2;
		}
	}

	//check middle part
	if (iByte+HEAD_SIZE < m_nSize)
	{
		if (memcmp(pData+iByte,cHead,HEAD_SIZE))
		{
			bFailed=true;
			TRACEIT2("invalid MELODY header\n");
			throw new CFormatException(CFormatException::formaterrInvalid);
		}
		iByte+=HEAD_SIZE;
	}
	else
	{
		bFailed=true;
		TRACEIT2("no MELODY header\n");
		throw new CFormatException(CFormatException::formaterrInvalid);
	}

	iScale=4;
	m_nEventCount=0;

	while (iByte < m_nSize && pData[iByte] != 0x0D)
	{
		bool bPause=false;
		int nDot=0;
		nLength=0;
		nNote=0;

		//is it an octave ?
		if (pData[iByte] == '*')
		{
			++iByte;
			iScale=pData[iByte]-'0';
			++iByte;
			if (iScale > 9)
			{
				bFailed=true;
				TRACEIT2("invalid octave\n");
				throw new CFormatException(CFormatException::formaterrInvalid);
			}
		}

		//check for sharp
		if (pData[iByte] == '#')
		{
			nNote=1;
			++iByte;
		}

		//check for note
		switch(pData[iByte])
		{
			case 'r':
			case 'R':
				bPause=true;
			break;
			case 'C':
			case 'c':
				nNote+=0;
			break;
			case 'D':
			case 'd':
				nNote+=2;
			break;
			case 'E':
			case 'e':
				nNote+=4;
			break;
			case 'F':
			case 'f':
				nNote+=5;
			break;
			case 'G':
			case 'g':
				nNote+=7;
			break;
			case 'A':
			case 'a':
				nNote+=9;
			break;
			case 'H':
			case 'h':
			case 'B':
			case 'b':
				nNote+=11;
			break;
			default:
				bFailed=true;
				TRACEIT2("unknown note found at byte %d\n",iByte);
				throw new CFormatException(CFormatException::formaterrInvalid);
		}
		++iByte;

		//is it a duration
		if (pData[iByte] >= '0' && pData[iByte] <= '5')
		{
			nLength=5-(pData[iByte]-'0');
			++iByte;
		}
		else
		{
			bFailed=true;
			TRACEIT2("invalid duration\n");
			throw new CFormatException(CFormatException::formaterrInvalid);
		}
		
		//is it a dot ?
		switch(pData[iByte])
		{
			case '.':
				nDot=1;
				++iByte;
			break;
			case ':':
				nDot=2;
				++iByte;
			break;
			case ';':
				nDot=3;
				++iByte;
			break;
		}
   		if (!bPause)
		{
			//create note on
			iNote=(nNote+(12*iScale));
			temp=nGanzeNote/nDura[nLength];
			switch (nDot)
			{
				//*
				case 1:
					temp+=temp/2;
				break;
				//**
				case 2:
					temp+=(3*temp)/4;
				break;
				//***
				case 3:
					temp=(2*temp)/3;
				break;
			}
		}
		else
		{
			temp=nGanzeNote/nDura[nLength];
			switch (nDot)
			{
				case 1:
					temp+=temp/2;
				break;
				case 2:
					temp+=(3*temp)/4;
				break;
				case 3:
					temp=(2*temp)/3;
				break;
			}
		}
		m_nLastQuanta+=temp;
		m_nEventCount++;
	};

	//copy tail
	if (bBigHead)
		iByte+=TAIL_BIGSIZE;

	return !bFailed;
}
