/*\
 * SMAFConverter.cpp
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
#include "../retroBase/Integer.h"
#include "../retroBase/MyString.h"
#include "../retroBase/MobileContent.h"
#include "SMAFConverter.h"
#ifdef SMAFUseConverter
#include "MIDIFileDoc.h"
#include "MIDISpec.h"
#endif
#include "SMAFDecoder.h"
#include "SMAFFile.h"
#include "SMAFTrack.h"
#include "SMAFEvent.h"

CSMAFConverter::CSMAFConverter()
{
}

CSMAFConverter::~CSMAFConverter()
{
}

#ifdef SMAFUseConverter
//converts a smaf sequence into a midi sequence
void CSMAFConverter::Convert(CMIDIFileDoc *pMIDI,CSMAFFile *pSMAF)
{
	ReadStruct rs;
	BOOL bFailed=FALSE;
	int nGanzeNote=2004,nChannel;
	int iScale,iNote,iTrack,iEvent;
	//UBYTE *pData;
	int nNote,nLength;
	CString str;
	CString strName;
	int nQuanta=0;
	int iStyle=0,i;
	int temp;
	CMIDITrack *pMidiTrack;
	CSMAFTrack *pSmafTrack;

	bool bDrumChannel[16];
	int nDrumNote[16];

	for (i=1;i < 16;i++)
	{
		pMidiTrack=new CMIDITrack;
		pMIDI->m_Tracks.Add(pMidiTrack);
	}

	ASSERT (pSMAF);
	ASSERT (pMIDI);
	if (pSMAF == NULL || pMIDI == NULL)
		return;

	rs.data=new unsigned char [1024];

	pMidiTrack=pMIDI->GetTracks()->GetAt(0);
	temp=60000000/120;
	rs.bMetaEvent=TRUE;
	rs.quanta=0;
	rs.nData=4;
	rs.data[0]=MIDI_TEMPO;
	rs.data[1]=(temp>>16)&0xFF;
	rs.data[2]=(temp>>8)&0xFF;
	rs.data[3]=temp&0xFF;
	pMidiTrack->AddEvent(&rs);
	pMIDI->SetTempo(temp);

    pMIDI->SetTiming(0x1e0,DEFAULTQUANTASIZE);
	pMIDI->SetChannelUsed(0,TRUE);
	//pData=m_pData;

	for (iTrack=0;iTrack  < pSMAF->nGetTrackCount();iTrack++)
	{
		nQuanta=0;
		pSmafTrack=pSMAF->pGetTrack(iTrack);
		ASSERT (pSmafTrack);
		memset(bDrumChannel,0,16*sizeof(bool));
		memset(nDrumNote,0,16*sizeof(int));
		for (iEvent=0;iEvent < pSmafTrack->nGetEventCount();iEvent++)
		{
			CSMAFEvent *pEvent=pSmafTrack->pGetEvent(iEvent);
			ASSERT(pEvent);
			if (pEvent->bIsControlEvent())
			{
				switch(pEvent->cGetCommand())
				{
					case 0x31:
						bDrumChannel[pEvent->nGetChannel()]=pEvent->pcGetData()[0] == 0x80;
					break;
					case 0x30:
						if (!bDrumChannel[pEvent->nGetChannel()])
						{
							ZeroMemory(rs.data,256);
							rs.bMetaEvent=FALSE;
							rs.data[0]=MIDI_PROGRAM|pEvent->nGetChannel();
							rs.data[1]=pEvent->pcGetData()[0];
							rs.nData=2;
							rs.quanta=pEvent->dwGetAt();
							pMIDI->m_Tracks[pEvent->nGetChannel()]->AddEvent(&rs);
						}
						else
							nDrumNote[pEvent->nGetChannel()]=pEvent->pcGetData()[0];
					break;
				}
			}
			else
			{
				nChannel=pEvent->nGetChannel();
				nNote=pEvent->nGetNote();
				if (bDrumChannel[pEvent->nGetChannel()])
				{
					nChannel=9;
					nNote=nDrumNote[pEvent->nGetChannel()];
				}
				//create note on
				ZeroMemory(rs.data,256);
				rs.bMetaEvent=FALSE;
				rs.data[0]=MIDI_NOTEON|nChannel;
				rs.data[1]=nNote;
				rs.data[2]=pEvent->nGetVolume();
				rs.nData=3;
				rs.quanta=pEvent->dwGetAt();
				pMIDI->m_Tracks[nChannel]->AddEvent(&rs);

				//create note off
				ZeroMemory(rs.data,256);
				rs.bMetaEvent=FALSE;
				rs.data[0]=MIDI_NOTEON|nChannel;
				rs.data[1]=nNote;
				rs.data[2]=0;
				rs.nData=3;
				rs.quanta=pEvent->dwGetAt()+pEvent->nGetDuration();
				pMIDI->m_Tracks[nChannel]->AddEvent(&rs);
			}
		}
	}
	pMIDI->SetEnd(nQuanta);
}
#endif
