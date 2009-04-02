/*\
 * RMFSequence.cpp
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
#ifndef WIN32
#include <netinet/in.h>
#else
#include <winsock2.h>
#endif
#include <strstream>
#include "../retroBase/Basics.h"
#include "RMFBasics.h"
#include "RMFSequence.h"
#include "../retroBase/MIDIFileWriter.h"
#include "../retroBase/Midispec.h"
#include "../retroBase/MobileContent.h"
#include "../include/Resource.h"

using namespace std;

CRMFSequence::CRMFSequence(int nEncoding,const TCHAR *pszName) : m_sName(_T("unknown")),m_pcSequenceBuffer(NULL),m_nSequenceSize(0),m_nEncoding(nEncoding)
{
	if (pszName != NULL)
		m_sName=pszName;
}

CRMFSequence::~CRMFSequence()
{
	if (m_pcSequenceBuffer != NULL)
		delete [] m_pcSequenceBuffer;
}

/*\
 * <---------- Compress ---------->
 * @m not implemented - one-shot RMFs are smaller with an uncompressed sequence
\*/
void CRMFSequence::Compress(void)
{
}

/*\
 * <---------- Decompress ---------->
 * @m sequence unfolding
 * --> I N <-- @p
 * uint32_tnUnfoldedSize - size of uncompressed data
\*/
void CRMFSequence::Decompress(uint32_t nUnfoldedSize)
{
	ASSERT(nUnfoldedSize);
	uint32_t nOutSize=nUnfoldedSize;

	int32_t nOutLeft;
	int32_t nInLeft=m_nSequenceSize;

	unsigned char cInByte,cBuffer;
	unsigned char cMask;
	unsigned char cCount;

	int32_t nBigValueAX;
	uint32_t nBigValueDX;

	unsigned char *pcInCX=m_pcSequenceBuffer;
	unsigned char *pcOut,*pcStart;
	unsigned char *pcUncompressed=new unsigned char[nOutSize];
	
	Log2(verbLevDebug1,IDS_PRG_DECSEQ,m_nSequenceSize);
	
	pcOut=pcUncompressed;
	pcInCX+=4;
	nInLeft-=4;
	nOutLeft=nOutSize;
	pcStart=pcInCX;

	while (--nInLeft > 0 && nOutLeft > 0)			
	{
		cInByte=*(pcInCX++);
		cBuffer=cInByte;
		cMask=0x01;
		while (cMask != 0x00)
		{
			cCount=cBuffer;
			//mask bit set?
			if (cCount & cMask)
			{	//yes->...
				if (--nInLeft >= 0 && --nOutLeft >= 0)
					*(pcOut++)=*(pcInCX++);
			}
			else
			{
				nInLeft-=2;
				if (nInLeft >= 0)
				{
					nBigValueDX=(((uint32_t)*pcInCX)<<8) | *(pcInCX+1);
					pcInCX+=2;
					nBigValueAX=(nBigValueDX>>12)+3;
					nBigValueDX=nBigValueDX & 0x0FFF;

					nOutLeft-=nBigValueAX;
					if (nOutLeft < 0)
						nBigValueAX+=nOutLeft;

					--nBigValueAX;
					if (nBigValueAX > 0)
					{
						int32_t nOffset=(int32_t)nBigValueDX-0x1000;
						uint32_t nCopyCount=nBigValueAX+1;
						if ( (pcOut-pcUncompressed) + nOffset >= 0 )
						{
							unsigned char *pcCopy=pcOut + nOffset;
							while (nCopyCount)
							{
								*(pcOut++)=*(pcCopy++);
								--nCopyCount;
							};
						}
						else
						{
							Log2(verbLevErrors,"failure in repeat mask\n");
							throw new CFormatException(CFormatException::formaterrInvalid,"failure in repeat mask");
						}
					}
				}
			}
			cMask=cMask << 1;
		};
	};

	m_nSequenceSize=(uint32_t)(pcOut-pcUncompressed);
	delete [] m_pcSequenceBuffer;
	m_pcSequenceBuffer=pcUncompressed;
	Log2(verbLevDebug1,IDS_PRG_SEQDECDONE,m_nSequenceSize);
}

/*\
 * <---------- ExportRaw ---------->
 * @m quick and dirty sequence file export
 * --> I N <-- @p
 * const char *pcpath - destination file path
\*/
void CRMFSequence::ExportRaw(const char *pcpath)
{
	FILE *fp;
	if ((fp=fopen(pcpath,"wb")) != NULL)
	{
		fwrite(m_pcSequenceBuffer,m_nSequenceSize,1,fp);
		fclose(fp);
	}
	else
	{
		Log2(verbLevErrors,"couldnt create output file \"%s\"\n",pcpath);
		throw new CFormatException(CFormatException::formaterrUnknown,"couldnt create output file");
	}
}

/*\
 * <---------- Serialize ---------->
 * @m read a sequence from a stream
 * --> I N <-- @p
 * istream &ar - stream reference
\*/
void CRMFSequence::Serialize(istream &ar)
{
	unsigned char nNameLen;
	TCHAR sBuffer[256];
	char pcBuffer[5];
	pcBuffer[4]=0;
	tstring s;
	unsigned int nPatch;

	ar.read((char *)&nPatch,4);
	nPatch=ntohl(nPatch);
	Log2(verbLevDebug2,"tag id %Xh\n",nPatch);
	ar.read((char *)&nNameLen,1);
	if (nNameLen > 0)
	{
		ar.read(sBuffer,nNameLen);
		sBuffer[nNameLen]=0;
		m_sName.assign((const TCHAR *)sBuffer);
	}
	Log2(verbLevDebug2,"name: \"%s\"\n",m_sName.c_str());

	uint32_t nBlockSize;
	unsigned char *pcSequence,*pcDecrypted;

	ar.read((char *)&nBlockSize,4);
	nBlockSize=ntohl(nBlockSize);
	Log2(verbLevDebug2,"ecmi block size: %d\n",nBlockSize);

	pcSequence=new unsigned char[nBlockSize];
	ar.read((char *)pcSequence,nBlockSize);

	if (m_nEncoding == seqEmid || m_nEncoding == seqEcmi) 
	{
		pcDecrypted=new unsigned char[nBlockSize];
		Log2(verbLevDebug1,IDS_PRG_SEQUCRYPDONE,nBlockSize);
		CRMFBasics::DecryptBinary(pcSequence,pcDecrypted,nBlockSize);
		//hexdump("decrypted sequence data: ",pcDecrypted,nBlockSize);
		delete [] pcSequence;
	}
	else
		pcDecrypted=pcSequence;

	m_nSequenceSize=nBlockSize;
	m_pcSequenceBuffer=new unsigned char [m_nSequenceSize];
	memcpy(m_pcSequenceBuffer,pcDecrypted,m_nSequenceSize);
	delete [] pcDecrypted;

	//is it a comressed MIDI or a compressed, encrypted MIDI?
	if (m_nEncoding == seqCmid || m_nEncoding == seqEcmi) 
	{	//yes->decompress
		uint32_t nSizeCode;								//this looks overblown but 
		memcpy(&nSizeCode,m_pcSequenceBuffer,4);				//is needed for certain host systems
		CRMFSequence::Decompress(ntohl(nSizeCode)&0x00FFFFFF);
	}
}

void CRMFSequence::Serialize(ostream &ar)
{
}

/*\
 * <---------- nRenderMIDI ---------->
 * @m internal one-shot RMF midi rendering
 * --> I N <-- @p
 * int nChannels - number of channels in sample data
 * int nSampleRate - sample rate for sample data
 * int nSampleSize - size in bytes of sample data
 * uint32_t nPlaytime - playback time in miliseconds
 * uint32_t nFadetime - fade out duration
 * unsigned char *pcDest - output buffer
 * uint32_t nDestSize - buffer size
 * <-- OUT --> @r
 * uint32_t- bytes used for encoding
\*/
uint32_t CRMFSequence::nRenderMIDI(int nChannels,int nSampleRate,int nSampleSize,uint32_t nPlaytime,uint32_t nFadetime,unsigned char *pcDest,uint32_t nDestSize)
{
	uint32_t nUsed=0;
	const int nDefaultTempo=120;	//120BPM
	CMIDIFileWriter midi;
	CMIDITrack *pTrack;
	ReadStruct rs;
	int nQuanta=0;

	uint64_t nSampleQuanta;
	uint32_t nPlayQuanta;
	uint32_t nFadeQuanta;
	
	nSampleQuanta=((uint64_t)500*nSampleSize)/nSampleRate;
	nPlayQuanta=nPlaytime;
	nFadeQuanta=nFadetime;
	
	rs.data=new unsigned char [1024];
	rs.nData=0;
	rs.bMetaEvent=FALSE;
	rs.quanta=0;

	//initialize the MIDI docunent
	midi.SetTiming(0x1e0,DEFAULTQUANTASIZE);
	midi.SetChannelUsed(0,TRUE);
	midi.SetSongName(m_sName.c_str());
	//get a pointer to the first midi-track
	pTrack=midi.GetTracks()->at(0);
	ASSERT (pTrack);

	uint32_t temp=60000000/nDefaultTempo;
	//event #1: MIDI TEMPO
	rs.bMetaEvent=TRUE;
	rs.quanta=nQuanta;
	rs.nData=4;
	rs.data[0]=MIDI_TEMPO;
	rs.data[1]=(unsigned char)((temp>>16)&0xFF);
	rs.data[2]=(unsigned char)((temp>>8)&0xFF);
	rs.data[3]=(unsigned char)(temp&0xFF);
	pTrack->AddEvent(&rs);

	//event #2: control change	00 02	(bank change msb)
	rs.bMetaEvent=FALSE;
	rs.data[0]=MIDI_CONTROL;
	rs.data[1]=0x00;
	rs.data[2]=0x02;
	rs.nData=3;
	rs.quanta=nQuanta;
	pTrack->AddEvent(&rs);

	//event #3: program change	00
	rs.bMetaEvent=FALSE;
	rs.data[0]=MIDI_PROGRAM;
	rs.data[1]=0x00;
	rs.nData=2;
	rs.quanta=nQuanta;
	pTrack->AddEvent(&rs);

	//event #4: note on
	rs.bMetaEvent=FALSE;
	rs.data[0]=MIDI_NOTEON;
	rs.data[1]=0x3C;
	rs.data[2]=0x7F;
	rs.nData=3;
	rs.quanta=nQuanta;
	pTrack->AddEvent(&rs);

	if (nPlayQuanta > 0)
		nQuanta+=nPlayQuanta;
	else
    	nQuanta+=(unsigned long)nSampleQuanta;

	if (nFadeQuanta > 0)
	{
		int i;
		uint32_t nSteps=16;
		uint32_t nVolume;
		double fQuanta=(double)nQuanta;
		double fQuantaStep=nFadeQuanta/nSteps;

		for (i=0;i < (int)nSteps;i++)
		{
			if (i > 0)
			{
				ASSERT((uint32_t)(fQuantaStep*i) < nFadeQuanta);
				nVolume=min((nSteps-i)*(128/16),127);
				//event #5+iFadeStep: volume change
				rs.bMetaEvent=FALSE;
				rs.data[0]=MIDI_CONTROL;
				rs.data[1]=0x07;
				rs.data[2]=(unsigned char)(nVolume&0xFF);
				rs.nData=3;
				rs.quanta=(uint32_t)fQuanta;
				pTrack->AddEvent(&rs);
			}
			fQuanta+=fQuantaStep;
		}
		nQuanta=(int)fQuanta;
	}

	//event #5+nFadeSteps: note off
	rs.bMetaEvent=FALSE;
	rs.data[0]=MIDI_NOTEON;
	rs.data[1]=0x3C;
	rs.data[2]=0x00;
	rs.nData=3;
	rs.quanta=nQuanta;
	pTrack->AddEvent(&rs);

	//render the midi document to a memory-stream
	strstream mem;
	midi.Save(mem);
	nUsed=mem.pcount();
	if (pcDest)
	{
		memcpy(pcDest,mem.str(),nUsed);
		pcDest+=nUsed;
	}
	delete [] rs.data;
	return nUsed;
}

/*\
 * <---------- nRender ---------->
 * @m create sequence based on arguments
 * --> I N <-- @p
 * rmfCACH *pCache - RMF-cache block
 * unsigned char *pDest - output memory buffer
 * int nChannels - number of channels in sample data
 * int nSampleRate - sample rate in sample data
 * uint32_tnSampleSize - size in bytes of sample data
 * uint32_tnPlaytime - blackback time in miliseconds
 * uint32_tnFadetime - fade out duration
 * <-- OUT --> @r
 * uint32_t- number of bytes used for encoding
\*/
uint32_t CRMFSequence::nRender(rmfCACH *pCache,unsigned char *pDest,int nChannels,int nSampleRate,uint32_t nSampleSize,uint32_t nPlaytime,uint32_t nFadetime)
{
	uint32_t nUsed=0;
	uint32_t nEMIDSize=nRenderMIDI(nChannels,nSampleRate,nSampleSize,nPlaytime,nFadetime);
	int nLen=(int)m_sName.length();
	
	if (pDest)
	{
		ASSERT(pCache);
		memcpy(pDest,"emid",4);
		memcpy(pCache->sTag,"emid",4);
		pDest+=4;
	}
	nUsed+=4;
	if (pDest)
	{
		pCache->nFirstValue=CRMFSequence::sequenceID;
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
		pCache->nDataSize=nEMIDSize;
		pDest=CRMFBasics::pRenderInteger(pDest,pCache->nDataSize);
	}
	nUsed+=4;
	if (pDest)
	{
		//compress sound data
		Log2(verbLevDebug1,"rendering RMF sequence...\n");
		nRenderMIDI(nChannels,nSampleRate,nSampleSize,nPlaytime,nFadetime,pDest,nEMIDSize);
		pCache->nDataOffset=nUsed;
		hexdump("raw midi:  ",pDest,nEMIDSize);
		//encrypt sound data
		CRMFBasics::EncryptBinary(pDest,pDest,nEMIDSize);
		//hexdump("encrypted: ",pDest,nEMIDSize);
		//unsigned char *pCompressed=new unsigned char [nEMIDSize*2];
		//compress sound data
		//hexdump("compressed: ",pDest,nEMIDSize);
		pDest+=nEMIDSize;
	}
	nUsed+=nEMIDSize;
	return nUsed;
}
