/*\
 * SMAFTrack.cpp
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
#include "../retroBase/Basics.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MyString.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/Midispec.h"
#include "SMAFDecoder.h"
#include "SMAFTrack.h"
#include "SMAFSample.h"
#include "SMAFEvent.h"

CSMAFTrack::CSMAFTrack(int nChannelOffset)
{
	m_nType=typeUndefined;
	m_bUsesHumanVoice=false;
	m_nChannelOffset=nChannelOffset;
	m_bUsesSynthesizer=false;
	m_nHighestLocatedDeviceId=0;
}

CSMAFTrack::~CSMAFTrack()
{
	unsigned int i;
	for (i=0;i < (unsigned int)m_Events.size();i++)
	{
		ASSERT (m_Events[i]);
		if (m_Events[i] != NULL)
			delete m_Events[i];
	}
	if (m_Events.size())
		m_Events.erase(m_Events.begin(), m_Events.end());
	for (i=0;i < (unsigned int)m_Samples.size();i++)
	{
		ASSERT (m_Samples[i]);
		if (m_Samples[i] != NULL)
			delete m_Samples[i];
	}
	if (m_Samples.size())
		m_Samples.erase(m_Samples.begin(), m_Samples.end());
}

/*\
 * <---------- CSMAFTrack :: sParseSysEx ----------> 
 * @m read all data of a system exclusive event, update the interal device identifier status 
 * Parse a System Exclusive Event
 * 0xF0 = Sysex Event
 * 0xXX = EvemtData-Size
 * 0x43 = Yamaha
 * 0x79 = SMAF?
 * 0x06 = MA3 / 0x07 = MA5 / 0x08 = MA7
 * 0x7F = ?
 * ...
 * 0xF7 = Terminator
 * --> I N <-- @p
 * char *pcEvent - pointer to the source event buffer
 * int nSize     - size of the event data packet
 * bool bMTSU    - indicator for setup-track specific data
 * <-- OUT --> @r
 * tstring - readable string describing the event and its parameters
\*/
tstring CSMAFTrack::sParseSysEx(char *pcEvent,int nSize,bool bMTSU)
{
	tstring sRet;
	char pcNumber[16];

	if (pcEvent)
	{	
		sRet="system exclusive: ";
		sprintf(pcNumber,"%d byte: ",nSize);
		sRet+=pcNumber;
		if (nSize < 5)
			return sRet + "too small for further investigation";
		if (*(unsigned char *)(pcEvent++) != MIDI_SYSXMID_YAMAHA)
			return sRet + "unexpected manufacturer";
		else
		{
			sRet+= "Yamaha, ";
			if (*(unsigned char *)(pcEvent++) != MIDI_SYSXGID_SMAF)
				return sRet + "unknown device group";
			else
			{
				sRet += "SMAF, ";
				m_nHighestLocatedDeviceId=max(m_nHighestLocatedDeviceId,*(unsigned char *)pcEvent);
				switch(*(unsigned char *)(pcEvent++))
				{
					case MIDI_SYSXPID_MA3:
						sRet += "MA3, ";
					break;
					case MIDI_SYSXPID_MA5:
						sRet += "MA5, ";
					break;
					case MIDI_SYSXPID_MA7:
						sRet += "MA7, ";
					break;
					default:
						return sRet + "unknown device";
				}
				if (*(unsigned char *)(pcEvent++) != 0x7F)
					return sRet + "invalid device addressing terminator";
				else
				{
					nSize-=5;
					if (!bMTSU)
					{
						switch(*pcEvent)
						{
							case 0x00:			//mixer main volume
								sRet += "mixer main volume ";
							break;
							case 0x0B:			//mixer panning
								sRet += "mixer panning ";
								++pcEvent;
								--nSize;
								sprintf(pcNumber,"%02X",*pcEvent);
								sRet += tstring("pcm channel #") + pcNumber + tstring(" ");
							break;
							default:
								sprintf (pcNumber,"%02X ",*pcEvent);
								sRet += pcNumber;
						}
					}
					else
					{
						switch(*pcEvent)
						{
							case 0x07:			//channel usage?
								sRet += "channel usage ";
							break;
							case 0x7F:			//channel usage?
								sRet += "reset";
							break;
							default:
								sprintf (pcNumber,"%02X ",*pcEvent);
								sRet += pcNumber;
						}
					}
					++pcEvent;
					while(--nSize)
					{
						sprintf(pcNumber,"%02X ",*(unsigned char *)pcEvent++);
						sRet += pcNumber;
					};

					if (*(unsigned char*)(pcEvent++) != 0xF7)
					{
						TRACEIT2("got a %02X\n",*(pcEvent-1));
						return sRet + "invalid device message terminator";
					}
				}
			}
		}
	}
	return sRet;
}

/*\
 * <---------- CSMAFTrack :: nRenderSysEx ----------> 
 * @m render a yamaha smaf specific system exclusive message
 * --> I N <-- @p
 * char *pcDest        - output buffer or NULL for size calculation mode
 * int nDeviceId       - device identifier
 * const char *pcMsg   - source message buffer
 * unsigned char nSize - size of the source message
 * <-- OUT --> @r
 * inr - number of bytes needed for storage
\*/
int CSMAFTrack::nRenderSysEx(char *pcDest,int nDeviceId,const char *pcMsg,unsigned char nSize)
{
	ASSERT(nSize < 122);
	int nEventSize = nSize+5;
	int nTotalSize = nEventSize+2;
	if (pcDest)
	{
		*(pcDest++)=(char)MIDI_SYSEXCL;						//
		*(pcDest++)=(char)nEventSize;						//
		*(pcDest++)=(char)MIDI_SYSXMID_YAMAHA;				//manufacturer id
		*(pcDest++)=(char)MIDI_SYSXGID_SMAF;				//device group id
		*(pcDest++)=(char)nDeviceId;						//device id
		*(pcDest++)=(char)0x7F;								//end of adressing
		memcpy(pcDest,pcMsg,nSize);	
		pcDest+=nSize;
		*(pcDest++)=(char)0xF7;								//end of data
	}
	return nTotalSize;
}

/*\
 * <---------- CSMAFTrack :: nEncodeVariableQuantity ----------> 
 * @m store a number within as little bytes as possible
 * --> I N <-- @p
 * unsigned int nValue     - a number
 * unsigned char *pcBuffer - destination buffer
 * <-- OUT --> @r
 * int - number of bytes needed for encoding
\*/
int CSMAFTrack::nEncodeVariableQuantity (unsigned int nValue,unsigned char *pcBuffer)
{
	int iByte=0;
	if ((unsigned int)nValue > 0x1FFFFFF)
	{
		pcBuffer[iByte++]=(((nValue>>21)&0x7F)|0x80)-1;
		pcBuffer[iByte++]=(((nValue>>14)&0x7F)|0x80)-1;
		pcBuffer[iByte++]=(((nValue>>7)&0x7F)|0x80)-1;
	}
	else if ((unsigned int)nValue > 0x3FFF)
	{
		pcBuffer[iByte++]=(((nValue>>14)&0x7F)|0x80)-1;
		pcBuffer[iByte++]=(((nValue>>7)&0x7F)|0x80)-1;
	}
	else if ((unsigned int)nValue > 0x7F)
	{
		pcBuffer[iByte++]=(((nValue>>7)&0x7F)|0x80)-1;
	}
	pcBuffer[iByte++]=nValue&0x7F;
	//pcBuffer+=iByte;
	return iByte;
}

/*\
 * <---------- CSMAFTrack :: nDecodeVariableLength ----------> 
 * @m decode a variable length encoded number using the Yamaha sysex size specifc algorithm
 * --> I N <-- @p
 * unsigned char **pcBuffer - pointer to source buffer pointer
 * unsigned char *pcLimit   - buffer end pointer
 * int *n                   - 
 * <-- OUT --> @r
 * int - bytes used by encoded number
\*/
int CSMAFTrack::nDecodeVariableLength(unsigned char **pcBuffer,unsigned char *pcLimit,int *n)
{
	int i=0;
	int m=0;
	int o=0;

	while (*pcBuffer+i < pcLimit)
    {
    	if (*(*pcBuffer+i) & 0x80)
        	m = (m<<7) + (*(*pcBuffer+i) & 0x7F);
		else
        {
			*n = (m<<7) + (*(*pcBuffer+i) & 0x7F);
			if ((*n & 0x0F000000) == 0x0F000000)
				*n|=0xFF000000;
			*pcBuffer+=i+1;
            return i+1;
		}
		++i;
	};
	*pcBuffer+=i;
    return i;
}

/*\
 * <---------- CSMAFTrack :: nDecodeVariableQuantity ----------> 
 * @m decode a variable length encoded number using the SMAF sequence timing specifc algorithm
 * --> I N <-- @p
 * unsigned char **pcBuffer - pointer to source buffer pointer
 * unsigned char *pcLimit   - buffer end pointer
 * int *n                   - 
 * <-- OUT --> @r
 * int - bytes used by encoded number
\*/
int CSMAFTrack::nDecodeVariableQuantity (unsigned char **pcBuffer,unsigned char *pcLimit,int *n)
{
	int i=0;
	int m=0;
	int o=0;

	while (*pcBuffer+i < pcLimit)
    {
    	if (*(*pcBuffer+i) & 0x80)
        	m = (m<<7) + (*(*pcBuffer+i) & 0x7F) + 1;
		else
        {
			*n = (m<<7) + (*(*pcBuffer+i) & 0x7F);
			if ((*n & 0x0F000000) == 0x0F000000)
				*n|=0xFF000000;
			*pcBuffer+=i+1;
            return i+1;
		}
		++i;
	};
	*pcBuffer+=i;
    return i;
}

/*

Delta time (variable length) + part (2bit) + octave (2bit) + musical scale (4bit) + release time (variable length)

event byte
	PPOOSSSS
21h 00100001	//C
11h 00010001	//C3
02h 00000010	//C1

*/

/*\
 * <---------- CSMAFTrack :: DecodeMtsq_handy ----------> 
 * @m decode sequence data - handyphone format
 * --> I N <-- @p
 * unsigned char *pcBuffer - source sequence buffer
 * unsigned int nSize      - size of the sequence data in bytes
\*/
void CSMAFTrack::DecodeMtsq_handy(unsigned char *pcBuffer,unsigned int nSize)
{
	CSMAFEvent *pEvent;
	bool bAwaitDelta=true;
	int nByteUsed,nEventDataSize,nSysexSize;
	int nDelta,nDuration,nChannel,nNote,nFullChannel,nValue;
	uint32_t dwAt=0;
	unsigned char cOctaveOffset[4]={1,1,1,1};
	unsigned char cVolume[4]={100,100,100,100};
	unsigned short int wBank[4]={0,0,0,0};
	unsigned char cProgram[4]={0,0,0,0};
	unsigned int nNotes[4]={0,0,0,0};
	unsigned char theByte;
	int nRunState=-1;
	unsigned char *pcLimit=pcBuffer+nSize;

	while (pcBuffer < pcLimit)
	{
		nEventDataSize=0;
		if (bAwaitDelta)
		{
			nByteUsed=nDecodeVariableQuantity(&pcBuffer,pcLimit,&nDelta);
			TRACEIT2("delta: %d (stored in %d bytes)\n",nDelta,nByteUsed);
			dwAt+=nDelta*m_nResolution;
			if (nDelta*m_nResolution >= 5000)
			{
				TRACEIT2("huge delta!\n");
			}
		}
		theByte=*(pcBuffer++);
		switch(theByte)
		{
			case 0xFF:
				switch(*pcBuffer)
				{
					case 0x00:
						TRACEIT2("%08Xh - found mute all event\n",dwAt);
						pcBuffer+=1;
					break;
					case 0xF0:
						++pcBuffer;
						TRACEIT2("%08Xh - found sysex - size: %d!!!\n",dwAt,*(pcBuffer+1));
						nDecodeVariableLength(&pcBuffer,pcBuffer+4,&nSysexSize);
						hexdump("Sysex: ",pcBuffer,nSysexSize);
						TRACEIT2("%s\n", sParseSysEx((char *)pcBuffer,nSysexSize,false).c_str());
						pcBuffer+=nSysexSize;
					break;
					case 0xFF:
						TRACEIT2("%08Xh - found NOP\n",dwAt);
						pcBuffer+=1;
					break;
					default:
						TRACEIT2("%08Xh - found unknown status FF-event: %02Xh\n",dwAt,*pcBuffer);
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
						//TRACEIT("CSMAFTrack::DecodeMtsq - %08Xh - expression (short) value: %02Xh on channel %d\n",dwAt,nValue,nChannel);
						nEventDataSize=0;
					break;
					case 0x10:	//pitch bend (short)
						TRACEIT2("%08Xh - pitch bend (short) value: %02Xh on channel %d\n",dwAt,nValue,nFullChannel);
						nEventDataSize=0;
					break;
					case 0x20:	//modulation (short)
						TRACEIT2("%08Xh - modulation (short) value: %02Xh on channel %d\n",dwAt,nValue,nFullChannel);							
						nEventDataSize=0;
					break;
					case 0x30:
						switch(nValue)
						{
							case 0x00:
								nEventDataSize=1;
								TRACEIT("program change %d\n",nChannel);
								pEvent=new CSMAFEvent(	dwAt,
														nFullChannel,
														*pcBuffer & 0x3F,
														pcBuffer+1,
														nEventDataSize);
								TRACEIT2("%08Xh - program change: %02X on channel %d\n",dwAt,*(pcBuffer+1),nFullChannel);
								m_Events.push_back(pEvent);
								cProgram[nChannel]=*(pcBuffer+1);
							break;
							case 0x01:
								nEventDataSize=1;
								pEvent=new CSMAFEvent(	dwAt,
														nFullChannel,
														*pcBuffer & 0x3F,
														pcBuffer+1,
														nEventDataSize);
								TRACEIT2("%08Xh - bank change: %02X on channel %d\n",dwAt,*(pcBuffer+1),nFullChannel);
								m_Events.push_back(pEvent);
								wBank[nChannel]=(unsigned short int)*(pcBuffer+1);
							break;
							case 0x02:
								nEventDataSize=1;
								TRACEIT2("%08Xh - octave change on channel %d\n",dwAt,nFullChannel);
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
								TRACEIT2("%08Xh - modulation (normal) on channel %d\n",dwAt,nFullChannel);
								nEventDataSize=1;
							break;
							case 0x04:
								TRACEIT2("%08Xh - pitch bend (normal) on channel %d\n",dwAt,nFullChannel);
								nEventDataSize=1;
							break;
							case 0x07:
								TRACEIT2("%08Xh - volume on channel %d\n",dwAt,nFullChannel);
								nEventDataSize=1;
								cVolume[nChannel]=*(pcBuffer+1);
							break;
							case 0x09:
								TRACEIT2("%08Xh - ??? channel %d\n",dwAt,nFullChannel);
								nEventDataSize=1;
							break;
							case 0x0A:
								TRACEIT2("%08Xh - pan on channel %d\n",dwAt,nFullChannel);
								nEventDataSize=1;
							break;
							case 0x0B:
								TRACEIT2("%08Xh - expression (normal) on channel %d\n",dwAt,nFullChannel);
								nEventDataSize=1;
							break;
							default:
								TRACEIT2("%08Xh - found unknown double-byte event 00h,%02Xh on channel %d !!!\n",dwAt,*pcBuffer,nFullChannel);
								throw new CFormatException(CFormatException::formaterrUnknownSequenceFormat,"found unknown double-byte event");
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
				
				if (!m_bUsesSynthesizer)
				{
					TRACEIT2("USES SYNTHESIZER!\n");
					m_bUsesSynthesizer=true;
				}

				if (nDuration == 0)
				{
					TRACEIT2("zero size note!!!\n");
				}
				if (nDuration >= 5000)
				{
					TRACEIT2("long size note!!!\n");
				}
				++nNotes[nChannel];
				TRACEIT2("At: %08Xh, Channel: %d, Note Value: %d, Note-Duration: %d (b: %d, p: %d)\n",dwAt,nFullChannel,nNote,nDuration,wBank[nChannel],cProgram[nChannel]);
				pEvent=new CSMAFEvent(dwAt,nFullChannel,nNote,nDuration,cVolume[nChannel]);
				m_Events.push_back(pEvent);
		}
	};
}

/*\
 * <---------- CSMAFTrack :: DecodeMtsq_mobile ----------> 
 * @m decode sequence data - mobile format
 * --> I N <-- @p
 * unsigned char *pcBuffer - source sequence buffer
 * unsigned int nSize      - size of the sequence data in bytes
\*/
void CSMAFTrack::DecodeMtsq_mobile(unsigned char *pcBuffer,unsigned int nSize)
{
	CSMAFEvent *pEvent;
	bool bAwaitDelta=true;
	int nByteUsed,nSysexSize;
	int nDelta,nDuration,nChannel,nNote,nFullChannel;
	uint32_t dwAt=0;
	unsigned char cOctaveOffset[8]={1,1,1,1,1,1,1,1};
	unsigned char cVolume[8]={100,100,100,100,100,100,100,100};
	unsigned short int wBank[8]={0,0,0,0,0,0,0,0};
	unsigned char cProgram[8]={0,0,0,0,0,0,0,0};
	unsigned char theByte;
	int nRunState=-1;
	int nEventDataSize;
	unsigned char *pcLimit=pcBuffer+nSize;

	while (pcBuffer < pcLimit)
	{
		nEventDataSize=0;
		if (bAwaitDelta)
		{
			nByteUsed=nDecodeVariableQuantity(&pcBuffer,pcLimit,&nDelta);
			TRACEIT2("delta: %d (stored in %d bytes)\n",nDelta,nByteUsed);
			dwAt+=nDelta*m_nResolution;
			if (nDelta*m_nResolution >= 5000)
			{
				TRACEIT2("huge delta!\n");
			}
		}
		theByte=*(pcBuffer++);
		if ((theByte&0x80) == 0x080)
		{
			nChannel=theByte&0x07;
			nFullChannel=nChannel+m_nChannelOffset;
			/*
			if (nFullChannel == 9)
			{
				++m_nChannelOffset;
				nFullChannel=nChannel+m_nChannelOffset;
			}
			*/
			switch(theByte&0xF0)
			{
				//note on, no velocity
				case 0x80:
				{	
					nNote=(unsigned int)(cOctaveOffset[nChannel]&0x7F)*12;
					nNote+=*pcBuffer;
					pcBuffer++;
					nByteUsed=nDecodeVariableQuantity(&pcBuffer,pcLimit,&nDuration);
					nDuration=nDuration*m_nResolution;
					if (nDuration == 0)
					{
						TRACEIT2("zero size note!!!\n");
					}
					if (nDuration >= 5000)
					{
						TRACEIT2("long size note!!!\n");
					}
					TRACEIT2("%08Xh - Ch: %d, Note: %d, Length: %d (Bank: %d, Prg: %d)\n",dwAt,nFullChannel,nNote,nDuration,wBank[nChannel],cProgram[nChannel]);
					pEvent=new CSMAFEvent(dwAt,nFullChannel,nNote,nDuration,cVolume[nChannel]);
					m_Events.push_back(pEvent);
					nEventDataSize=0;
					if (wBank[nChannel] != 32000)
					{
						if (!m_bUsesSynthesizer)
						{
							TRACEIT2("USES SYNTHESIZER!\n");
							m_bUsesSynthesizer=true;
						}
					}
				}
				break;
				//note on, with velocity
				case 0x90:
				{
					int nVel;

					nNote=(unsigned int)(cOctaveOffset[nChannel]&0x7F)*12;
					nNote+=*pcBuffer;
					++pcBuffer;	
					nVel=*pcBuffer;
					++pcBuffer;	//skip velocity
					nByteUsed=nDecodeVariableQuantity(&pcBuffer,pcLimit,&nDuration);
					nDuration=nDuration*m_nResolution;
					if (nDuration == 0)
					{
						TRACEIT2("zero size note!!!\n");
					}
					if (nDuration >= 5000)
					{
						TRACEIT2("long size note!!!\n");
					}
					TRACEIT2("%08Xh - Ch: %d, Note: %d, Vel: %d, Length: %d (Bank: %d, Prg: %d)\n",dwAt,nFullChannel,nNote,nVel,nDuration,wBank[nChannel],cProgram[nChannel]);
					pEvent=new CSMAFEvent(dwAt,nFullChannel,nNote,nDuration,cVolume[nChannel]);
					m_Events.push_back(pEvent);
					nEventDataSize=0;
					if (wBank[nChannel] != 32000)
					{
						if (!m_bUsesSynthesizer)
						{
							TRACEIT2("USES SYNTHESIZER!\n");
							m_bUsesSynthesizer=true;
						}
					}
				}
				break;
				//control change
				case 0xB0:
					switch(*pcBuffer)
					{
						case 0x03:
							TRACEIT2("%08Xh - modulation %02Xh (normal) on channel %d\n",dwAt,*(pcBuffer+1),nFullChannel);
						break;
						case 0x04:
							TRACEIT2("%08Xh - pitch bend %02Xh (normal) on channel %d\n",dwAt,*(pcBuffer+1),nFullChannel);
						break;
						case 0x07:
							TRACEIT2("%08Xh - volume %02Xh on channel %d\n",dwAt,*(pcBuffer+1),nFullChannel);
							cVolume[nChannel]=*(pcBuffer+1);
						break;
						case 0x0A:
							TRACEIT2("%08Xh - pan %02Xh on channel %d\n",dwAt,*(pcBuffer+1),nFullChannel);
						break;
						case 0x0B:
							TRACEIT2("%08Xh - expression %02Xh (normal) on channel %d\n",dwAt,*(pcBuffer+1),nFullChannel);
						break;
						case 0x00:
							nEventDataSize=1;
							pEvent=new CSMAFEvent(	dwAt,
													nFullChannel,
													*pcBuffer,
													pcBuffer+1,
													nEventDataSize);
							TRACEIT2("%08Xh - bank change high %02Xh on channel %d\n",dwAt,*(pcBuffer+1),nChannel);
							m_Events.push_back(pEvent);
							wBank[nChannel]=((unsigned int)*(pcBuffer+1)) << 8;
						break;
						case 0x20:
							nEventDataSize=1;
							pEvent=new CSMAFEvent(	dwAt,
													nFullChannel,
													*pcBuffer,
													pcBuffer+1,
													nEventDataSize);
							TRACEIT2("%08Xh - bank change low %02Xh on channel %d\n",dwAt,*(pcBuffer+1),nChannel);
							m_Events.push_back(pEvent);
							wBank[nChannel]|=(unsigned int)*(pcBuffer+1);
						break;
						default:
							TRACEIT2("%08Xh - unknown control change %02Xh,%02Xh on channel %d\n",dwAt,*pcBuffer,*(pcBuffer+1),nChannel);
					}
					nEventDataSize=2;
				break;
				//program change
				case 0xC0:
					nEventDataSize=1;
					pEvent=new CSMAFEvent(	dwAt,
											nFullChannel,
											theByte,
											pcBuffer,
											nEventDataSize);
					TRACEIT2("%08Xh - program change: %02X on channel %d\n",dwAt,*pcBuffer,nFullChannel);
					m_Events.push_back(pEvent);
					cProgram[nChannel]=*pcBuffer;
				break;
				//pitch bend
				case 0xE0:
					TRACEIT2("%08Xh - pitch bend %02Xh,%02Xh on channel %d\n",dwAt,*pcBuffer,*(pcBuffer+1),nFullChannel);
					nEventDataSize=2;
				break;
				//sysex
				case 0xF0:
					if (theByte != 0xFF)
					{
						TRACEIT2("%08Xh - found sysex - size: %d  !!!\n",dwAt,*pcBuffer);
						nDecodeVariableLength(&pcBuffer,pcBuffer+4,&nSysexSize);
						//hexdump("Sysex: ",pcBuffer,nSysexSize);
						TRACEIT2("parsing...\n%s\n",sParseSysEx((char *)pcBuffer,nSysexSize,false).c_str());
						pcBuffer+=nSysexSize;
						nEventDataSize=0;
					}
					else
					{
						switch (*pcBuffer)
						{
							case 0x00:
								TRACEIT2("%08Xh - found NOP\n",dwAt);
								nEventDataSize=1;
							break;
							case 0x21:									//not sure if this is any good or correct?!
								TRACEIT2("%08Xh - found EOS\n",dwAt);
								nEventDataSize=2;
							break;
							case 0x2F:									//this looks more like if it was the EOS marker
								TRACEIT2("%08Xh - found EOS\n",dwAt);
								nEventDataSize=2;
							break;
							default:
								TRACEIT2("%08Xh - something %02X %d bytes long\n",dwAt,*pcBuffer,nEventDataSize);
						}
					}
				break;
			}
			pcBuffer+=nEventDataSize;
		}
	};
}

/*\
 * <---------- CSMAFTrack :: DecodeMtsu_handy ----------> 
 * @m decode sequence setup data - handyphone format
 * --> I N <-- @p
 * unsigned char *pcBuffer - 
 * unsigned int nSize      - 
\*/
void CSMAFTrack::DecodeMtsu_handy(unsigned char *pcBuffer,unsigned int nSize)
{
	unsigned char theByte;
	unsigned char *pcLimit=pcBuffer+nSize;
	uint32_t	dwAt=0;
	int nSysexSize;
	tstring sSysexDescription;

	while (pcBuffer < pcLimit)
	{
		theByte=*(pcBuffer++);
		switch(theByte)
		{
			case 0xFF:
				switch(*pcBuffer)
				{
					case 0xF0:
						++pcBuffer;
						nDecodeVariableLength(&pcBuffer,pcBuffer+16,&nSysexSize);
						Log2(verbLevDebug1, "parsing sysex...\n");
						sSysexDescription=sParseSysEx((char *)pcBuffer,nSysexSize,false);
						Log2(verbLevDebug1, "%s\n",sSysexDescription.c_str());
						pcBuffer+=nSysexSize;
						/*
						for (i=0;i < *pcBuffer;i++)
						{
							TRACEIT("%02Xh,",(unsigned char)*(pcBuffer+i));
						}
						TRACEIT("\n");
						*/
						/*
						pcBuffer+=*pcBuffer;
						ASSERT(pcBuffer < pcLimit);
						if (*pcBuffer != 0xF7)
						{
							TRACEIT2("found invalid sysex terminator: %02Xh\n",*pcBuffer);
							throw new CFormatException(CFormatException::formaterrInvalid,"invalid sysex terminator");
						}
						++pcBuffer;
						*/
					break;
					default:
						TRACEIT2("found unknown status FF-event: %02Xh\n",*pcBuffer);
						throw new CFormatException(CFormatException::formaterrInvalid,"unknown status FF-event");
				}
			break;
			default:
				TRACEIT2("found invalid status: %02Xh\n",theByte);
				throw new CFormatException(CFormatException::formaterrInvalid,"invalid status");
		}
	};
}

/*\
 * <---------- CSMAFTrack :: DecodeMtsu_mobile ----------> 
 * @m decode sequence setup data - mobile format
 * --> I N <-- @p
 * unsigned char *pcBuffer - 
 * unsigned int nSize      - 
\*/
void CSMAFTrack::DecodeMtsu_mobile(unsigned char *pcBuffer,unsigned int nSize)
{
	unsigned char *pcLimit=pcBuffer+nSize;
	int nSysexSize;
	uint32_t dwAt=0;

	hexdump("mtsu data: ",pcBuffer,nSize);
	while (pcBuffer < pcLimit)
	{
		switch(*pcBuffer)
		{
			case 0xF0:
				++pcBuffer;
				nDecodeVariableLength(&pcBuffer,pcBuffer+4,&nSysexSize);
				hexdump("sysex: ",pcBuffer,nSysexSize);
				TRACEIT2("parsing...\n%s\n",sParseSysEx((char *)pcBuffer,nSysexSize,true).c_str());
				pcBuffer+=nSysexSize;
			break;
			default:
				TRACEIT2("found unexpected setup-event: %02Xh\n",*pcBuffer);
				throw new CFormatException(CFormatException::formaterrInvalid);
		}
	};
	pcBuffer=pcLimit;
}

/*\
 * <---------- CSMAFTrack :: DecodeMthv ----------> 
 * @m decode human voice synth sequence
 * --> I N <-- @p
 * unsigned char *pcBuffer - 
 * unsigned int nSize      - 
\*/
void CSMAFTrack::DecodeMthv(unsigned char *pcBuffer,unsigned int nSize)
{
	map<tstring,int> :: const_iterator iterChunk;
	map<tstring,int> mapChunkId;

	mapChunkId[tstring(_T("Mhvs"))]=tcidMsp;
	mapChunkId[tstring(_T("MspI"))]=tcidMspI;
	mapChunkId[tstring(_T("Mtsq"))]=tcidMtsq;
	mapChunkId[tstring(_T("Mtsp"))]=tcidMtsp;
	mapChunkId[tstring(_T("Mtsu"))]=tcidMtsu;
	mapChunkId[tstring(_T("Mthv"))]=tcidMthv;
	mapChunkId[tstring(_T("MSTR"))]=tcidMSTR;

	while (nSize)
	{
		unsigned int nChunkSize;
		unsigned char cAttribute;
		tstring sChunk;
		unsigned char *pLim=pcBuffer+nSize;

		DecodeChunk(&pcBuffer,pLim,sChunk,&nChunkSize,&cAttribute);
		TRACEIT2("decoded subchunk: %s - size: %d\n",sChunk.c_str(),nChunkSize);

		pcBuffer+=nChunkSize;
		nSize-=nChunkSize+8;
	};
}

/*\
 * <---------- CSMAFTrack :: DecodeMtsp ----------> 
 * @m decode streaming pcm track data
 * --> I N <-- @p
 * unsigned char *pcBuffer - 
 * unsigned int nSize      - 
\*/
void CSMAFTrack::DecodeMtsp(unsigned char *pcBuffer,unsigned int nSize)
{
	while (nSize)
	{
		unsigned int nChunkSize;
		unsigned char cAttribute;
		tstring sChunk;
		unsigned char *pLim=pcBuffer+nSize;
		CSMAFSample *pSample;

		DecodeChunk(&pcBuffer,pLim,sChunk,&nChunkSize,&cAttribute);
		TRACEIT2("decoded subchunk: %s - size: %d\n",sChunk.c_str(),nChunkSize);

		if (sChunk == tstring(_T("Mwa")))
		{
			TRACEIT2("sample data - chunk attribute: %02Xh (wave id)\n",cAttribute);
			TRACEIT2("format type: %02X\n",*pcBuffer);
			TRACEIT2("sample rate: %02X\n",*(pcBuffer+1));
			TRACEIT2("sample rate: %02X\n",*(pcBuffer+2));
			int nSampleRate=8000;
			int nBitsPerSample=16;
			int nSampleSize=nChunkSize;
			int nChannels=(((*pcBuffer)&0x80)>>7)+1;
			int nFormat=(*pcBuffer)&0x70;

			switch(nFormat)
			{
				//pcm (signed)
				case 0x00:
					TRACEIT2("sample is PCM encoded !!!\n");
					throw new CFormatException(CFormatException::formaterrIncompatibleSampleFormat,"sample is PCM encoded");
				break;
				//pcm (offset)
				case 0x10:
					TRACEIT2("sample is PCM (offs) encoded !!!\n");
					throw new CFormatException(CFormatException::formaterrIncompatibleSampleFormat,"sample is PCM offs encoded");
				break;
				//adpcm
				case 0x20:
					TRACEIT2("sample is %d channel/s ADPCM encoded\n",nChannels);			
				break;
				default:
					TRACEIT2("sample uses invalid encoding scheme: %02X\n",(*pcBuffer)&0x70);
					throw new CFormatException(CFormatException::formaterrIncompatibleSampleFormat,"sample uses invalid encoding scheme");
			}
			nBitsPerSample=16;		//TODO: this surely isnt correct but we currently dont know how the sample width is described in this context
			
			nChunkSize-=1;
			pcBuffer+=1;
			nSampleRate=(unsigned int)(*pcBuffer & 0xFF)<<8;
			nChunkSize-=1;
			pcBuffer+=1;
			nSampleRate|=*pcBuffer;
			nChunkSize-=1;
			pcBuffer+=1;
			pSample=new CSMAFSample(nSampleRate,nChannels);
			/*
			char sFile[20];
			sprintf(sFile,"mtsp_adpcm_%d.raw",m_Samples.size());
			FILE *fp=fopen(sFile,"wb");
			fwrite(pcBuffer,1,nChunkSize,fp);
			fclose(fp);
			*/
#ifndef YAMAHA_NODECODE
			pSample->Decode(pcBuffer,nChunkSize);
#endif
			m_Samples.push_back(pSample);
			nSize-=3;
		}
		pcBuffer+=nChunkSize;
		nSize-=nChunkSize+8;
	};
}

/*\
 * <---------- CSMAFTrack :: Decode ----------> 
 * @m decode a SMAF track
 * --> I N <-- @p
 * unsigned char **pcBuffer - 
 * unsigned int nSize       - 
\*/
void CSMAFTrack::Decode(unsigned char **pcBuffer,unsigned int nSize)
{
	bool bDone=false;
	map<tstring,int> :: const_iterator iterChunk;
	map<tstring,int> mapChunkId;

	mapChunkId[tstring(_T("Msp"))]=tcidMsp;
	mapChunkId[tstring(_T("MspI"))]=tcidMspI;
	mapChunkId[tstring(_T("Mtsq"))]=tcidMtsq;
	mapChunkId[tstring(_T("Mtsp"))]=tcidMtsp;
	mapChunkId[tstring(_T("Mtsu"))]=tcidMtsu;
	mapChunkId[tstring(_T("Mthv"))]=tcidMthv;
	mapChunkId[tstring(_T("MSTR"))]=tcidMSTR;
	
/*
Format Type (required) 
Sequence Type (required) 
TimeBase_D (required) 
TimeBase_G (required) 
Channel Status (required) 
Seek & Phrase Info Chunk (optional) 
Set-up Data Chunk (optional) 
Sequence Data Chunk (required) 
Stream PCM Data Chunk (optional)
(only if Format Type= "Mobile Standard") 
*/
	m_nFormat=**pcBuffer;
	TRACEIT2("format type: %02X\n",m_nFormat);
	*pcBuffer+=1;
	nSize-=1;

	TRACEIT2("sequence type: %02X\n",**pcBuffer);
	*pcBuffer+=1;
	nSize-=1;

	TRACEIT2("timebase duration: %02X\n",**pcBuffer);
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
			TRACEIT2("timebase duration invalid value: %02X !!!!\n",**pcBuffer);
	}
	*pcBuffer+=1;
	nSize-=1;

	TRACEIT2("timebase gatetime: %02X\n",**pcBuffer);
	*pcBuffer+=1;
	nSize-=1;

	TRACEIT2("channel status raw #1: %02X\n",**pcBuffer);	
	*pcBuffer+=1;
	nSize-=1;

	TRACEIT2("channel status raw #2: %02X\n",**pcBuffer);
	*pcBuffer+=1;
	nSize-=1;

	switch (m_nFormat)
	{
		case formatHandyphone:
			TRACEIT2("format: handyphone\n");
		break;
		case formatMobileCompressed:
			TRACEIT2("format: mobile compressed - not supported\n");
			throw new CFormatException(CFormatException::formaterrUnknownTrackFormat,"mobile compressed - not supported");
		break;
		case formatMobileUncompressed:
			TRACEIT2("format: mobile uncompressed - skipping 14 bytes of junk\n");
			hexdump("junk: ",(unsigned char *)*pcBuffer,14);
			*pcBuffer+=14;
			nSize-=14;
		break;
		case formatMA7:
			TRACEIT2("format: mobile new - skipping 30 bytes of junk\n");
			hexdump("junk: ",(unsigned char *)*pcBuffer,30);
			*pcBuffer+=30;
			nSize-=30;
		break;
		default:		
			TRACEIT2("format: unknown (%02Xh) !!!\n",m_nFormat);
			throw new CFormatException(CFormatException::formaterrUnknownTrackFormat,"track format unknown");
	}
	while (nSize)
	{
		unsigned int nChunkSize;
		unsigned char cAttribute;
		tstring sChunk;
		unsigned char *pLim=*pcBuffer+nSize;

		DecodeChunk(pcBuffer,pLim,sChunk,&nChunkSize,&cAttribute);
		TRACEIT2("decoded subchunk: %s - size: %d\n",sChunk.c_str(),nChunkSize);
		if ((iterChunk=mapChunkId.find(sChunk)) == mapChunkId.end())
		{
			TRACEIT2("unknown sub chunk\n");
			throw new CFormatException(CFormatException::formaterrUnknownSubChunk,"unknown sub chunk");
		}
		switch(iterChunk->second)
		{
			case tcidMsp:
				TRACEIT2("weird Msp-chunk found !!!\n");				
				nSize-=nChunkSize+7;
			break;
			//instrument settings
			case tcidMspI:
				TRACEIT2("seek and phrase info\n");				
				nSize-=nChunkSize+8;
			break;
			//setup data
			case tcidMtsu:
				TRACEIT2("setup data\n");
				if (m_nFormat == formatHandyphone)
					DecodeMtsu_handy(*pcBuffer,nChunkSize);
				else
					DecodeMtsu_mobile(*pcBuffer,nChunkSize);
				nSize-=nChunkSize+8;
			break;
			//PCM sample data
			case tcidMtsp:
				TRACEIT2("streaming sample data\n");
				DecodeMtsp(*pcBuffer,nChunkSize);
				nSize-=nChunkSize+8;
			break;
			//master track
			case tcidMSTR:
				nSize-=nChunkSize+8;
			break;
			//sequence data
			case tcidMtsq:
				TRACEIT2("sequence data\n");
				if (m_nFormat == formatHandyphone)
					DecodeMtsq_handy(*pcBuffer,nChunkSize);
				else
					DecodeMtsq_mobile(*pcBuffer,nChunkSize);
				nSize-=nChunkSize+8;
			break;
			//human voice track
			case tcidMthv:
				m_bUsesHumanVoice=true;
				DecodeMthv(*pcBuffer,nChunkSize);
				nSize-=nChunkSize+8;
			break;
			default:
				nSize-=nChunkSize+8;
		}
		//TRACEIT("size:y %d \n",nSize);
		*pcBuffer+=nChunkSize;
	};
	TRACEIT("track done\n");
}

CSMAFSample *CSMAFTrack::pGetSample(int iIndex)
{
	CSMAFSample *pRet=NULL;
	ASSERT((unsigned int)iIndex < m_Samples.size());
	if ((unsigned int)iIndex < m_Samples.size())
		pRet=m_Samples[iIndex];
	return pRet;
}

uint32_t CSMAFTrack::nGetMaxSamplePlaytime()
{
	CSMAFSample *pSample;
	uint32_t i,nMax=0;
	for (i=0;i < m_Samples.size();i++)
	{
		pSample=m_Samples[i];
		if (pSample->nGetPlaytime() > nMax)
			nMax=pSample->nGetPlaytime();
	}
	return nMax;
}

uint32_t CSMAFTrack::nGetPlaytime(void)
{
	uint32_t nRet=0;
	ASSERT(nGetEventCount()); 
	if (nGetEventCount())
	{
		CSMAFEvent *pLast=m_Events[nGetEventCount()-1];
		ASSERT(pLast);
		if (pLast)
			nRet=pLast->dwGetAt()+pLast->nGetDuration();
	}
	return nRet;
}

int CSMAFTrack::nRenderSetupResetSysEx(	char *pcDest,	int nDeviceId)
{
	unsigned char pcsysxSuReset[1]={					0x7F		};
	return CSMAFTrack::nRenderSysEx(pcDest,	nDeviceId, (const char *)pcsysxSuReset, 1);
}

int CSMAFTrack::nRenderSetupChnReserveSysEx(	char *pcDest,	int nDeviceId, unsigned char cChannels)
{
	unsigned char pcsysxSuPCMChannelReserve[2]={		0x07, 0x01	};
	unsigned char pcsysxSuPCMChannelReserveMA7[2]={		0x22, 0x00	};
	unsigned char *pc=nDeviceId == MIDI_SYSXPID_MA7 ? pcsysxSuPCMChannelReserveMA7 : pcsysxSuPCMChannelReserve;
	//pc[1]=cChannels;
	return CSMAFTrack::nRenderSysEx(pcDest,	nDeviceId, (const char *)pc, 2);
}

int CSMAFTrack::nRenderMainVolumeSysEx(	char *pcDest,	int nDeviceId, unsigned char cVol)
{
	unsigned char pcsysxMainVolume[2]={			0x00, 0xFF				};
	unsigned char pcsysxMainVolumeMA7[2]={		0x20, 0xFF				};
	unsigned char *pc=nDeviceId == MIDI_SYSXPID_MA7 ? pcsysxMainVolumeMA7 : pcsysxMainVolume;
	pc[1]=cVol;
	return CSMAFTrack::nRenderSysEx(pcDest,	nDeviceId, (const char *)pc, 2);
}

void CSMAFTrack::CheckBankChanges()
{

	/*
	CSMAFEvent *pEvent;
	//printf ("checking track - %d events\n",nGetEventCount());

	for (int iEvent=0;iEvent < nGetEventCount();iEvent++)
	{
		pEvent=pGetEvent(iEvent);
		//printf ("%d - cmd: %d\n",pEvent->dwGetAt(),pEvent->cGetCommand());
		switch(pEvent->cGetCommand())
		{
			case 0x31:
				printf ("bank change at %d",pEvent->dwGetAt());
			break;
		}
	}
	*/
}
