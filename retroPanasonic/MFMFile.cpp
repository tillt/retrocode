/*\
 * MFMFile.cpp
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
#ifndef WIN32
#include <netinet/in.h>
#else
#include <winsock2.h>
#endif
#include "../include/Resource.h"
#include "../retroBase/Basics.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MyString.h"
#include "../retroBase/Endian.h"
#include "../retroBase/Packer.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/Adpcm.h"
#include "MFMSample.h"
#include "MFMFile.h"
#include "MFMProperty.h"

DYNIMPPROPERTY(CMFMFile,CMFMProperty)

/*\ 
 * <---------- CMFMFile::CMFMFile ----------> 
 * @m 
\*/ 
 CMFMFile::CMFMFile(void)
{
	m_nMagicSize=4;
	m_pcMagic="mfmp";
	m_sFormatName="Panasonic MxxFxxMxx (MFM)";
	m_sDefaultExtension=_T("mfm");
	m_pCurrentSample=NULL;
	ASSERT(m_sFormatDescription.Load(IDS_FORMDESC_MFM));
	m_sFormatCredits=_T("Retro's MFM codec is based on: \"Dialogic ADPCM Algorithm\", Copyright (c) 1988 by Dialogic Corporation.");
	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono].addDigit(8000);
	m_encodingPara.addPara(cmdParaString,paraStrTitle);
	m_encodingPara.addPara(cmdParaString,paraStrCopyright);
	m_encodingPara.addPara(cmdParaString,paraStrEncoder);
}


/*\ 
 * <---------- CMFMFile::~CMFMFile----------> 
 * @m 
\*/ 
CMFMFile::~CMFMFile(void)
{
	if (m_pCurrentSample)
	{
		TRACEIT2("deleting sample object");
		delete m_pCurrentSample;
	}
}

/*\ 
 * <---------- CMFMFile::read ----------> 
 * @m 
 * --> I N <-- @p
 * istream &ar - input stream reference
\*/ 
void CMFMFile::Read(istream &ar)
{
	mfmHeader header;
	int nSize;
	Unpacker up(ar,false);
	Log2(verbLevDebug1,"reading header\n");
	//Read the first 4 bytes, which will hold 'mfmp'
	ar.read((char *)&header.mfmp,sizeof(header.mfmp)); 
	//Now, read the size
	up.read("l",&header.size); 
	//header.size=ntohl(header.size);
	//Read the version info??
	up.read("s",&header.wVersion); 
	header.wVersion=ntohs(header.wVersion);
	//Read the subtag count??
	up.read("s",&header.wSubTagCount); 
	Log2(verbLevDebug1,"tag count per header: %02X\n",header.wSubTagCount);
	//Read the header size??
	up.read("s",&header.wHeaderSize); 
	//header.wHeaderSize=ntohs(header.wHeaderSize);
	Log2(verbLevDebug1,"header size per header: %02X\n",header.wHeaderSize);
	//read subchunks
	nSize=header.size-36;
	try
	{
		Log2(verbLevDebug1,"initial size: %d\n",nSize);
		while (nSize)
		{
			nSize-=ReadSubchunk(ar,&header);
			Log2(verbLevDebug1,"size left: %d\n",nSize);
		};
	}
	catch (istream::failure const &e)
	{
		Log2(verbLevErrors,"File Access Exception\n\t%s\n",e.what());
	}
}

/*\ 
 * <---------- CCMXFile::RenderDestination ----------> 
 * @m 
 * --> I N <-- @p
 * ostream &out - reference to output stream
 * CMobileSampleContent *pSource - source sample data
\*/ 
void CMFMFile::Write(ostream &out)
{
	const unsigned char pTrackData[4][8]=
	{
		{0x06,0x7F,0x00,0x3F, 0x00,0xFF,0xBF,0x6A},
		{0x60,0xFF,0xDE,0x00, 0x60,0xFF,0xDF,0x00},
		{0xFF,0xFF,0xB1,0x00},		
		{0xFF,0xFF,0xB1,0x00}
	};
	unsigned int i,o;
	uint32_t nFileSize=0;
	tstring sChunk;
	unsigned short int nHeaderSize=0;
	CMFMSample *pSample;
	uint32_t nNoteRepeat;
	Packer pk(out,false);

	tstring sTitle,sCopyright,sPublisher,sTool;

	pSample=new CMFMSample(NULL,0);
	pSample->Encode((short *)m_pCSSource->m_pcCSBuffer,m_pCSSource->m_nCSSize);

	sTitle=m_pCSSource->m_strInfo[infoTitle];
	if (!sTitle.empty())
		nHeaderSize+=(unsigned short int)sTitle.length();

	sCopyright=m_pCSSource->m_strInfo[infoCopyright];
	if (!sCopyright.empty())
		nHeaderSize+=(unsigned short int)sCopyright.length();

	sTool=m_pCSSource->m_strInfo[infoEncodedBy];
	Log2(verbLevDebug1,"tool: %s\n",sTool.c_str());

	if (!sTool.empty())
		nHeaderSize+=(unsigned short int)sTool.length();

	nNoteRepeat=pSample->nGetSize()/4096;
	if (nNoteRepeat == 0)
		nNoteRepeat=1;

	nFileSize=pSample->nGetSize()+ nHeaderSize + 47 + (4*(4+4)) + (8+4+4) + (8+4+(nNoteRepeat*4));

	out.write("mfmp",4);					//mfmp
	pk.write("l",&nFileSize);				//filesize
	out.write("\000\001\000\005\000\140",6);//fixed stuff
	out.write("titl",4);					//titl
	pk.write((uint16_t)sTitle.length());
	if (!sTitle.empty())
		out.write(sTitle.c_str(),(unsigned short int)sTitle.length());

	out.write("copy",4);					//copy
	pk.write((uint16_t)sCopyright.length());	
	if (!sCopyright.empty())
		out.write(sCopyright.c_str(),(unsigned short int)sCopyright.length());

	out.write("supt",4);					//supt
	pk.write((uint16_t)sTool.length());
	if (!sTool.empty())
		out.write(sTool.c_str(),(unsigned short int)sTool.length());

	out.write("sorc",4);					//sorc
	pk.write((uint16_t)1);		
	out.write("\001",1);

	out.write("note",4);					//note
	pk.write((uint16_t)2);	
	out.write("\000\001",2);

	out.write("wave",4);					//wave
	pk.write(pSample->nGetSize()+8);
	pk.write((uint16_t)1);
	pk.write(pSample->nGetSize()+2);
	pk.write((uint16_t)5);
	out.write((char *)pSample->pcGetAdpcm(),pSample->nGetSize());

	for (i=0;i < 4;i++)
	{
		const unsigned char *pData=pTrackData[i];
		uint32_t nTrackSize=sizeof(pTrackData[i][0]);
		switch(i)
		{
			case 0:		nTrackSize=8;					break;	
			case 2:	
			case 3:		nTrackSize=4;					break;
			case 1:		nTrackSize=4+(4*nNoteRepeat);	break;
		}
		out.write("trac",4);								//trac
		//dwChunkSize=htonl((unsigned short int)nTrackSize);
		pk.write("l",&nTrackSize);
		switch(i)
		{
			case 0:
			case 2:
			case 3:
				out.write((char *)pData,nTrackSize);
			break;
			case 1:
				for(o=0;o < nNoteRepeat;o++)
					out.write((char *)pData,4);
				out.write((char *)pData+4,4);
			break;
		}
	}

	m_nCSChannels=m_pCSSource->m_nCSChannels;
	m_nCSBitsPerSample=4;
	m_nCSSamplesPerSecond=m_pCSSource->m_nCSSamplesPerSecond;
	m_nCSSize=pSample->nGetSize();
	m_nFileSize=out.tellp();
}


/*\ 
 * <---------- CCMXFile::ReadSubchunk ----------> 
 * @m 
 * --> I N <-- @p
 * istream &ar - input stream reference
 * CMXHEADER *pCmx - pointer to header
 * <-- OUT --> @r
 * int - number of bytes read
\*/ 
int CMFMFile::ReadSubchunk(istream &ar,mfmHeader *pHeader)
{
	int nRet=0;

	char sChunk[5],sAttribute[256];
	uint32_t dwSize;
	unsigned short int wSize,nRead;
	map<tstring,int> :: const_iterator iterChunk;
	map<tstring,int> mapChunkId;

	Unpacker up(ar,false);

	wSize=0;
	dwSize=0;

	ar.read(sChunk,4);
	sChunk[4]=0;
	nRet+=4;

	mapChunkId[tstring(_T("titl"))]=cidTITL;
	mapChunkId[tstring(_T("copy"))]=cidCOPY;
	mapChunkId[tstring(_T("note"))]=cidNOTE;
	mapChunkId[tstring(_T("supt"))]=cidSUPT;
	mapChunkId[tstring(_T("sorc"))]=cidSORC;
	mapChunkId[tstring(_T("wave"))]=cidWAVE;
	mapChunkId[tstring(_T("trac"))]=cidTRAC;

	Log2(verbLevDebug1,"decoded chunk: %s\n",sChunk);
	if ((iterChunk=mapChunkId.find(sChunk)) == mapChunkId.end())
	{
		Log2(verbLevErrors,"unknown chunk\n");
		throw new CFormatException(CFormatException::formaterrInvalid,"unknown chunk");
	}
	//read data size
	if (iterChunk->second == cidWAVE || iterChunk->second == cidTRAC)
	{
		//read data size
		up.read("l",&dwSize);
		wSize=0;
		Log2(verbLevDebug2,"long chunk size: %d\n",dwSize);
		nRet+=dwSize;
	}
	else
	{
		//read data size
		up.read("s",&wSize);
		dwSize=wSize;
		Log2(verbLevDebug2,"short chunk size: %d\n",wSize);
		nRet+=wSize;
	}

	if (dwSize)
	{
		switch(iterChunk->second)
		{
			case cidTITL:
				nRead=255;
				if (dwSize < 255)
					nRead=(unsigned short)dwSize;
				if (nRead)
					ar.read((char*)&pHeader->sTitle,nRead);
				pHeader->sTitle[nRead]=0;				
				dwSize-=nRead;
				Log2(verbLevDebug1,"title: %s\n",pHeader->sTitle);
				m_strInfo[infoTitle]=tstring((TCHAR *)pHeader->sTitle);
			break;
			case cidSUPT:
				ar.read(sAttribute,dwSize);
				sAttribute[dwSize]=0;
				dwSize=0;
				Log2(verbLevDebug1,"tool: %s\n",sAttribute);
			break;
			case cidTRAC:
				Log2(verbLevDebug1,"parse track...\n");
				if (!bParseTRAC(ar,dwSize,&dwSize))
				{
					Log2(verbLevErrors,"failed parsing TRAC chunk\n");
					throw new CFormatException(CFormatException::formaterrInvalid);
				}
			break;				
			case cidWAVE:
				Log2(verbLevDebug1,"parse wave...\n");
				if (!bParseWAVE(ar,dwSize,&dwSize))
				{
					Log2(verbLevErrors,"failed parsing WAVE chunk\n");
					throw new CFormatException(CFormatException::formaterrInvalid);
				}
			break;
			case cidNOTE:
				Log2(verbLevDebug1,"parse note...\n");
			break;
			case cidSORC:
				Log2(verbLevDebug1,"parse source...\n");
			break;
			default:
				Log2(verbLevWarnings,"unknown MFM subtag (%s)\n",sChunk);
		}
		//skip unwanted data
		if (dwSize)
		{
			Log2(verbLevDebug2,"skipping %d bytes\n",dwSize);
			ar.seekg((int)ar.tellg()+dwSize);
		}
	}
	return nRet;
}

/*\ 
 * <---------- CCMXFile::bParseTRAC ----------> 
 * @m 
 * --> I N <-- @p
 * istream &ar
 * uint32_tint dwSize
 * uint32_tint *pdwSizeRemaining
 * <-- OUT --> @r
 * bool 
\*/ 
bool CMFMFile::bParseTRAC(istream &ar, uint32_t dwSize,uint32_t *pdwSizeRemaining)
{
	bool bRet=false;

	*pdwSizeRemaining=dwSize;
	Log2(verbLevDebug1,"parsing track data...\n");
	if (dwSize)
	{
		unsigned char *pBuffer=new unsigned char [dwSize];
		if (pBuffer)
		{
			unsigned char *p;
			ar.read((char *)pBuffer,dwSize);
			*pdwSizeRemaining-=dwSize;
			p=pBuffer;
			hexdump("track data: ",(unsigned char *)p,min(64,dwSize));
			bRet=true;

			delete [] pBuffer;
		}
	}
	return bRet;
}

bool CMFMFile::bParseWAVE(istream &ar, uint32_t dwSize,uint32_t *pdwSizeRemaining)
{
	bool bRet=false;

	*pdwSizeRemaining=dwSize;
	Log2(verbLevDebug1,"parsing wave data...\n");
	if (dwSize)
	{
		unsigned char *pBuffer=new unsigned char [dwSize];
		if (pBuffer)
		{
			unsigned char *p;
			ar.read((char *)pBuffer,dwSize);
			*pdwSizeRemaining-=dwSize;
			p=pBuffer;
			hexdump("wave data: ",(unsigned char *)p,min(64,dwSize));
			bRet=true;
			p+=8;
			if (m_pCurrentSample)
				delete m_pCurrentSample;
			m_pCurrentSample=new CMFMSample(p,dwSize-8);
			delete [] pBuffer;
		}
	}

	if (bRet == true && m_pCurrentSample)
	{
		uint32_t nDecodedSize;
		signed short int *pnDecoded=NULL;
		Log2(verbLevDebug1,"decoding sample...\n");
		
		nDecodedSize=m_pCurrentSample->nGetSize()*4;
		Log2(verbLevDebug3,"expecting %d bytes\n",nDecodedSize);
		ASSERT(nDecodedSize);
		pnDecoded=(signed short int *)Alloc(nDecodedSize);
		ASSERT(pnDecoded);
		m_pCurrentSample->Decode(pnDecoded,nDecodedSize);
		m_nCSBitsPerSample=16;
		m_nCSChannels=1;
		m_nCSSamplesPerSecond=8000;
		m_nCSSize=nDecodedSize;
		m_pcCSBuffer=(void *)pnDecoded;
	}
	return bRet;
}
