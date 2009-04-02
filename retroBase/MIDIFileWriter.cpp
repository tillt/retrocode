/*\
 * MIDIFileWriter.cpp
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
#include "Basics.h"
#include <memory.h>
#ifdef WIN32
#include <tchar.h>
#include "Winsock2.h"
#else
#include <netinet/in.h>
#endif
#ifdef _AFX
#include "AfxSock.h"
#include "DMusic.h"
#endif
#include "MIDIFileWriter.h"
#include "Midispec.h"
#include "../include/Resource.h"

#ifdef _AFX
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
#endif

/*\
 *<------------ CMIDIFileWriter ------------>
 @m default constructor
 */
CMIDIFileWriter::CMIDIFileWriter()
{
	m_llLastTrackHeaderOffset=0;
	m_cRunningStatus=0;
	m_bIncludeMIP=true;
	m_bIncludeText=false;
	m_bPlaybackSave=false;
}

/*\
 *<------------ ~CMIDIFileWriter ------------>
 @m destructor
 */
CMIDIFileWriter::~CMIDIFileWriter()
{
}

/*\
 *<------------ WritePreamble ------------>
 @m write midi file header to stream
 *--> I N <-- @p
 * CArchive &ar - opened archive output stream
 *<-- OUT --> @r
 * int - CMIDIFile-status (ok,endOfStream)
 */
int CMIDIFileWriter::WritePreamble(ostream &ar)
{
	char		typebuf[8]={"MThd"};
	uint32_t	size;
	int16_t		fmt, tracks, div,i;
	tstring str;

    if (!WriteChunkType (ar,(char *)typebuf))		//
		return CMIDIFile::endOfStream;

	tracks=0;
	for (i=0;i < (int)m_Tracks.size();i++)
	{
		if (m_Tracks[i]->GetSize())
			++tracks;
	}

	size=6;
	TRACEIT2 ("size:%d - WritePreamble\n",size);
	size=htonl(size);
	ar.write((char *)&size,sizeof(int32_t));

	fmt=tracks > 1 ? 1 : 0;
	TRACEIT2 ("format-level:%d - WritePreamble\n",fmt);
	fmt=htons(fmt);
	ar.write((char *)&fmt,sizeof(int16_t));
    
	TRACEIT2 ("tracks:%d - WritePreamble\n",tracks);
	tracks=htons(tracks);
	ar.write((char *)&tracks,sizeof(int16_t));

	div=m_nDivision;
	TRACEIT2 ("div:%d - WritePreamble\n",div);
	div=htons(div);
	ar.write((char *)&div,sizeof(int16_t));

    return ok;
}

/*\
 *<------------ WriteChunkType ------------>
 @m write midi chunk (4 bytes)
 *--> I N <-- @p
 * CArchive &ar - opened archive output stream
 * BYTE *pBuffer - pointer to chunk data
 *<-- OUT --> @r
 * BOOL - TRUE=done
 */
BOOL CMIDIFileWriter::WriteChunkType (ostream &ar,char *pBuffer)
{
    ar.write((const char *)pBuffer,4);
    *(pBuffer+4) = '\0';
    return TRUE;
}

/*\
 *<------------ WriteTrackHeader ------------>
 @m write track header data
 *--> I N <-- @p
 * CArchive &ar - opened archive output stream
 *<-- OUT --> @r
 * int - CMIDIFile-status (ok,endOfStream)
 */
int CMIDIFileWriter::WriteTrackHeader (ostream &ar,int iTrack)
{
	char typebuf[8]={"MTrk"};
    int32_t size;
    double n;

	// tempo in file is in micro-seconds per quarter note
    n = 60000000.0/(double)DEFAULTTEMPO;      //
    // division is the number of delta time "ticks" that make up a
    // quarter note. Quanta size is in micro seconds.
    m_fTimeScale = n / (double)(m_nDivision * m_nQuantaSize);

    TRACEIT2("writing Track header %d - writeTrackHeader\n",iTrack);

    m_nCurrentTime=0;
	m_cRunningStatus=0;

    if (!WriteChunkType (ar,(char *)typebuf))
    {
		TRACEIT2("failed when writing chunk !\n");
        return CMIDIFile::endOfStream;
	}
	ar.flush();
	m_llLastTrackHeaderOffset=ar.tellp();
	size=0xFFFF;
	size=htonl(size);
	ar.write((char *)&size,sizeof(int32_t));

    return CMIDIFile::ok;
}

/*\
 *<------------ WriteEndOfTrack ------------>
 @m write end of track mark to file
 *--> I N <-- @p
 * CArchive &ar - stream
 *<-- OUT --> @r
 * int - CMIDIFile-status (ok,endOfStream)
 */
int CMIDIFileWriter::WriteEndOfTrack(ostream &ar)
{
	int64_t llOldPos=ar.tellp();
	int32_t nSize=(int32_t )((llOldPos+4)-(m_llLastTrackHeaderOffset+4));
	unsigned char pcEndOfTrack[]={0x00,0xff,TRACKCHANGE,0x00};
	
	//seek to track header
	ar.seekp((unsigned long)m_llLastTrackHeaderOffset);
	nSize=htonl(nSize);
	//write size of track
	ar.write((const char *)&nSize,sizeof(int32_t));
	//seek back to end of track
	ar.seekp((unsigned long)llOldPos);
	//write end marker
	ar.write((const char *)pcEndOfTrack,sizeof(int32_t));

	return ok;
}

/*\
 *<------------ GetFirstEvent ------------>
 @m get first song event
 *--> I N <-- @p
 * CMIDIEvent **pOut - pointer to pointer to first event
 *<-- OUT --> @r
 * BOOL - TRUE=done
 */
bool CMIDIFileWriter::GetFirstEvent (CMIDIEvent **pOut)
{
	ZeroMemory(&m_iTrackPos,sizeof(int)*64);
	m_nCurrentTime=0;
	return GetNextEvent(pOut);
}

bool CMIDIFileWriter::GetFirstEvent (CMIDIEvent **pOut,int iChannel)
{
	//ZeroMemory(&m_iTrackPos,sizeof(int)*64);
	m_iTrackPos[iChannel]=0;
	m_nCurrentTime=0;
	return GetNextEvent(pOut,iChannel);
}


/*\
 *<------------ GetNextEvent ------------>
 @m get next song event
 *--> I N <-- @p
 * CMIDIEvent **pOut - pointer to pointer to next event
 *<-- OUT --> @r
 * BOOL - TRUE=done
 */
bool CMIDIFileWriter::GetNextEvent (CMIDIEvent **pOut)
{
	int i;
	bool bRet=false;
	ASSERT (pOut);
	uint32_t nFirst=0xFFFFFFFF;
	int iWinner=0;

	*pOut=NULL;
	for (i=0;i < (int)m_Tracks.size();i++)
	{
		CMIDITrack *pTrack=m_Tracks[i];
		if (m_iTrackPos[i] < pTrack->GetSize())
		{
			CMIDIEvent *pEvent=pTrack->GetEvent(m_iTrackPos[i]);
			ASSERT (pEvent);
			if (pEvent->GetAt() < nFirst)
			{
				iWinner=i;
				nFirst=pEvent->GetAt();
				*pOut=pEvent;
				bRet=true;
			}
		}
	}
	if (*pOut)
		m_iTrackPos[iWinner]++;

	return bRet;
}

bool CMIDIFileWriter::GetNextEvent (CMIDIEvent **pOut,int iChannel)
{
	bool bRet=false;
	ASSERT (pOut);
	uint32_t nFirst=0xFFFFFFFF;

	*pOut=NULL;
	CMIDITrack *pTrack=m_Tracks[iChannel];
	if (m_iTrackPos[iChannel] < pTrack->GetSize())
	{
		CMIDIEvent *pEvent=pTrack->GetEvent(m_iTrackPos[iChannel]);
		ASSERT (pEvent);
		if (pEvent->GetAt() < nFirst)
		{
			++m_iTrackPos[iChannel];
			nFirst=pEvent->GetAt();
			*pOut=pEvent;
			bRet=true;
		}
	}
	return bRet;
}


//#define SCALEQUANTA(quanta) ((int)(0.5+(m_fTimeScale * (double)quanta)))
/*
b = ?
c = 0.5 + a * b
c-0.5 = a*b
c-0.5
------  = b
a
*/

/*\
 *<------------ WriteEvent ------------>
 @m write one event to the output stream
 *--> I N <-- @p
 * CArchive &ar - reference of an opened archive output stream
 * CMIDIEvent *pOut - pointer to event
 *<-- OUT --> @r
 * int - MIDIFile-status (ok,endOfStream)
 */
 int CMIDIFileWriter::WriteEvent (ostream &ar,CMIDIEvent *pOut)
{
	unsigned int deltaTime,quantaTime;

	ASSERT (pOut);
    quantaTime = DESCALEQUANTA(pOut->GetAt());
	deltaTime=quantaTime-m_nCurrentTime;
/*
	Metaevent	IncludeText		IsTempo		Result
	1			1				1			1
	1			0				1			1
	0			1				1			1
	0			0				1			1
	1			1				0			1
	1			0				0			0
	0			1				0			1
	0			0				0			1

	0			1							1
	0			0							0
	1			1							1
	1			0							1
*/
	if (((!pOut->IsMetaevent()) | m_bIncludeText) | (pOut->GetData()[0] == MIDI_TEMPO))
	{
		WriteVariableQuantity (ar,deltaTime);
		switch (pOut->GetData()[0])
		{
			/*
			case MIDI_TEMPO:
			{
				ASSERT (pOut->GetSize());
				unsigned char pcTempoChange[]={0xFF,TEMPOCHANGE,0};
				pcTempoChange[2]=pOut->GetSize()-1;
				pFile->Write(pcTempoChange,3);
				pFile->Write(pOut->GetData()+1,pOut->GetSize()-1);
				m_cRunningStatus=0;
			}
			break;
			*/
			case MIDI_SYSEXCL:
			{
			    TRACEIT("writing sysex data - WriteEvent\n");
				unsigned char cSize=pOut->GetSize()-1;
				ar.write((const char *)pOut->GetData(),1);
				ar.write((const char *)&cSize,1);
				ar.write((const char *)pOut->GetData()+1,pOut->GetSize()-1);
			}
			break;
			default:
			{
				if (!pOut->IsMetaevent())
				{
					if (pOut->GetData()[0] == m_cRunningStatus)
						ar.write((const char *)pOut->GetData()+1,pOut->GetSize()-1);
					else
					{
						ar.write((const char *)pOut->GetData(),pOut->GetSize());
						m_cRunningStatus=pOut->GetData()[0];
					}
					m_cRunningStatus=pOut->GetData()[0];
				}
				else
				{
				    TRACEIT2("writing meta event - WriteEvent\n");
					ASSERT (pOut->GetSize());
					unsigned char pcMeta[3]={0xFF};
					if (pOut->GetData()[0] == MIDI_TEMPO)
						pcMeta[1]=TEMPOCHANGE;
					else
						pcMeta[1]=pOut->GetData()[0];
					pcMeta[2]=pOut->GetSize()-1;
					ar.write((const char *)pcMeta,3);
					ar.write((const char *)pOut->GetData()+1,pOut->GetSize()-1);
					m_cRunningStatus=0;
				}
			}
		}

		m_nCurrentTime=quantaTime;
	}
	return ok;
}

/*\
 *<------------ WriteVariableQuantity ------------>
 @m write a value of variable size to the output stream
 *--> I N <-- @p
 * CArchive &ar - reference of an opened archive output stream
 * int value - value to write
 *<-- OUT --> @r
 * int - CMIDIFile-status (ok,endOfStream)
 */
int CMIDIFileWriter::WriteVariableQuantity (ostream &ar,int value)
{
	unsigned char temp;
	if ((unsigned int)value > 0x1FFFFFF)
	{
		temp=(((value>>21)&0x7F)|0x80);
		ar.write((const char *)&temp,1);
		temp=(((value>>14)&0x7F)|0x80);
		ar.write((const char *)&temp,1);
		temp=(((value>>7)&0x7F)|0x80);
		ar.write((const char *)&temp,1);
	}
	else if ((unsigned int)value > 0x3FFF)
	{
		temp=(((value>>14)&0x7F)|0x80);
		ar.write((const char *)&temp,1);
		temp=(((value>>7)&0x7F)|0x80);
		ar.write((const char *)&temp,1);
	}
	else if ((unsigned int)value > 0x7F)
	{
		temp=(((value>>7)&0x7F)|0x80);
		ar.write((const char *)&temp,1);
	}
	temp=value&0x7F;
	ar.write((const char *)&temp,1);
	return ok;
}

/*\
 *<------------ WriteEnding ------------>
 @m write midi postfix to stream
 *--> I N <-- @p
 * CArchive &ar - reference of an opened archive output stream
 *<-- OUT --> @r
 * int - CMIDIFile-status (ok)
 */
int CMIDIFileWriter::WriteEnding(ostream &ar)
{
	return ok;
}

/*\
 *<------------ SetEnd ------------>
 @m define song end time
 *--> I N <-- @p
 * UINT nEnd - end time value
 *<-- OUT --> @r
 * BOOL - TRUE=ok
 */
BOOL CMIDIFileWriter::SetEnd(uint32_t nEnd)
{
	m_nLastOutTime=nEnd;
	return TRUE;
}

/*\
 *<------------ SetTiming ------------>
 @m set midi timings
 *--> I N <-- @p
 * int nDivisor - divisor value
 * int nQuantaSize - quanta size value
 */
void CMIDIFileWriter::SetTiming(int nDivisor, int nQuantaSize)
{
	m_nDivision=nDivisor;
	m_nQuantaSize=nQuantaSize;
	m_fTimeScale = 60000000.0 / (double)(m_nDivision * m_nQuantaSize);
}

int CMIDIFileWriter::Save (const char *szFile)
{
	int iError=ok;
	ofstream ar;
	try
	{  
		ar.open(szFile,ios::binary);
		ar.exceptions ( ofstream::eofbit | ofstream::failbit | ofstream::badbit );
		iError=Save(ar);
		ar.close();
	}
	catch (ofstream::failure const &e)
	{
		Log2(verbLevErrors,"File Access Exception\n\t%s\n",e.what());
		iError=CMIDIFile::fileCorrupt;
	}
	return iError;
}

int CMIDIFileWriter::Save(ostream &ar,int iChannel)
{
	bool bPlayChannel[16];
	ZeroMemory(bPlayChannel,16);
	if (iChannel >= 0)
		bPlayChannel[iChannel]=true;
	else
	{
		int i;
		for (i=0;i < 16;i++)
			bPlayChannel[i]=true;
	}
	return Save(ar,bPlayChannel);
}

int CMIDIFileWriter::Save(ostream &ar,bool bPlayChannel[])
{
	CMIDIEvent *pEvent;
	int iError=CMIDIFile::ok;

	if ((iError=WritePreamble (ar)) != CMIDIFile::ok)
	{
		TRACEIT2("writing preamble failed\n");
	}
	else
	{
		int i;
		bool bVibraWritten=false;
		bool bDo;
		for (i=0;i < (int)m_Tracks.size();i++)
		{
			if (m_Tracks[i]->GetSize())
			{
				if ((iError=WriteTrackHeader(ar,i)) != CMIDIFile::ok)
				{
					TRACEIT2 ("writing track header failed\n");
					break;
				}
				else
				{
					for (bDo=CMIDIFileWriter::GetFirstEvent(&pEvent,i);bDo;bDo=CMIDIFileWriter::GetNextEvent(&pEvent,i))
					{
						//bool bIsControl=false;
						bool bWrite=true;
						unsigned char theByte=pEvent->GetData()[0];
						switch (theByte & 0xF0)
						{
							case MIDI_SYSEXCL:
								bWrite=m_bIncludeMIP;
							break;
							case MIDI_NOTEOFF:
								//patch note-offs to pseudo note-offs to reduce the file-size
								ASSERT(pEvent->GetSize() > 2);
								pEvent->GetData()[0]=(theByte&0x0F)|MIDI_NOTEON;
								pEvent->GetData()[2]=0x00; 
							case MIDI_CONTROL:
							case MIDI_PROGRAM:
							case MIDI_NOTEON:
							case MIDI_POLYPRES:
							case MIDI_CHANPRES:
							case MIDI_PITCH:
								if ((bWrite=IsChannelPlaybackActive(theByte&0x0F)))
									bWrite=bPlayChannel[theByte&0x0F];
							break;
						}
						if (pEvent->IsMetaevent())
						{
							/*
							if (theApp.GetEventstripping(CMIDIscaleApp::STRIPPING_TEXT))
							{
							
							}
							*/
						}
		
						if (bWrite)
						{
#ifdef MIDISCALE
							if ((theByte&0xF0) == MIDI_PROGRAM)
								pEvent->GetData()[1]=GetMappedInstrument(pEvent->GetData()[1],CDMusic::GetInstrumentMap());
#endif
							switch(theByte&0xF0)
							{
								case MIDI_NOTEON:
								case MIDI_NOTEOFF:
								case MIDI_CONTROL:
								case MIDI_PROGRAM:
								case MIDI_POLYPRES:
								case MIDI_CHANPRES:
								case MIDI_PITCH:
									if ((theByte&0x0F) == m_iVibraChannel && theByte != MIDI_SYSEXCL)
									{
										if (!bVibraWritten)
										{
											CMIDIEvent *pVibraEvents[10];
											int nVibraEventCount=0,iEventIndex;
											GenerateVibra(pEvent->GetAt(),m_iVibraChannel,pVibraEvents,&nVibraEventCount);
											for (iEventIndex=0;iEventIndex < nVibraEventCount;iEventIndex++)
											{
												if ((iError=WriteEvent (ar,pVibraEvents[iEventIndex])) != CMIDIFile::ok)
													break;
												delete pVibraEvents[iEventIndex];
											}
											bVibraWritten=true;
										}
									}
								break;
							}
							if (m_iVibraChannel != -1 && (theByte&0x0F) == m_iVibraChannel)
							{
								switch(theByte & 0xF0)
								{
									case MIDI_CONTROL:
										//bank change MSB ?
										if (pEvent->GetData()[1] == 0x00)
											bWrite=false;
										//bank change LSB ?
										if (pEvent->GetData()[1] == 0x20)
											bWrite=false;
										//volume change ?
										if (pEvent->GetData()[1] == 0x07)
											bWrite=false;
									break;
									case MIDI_PROGRAM:
										bWrite=false;
									break;
								}
							}

							if (m_bPlaybackSave)
							{
								if (m_bAdditionalPercussion && ((theByte&0x0F) == 0x0A))
								{
									pEvent->GetData()[0]=(theByte&0xF0)|0x09;
									TRACEIT2("patched percussion event: %02X\n",pEvent->GetData()[0]);
								}
							}
						}
						if (bWrite)
						{
							if ((iError=WriteEvent (ar,pEvent)) != CMIDIFile::ok)
								break;
						}
					}
					if ((iError=WriteEndOfTrack(ar)) != CMIDIFile::ok)
					{
						TRACEIT2("writing end of track failed\n");
						break;
					}
				}
			}
		}
		WriteEnding (ar);
	}
	return iError;
}

void CMIDIFileWriter::SetIncludeText(bool bEnable)
{
	m_bIncludeText=bEnable;
}

void CMIDIFileWriter::SetIncludeMIP(bool bEnable)
{
	m_bIncludeMIP=bEnable;
}
