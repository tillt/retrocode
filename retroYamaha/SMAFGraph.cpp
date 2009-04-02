/*\
 * SMAFGraph.cpp
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
#include <WinSock2.h>
#endif
#include <map>
#include "../retroBase/Integer.h"
#include "../retroBase/MyString.h"
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "SMAFDecoder.h"
#include "SMAFGraph.h"
#include "SMAFSample.h"
#include "SMAFEvent.h"

CSMAFImage::CSMAFImage(const char *pcSrc,uint32_t nSize)
{
	ASSERT(pcSrc);
	ASSERT(nSize);
	m_pcData=new char[nSize];
	ASSERT(m_pcData);
	CopyMemory(m_pcData,pcSrc,nSize);
	m_nSize=nSize;
}

CSMAFImage::~CSMAFImage()
{
	if (m_pcData)
		delete [] (char *)m_pcData;
}

void CSMAFImage::Write(ostream &out)
{
	//const unsigned char pPng[]={0x89,0x50,0x4E,0x47};
	//out.write((const char *)pPng,4);
	//out.write((const char *)m_pcData+5,m_nSize-5);
	out.write((const char *)m_pcData,m_nSize);
}

CSMAFGraph::CSMAFGraph(int nChannelOffset)
{
	m_nChannelOffset=nChannelOffset;
}

CSMAFGraph::~CSMAFGraph()
{
	unsigned int i;
	for (i=0;i < (unsigned int)m_Images.size();i++)
	{
		ASSERT (m_Images[i]);
		if (m_Images[i] != NULL)
			delete m_Images[i];
	}
	if (m_Images.size())
		m_Images.erase(m_Images.begin(), m_Images.end());
}

void CSMAFGraph::ExportImages(const char *pszPathPrefix)
{
	char szPath[_MAX_PATH];
	for (unsigned int i=0;i < m_Images.size();i++)
	{
		sprintf(szPath,"%s_%03d.png",pszPathPrefix,i);
		ofstream out(szPath,ios_base::out | ios_base::binary);
		m_Images[i]->Write(out);
		Log2(verbLevDebug1,"image #%d exported as %s\n",i+1,szPath);
		out.close();
	}
}

/*\
 * <---------- CSMAFGraph :: Decode ----------> 
 * @m 
 * --> I N <-- @p
 * unsigned char **pcBuffer - 
 * unsigned int nSize       - 
 * <-- OUT --> @r
 * 
\*/
void CSMAFGraph::Decode(unsigned char **pcBuffer,unsigned int nSize)
{
	bool bDone=false;
	unsigned char *pLim=*pcBuffer+nSize;
	map<tstring,int> :: const_iterator iterChunk;
	map<tstring,int> mapChunkId;

	mapChunkId[tstring(_T("Gtsu"))]=gcidGtsu;
	mapChunkId[tstring(_T("Gsq"))]=gcidGsq;
	mapChunkId[tstring(_T("Gimd"))]=gcidGimd;
	//most following bytes are unknown to me
	TRACEIT2("byte 0: %02X\n",**pcBuffer);
	*pcBuffer+=1;
	nSize-=1;
	TRACEIT2("byte 1: %02X\n",**pcBuffer);
	*pcBuffer+=1;
	nSize-=1;
	TRACEIT2("byte 2: %02X\n",**pcBuffer);
	*pcBuffer+=1;
	nSize-=1;
	TRACEIT2("byte 3: %02X\n",**pcBuffer);
	*pcBuffer+=1;
	nSize-=1;
	TRACEIT2("byte 4: %02X\n",**pcBuffer);
	*pcBuffer+=1;
	nSize-=1;
	TRACEIT2("byte 5: %02X\n",**pcBuffer);
	*pcBuffer+=1;
	nSize-=1;
	TRACEIT2("byte 6: %02X\n",**pcBuffer);
	*pcBuffer+=1;
	nSize-=1;
	TRACEIT2("byte 7: %02X\n",**pcBuffer);
	*pcBuffer+=1;
	nSize-=1;
	TRACEIT2("byte 8: %02X\n",**pcBuffer);
	*pcBuffer+=1;
	nSize-=1;
	TRACEIT2("byte 9: %02X\n",**pcBuffer);
	*pcBuffer+=1;
	nSize-=1;
	TRACEIT2("byte 10: %02X\n",**pcBuffer);		//resolution: 0x78=Q, 0xF0=QQ
	*pcBuffer+=1;
	nSize-=1;
	TRACEIT2("byte 11: %02X\n",**pcBuffer);
	*pcBuffer+=1;
	nSize-=1;
	TRACEIT2("byte 12: %02X\n",**pcBuffer);		//resolution: 0x78=Q, 0xF0=QQ
	*pcBuffer+=1;
	nSize-=1;
	TRACEIT2("byte 13: %02X\n",**pcBuffer);
	*pcBuffer+=1;
	nSize-=1;
	TRACEIT2("byte 14: %02X\n",**pcBuffer);
	*pcBuffer+=1;
	nSize-=1;
	TRACEIT2("byte 15: %02X\n",**pcBuffer);
	*pcBuffer+=1;
	nSize-=1;
	TRACEIT2("byte 16: %02X\n",**pcBuffer);
	*pcBuffer+=1;
	nSize-=1;
	TRACEIT2("byte 17: %02X\n",**pcBuffer);
	*pcBuffer+=1;
	nSize-=1;
	while (nSize)
	{
		unsigned int nChunkSize;
		unsigned char cAttribute;
		tstring sChunk;
		unsigned char *pLim=*pcBuffer+nSize;
		DecodeGraphChunk(pcBuffer,pLim,sChunk,&nChunkSize,&cAttribute);
		nSize-=8;
		TRACEIT2("decoded subchunk: %s - size: %d\n",sChunk.c_str(),nChunkSize);
		if ((iterChunk=mapChunkId.find(sChunk)) == mapChunkId.end())
		{
			TRACEIT2("unknown sub chunk\n");
			throw new CFormatException(CFormatException::formaterrUnknownSubChunk);
		}
		switch(iterChunk->second)
		{
			case gcidGimd:
				DecodeGIMD(pcBuffer,nChunkSize);
			break;
			default:
				*pcBuffer+=nChunkSize;
		}
		nSize-=nChunkSize;
	};
}

/*\
 * <---------- CSMAFGraph :: DecodeGIMD ----------> 
 * @m decode image list
 * --> I N <-- @p
 * unsigned char **pcBuffer - input smaf buffer
 * unsigned int nSize       - size of buffer
\*/
void CSMAFGraph::DecodeGIMD(unsigned char **pcBuffer,unsigned int nSize)
{
	bool bDone=false;
	map<tstring,int> :: const_iterator iterChunk;
	map<tstring,int> mapChunkId;

	//doesnt really make sense to use a mapping at this point, but who knows, maybe Yamaha decides to enhance the possible subtags of an image
	mapChunkId[tstring(_T("Gig"))]=gimdGig;
	while (nSize)
	{
		unsigned int nChunkSize;
		unsigned char cAttribute;
		tstring sChunk;
		unsigned char *pLim=*pcBuffer+nSize;

		DecodeGraphChunk(pcBuffer,pLim,sChunk,&nChunkSize,&cAttribute);
		nSize-=8;
		TRACEIT2("decoded subchunk: %s - size: %d\n",sChunk.c_str(),nChunkSize);
		if ((iterChunk=mapChunkId.find(sChunk)) == mapChunkId.end())
		{
			TRACEIT2("unknown sub chunk\n");
			throw new CFormatException(CFormatException::formaterrUnknownSubChunk,"invalid GIMD subchunk");
		}
		else
		{
			ASSERT(nChunkSize);
			if (nChunkSize)
			{
				CSMAFImage *pImage=new CSMAFImage((const char *)*pcBuffer,nChunkSize);
				ASSERT(pImage);
				if (pImage)
				{
					m_Images.push_back(pImage);
					Log2(verbLevDebug1,"image #%d decoded and stashed\n",m_Images.size());
				}
			}
		}
		*pcBuffer+=nChunkSize;
		nSize-=nChunkSize;
		TRACEIT2("size left: %d\n",nSize);
	};
	TRACEIT2("finished GIMD\n");
}

/*\
 * <---------- CSMAFGraph :: DecodeGraphChunk ----------> 
 * @m 
 * --> I N <-- @p
 * unsigned char **pcBuffer        - 
 * unsigned char *pcLimit          - 
 * tstring &sIdentifier            - 
 * unsigned int *pnSize            - 
 * unsigned char *pcChunkAttribute - 
\*/
void CSMAFGraph::DecodeGraphChunk(unsigned char **pcBuffer,unsigned char *pcLimit,tstring &sIdentifier,unsigned int *pnSize,unsigned char *pcChunkAttribute)
{
	int i;

	if (*pcBuffer+4 >= pcLimit)
	{
		TRACEIT2("truncated chunk - id missing/incomplete\n");
		throw new CFormatException(CFormatException::formaterrTruncated,"id missing/incomplete");
	}

	sIdentifier.clear();
	for (i=0;i < 4;i++)
	{
		if (**pcBuffer == '#')
			break;
		if (**pcBuffer < ' ' || **pcBuffer > 'z')
			break;
		sIdentifier+=**pcBuffer;
		*pcBuffer+=1;
		if (i == 2 && (sIdentifier == "Gig" || sIdentifier == "Gsq"))
		{
			++i;
			break;
		}
	}
	while (i++ < 4)
	{
		*pcChunkAttribute=**pcBuffer;
		*pcBuffer+=1;
	};

	if (*pcBuffer+4 > pcLimit)
	{
		TRACEIT2("truncated chunk - size missing/incomplete\n");
		throw new CFormatException(CFormatException::formaterrTruncated,"size missing/incomplete");
	}

	unsigned int nChunkSize;
	memcpy(&nChunkSize,*pcBuffer,4);
	*pnSize=ntohl(nChunkSize);
	*pcBuffer+=4;

	if (*pcBuffer+*pnSize > pcLimit)
	{
		TRACEIT2("chunk (%s) size exceeds file by %d byte\n",sIdentifier.c_str(),(*pcBuffer+*pnSize)-pcLimit);
		throw new CFormatException(CFormatException::formaterrInvalidChunkSize,"chunk size exceeds file size");
	}
}
