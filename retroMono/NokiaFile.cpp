/*\
 * NokiaFile.cpp
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
#include "NokiaFile.h"
#include "NokiaProperty.h"

DYNIMPPROPERTY(CNokiaFile,CNokiaProperty)

CNokiaFile::CNokiaFile(void)
{
	m_pcData=NULL;
	m_nMagicSize=3;
	m_pcMagic=new char[4];
	m_pcMagic[0]=0x02;
	m_pcMagic[1]=0x4A;
	m_pcMagic[2]=0x3A;
	m_sFormatName="Nokia SmartMessaging";
	m_nMaxSize=128;
}

CNokiaFile::~CNokiaFile(void)
{
	delete [] m_pcMagic;
}

void CNokiaFile::Read(istream &ar)
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

bool CNokiaFile::bMagicHead(std::istream &ar,uint32_t nSize)
{
	bool bRet=false;
	unsigned char theChar;
	unsigned char pcValue[3]={0,0,0};
	unsigned char pcHead[4];
	pcValue[2]=0;
	if (m_nMagicSize == 0 || (int)nSize < m_nMagicSize)
		return false;
	try 
	{
		ar.read((char *)&theChar,1);
		if (bIsHex(theChar))
		{
			m_nEncoding=encodeHex;
			pcValue[0]=theChar;
			ar.read((char *)&pcValue[1],1);
			sscanf((const char *)pcValue,"%02x",pcHead);
			pcHead[1]=cReadHexByte(ar);
			pcHead[2]=cReadHexByte(ar);
		}
		else
		{
			m_nEncoding=encodeBinary;
			pcHead[0]=theChar;
			ar.read((char *)pcHead+1,2);
		}		
		if (!memcmp(pcHead,m_pcMagic,3))
			bRet=true;
	}
	catch(istream::failure const &e)
	{
		TRACEIT2("catching file exception in bMagicHead\n");
		bRet=false;
	}
	return bRet;
}

unsigned char CNokiaFile::DecodeBits(const unsigned char *pBuffer, int &iByte, int &iBit, int nBits)
{
	ASSERT (pBuffer);
	const unsigned char cMask[8]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
	const unsigned char *p=pBuffer+iByte;
	unsigned char ch=0;
	int i;

	for (i=nBits-1;i >= 0;i--)
	{	
		if (((*p)&cMask[iBit]) == cMask[iBit])
			ch|=cMask[i];
		if (iBit == 0)
		{
			++iByte;
			iBit=7;
			++p;
		}
		else
			--iBit;
	}
	return ch;
}

bool CNokiaFile::Decode(void)
{
	const unsigned int nDura[]={1,2,4,8,16,32};
	const int nBPMCode[32]=
	{
		25,28,31,35,40,45,50,56,63,70,80,90,100,112,125,
		140,160,180,200,225,250,285,320,355,400,450,500,
		565,635,715,800,900
	};
	bool bFailed=FALSE;
	int iByte,iBit,iScale,iNote;
	unsigned char *pData,ch;
	unsigned char cNote,cLength,cSpec;

	int len,i,nPatterns,iPattern,nEventCount,iEvent;
	int iStyle=0;
	int temp;

	iByte=0;
	iBit=7;
	iScale=6;
	unsigned int nGanzeNote=2004;

	m_nLastQuanta=0;
	pData=m_pcData;

	m_nEventCount=0;
	
	ch=DecodeBits(pData,iByte,iBit,8);
	if (ch != 0x02)
	{
		bFailed=true;
		TRACEIT2("invalid command block size\n");
		throw new CFormatException(CFormatException::formaterrUnknown,"invalid command block size");
	}
	else
		TRACEIT2("found default command-block length (02h)\n");

	ch=DecodeBits(pData,iByte,iBit,7);
	if (!bFailed && ch == OTT_COMMAND_RINGTONE)
		TRACEIT ("found ringtone command (25h)\n");
	else
	{
		bFailed=TRUE;
		TRACEIT2("invalid ringtone command - file format is unknown\n");
		throw new CFormatException(CFormatException::formaterrUnknown,"invalid ringtone command");
	}
	
	DecodeBits(pData,iByte,iBit,1);	//skip filler
	
	ch=DecodeBits(pData,iByte,iBit,7);
	if (!bFailed && ch == OTT_COMMAND_SOUND)
	{
		TRACEIT2("found sound command (1Dh)\n");
	}
	else
	{
		bFailed=TRUE;
		TRACEIT2("invalid sound command - file format is unknown\n");
		throw new CFormatException(CFormatException::formaterrUnknown,"invalid sound command - file format is unknown");
	}

	ch=DecodeBits(pData,iByte,iBit,3);
	if (!bFailed && ch == 0x01)
	{
		TRACEIT2("found basic song type (01h)\n");
	}
	else
	{
		bFailed=TRUE;
		TRACEIT2("invalid type value - file format is unknown\n");
		throw new CFormatException(CFormatException::formaterrUnknown,"invalid type value - file format is unknown");
	}

	if (!bFailed)
	{
		len=DecodeBits(pData,iByte,iBit,4);
		m_strName.empty();
		for (i=0;i < len;i++)
			m_strName+=DecodeBits(pData,iByte,iBit,8);
		TRACEIT2("song name: %s\n",m_strName.c_str());
		nPatterns=DecodeBits(pData,iByte,iBit,8);
		TRACEIT2("sequence patterns: %d\n",nPatterns);

		for (iPattern=0;iPattern < nPatterns && !bFailed;iPattern++)
		{
			ch=DecodeBits(pData,iByte,iBit,3);
			if (!bFailed && ch == 0x00)
			{
				TRACEIT ("pattern header (00h)\n");
			}
			else
			{
				bFailed=TRUE;
				TRACEIT2("invalid pattern header - file format is unknown\n");
				throw new CFormatException(CFormatException::formaterrInvalid,"invalid pattern header");
			}

			ch=DecodeBits(pData,iByte,iBit,2);
			if (!bFailed && ch == iPattern)
			{
				TRACEIT2 ("pattern id (%02Xh)\n",ch);				
			}
			else
			{
				TRACEIT2 ("invalid pattern id (%02Xh)\n",ch);				
				bFailed=TRUE;
				TRACEIT2("invalid pattern id - file format is unknown\n");
				throw new CFormatException(CFormatException::formaterrInvalid,"invalid pattern id");
			}
			ch=DecodeBits(pData,iByte,iBit,4);
			TRACEIT ("CMIDIConverter::Decode - loop value (%02Xh)\n",ch);
			m_nLoop=ch;

			nEventCount=DecodeBits(pData,iByte,iBit,8);			
			TRACEIT2 ("CMIDIConverter::Decode - pattern has %d events\n",nEventCount);
			
			for (iEvent=0;iEvent < nEventCount && !bFailed;iEvent++)
			{
				ch=DecodeBits(pData,iByte,iBit,3);
				switch (ch)
				{
					case 0x05:		//volume
						ch=DecodeBits(pData,iByte,iBit,4);
						TRACEIT2 ("event %d - volume set to %d\n",iEvent,ch);						
					break;
					case 0x04:		//tempo
						ch=DecodeBits(pData,iByte,iBit,5);
						TRACEIT2 ("event %d - tempo set to %d\n",iEvent,ch);
						m_nTempo=nBPMCode[ch];;
					break;
					case 0x03:		//style
						ch=DecodeBits(pData,iByte,iBit,2);
						TRACEIT2 ("event %d - style set to %d\n",iEvent,ch);						
						iStyle=ch;
					break;
					case 0x02:		//scale (octave)
						ch=DecodeBits(pData,iByte,iBit,2);
						TRACEIT2 ("event %d - scale set to %d\n",iEvent,ch);
						iScale=ch+4;
					break;
					case 0x01:		//note
						cNote=DecodeBits(pData,iByte,iBit,4);
						cLength=DecodeBits(pData,iByte,iBit,3);
						cSpec=DecodeBits(pData,iByte,iBit,2);
						if (cNote == 0x00)
						{
							TRACEIT2 ("event %d - pause, duration %d, specifier %d\n",iEvent,cLength,cSpec);
							temp=nGanzeNote/nDura[cLength];
							switch(cSpec)
							{
								case 0:	break;
								case 1:	temp+=(temp/2);		break;	//*
								case 2:	temp+=((3*temp)/4);	break;	//**
								case 3:	temp=(2*temp)/3;	break;	//***
							}
						}
						else
						{
							TRACEIT2 ("event %d - note %d, duration %d, specifier %d\n",iEvent,cNote,cLength,cSpec);
							//create note on
							iNote=(cNote+(12*iScale))-1;

							//create note off
							temp=nGanzeNote/nDura[cLength];
							switch(cSpec)
							{
								case 0:	break;
								case 1:	temp+=(temp/2);		break;	//*
								case 2:	temp+=((3*temp)/4);	break;	//**
								case 3:	temp=(2*temp)/3;	break;	//***
							}
						}
						m_nLastQuanta+=temp;;
					break;
					default:
						bFailed=true;
						TRACEIT2("unknown chunk\n");
						throw new CFormatException(CFormatException::formaterrUnknown,"unknown chunk");
				}
			}
			m_nEventCount+=nEventCount;			
		}
		if (!bFailed && iBit > 0)
		{
			TRACEIT2 ("%d filler bits\n",iBit);
			ch=DecodeBits(pData,iByte,iBit,iBit);
		}
		ch=DecodeBits(pData,iByte,iBit,8);
		bFailed=FALSE;
	}
	return !bFailed;
}
