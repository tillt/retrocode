/*\
 * MotorolaFile.cpp
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
#include "MotorolaFile.h"
#include "MotorolaProperty.h"

DYNIMPPROPERTY(CMotorolaFile,CMotorolaProperty)

CMotorolaFile::CMotorolaFile(void)
{
	m_pcData=NULL;
	m_nMagicSize=4;
	m_pcMagic="L35&";
	m_sFormatName="Motorola Ringertone";
	m_nMaxSize=160;
	m_nEncoding=encodeText;
}

CMotorolaFile::~CMotorolaFile(void)
{
}

void CMotorolaFile::Read(istream &ar)
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

bool CMotorolaFile::Decode(void)
{
	bool bFailed=false;
	int nGanzeNote=2004;
	unsigned int nDura[]={100,50,25,12,6,3};
	int iByte,iBit,iScale,iNote;
	unsigned char *pData;
	int nNote,nLength;

	int iStyle=0;
	int temp;

	iByte=0;
	iBit=7;
	iScale=0;

	m_nLastQuanta=0;

	pData=m_pcData;

	if (memcmp(pData,"L35&",4))
	{
		bFailed=true;
		TRACEIT("CMotorolaFile::Decode - invalid header - file format is unknown\n");
		throw new CFormatException(CFormatException::formaterrUnknown);
	}
	iByte+=4;

	//get tempo
	sscanf((const char *)pData+iByte,"%d",&temp);
	if (!temp || temp > 4)
	{
		bFailed=true;
		TRACEIT("CMotorolaFile::Decode - invalid tempo - file format is unknown\n");
		throw new CFormatException(CFormatException::formaterrInvalid);
	}
	iByte+=2;
	m_nTempo=temp*60;
	m_nEventCount=0;

	while (iByte < m_nSize)
	{
		bool bPause=false;
		bool bDot=false;
		nLength=0;
		nNote=0;

		if (pData[iByte] == ' ')
			++iByte;
		else
		{
			if (pData[iByte] == '&' && pData[iByte+1] == '&')
			{
				//calc checksum
				iByte+=2;

				int i;
				unsigned char cByte1,cByte2;
				unsigned char cCheckSum=pData[6];
				for (i=6+1;i < iByte;i++)
					cCheckSum^=pData[i];
				cByte1=cCheckSum&0x0F;
				cByte2=(cCheckSum&0xF0)>>4;
				if (pData[iByte]-0x30 != cByte2 || pData[iByte+1]-0x30 != cByte1)
				{
					TRACEIT("CMotorolaFile::Decode - invalid checksum (%c%c != %c%c) - file may be damaged",pData[iByte],pData[iByte+1],cByte2+0x30,cByte1+0x30);
					throw new CFormatException(CFormatException::formaterrChecksum);
				}
				iByte+=2;
			}
			else
			{
				if (pData[iByte] == 'r' || pData[iByte] == 'R')	//is it a pause ?
				{	//yes->...
					bPause=true;
					++iByte;
				}
				else
				{	//nope -> must be a note
					switch(pData[iByte])
					{
						case 'C':
						case 'c':
							nNote=0;
						break;
						case 'D':
						case 'd':
							nNote=2;
						break;
						case 'E':
						case 'e':
							nNote=4;
						break;
						case 'F':
						case 'f':
							nNote=5;
						break;
						case 'G':
						case 'g':
							nNote=7;
						break;
						case 'A':
						case 'a':
							nNote=9;
						break;
						case 'H':
						case 'h':
						case 'B':
						case 'b':
							nNote=11;
						break;
						default:
							TRACEIT("unknown note found at byte %d\n",iByte);
							TRACEIT("CMotorolaFile::Decode - invalid event (%c) at %d - file may be damaged",pData[iByte],iByte);
							throw new CFormatException(CFormatException::formaterrInvalid);
					}

					++iByte;
					if (pData[iByte] == '#')
					{
						++nNote;
						++iByte;
					}
					//is it a scale ?
					switch(pData[iByte])
					{
						case '-':
							iScale=5;
							++iByte;
						break;
						case '+':
							iScale=7;
							++iByte;
						break;
						default:
							iScale=6;

					}			
					if (iScale == 7)
					{
						switch(nNote)
						{
							//Note: A
							case 9:
							//Note: A#
							case 10:
							//Note: B / H
							case 11:
								iScale=6;
							break;
						}
					}	
				}
				if (pData[iByte] >= '1' || pData[iByte] <= '6')
				{	//yes->get it
					sscanf((const char *)pData+iByte,"%d",&nLength);
					++iByte;
				}
				else
				{
					TRACEIT("invalid event\n");
					TRACEIT("CMotorolaFile::Decode - invalid event duration - file format is unknown");
					throw new CFormatException(CFormatException::formaterrInvalid);
				}
				
				//TRACE ("CMIDIConverter::Convert - event %d - note %d, duration %d, specifier %d\n",iEvent,cNote,cLength,cSpec);
				if (!bPause)
				{
					//create note on
					iNote=(nNote+(12*iScale));
					temp=(nGanzeNote*nDura[5-(nLength-1)])/100;
				}
				else
					temp=(nGanzeNote*nDura[5-(nLength-1)])/100;
				m_nLastQuanta+=temp;
			}
			++m_nEventCount;
		}
	};
	/*
	if (iByte < m_nSize)
	{
		TRACEIT("file contains unwanted data\n");
		strError.Format("file contains unwanted data");
		LogWarning(strError);
	}
	*/
	return !bFailed;
}
