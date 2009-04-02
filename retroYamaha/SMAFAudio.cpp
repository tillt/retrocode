/*\
 * SMAFAudio.cpp
 * Copyright (C) 2004-2008, MMSGURU - written by Till Toenshoff
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
#include "../include/Resource.h"
#include "../retroBase/Basics.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MyString.h"
#include "../retroBase/MobileContent.h"
#include "SMAFEvent.h"
#include "SMAFDecoder.h"
#include "SMAFTrack.h"
#include "SMAFAudio.h"
#include "SMAFSample.h"

CSMAFAudio::CSMAFAudio()
{
	m_nChannels=1;
	m_nSampleRate=8000;
}

CSMAFAudio::~CSMAFAudio()
{
	if (m_Samples.size())
		m_Samples.erase(m_Samples.begin(), m_Samples.end());
}

/*\
 * <---------- *CSMAFAudio :: pGetSample ----------> 
 * @m get a specific smaf sample
 * --> I N <-- @p
 * int iIndex - index of the sample
 * <-- OUT --> @r
 * CSMAFSample * - pointer to sample or NULL
\*/
CSMAFSample *CSMAFAudio::pGetSample(int iIndex)
{
	CSMAFSample *pRet=NULL;
	if ((unsigned int)iIndex < m_Samples.size())
		pRet=m_Samples[iIndex];
	return pRet;
}

/*\
 * <---------- CSMAFAudio :: Decode ----------> 
 * @m decode entire audio track
 * --> I N <-- @p
 * unsigned char **pcBuffer - pointer to source buffer pointer
 * unsigned int nSize       - total size in bytes
\*/
void CSMAFAudio::Decode(unsigned char **pcBuffer,uint32_t nSize)
{
	bool bDone=false;
	map<tstring,uint32_t> :: const_iterator iterChunk;
	map<tstring,uint32_t> mapChunkId;

	mapChunkId[tstring(_T("Atsq"))]=acidAtsq;
	mapChunkId[tstring(_T("AspI"))]=acidAspI;
	mapChunkId[tstring(_T("Awa"))]=acidAwa;
	
	TRACEIT2("unknown: %02X\n",**pcBuffer);
	*pcBuffer+=1;
	nSize-=1;

	TRACEIT2("unknown: %02X\n",**pcBuffer);
	*pcBuffer+=1;
	nSize-=1;

	if (((**pcBuffer)&0x80) == 0x80)
	{
		Log2(verbLevDebug3,"sample is stereo\n");
		m_nChannels=2;
	}
	else
	{
		Log2(verbLevDebug3,"sample is mono\n");
		m_nChannels=1;
	}
	m_nFormat=(**pcBuffer)&0x70;
	switch(m_nFormat)
	{
		//pcm (signed)
		case 0x00:
			Log2(verbLevErrors,IDS_PRG_SMPPCM);
			throw new CFormatException(CFormatException::formaterrIncompatibleSampleFormat);
		break;
		//adpcm
		case 0x10:
			Log2(verbLevDebug1,IDS_PRG_SMPADPCM);
		break;
		//TwinVQ
		case 0x20:
			Log2(verbLevErrors,IDS_PRG_SMPTVQ);
			throw new CFormatException(CFormatException::formaterrIncompatibleSampleFormat,"twin-vq codec not supported");
		break;
		//MP3
		case 0x30:
			Log2(verbLevErrors,IDS_PRG_SMPMP3);
			throw new CFormatException(CFormatException::formaterrIncompatibleSampleFormat,"mp3 codec not supported");
		break;
		default:
			Log2(verbLevErrors,IDS_ERR_UNKNSMPENCN,(**pcBuffer)&0x70);
			throw new CFormatException(CFormatException::formaterrIncompatibleSampleFormat,"unknown sample encoding");
	}
	switch((**pcBuffer)&0x0F)
	{
		case 0x00:	m_nSampleRate=4000;		break;
		case 0x01:	m_nSampleRate=8000;		break;
		case 0x02:	m_nSampleRate=11025;	break;
		case 0x03:	m_nSampleRate=22050;	break;
		case 0x04:	m_nSampleRate=44100;	break;
		default:
			Log2(verbLevWarnings,IDS_ERR_INVRATEID,**pcBuffer);
			m_nSampleRate=8192;
	}
	Log2(verbLevDebug1,IDS_PRG_RATE,m_nSampleRate);
	*pcBuffer+=1;
	nSize-=1;

	switch((**pcBuffer)&0xF0)
	{
		case 0x00:			m_nBitsPerSample=4;		break;
		case 0x10:			m_nBitsPerSample=8;		break;
		case 0x20:			m_nBitsPerSample=12;	break;
		case 0x30:			m_nBitsPerSample=16;	break;
		default:
			Log2(verbLevWarnings,IDS_ERR_INVBITID,**pcBuffer);
			m_nBitsPerSample=16;
	}
	*pcBuffer+=1;
	nSize-=1;

	Log2(verbLevDebug2,"timebase d: %02X\n",**pcBuffer);
	switch(**pcBuffer)
	{
		case 0x00:	m_nResolution=1;	break;
		case 0x01:	m_nResolution=2;	break;
		case 0x02:	m_nResolution=4;	break;
		case 0x03:	m_nResolution=5;	break;
		case 0x10:	m_nResolution=10;	break;
		case 0x11:	m_nResolution=20;	break;
		case 0x12:	m_nResolution=20;	break;
		case 0x13:	m_nResolution=50;	break;
		default:
			Log2(verbLevWarnings,IDS_ERR_INVTIMBASID,**pcBuffer);
	}
	*pcBuffer+=1;
	nSize-=1;

	Log2(verbLevDebug2,"timebase g: %02X\n",**pcBuffer);
	*pcBuffer+=1;
	nSize-=1;
	
	//TRACEIT("audio size: %d\n",nSize);

	while (nSize)
	{
		unsigned int nChunkSize;
		unsigned char cAttribute;
		tstring sChunk;
		CSMAFSample *pSample;
		unsigned char *pLim=*pcBuffer+nSize;

		DecodeChunk(pcBuffer,pLim,sChunk,&nChunkSize,&cAttribute);
		Log2(verbLevDebug2,"decoded subchunk: %s - size: %d\n",sChunk.c_str(),nChunkSize);
		if ((iterChunk=mapChunkId.find(sChunk)) == mapChunkId.end())
		{
			Log2(verbLevErrors,IDS_ERR_UNKSUBCHNK);
			throw new CFormatException(CFormatException::formaterrUnknownSubChunk,"unknown subchunk");
		}
		else
		{
			switch(iterChunk->second)
			{
				//instrument settings
				case acidAspI:
					TRACEIT2("audio track seek and phrase info\n");
				break;
				//PCM sample data
				case acidAwa:
				{
					TRACEIT2("sample data, wave id: %02Xh\n",cAttribute);
					pSample=new CSMAFSample(m_nSampleRate,m_nChannels);
					/*
					//debugging code
					FILE *fp=fopen("decoder_adpcm.raw","wb");
					fwrite(*pcBuffer,1,nChunkSize,fp);
					fclose(fp);
					*/
					pSample->Decode(*pcBuffer,nChunkSize);
					m_Samples.push_back(pSample);
				}
				break;
				//sequence data
				case acidAtsq:
					TRACEIT2("sequence data\n");
					DecodeAtsq(*pcBuffer,nChunkSize);
				break;
			}
			*pcBuffer+=nChunkSize;
			nSize-=nChunkSize+8;
			TRACEIT("size lefct: %d\n",nSize);
		}
	};
}

/*\
 * <---------- CSMAFAudio :: DecodeAtsq ----------> 
 * @m decode audio track sequence tag
 * --> I N <-- @p
 * unsigned char *pcBuffer - source buffer
 * unsigned int nSize      - size of the tag
\*/
void CSMAFAudio::DecodeAtsq(unsigned char *pcBuffer,uint32_t nSize)
{
	CSMAFEvent *pEvent;
	bool bAwaitDelta=true;
	int nByteUsed,nEventDataSize;
	int nDelta,nDuration,nChannel,nNote,nFullChannel,nValue;
	uint32_t dwAt=0;
	unsigned char cOctaveOffset[4]={1,1,1,1};
	unsigned char cVolume[4]={100,100,100,100};
	unsigned char theByte;
	int nRunState=-1;
	unsigned char *pcLimit=pcBuffer+nSize;

	while (pcBuffer < pcLimit)
	{
		nEventDataSize=0;
		if (bAwaitDelta)
		{
			nByteUsed=nDecodeVariableQuantity(&pcBuffer,pcLimit,&nDelta);
			Log2(verbLevDebug1,"delta: %d (stored in %d bytes)\n",nDelta,nByteUsed);
			dwAt+=nDelta*m_nResolution;
			if (nDelta*m_nResolution >= 5000)
				Log2(verbLevDebug2,"huge delta!\n");
		}
		theByte=*(pcBuffer++);
		switch(theByte)
		{
			case 0xFF:
				switch(*pcBuffer)
				{
					case 0x00:
						Log2(verbLevDebug1,"%08Xh - found mute all event\n",dwAt);
						pcBuffer+=1;
					break;
					case 0xF0:
						pcBuffer+=1;
						Log2(verbLevDebug1,"%08Xh - found sysex - size: %d!!!\n",dwAt,*pcBuffer);
						pcBuffer+=*pcBuffer+1;
					break;
					case 0xFF:
						Log2(verbLevDebug1,"%08Xh - found NOP\n",dwAt);
						pcBuffer+=1;
					break;
					default:
						Log2(verbLevWarnings,"%08Xh - found unknown status FF-event: %02Xh\n",dwAt,*pcBuffer);
				}
			break;
			case 0x00:
				//TRACEIT("CSMAFTrack::DecodeMtsq - double byte event - %02Xh\n",*(pcBuffer+1));
				//nRunState=theByte;
				nChannel=(*pcBuffer>>6)&0x03;
				nFullChannel=nChannel+m_nChannelOffset;
				if (nFullChannel == 9)
				{
					nFullChannel=nChannel+m_nChannelOffset;
					++m_nChannelOffset;
				}
				nValue=*pcBuffer&0x0F;
				switch(*pcBuffer&0x30)
				{
					case 0x00:	//expression (short)
						Log2(verbLevDebug1,"%08Xh - expression (short) value: %02Xh on channel %d\n",dwAt,nValue,nChannel);
						nEventDataSize=0;
					break;
					case 0x10:	//pitch bend (short)
						Log2(verbLevDebug1,"%08Xh - pitch bend (short) value: %02Xh on channel %d\n",dwAt,nValue,nChannel);
						nEventDataSize=0;
					break;
					case 0x20:	//modulation (short)
						Log2(verbLevDebug1,"%08Xh - modulation (short) value: %02Xh on channel %d\n",dwAt,nValue,nChannel);							
						nEventDataSize=0;
					break;
					case 0x30:
						switch(nValue)
						{
							case 0x00:
								nEventDataSize=1;
								Log2(verbLevDebug1,"program change %d\n",nChannel);
								pEvent=new CSMAFEvent(	dwAt,
														nFullChannel,
														*pcBuffer & 0x3F,
														pcBuffer+1,
														nEventDataSize);
								Log2(verbLevDebug1,"%08Xh - program change: %02X on channel %d\n",dwAt,*(pcBuffer+1),nChannel);
								m_Events.push_back(pEvent);
							break;
							case 0x01:
								nEventDataSize=1;
								pEvent=new CSMAFEvent(	dwAt,
														nFullChannel,
														*pcBuffer & 0x3F,
														pcBuffer+1,
														nEventDataSize);
								Log2(verbLevDebug1,"%08Xh - bank change: %02X on channel %d\n",dwAt,*(pcBuffer+1),*(pcBuffer+2),*(pcBuffer+3),*(pcBuffer+4),nChannel);
								m_Events.push_back(pEvent);
							break;
							case 0x02:
								nEventDataSize=1;
								Log2(verbLevDebug1,"%08Xh - octave change on channel %d\n",dwAt,nChannel);
								switch(*(pcBuffer+1))
								{
									case 0x84:	cOctaveOffset[nChannel]=1;	break;
									case 0x83:	cOctaveOffset[nChannel]=2;	break;
									case 0x82:	cOctaveOffset[nChannel]=3;	break;
									case 0x81:	cOctaveOffset[nChannel]=4;	break;
									default:	cOctaveOffset[nChannel]=*(pcBuffer+1)+5;
								}
							break;
							case 0x03:
								Log2(verbLevDebug1,"%08Xh - modulation (normal) on channel %d\n",dwAt,nChannel);
								nEventDataSize=1;
							break;
							case 0x04:
								Log2(verbLevDebug1,"%08Xh - pitch bend (normal) on channel %d\n",dwAt,nChannel);
								nEventDataSize=1;
							break;
							case 0x07:
								Log2(verbLevDebug1,"%08Xh - volume on channel %d\n",dwAt,nChannel);
								nEventDataSize=1;
								cVolume[nChannel]=*(pcBuffer+1);
							break;
							case 0x0A:
								Log2(verbLevDebug1,"%08Xh - pan on channel %d\n",dwAt,nChannel);
								nEventDataSize=1;
							break;
							case 0x0B:
								Log2(verbLevDebug1,"%08Xh - expression (normal) on channel %d\n",dwAt,nChannel);
								nEventDataSize=1;
							break;
							default:
								Log2(verbLevWarnings,"%08Xh - found unknown double-byte event 00h,%02Xh !!!\n",dwAt,*pcBuffer);
								//throw new CFormatException(CFormatException::formaterrUnknownSequenceFormat,"unknown double byte event found");
						}
					break;
				}
				pcBuffer+=nEventDataSize+1;				
			break;
			default:
				nChannel=(theByte>>6)&0x03;
				nFullChannel=nChannel+m_nChannelOffset;
				if (nFullChannel == 9)
				{
					nFullChannel=nChannel+m_nChannelOffset;
					++m_nChannelOffset;
				}
				nNote=(unsigned int)(cOctaveOffset[nChannel]&0x7F)*12;
				nNote+=((theByte>>4)&0x03)*12;
				nNote+=theByte&0x0F;
				nByteUsed=nDecodeVariableQuantity(&pcBuffer,pcLimit,&nDuration);
				nDuration=nDuration*m_nResolution;
				if (nDuration == 0)
					Log2(verbLevDebug2,"zero size note!!!\n");
				if (nDuration >= 5000)
					Log2(verbLevDebug2,"long size note!!!\n");
				//TRACEIT("CSMAFTrack::DecodeMtsq - %08Xh - Channel: %d, Note Value: %d, Note-Duration: %d\n",dwAt,nFullChannel,nNote,nDuration);
				pEvent=new CSMAFEvent(dwAt,nFullChannel,nNote,nDuration,cVolume[nChannel]);
				m_Events.push_back(pEvent);
		}
	};
}

/*\
 * <---------- CSMAFAudio :: nGetMaxPlaytime ----------> 
 * @m get longest sample playtime
 * --> I N <-- @p
 * int *pIndex - pointer to index of longest sample
 * <-- OUT --> @r
 * int - playtime in milliseconds
\*/
uint32_t CSMAFAudio::nGetMaxPlaytime(int *pIndex)
{
	CSMAFSample *pSample;
	uint32_t i,nMax=0;
	for (i=0;i < (uint32_t)m_Samples.size();i++)
	{
		pSample=m_Samples[i];
		if (pSample->nGetPlaytime() > nMax)
		{
			nMax=pSample->nGetPlaytime();
			if (pIndex)
				*pIndex=i;
		}
	}	
	return nMax;
}

/*\
 * <---------- CSMAFAudio :: nGetPlaytime ----------> 
 * @m get total sequence playback duration
 * <-- OUT --> @r
 * int - number of milliseconds
\*/
uint32_t CSMAFAudio::nGetPlaytime(void)
{
	uint32_t nRet=0;
	ASSERT(nGetEventCount()); 
	if (nGetEventCount())
	{
		CSMAFEvent *pLast=m_Events[nGetEventCount()-1];
		ASSERT(pLast);
		if (pLast)
		{
			nRet=pLast->dwGetAt()+pLast->nGetDuration();
			Log2(verbLevDebug3,"%d -> %d\n",nRet,nRet/5);
		}
	}
	//why this fixed devider by 5? - that smells foul!
	return nRet/5;
	//todo: check if where this comes from
}

/*\
 * <---------- CSMAFAudio :: cEncodeSampleRate ----------> 
 * @m encode a sample rate into an index
 * --> I N <-- @p
 * uint32_tnSampleRate - samples per second
 * <-- OUT --> @r
 * unsigned char - smaf rate index
\*/
unsigned char CSMAFAudio::cEncodeSampleRate(uint32_t nSampleRate)
{
	unsigned char cOut=0xFF;
	switch(nSampleRate)
	{
		case 4000:	cOut=0x00;		break;
		case 8000:	cOut=0x01;		break;
		case 11025:	cOut=0x02;		break;
		case 22050:	cOut=0x03;		break;
		case 44100:	cOut=0x04;		break;
		default:
			Log2(verbLevErrors,IDS_ERR_INVRATE,nSampleRate);
	}
	return cOut;
}
