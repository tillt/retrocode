/*\
 * SagemFile.cpp
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
#include "../retroBase/FIDProperty.h"
#include "MonoFile.h"
#include "SagemFile.h"
#include "SagemProperty.h"
#include "resource.h"

DYNIMPPROPERTY(CSagemFile,CSagemProperty)

CSagemFile::CSagemFile(void)
{
	m_pcData=NULL;
	m_nMagicSize=9;
	m_pcMagic="$MUS$1.0$";
	m_sFormatName="Sagem Ringertone";
	m_nMaxSize=160;
	m_nEncoding=encodeText;
}

CSagemFile::~CSagemFile(void)
{
}

void CSagemFile::Read(istream &ar)
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
			Decode();
			bRet=true;
		}
	}
}

bool CSagemFile::Decode(void)
{
	bool bFailed=false;
	#undef HEAD_SIZE
	#define HEAD_SIZE		16

	#undef MID_SIZE
	#define MID_SIZE		5
	const unsigned char cHead[HEAD_SIZE] = {'$','M','U','S','$','1','.','0','$',0x0A,'T','i','t','r','e',':'};
	const unsigned char cMid[MID_SIZE] = {'D','a','t','a',':'};
	unsigned int nDura[]={1,2,4,8,16};
	const unsigned char cTransLength[]={0x2D,0x2A,0x27,0x24,0x21,0x00};
	const unsigned char cTransNote[]=
	{
		0x21,	//1 C
		0x22,	//1 C#
		0x24,	//1 D
		0x25,	//1 D#
		0x27,	//1 E			HACK ALERT - brunet cant deliver 0x27 !!!
		0x2A,	//1 F
		0x2B,	//1 F#
		0x2D,	//1 G
		0x2E,	//1 G#
		0x30,	//1 A
		0x31,	//1 A#
		0x33,	//1 H

		0x36,	//2 C
		0x37,	//2 C#
		0x39,	//2 D
		0x3A,	//2 D#
		0x3C,	//2 E
		0x3F,	//2 F
		0x40,	//2 F#
		0x42,	//2 G
		0x43,	//2 G#
		0x45,	//2 A
		0x46,	//2 A#
		0x48,	//2 H

		0x4B,	//3 C
		0x4C,	//3 C#
		0x4E,	//3 D
		0x4F,	//3 D#
		0x51,	//3 E
		0x54,	//3 F
		0x55,	//3 F#
		0x57,	//3 G
		0x58,	//3 G#
		0x5A,	//3 A
		0x61,	//3 A#
		0x63,	//3 H
	
		0x66,	//4 C
		0x67,	//4 C#
		0x69,	//4 D
		0x6A,	//4 D#
		0x6C,	//4 E
		0x6F,	//4 F
		0x70,	//4 F#
		0x72,	//4 G
		0x73,	//4 G#
		0x75,	//4 A
		0x76,	//4 A#
		0x78,	//4 H	
		0x00
	}; 
	
	int nGanzeNote=2004;

	int iByte,iBit,iScale,iNote;
	unsigned char *pData;
	int nNote,nLength,nEvents;

	int i;
	int iStyle=0;
	int temp;

	m_nLastQuanta=0;
	iByte=0;
	iBit=7;
	iScale=0;

	pData=m_pcData;

	//check header
	if (memcmp(pData,cHead,HEAD_SIZE))
	{
		bFailed=true;
		TRACEIT("CSagemDoc::Convert - invalid header\n");
		throw new CFormatException(CFormatException::formaterrInvalid,"invalid header");
	}
	iByte+=HEAD_SIZE;

	//get name
	m_strName.empty();
	for (i=0;pData[iByte] != 0x0A;i++)
		m_strName+=pData[iByte++];
	TRACEIT ("CMIDIConverter::Convert - song name: %s\n",m_strName.c_str());

	//skip LF
	++iByte;

	//check middle part
	if (memcmp(pData+iByte,cMid,MID_SIZE))
	{
		bFailed=true;
		TRACEIT("CSagemDoc::Convert - invalid middle part\n");
		throw new CFormatException(CFormatException::formaterrInvalid,"invalid middle part");
	}
	iByte+=MID_SIZE;

	//get tempo
	sscanf((const char *)pData+iByte,"%d-%d-%d:",&nEvents,&m_nLoopDelay,&m_nTempo);
	while (*(pData+iByte) != 0x0A)
		++iByte;
	++iByte;

	m_nEventCount=0;

	while (iByte < m_nSize)
	{
		bool bPause=false;
		bool bDot=false;
		nLength=-1;
		nNote=-1;
		//is it a duration ?
		for (i=0;cTransLength[i] != 0x00;i++)
		{
			if (cTransLength[i] == pData[iByte])
			{
				nLength=i;
				break;
			}
			if ((cTransLength[i]+1 == pData[iByte]) || (cTransLength[i]+2 == pData[iByte]))
			{
				nLength=i;
				bDot=true;
				break;
			}
		}
		if (nLength == -1)
		{
			TRACEIT("CSagemDoc::Decode - invalid length at byte %d: %02Xh\n",iByte,pData[iByte]);
			throw new CFormatException(CFormatException::formaterrInvalid,"invalid length");
		}
		++iByte;
        
		//is it a pause ?
		if (pData[iByte] == 0x20)
		{	//yes
			bPause=true;
			++iByte;
		}
		else
		{	//nope -> must be a note
			for (i=0;i < cTransNote[i] != 0x00;i++)
			{
				if (cTransNote[i] == pData[iByte])
				{
					nNote=i%12;
					iScale=(i/12)+4;
					break;
				}
			}
			if (nNote == -1)
			{
				TRACEIT("CSagemDoc::Decode - invalid note\n");
				throw new CFormatException(CFormatException::formaterrInvalid,"invalid note");
			}
			++iByte;
		}
		
		//TRACEIT ("CMIDIConverter::Convert - event %d - note %d, duration %d, specifier %d\n",iEvent,cNote,cLength,cSpec);
		if (!bPause)
		{
			//create note on
			iNote=(nNote+(12*iScale));
			temp=nGanzeNote/nDura[nLength];
			if (bDot)
				temp+=temp/2;
		}
		else
		{
			temp=nGanzeNote/nDura[nLength];
			if (bDot)
				temp+=temp/2;
		}
		m_nLastQuanta+=temp;
		++m_nEventCount;
	};
	/*
	if (iByte < m_nSize)
	{
		TRACEIT("file contains unwanted data\n");
		strError.Format("file contains unwanted data");
		LogWarning(strError);
		TRACEIT("CSagemDoc::Decode - invalid note\n");
		throw new CFormatException(CFormatException::formaterrInvalid);
	}
	*/
	return !bFailed;
}
