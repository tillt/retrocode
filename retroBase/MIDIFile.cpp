/*\
 * MIDIFile.cpp
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
#include "Integer.h"
#include "Basics.h"
#include "MyString.h"
#ifdef WIN32
#include <tchar.h>
#include "Winsock2.h"
#else
#include <netinet/in.h>
#endif
#include <iostream>
#include <fstream>
#include <memory.h>
#include "MIDIFile.h"
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
 *<------------ CMIDIEvent ------------>
 @m default constructor
 */
CMIDIEvent::CMIDIEvent()
{
	m_bMetaevent=false;
	m_pcEvent=NULL;
	m_nAt=0xFFFFFFFF;
	m_nSize=0;
}

/*\
 *<------------ CMIDIEvent ------------>
 @m alternative constructor
 *--> I N <-- @p
 * int nAt - event time
 * UBYTE *pcEvent - pointer to event data
 * int nSize - size of event data
 */
CMIDIEvent::CMIDIEvent(int nAt,unsigned char *pcEvent,int nSize,bool bMeta)
{
	//TRACEIT ("CMIDIEvent - at:%d data[0]:%02X size:%d\n",nAt,*pcEvent,nSize);
	m_nAt=nAt;
	ASSERT (nSize && nSize < 0xFFFF);
	ASSERT (pcEvent);
	if (nSize && pcEvent)
	{
		m_nSize=nSize;
		m_pcEvent=new unsigned char[nSize];
		m_bMetaevent=bMeta;
		ASSERT (m_pcEvent);
		CopyMemory(m_pcEvent,pcEvent,nSize);
	}
	else
		TRACEIT ("CMIDIEvent - no event created\n");
}

/*\
 *<------------ CMIDIEvent ------------>
 @m copy constructor
 *--> I N <-- @p
 * CMIDIEvent &src - reference of source event object
 */
CMIDIEvent::CMIDIEvent(CMIDIEvent &src)
{
	m_nAt=src.m_nAt;
	m_nSize=src.m_nSize;
	ASSERT (src.m_pcEvent);
	ASSERT (src.m_nSize);
	if (src.m_nSize && src.m_pcEvent)
	{
		m_nSize=src.m_nSize;
		ASSERT (m_nSize && m_nSize < 0xFFFF);
		m_pcEvent=new unsigned char[m_nSize];
		ASSERT (m_pcEvent);
		CopyMemory (m_pcEvent,src.m_pcEvent,m_nSize);
		m_bMetaevent=src.m_bMetaevent;
	}
}

/*\
 *<------------ ~CMIDIEvent ------------>
 @m destructor
 */
CMIDIEvent::~CMIDIEvent()
{
	ASSERT (m_pcEvent);
	if(m_pcEvent && m_nSize)
	{
		delete [] m_pcEvent;
	}
}

/*\
 *<------------ CMIDITrack ------------>
 @m default constructor
 */
CMIDITrack::CMIDITrack()
{
	m_strName.erase();
	m_nLastQuanta=0;
	//m_Events.RemoveAll();
	if (m_Events.size())
		m_Events.erase(m_Events.begin(), m_Events.end());
}

/*\
 *<------------ CMIDITrack ------------>
 @m copy constructor
 *--> I N <-- @p
 * CMIDITrack &src - reference of source track
 */
CMIDITrack::CMIDITrack(CMIDITrack &src)
{
	unsigned int i;
	CMIDIEvent *pEvent;
	m_strName=src.m_strName;
	if (m_Events.size())
		m_Events.erase(m_Events.begin(), m_Events.end());
	for (i=0;i < m_Events.size();i++)
	{
		pEvent=new CMIDIEvent(*src.m_Events[i]);
		m_Events.push_back(pEvent);
	}
}

/*\
 *<------------ ~CMIDITrack ------------>
 @m destructor
 */
CMIDITrack::~CMIDITrack()
{
	Reset();
}

/*\
 *<------------ CMIDIFile ------------>
 @m default constructor
 */
CMIDIFile::CMIDIFile()
{
	Init();
}

void CMIDIFile::Reset(void)
{
	unsigned int i;
	for (i=0;i < m_Tracks.size();i++)
	{
		ASSERT (m_Tracks[i]);
		if (m_Tracks[i] != NULL)
			delete m_Tracks[i];
	}
	if (m_Tracks.size())
		m_Tracks.erase(m_Tracks.begin(), m_Tracks.end());

	if (m_ReadInfo.data)
	{	
		delete [] m_ReadInfo.data;
		m_ReadInfo.data=NULL;
	}

	for (i=0;i < m_listPatches.size();i++)
	{
		ASSERT (m_listPatches[i]);
		if (m_listPatches[i] != NULL)
			delete m_listPatches[i];
	}
	if (m_listPatches.size())
		m_listPatches.erase(m_listPatches.begin(), m_listPatches.end());
}


uint32_t CMIDIFile::GetPlaytime(void)
{
	return (uint32_t)((m_nLastOutTime * 120) / (((double)60000000.0/m_nTempo)));
}

tstring CMIDIFile::GetInstrumentName(int nBank,int iInstrument,int iChannel)
{
	int i;
	tstring strName;
	const InstrumentMap *pMap;
	const InstrumentMap listGMMelodicNames[]=
	{
		{0,"Acoustic Grand"},
		{1,"Bright Acoustic"},
		{2,"Electric Grand"},
		{3,"Honky-Tonk"},
		{4,"Electric Piano 1"},
		{5,"Electric Piano 2"},
		{6,"Harpsichord"},
		{7,"Clav"},
		{8,"Celesta"},
		{9,"Glockenspiel"},
		{10,"Music Box"},
		{11,"Vibraphone"},
		{12,"Marimba"},
		{13,"Xylophone"},
		{14,"Tubular Bells"},
		{15,"Dulcimer"},
		{16,"Drawbar Organ"},
		{17,"Percussive Organ"},
		{18,"Rock Organ"},
		{19,"Church Organ"},
		{20,"Reed Organ"},
		{21,"Accoridan"},
		{22,"Harmonica"},
		{23,"Tango Accordian"},
		{24,"Acoustic Guitar(nylon)"},
		{25,"Acoustic Guitar(steel)"},
		{26,"Electric Guitar(jazz)"},
		{27,"Electric Guitar(clean)"},
		{28,"Electric Guitar(muted)"},
		{29,"Overdriven Guitar"},
		{30,"Distortion Guitar"},
		{31,"Guitar Harmonics"},
		{32,"Acoustic Bass"},
		{33,"Electric Bass(finger)"},
		{34,"Electric Bass(pick)"},
		{35,"Fretless Bass"},
		{36,"Slap Bass 1"},
		{37,"Slap Bass 2"},
		{38,"Synth Bass 1"},
		{39,"Synth Bass 2"},
		{40,"Violin"},
		{41,"Viola"},
		{42,"Cello"},
		{43,"Contrabass"},
		{44,"Tremolo Strings"},
		{45,"Pizzicato Strings"},
		{46,"Orchestral Strings"},
		{47,"Timpani"},
		{48,"String Ensemble 1"},
		{49,"String Ensemble 2"},
		{50,"SynthStrings 1"},
		{51,"SynthStrings 2"},
		{52,"Choir Aahs"},
		{53,"Voice Oohs"},
		{54,"Synth Voice"},
		{55,"Orchestra Hit"},
		{56,"Trumpet"},
		{57,"Trombone"},
		{58,"Tuba"},
		{59,"Muted Trumpet"},
		{60,"French Horn"},
		{61,"Brass Section"},
		{62,"SynthBrass 1"},
		{63,"SynthBrass 2"},
		{64,"Soprano Sax"},
		{65,"Alto Sax"},
		{66,"Tenor Sax"},
		{67,"Baritone Sax"},
		{68,"Oboe"},
		{69,"English Horn"},
		{70,"Bassoon"},
		{71,"Clarinet"},
		{72,"Piccolo"},
		{73,"Flute"},
		{74,"Recorder"},
		{75,"Pan Flute"},
		{76,"Blown Bottle"},
		{77,"Skakuhachi"},
		{78,"Whistle"},
		{79,"Ocarina"},
		{80,"Lead 1 (square)"},
		{81,"Lead 2 (sawtooth)"},
		{82,"Lead 3 (calliope)"},
		{83,"Lead 4 (chiff)"},
		{84,"Lead 5 (charang)"},
		{85,"Lead 6 (voice)"},
		{86,"Lead 7 (fifths)"},
		{87,"Lead 8 (bass+lead)"},
		{88,"Pad 1 (new age)"},
		{89,"Pad 2 (warm)"},
		{90,"Pad 3 (polysynth)"},
		{91,"Pad 4 (choir)"},
		{92,"Pad 5 (bowed)"},
		{93,"Pad 6 (metallic)"},
		{94,"Pad 7 (halo)"},
		{95,"Pad 8 (sweep)"},
		{96,"FX 1 (rain)"},
		{97,"FX 2 (soundtrack)"},
		{98,"FX 3 (crystal)"},
		{99,"FX 4 (atmosphere)"},
		{100,"FX 5 (brightness)"},
		{101,"FX 6 (goblins)"},
		{102,"FX 7 (echoes)"},
		{103,"FX 8 (sci-fi)"},
		{104,"Sitar"},
		{105,"Banjo"},
		{106,"Shamisen"},
		{107,"Koto"},
		{108,"Kalimba"},
		{109,"Bagpipe"},
		{110,"Fiddle"},
		{111,"Shanai"},
		{112,"Tinkle Bell"},
		{113,"Agogo"},
		{114,"Steel Drums"},
		{115,"Woodblock"},
		{116,"Taiko Drum"},
		{117,"Melodic Tom"},
		{118,"Synth Drum"},
		{119,"Reverse Cymbal"},
		{120,"Guitar Fret Noise"},
		{121,"Breath Noise"},
		{122,"Seashore"},
		{123,"Bird Tweet"},
		{124,"Telephone Ring"},
		{125,"Helicopter"},
		{126,"Applause"},
		{127,"Gunshot"},
	};
	const InstrumentMap listGMPercussiveNames[]=
	{
		{35,"Acoustic Bass Drum"},
		{36,"Bass Drum 1"},
		{37,"Side Stick"},
		{38,"Acoustic Snare"},
		{39,"Hand Clap"},
		{40,"Electric Snare"},
		{41,"Low Floor Tom"},
		{42,"Closed Hi-Hat"},
		{43,"High Floor Tom"},
		{44,"Pedal Hi-Hat"},
		{45,"Low Tom"},
		{46,"Open Hi-Hat"},
		{47,"Low-Mid Tom"},
		{48,"Hi-Mid Tom"},
		{49,"Crash Cymbal 1"},
		{50,"High Tom"},
		{51,"Ride Cymbal 1"},
		{52,"Chinese Cymbal"},
		{53,"Ride Bell"},
		{54,"Tambourine"},
		{55,"Splash Cymbal"},
		{56,"Cowbell"},
		{57,"Crash Cymbal 2"},
		{58,"Vibraslap"},
		{59,"Ride Cymbal 2"},
		{60,"Hi Bongo"},
		{61,"Low Bongo"},
		{62,"Mute Hi Conga"},
		{63,"Open Hi Conga"},
		{64,"Low Conga"},
		{65,"High Timbale"},
		{66,"Low Timbale"},
		{67,"High Agogo"},
		{68,"Low Agogo"},
		{69,"Cabasa"},
		{70,"Maracas"},
		{71,"Short Whistle"},
		{72,"Long Whistle"},
		{73,"Short Guiro"},
		{74,"Long Guiro"},
		{75,"Claves"},
		{76,"Hi Wood Block"},
		{77,"Low Wood Block"},
		{78,"Mute Cuica"},
		{79,"Open Cuica"},
		{80,"Mute Triangle"},
		{81,"Open Triangle"}
	};
	if (iChannel == 0x09 || (iChannel == 0x0A && nBank == 15360))
	{
		pMap=listGMPercussiveNames;
		strName="Unknown Percussive";
		for(i=0;i < 47;i++)
		{
			if (pMap[i].iInstrument == iInstrument)
			{
				strName=pMap[i].pszName;
				break;
			}
		}		
	}
	else
	{
		pMap=listGMMelodicNames;
		strName="Unknown";

		//patch to reduced SP-MIDI instrument set if needed
		//iInstrument=pDoc->GetMappedInstrument(iInstrument,CDMusic::GetInstrumentMap());

		for(i=0;i < 128;i++)
		{
			if (pMap[i].iInstrument == iInstrument)
			{
				strName=pMap[i].pszName;
				break;
			}
		}
	}
	return strName;
}

/*\
 *<------------ SetChannelUsed ------------>
 @m mark channel usage
 *--> I N <-- @p
 * int iIndex - channel index
 * bool bUsed - true=used,FALSE=unused
 */
void CMIDIFile::SetChannelUsed(int iIndex,bool bUsed)
{
	m_bChannelUsed[iIndex]=bUsed;
}

/*\
 *<------------ IsChannelUsed ------------>
 @m check if channel is marked as used
 *--> I N <-- @p
 * int iChannel - channel index
 *<-- OUT --> @r
 * bool - true=used
 */
bool CMIDIFile::IsChannelUsed(int iChannel)
{
	return m_bChannelUsed[iChannel&0x0F];
}

/*\
 *<------------ GetChannelCount ------------>
 @m get number of channels used
 *<-- OUT --> @r
 * int - number of channels used
 */
int CMIDIFile::GetChannelCount()
{
	int i,nUsed=0;
	for (i=0;i < 16;i++)
	{
		if (IsChannelUsed(i))
			++nUsed;
	}
	return nUsed;
}

void CMIDIFile::Init(void)
{
	m_nDivision=0;
	m_nFormat=0;
	m_nFirstOutTime=0x00000000;
	m_nLastOutTime=0x00000000;
	m_nQuantaSize=DEFAULTQUANTASIZE;
	m_nTempo=500000;
	//60000000/BPM = x
	CMIDITrack *pTrack=new CMIDITrack;
	m_Tracks.push_back(pTrack);
	m_ReadInfo.nData=0xFFFF;
	m_ReadInfo.data=new unsigned char[m_ReadInfo.nData];
	ASSERT (m_ReadInfo.data);
	m_iCurrentTrack = 0;
	m_nCurrentTime = 0;
	m_strSongName.erase();
	m_bDidTitle=FALSE;
	ZeroMemory (m_bChannelUsed,sizeof(bool)*16);
	ZeroMemory (m_nMaxPolyphony,sizeof(int)*16);
	ZeroMemory (m_nActivePolyphony,sizeof(int)*16);
	ZeroMemory (m_iChannelPriority,sizeof(int)*16);
	ZeroMemory (m_iInstrument,sizeof(int)*16);
	ZeroMemory (m_bInstrumentSet,sizeof(bool)*16);
	ZeroMemory (m_nMIPValue,sizeof(int)*16);
	ZeroMemory (m_nEventCount,sizeof(int)*16);
	ZeroMemory (m_nVelocities,sizeof(int)*16);
	int i;
	for (i=0;i < 16;i++)
	{
		if(m_mapPolyphony[i].size())
			m_mapPolyphony[i].erase(m_mapPolyphony[i].begin(), m_mapPolyphony[i].end());
		m_bPlayChannel[i]=true;
		m_nVolume[i]=100;
		m_nMaxVolume[i]=100;
		m_bInstrumentSet[i]=0;
		m_iInstrument[i]=0;
		m_bBankSet[i][0]=false;
		m_bBankSet[i][1]=false;
		m_iBank[i]=0;
	}	
	//m_iMIPPosition=-1;
	m_bContainedMIP=false;
	m_iVibraChannel=-1;
	m_nVibraMode=0;
	m_nSequencePolyphony=0;
	m_bAdditionalPercussion=false;

	/*
	{0,"Acoustic Grand"}, 
	18,19,20,21,22
	*/
	for (i=0;i < 128;i++)
		m_bUsedSample[i]=false;

	m_listSamples.push_back(new CPatchSample(1, "bass", 358));
	m_listSamples.push_back(new CPatchSample(2, "claves", 105));
	m_listSamples.push_back(new CPatchSample(3, "conga hi", 212));
	m_listSamples.push_back(new CPatchSample(4, "conga low", 872));
	m_listSamples.push_back(new CPatchSample(5, "flute high", 322));
	m_listSamples.push_back(new CPatchSample(6, "flute low", 263));
	m_listSamples.push_back(new CPatchSample(7, "guitar mid", 354));
	m_listSamples.push_back(new CPatchSample(8, "guitar high", 224));
	m_listSamples.push_back(new CPatchSample(9, "guitar low", 380));
	m_listSamples.push_back(new CPatchSample(10, "hihat", 2284));
	m_listSamples.push_back(new CPatchSample(11, "kick", 431));
	m_listSamples.push_back(new CPatchSample(12, "maraca", 396));
	m_listSamples.push_back(new CPatchSample(13, "marimba", 494));			//glockenspiel: 1251, 
	m_listSamples.push_back(new CPatchSample(14, "organ low", 79));
	m_listSamples.push_back(new CPatchSample(15, "organ mid", 72));
	m_listSamples.push_back(new CPatchSample(16, "organ high", 53));
	m_listSamples.push_back(new CPatchSample(17, "organ xhigh", 80));
	m_listSamples.push_back(new CPatchSample(18, "piano low", 412));		//3151byte
	m_listSamples.push_back(new CPatchSample(19, "piano mid", 372));
	m_listSamples.push_back(new CPatchSample(20, "piano high", 210));
	m_listSamples.push_back(new CPatchSample(21, "piano xhigh", 160));
	m_listSamples.push_back(new CPatchSample(22, "piano xxhigh", 104));
	m_listSamples.push_back(new CPatchSample(23, "cymbal", 2164));
	m_listSamples.push_back(new CPatchSample(24, "sine", 74));
	m_listSamples.push_back(new CPatchSample(25, "snare", 1312));
	m_listSamples.push_back(new CPatchSample(26, "steeldrum", 1000));		//2251byte
	m_listSamples.push_back(new CPatchSample(27, "strings high", 84));
	m_listSamples.push_back(new CPatchSample(28, "strings low", 84));
	m_listSamples.push_back(new CPatchSample(29, "string low", 1938));
	m_listSamples.push_back(new CPatchSample(30, "string high", 2550));
	m_listSamples.push_back(new CPatchSample(31, "leadsynth", 1444));
	m_listSamples.push_back(new CPatchSample(32, "tambourine", 1051));
	m_listSamples.push_back(new CPatchSample(33, "trumpet low", 247));
	m_listSamples.push_back(new CPatchSample(34, "trumpet mid", 305));
	m_listSamples.push_back(new CPatchSample(35, "telephone", 1134));		//2470byte	//2519
	m_listSamples.push_back(new CPatchSample(36, "toms", 560));
	m_listSamples.push_back(new CPatchSample(37, "trumpet high", 290));
	m_listSamples.push_back(new CPatchSample(38, "trumpet xhigh", 245));
	m_listSamples.push_back(new CPatchSample(39, "woodwind low", 128));
	m_listSamples.push_back(new CPatchSample(40, "woodwind mid", 83));
	m_listSamples.push_back(new CPatchSample(41, "woodwind high", 76));
	m_listSamples.push_back(new CPatchSample(42, "woodwind xhigh", 74));
}


/*\
 *<------------ ~CMIDIFile ------------>
 @m destructor
 */
CMIDIFile::~CMIDIFile()
{
	Reset();

	unsigned int i;
	for (i=0;i < (unsigned )m_listSamples.size();i++)
	{
		ASSERT (m_listSamples[i]);
		if (m_listSamples[i] != NULL)
			delete m_listSamples[i];
	}
	if (m_listSamples.size())
		m_listSamples.erase(m_listSamples.begin(), m_listSamples.end());

}

/*\
 *<------------ CMIDIFileEdit ------------>
 @m default constructor
 */
CMIDIFileEdit::CMIDIFileEdit()
{
	ZeroMemory(&m_iTrackPos,sizeof(int)*16);
	m_iEventPos=0;
	m_iChannel=0;
}

/*\
 *<------------ GetFirstOutTime ------------>
 @m get first event time
 *<-- OUT --> @r
 * unsigned int - time value
 */
unsigned int CMIDIFileEdit::GetFirstOutTime()
{
	return m_nFirstOutTime;
}


/*\
 *<------------ ~CMIDIFileEdit ------------>
 @m destructor
 */
CMIDIFileEdit::~CMIDIFileEdit()
{ 
}

/*\
 *<------------ GetTrackCount ------------>
 @m get number of tracks
 *<-- OUT --> @r
 * int - track count
 */
int CMIDIFileEdit::GetTrackCount()
{
	return (int)m_Tracks.size();
}

/*\
 *<------------ GetFirstEvent ------------>
 @m get first event from a track
 *--> I N <-- @p
 * MIDIViewEvent *pOut - pointer to destination event buffer
 *<-- OUT --> @r
 * BOOL - TRUE=ok
 */
BOOL CMIDIFileEdit::GetFirstEvent (MIDIViewEvent *pOut)
{
	ZeroMemory(&m_iTrackPos,sizeof(int)*64);
	m_iEventPos=0;
	return GetNextEvent(pOut);
}

/*\
 *<------------ GetNextEvent ------------>
 @m get next event from a track
 *--> I N <-- @p
 * MIDIViewEvent *pOut - pointer to destination event buffer 
 *<-- OUT --> @r
 * BOOL - TRUE=ok
 */
BOOL CMIDIFileEdit::GetNextEvent (MIDIViewEvent *pOut)
{
	unsigned int i,o;
	BOOL bRet=FALSE;
	ASSERT (pOut);
	unsigned int nFirst=0xFFFFFFFF;

	for (i=0;i < m_Tracks.size();i++)
	{
		CMIDITrack *pTrack=m_Tracks[i];
		if ((unsigned int)m_iTrackPos[i] < pTrack->m_Events.size())
		{
			CMIDIEvent *pEvent=pTrack->m_Events[m_iTrackPos[i]];
			ASSERT (pEvent);
			if (pEvent->m_nAt < nFirst)
			{
				ZeroMemory (pOut,sizeof(MIDIViewEvent));
				nFirst=pEvent->m_nAt;
				pOut->nTime=pEvent->m_nAt;
				switch (pEvent->m_pcEvent[0] & 0xF0)
				{
					case MIDI_NOTEON:
						if (pEvent->m_pcEvent[2] > 0x00)
						{
							pOut->nNote=(unsigned int)(pEvent->m_pcEvent[1]);
							pOut->nChannel=pEvent->m_pcEvent[0] & 0x0F;
							pOut->bNoteOn=TRUE;
							pOut->bTempo=FALSE;
							break;
						}
					case MIDI_NOTEOFF:
						pOut->nNote=(unsigned int)(pEvent->m_pcEvent[1]);
						pOut->nChannel=pEvent->m_pcEvent[0] & 0x0F;
						pOut->bTempo=FALSE;
						pOut->bNoteOn=FALSE;
					break;
					case MIDI_TEMPO:
						pOut->bNoteOn=FALSE;
						pOut->bTempo=TRUE;
						pOut->nTempo=(((unsigned long int)(pEvent->m_pcEvent[1]))<<16) | (((unsigned long int)(pEvent->m_pcEvent[2]))<<8) | ((unsigned long int)(pEvent->m_pcEvent[3]));
						if (pOut->nTempo)
							pOut->nTempo=60000000/pOut->nTempo;						
					break;
				}
				//note opened ?
				if (pOut->bNoteOn)
				{	//yes->find note off (to get the duration)
					BOOL bFoundNoteOff=FALSE;
					o=m_iTrackPos[i];
					pOut->nDuration=0;
					while (o < pTrack->m_Events.size() && !bFoundNoteOff)
					{
						pEvent=pTrack->m_Events[o];
						switch (pEvent->m_pcEvent[0] & 0xF0)
						{
							case MIDI_NOTEON:
								//is it a note-on with zero volume ?
								if (pEvent->m_pcEvent[2] == 0x00)
								{	//yes->is it the "opened" note ?
									if (pEvent->m_pcEvent[1] == pOut->nNote)
									{	//yes->calc delta to get the duration
										pOut->nDuration=pEvent->m_nAt - pOut->nTime;
										bFoundNoteOff=TRUE;
									}
								}
							break;
							case MIDI_NOTEOFF:
								//is it the "opened" note ?
								if (pEvent->m_pcEvent[1] == pOut->nNote)
								{	//yes->calc delta to get the duration
									pOut->nDuration=pEvent->m_nAt - pOut->nTime;
									bFoundNoteOff=TRUE;
								}
							break;
						}
						++o;
					};
					ASSERT (bFoundNoteOff);
				}
				++m_iTrackPos[i];
				bRet=TRUE;
			}
		}
	}
	return bRet;
}


//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

/*\
 *<------------ CMIDIFileLoader ------------>
 @m default constructor
 */
CMIDIFileLoader::CMIDIFileLoader()
{
}

/*\
 *<------------ ~CMIDIFileLoader ------------>
 @m destructor
 */
CMIDIFileLoader::~CMIDIFileLoader()
{
}

/*\
 *<------------ ReadMetaevent ------------>
 @m read a metaevent from the input-stream
 *--> I N <-- @p
 * CArchive &ar - reference of the input stream
 *<-- OUT --> @r
 * int - ok=done,endOfStream=failed,unrecognized=failed
 */
int CMIDIFileLoader::ReadMetaevent (istream &ar)
{
	unsigned char theByte,dump;

    short val;
    int temp;
    int len;

	try 
	{
		ar.read((char *)&theByte,1);

		if (!ReadVariableQuantity (ar,&len))
			return endOfStream;

		switch (theByte)
		{
			/*
			case KARAOKETEXT:
				m_ReadInfo.data[0]=text;
				m_ReadInfo.nData=0;
				while (len)
				{
					ar >> dump;
					--len;
				};
			break;
			*/
			case SEQUENCENUMBER:
				m_ReadInfo.data[0]=seqNumber;
				ar.read((char *)&val,1);
				m_ReadInfo.nData = 0;
				len -= 2;
			break;
			case CHANNELPREFIX:
				m_ReadInfo.data[0]=channelPrefix;   //set our internal meta-event command
				if (m_ReadInfo.nData >= 2)
					temp=min (len,m_ReadInfo.nData-2);
				else
					temp=len;				
				ar.read((char *)&m_ReadInfo.data[1],temp);
				len-=temp;
				while (len)
				{
					ar.read((char *)&dump,1);
					--len;
				};
			break;
			case 0x21:
				m_ReadInfo.data[0]=0;
				while (len)
				{
					ar.read((char *)&dump,1);
					--len;
				};
				m_ReadInfo.nData =  len = 0;                 //not a stored event !
			break;
			case TRACKCHANGE:
				m_ReadInfo.nData = 3;
				m_ReadInfo.data[0] = trackChange;   //set our internal track-change command
				m_ReadInfo.data[1] = ((m_iCurrentTrack+1) >> 8);
				m_ReadInfo.data[2] = m_iCurrentTrack+1;
				TRACEIT2("time:%08d track:%02d track change - readMetaevent\n",m_nCurrentTime,m_iCurrentTrack);
			break;
			case TEMPOCHANGE:
				m_ReadInfo.nData = 4;
				m_ReadInfo.data[0] = MIDI_TEMPO;
				ar.read((char *)m_ReadInfo.data+1,3);
				len -= 3;
				//#if DEBUGLEVEL >= 2
				temp = (m_ReadInfo.data[1] << 16) | (m_ReadInfo.data[2] << 8) | m_ReadInfo.data[3];
				TRACEIT2("time:%08d track:%02d tempo:%d - readMetaevent\n",m_nCurrentTime,m_iCurrentTrack,60000000/temp);
				//#endif
			break;
			case SMPTEOFFSET:
				m_ReadInfo.data[0] = smpteOffset;   //set our internal smpte-offset command
				m_ReadInfo.nData = 0;                        //not a stored event !
				TRACEIT2("time:%08d track:%02d channel prefix - readMetaevent\n",m_nCurrentTime,m_iCurrentTrack);
			break;
			case TIMESIG:
				m_ReadInfo.data[0] = timeSig;
				ar.read((char *)m_ReadInfo.data+1,4);
				m_ReadInfo.nData = 0;                        //not a stored event !
				len  -= 4;
				TRACEIT2("time:%08d track:%02d timesig - readMetaevent\n",m_nCurrentTime,m_iCurrentTrack);
			break;
			case KEYSIG:
				m_ReadInfo.data[0] = keySig;
				ar.read((char *)m_ReadInfo.data+1,2);
				m_ReadInfo.nData = 0;                        //not a stored event !
				len -= 2;
				TRACEIT2("time:%08d track:%02d keysig - readMetaevent\n",m_nCurrentTime,m_iCurrentTrack);
			break;
			case SEQUENCERMETA:
				m_ReadInfo.data[0] = seqMeta;
				m_ReadInfo.bMetaEvent=TRUE;
				m_ReadInfo.nData = len+1;            //nData doesn't include the \0
				if (!ar.read((char *)&m_ReadInfo.data[1],len))
					return endOfStream;
				int i;
				TRACEIT2("time:%08d track:%02d sequencer meta - readMetaevent\n",m_nCurrentTime,m_iCurrentTrack);
				for (i=0;i < m_ReadInfo.nData;i++)
					TRACEIT ("%02X ",m_ReadInfo.data[i]);
				TRACEIT ("\n");
				len=0;
			break;
			default:
				if (theByte >= 0x01 && theByte <= 0x0f)  //Text meta events
				{
					m_ReadInfo.data[0] = theByte;
					m_ReadInfo.nData = len+1;          //nData doesn't include the \0
					temp=min(m_ReadInfo.nData,len);
					i=1;
					while (temp--)
						ar.read((char *)&m_ReadInfo.data[i++],1);
					m_ReadInfo.data[i] = 0;
					TRACEIT2("textevent:%02X - time:%08d track:%02d: text: %s - readMetaEvent\n",theByte,m_nCurrentTime,m_iCurrentTrack,m_ReadInfo.data+1);
					len=0;
				}
				else                      //skip unrecognized meta events
				{
					if (!ReadVariableQuantity(ar,&temp))
						return endOfStream;
					while (temp--)
						ar.read((char *)&dump,1);
					TRACEIT2("%08d track:%d unknown metaevent:%02X (%d bytes) - readMetaevent\n",m_ReadInfo.quanta,m_iCurrentTrack,theByte,m_ReadInfo.nData);
					for (i=0;i < m_ReadInfo.nData;i++)
						TRACEIT ("%02X ",m_ReadInfo.data[i]);
					TRACEIT ("\n");
					m_ReadInfo.data[0]=0;
					m_ReadInfo.nData = 0;                        //not a stored event !
					return unrecognized;
				}
		}
		while (len)					//Skip any extra length in field. 
		{
			ar.read((char *)&dump,1);
			--len;
		};
	}
	catch (ifstream::failure const &e)
	{
		//if (e.w == CArchiveException::endOfFile)
		if (ar.eof())
			return endOfStream;
		else
		{
			Log2(verbLevErrors,"File Access Exception\n\t%s\n",e.what());
			return unrecognized;
		}
	}
	return ok;
}

/*\
 *<------------ ReadChunkType ------------>
 @m get type of chunk from our input stream
 *--> I N <-- @p
 * CArchive &ar - reference of the opened stream
 * BYTE *pBuffer - destination buffer for the chunk type (4 bytes)
 *<-- OUT --> @r
 * BOOL - TRUE=done
 */
bool CMIDIFileLoader::ReadChunkType (istream &ar,char *pBuffer)
{
	bool bRet=true;
	try
	{
		Log2(verbLevDebug3,"pos: %d\n",(int)ar.tellg());
		ar.read((char *)pBuffer,4);
		*(pBuffer+4) = '\0';
	}
	catch (ifstream::failure const &e)
	{
		if (!ar.eof())
			Log2(verbLevErrors,"%s\n",e.what());
		bRet=false;
	}
    return bRet;
}

/*\
 *<------------ ReadSysExclEvent ------------>
 @m read system exclusive data from input stream
 *--> I N <-- @p
 * CArchive &ar - reference of the opened input stream
 * int oldState - last sysex state
 *<-- OUT --> @r
 * int - midi-status (ok,endOfStream,unrecognized)
 */
int CMIDIFileLoader::ReadSysExclEvent (istream &ar,int oldState)
{
	int 			len;
	unsigned char	*ptr;

	if (!ReadVariableQuantity (ar,&len))
		return endOfStream;

	if (oldState == undefined)
	{
        //if (!checkRealloc (p,len+1))    // len doesn't include data[0]
		//	return endOfStream;
		m_ReadInfo.data[0] = MIDI_SYSEXCL;
        m_ReadInfo.nData = len+1;            //
        ptr = &m_ReadInfo.data[1];	         //
	}
	else				  // firstISysExcl or middleISysExcl
	{
		TRACEIT2 ("middle SYSEX - readSysExclEvent\n");
		//if (!checkRealloc (p,len + p->m->nData))
		//	return endOfStream;
		ptr = &m_ReadInfo.data[m_ReadInfo.nData];
		m_ReadInfo.nData+=len;
	}
	ar.read ((char *)ptr,len);
	if(ar.fail())
		return endOfStream;

	TRACEIT2("time:%08d reading %d SYSEX-bytes ! - ReadSysExclEvent\n",m_nCurrentTime,m_ReadInfo.nData);
	return ((m_ReadInfo.data[m_ReadInfo.nData-1] == MIDI_EOX) ?
			((oldState == undefined) ? sysExcl : endISysExcl) :
			((oldState == undefined) ? firstISysExcl : middleISysExcl));
}

/*\
 *<------------ ReadEscapeEvent ------------>
 @m read an escape event from our input stream
 *--> I N <-- @p
 * CArchive &ar - opened input stream
 *<-- OUT --> @r
 * int - midi-status (ok,endOfStream,unrecognized)
 */
int CMIDIFileLoader::ReadEscapeEvent (istream &ar)
{
	if (!ReadVariableQuantity (ar,&m_ReadInfo.nData))
		return endOfStream;

	if (m_ReadInfo.nData < 64)
		ar.read((char *)&m_ReadInfo.data,m_ReadInfo.nData);
	else
	{
		m_ReadInfo.nData=63;
		ar.read((char *)&m_ReadInfo.data,m_ReadInfo.nData);
	}
	return ok;
}

/*\
 *<------------ ReadVariableQuantity ------------>
 @m read variable amount of data from input stream
 *--> I N <-- @p
 * CArchive &ar - reference of the opened input stream
 * int *n - pointer to destition buffer
 *<-- OUT --> @r
 * int - number of bytes read
 */
int CMIDIFileLoader::ReadVariableQuantity (istream &ar,int *n)
{
	unsigned int i=0;
	int m = 0;
	unsigned char temp;

	try
	{
		while (ar.read((char *)&temp,1) > 0)
		{
			++i;
    		if (temp & 0x80)
        		m = (m<<7) + (temp & 0x7F);
			else
			{
				*n = (m<<7) + (temp & 0x7F);
				if ((*n & 0x0F000000) == 0x0F000000)
					*n|=0xFF000000;
				if (i > 0xFFFFF)
					TRACEIT("ui - rot alarm %d\n",i);
				return i;
			}
		};
		if (i > 0xFFFFF)
			TRACEIT("ui - rot alarm %d\n",i);
	}
	catch(ifstream::failure const &e)
	{
		Log2(verbLevErrors,"File Access Exception\n\t%s\n",e.what());
		TRACEIT2("ifstream::failure\n");
		i=0;
	}
    return i;
}

/*\
 *<------------ ReadEvent ------------>
 @m read a midi event from input stream
 *--> I N <-- @p
 * CArchive &ar - reference of the opened stream
 *<-- OUT --> @r
 * int - midi-status (ok,endOfStream,unrecognized)
 */
int CMIDIFileLoader::ReadEvent (istream &ar)
{
	int deltaTime,quantaTime,state = undefined;
    unsigned char theByte,newRunningStatus;
	int len;

	//try to read the time-delta for the next event
	try
	{
		while ((len=ReadVariableQuantity (ar,&deltaTime)) > 0)
		{
			m_nCurrentTime += deltaTime;     //add it to our time
			if (deltaTime < 0)
				TRACEIT2("event delta negative! - %d\n",deltaTime);
			quantaTime = SCALEQUANTA (m_nCurrentTime);

			//read command-byte
			ar.read((char *)&theByte,1);
			switch (theByte)
			{
				case 0xFF:
					state = ReadMetaevent (ar);		//
					m_ReadInfo.bMetaEvent = TRUE;   //
					if (state != unrecognized)		//do we know it ?
					{
						m_ReadInfo.quanta = quantaTime;
						return state;               //yes, ok !
					}
				break;
				case MIDI_SYSEXCL:
					// system exclusive
					state = ReadSysExclEvent (ar,state);
					m_ReadInfo.bMetaEvent = FALSE;
					switch (state)
					{
						case firstISysExcl:
							m_ReadInfo.quanta = quantaTime;
						break;
						case middleISysExcl:
							m_ReadInfo.quanta += quantaTime;
						break;
						case endISysExcl:
							m_ReadInfo.quanta += quantaTime;
							return ok;
						case endOfStream:
						case sysExcl:
							m_ReadInfo.quanta = quantaTime;
							return ok;
					}
				break;
				case 0xF7:			       // special "escape" code ?
					m_ReadInfo.quanta = quantaTime;
					return ReadEscapeEvent(ar);
				default:                    // normal MIDI !
					newRunningStatus = (theByte & MIDI_STATUSBIT);
					if (newRunningStatus)
						m_cRunningStatus = theByte;

					m_ReadInfo.bMetaEvent = FALSE;
					m_ReadInfo.quanta = quantaTime;
					m_ReadInfo.nData = MIDI_EVENTSIZE (m_cRunningStatus);
					m_ReadInfo.data[0] = m_cRunningStatus;
					if (m_ReadInfo.nData > 1)
					{
						if (newRunningStatus)
							ar.read((char *)m_ReadInfo.data+1,1);
						else
							m_ReadInfo.data[1] = theByte;

						if (m_ReadInfo.nData > 2)
							ar.read((char *)m_ReadInfo.data+2,1);
					}
					return ok;
			}
		};
	}
	catch (ifstream::failure const &e)
	{
		Log2(verbLevErrors,"File Access Exception\n\t%s\n",e.what());
	}
	return endOfStream;
}

void CMIDIFile::SetChannelPriority(int iChannel,int iPrio)
{
	int i,nUsed;
	int iConflictAt=-1;
	int iReorderFirst,iReorderLast;

	//handle conflicts
	for (i=0;i < 16;i++)
	{
		if (m_iChannelPriority[i] == iPrio && i != iChannel)	//is this a conflic ?
			iConflictAt=i;										//yes->this is a conflict
	}

	nUsed=GetChannelCount();
	if (iConflictAt >= 0)
	{
		iReorderFirst=min(iPrio,m_iChannelPriority[iChannel]);
		iReorderLast=max(iPrio,m_iChannelPriority[iChannel]);

		for (i=0;i < 16;i++)
		{
			if (i != iChannel)
			{
				if (m_iChannelPriority[i] >= iReorderFirst && m_iChannelPriority[i] <= iReorderLast)
				{
					if (m_iChannelPriority[iChannel] > iPrio)
						++m_iChannelPriority[i];
					else
						--m_iChannelPriority[i];
				}
			}
		}
	}
	//now set priority to channel
	m_iChannelPriority[iChannel]=iPrio;
}

void CMIDIFile::SetPresetMaxPolyphony(int iChannel,int nMip)
{
	ASSERT(iChannel <= 0x0F);
	m_nPresetMaxPolyphony[iChannel&0x0F]=nMip;
}

void CMIDIFileLoader::InitPresetPolyphonyWithMaxPolyphony(void)
{
	int i;
	for (i=0;i <  16;i++)
		SetPresetMaxPolyphony(i,GetMaxPolyphony(i));
}


/*\
 *<------------ ProcessMip ------------>
 @m process SP-MIDI MIP message
 */
bool CMIDIFileLoader::ProcessMip(void)
{
	int iActivePrio=1;
	int nUsedVoices=0;
	int i;

	TRACEIT2("processing MIP message\n");
	m_bContainedMIP=TRUE;

	for (i=5;m_ReadInfo.data[i] != MIDI_EOX;i+=2)
	{	
		unsigned char iChannel=m_ReadInfo.data[i];
		unsigned char nMIP=m_ReadInfo.data[i+1];

		if (iChannel > 0x0F)
		{
			TRACEIT2("invalid channel index (%d) in SP-MIDI MIP-Message\n",iChannel);
			break;
		}
		if (nMIP >= 0x7F)
		{
			TRACEIT2("invalid MIP (%02Xh) in SP-MIDI MIP-Message\n",nMIP);
			break;
		}

		SetChannelPriority(iChannel,iActivePrio);
		if (nMIP-nUsedVoices < 0)
			++m_nBrokenChannels;		
		SetPresetMaxPolyphony(iChannel,nMIP-nUsedVoices);

		nUsedVoices=nMIP;
		++iActivePrio;
	}
	return m_ReadInfo.data[i] == MIDI_EOX;
}

/*\
 *<------------ Load ------------>
 @m read a midi from input stream
 *--> I N <-- @p
 * CArchive &ar - opened input stream
 */
int CMIDIFileLoader::Load(istream &ar)
{
	int i;
	int iError=CMIDIFile::ok;
	int iVibraRecogState[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

	bool bFound79=false;
	bool bFound78=false;

	unsigned char theByte;

	m_iCurrentTrack = 0;
	m_nCurrentTime = 0;
	m_ReadInfo.nData = 0;
	m_bDidTitle=FALSE;
	m_nBrokenChannels=0;

	bool bGotMip=false;

	//make sure the document is empty in initialized
	Reset();
	Init();
	
	//SetSongName(ar.GetFile()->GetFileTitle());
	m_strSongName.erase();
	m_strCopyright.erase();

	if ((iError=ReadPreamble (ar)) != CMIDIFile::ok)
	{
		TRACEIT2 ("loading preamble failed\n");
	}
	else
	{
		BOOL bAddEvent;
		while (ReadEvent (ar))
		{
			bAddEvent=FALSE;
			if (m_ReadInfo.nData)
			{
				theByte=m_ReadInfo.data[0];
				//calc polyphony
				CalcPolyphony(m_ReadInfo.quanta,m_ReadInfo.data);
				//remember instrument usage
				InstrumentUsage(m_ReadInfo.quanta,m_ReadInfo.data);
				
				//identidy MIPs
				if (theByte == MIDI_SYSEXCL)
				{
					if (m_ReadInfo.nData > 7 && m_ReadInfo.data[1] == 0x7F)
					{
						if (m_ReadInfo.data[3] == 0x0B &&		//S1 sub-ID#1 = 0B (Scalable Polyphony MIDI) 
							m_ReadInfo.data[4] == 0x01)			//S2 sub-ID#2 = 01 (MIP Message) 
						{
							bAddEvent=TRUE;
							if (bGotMip)
								bAddEvent=FALSE;
							else if (ProcessMip())
								bGotMip=true;
						}
					}
#ifdef MIDISCALE
					else
						bAddEvent=!theApp.GetEventstripping(CMIDIscaleApp::STRIPPING_SYSEX);
#else
					else
						bAddEvent=TRUE;
#endif
				}
				switch (theByte & 0xF0)
				{
					case MIDI_NOTEON:
						if (m_ReadInfo.data[2])
						{
							m_nVelocities[theByte&0x0F]+=m_ReadInfo.data[2];
							++m_nEventCount[theByte&0x0F];
						}
					case MIDI_CONTROL:
					case MIDI_PROGRAM:
					case MIDI_NOTEOFF:
					case MIDI_POLYPRES:
					case MIDI_CHANPRES:
					case MIDI_PITCH:
					case MIDI_TEMPO:
						if (theByte != MIDI_TEMPO)
							SetChannelUsed(theByte & 0x0F,true);
						else
							m_nTempo = (((unsigned long int)m_ReadInfo.data[1])<<16) | (((unsigned long int)m_ReadInfo.data[2])<<8) | ((unsigned long int)m_ReadInfo.data[3]);
						if ((theByte&0xF0) == MIDI_PROGRAM)
						{
							SetChannelInstrument(theByte&0x0F,m_ReadInfo.data[1]);
							if (m_ReadInfo.data[1] == 0x7D && iVibraRecogState[theByte&0x0F] == 3)
							{
								SetVibraChannel(theByte&0x0F);
								SetVibraMode(1);
							}
							else if (m_ReadInfo.data[1] == 0x7C && iVibraRecogState[theByte&0x0F] == 3)
							{
								SetVibraChannel(theByte&0x0F);
								SetVibraMode(2);
							}
							iVibraRecogState[theByte&0x0F]=0;
						}
						if ((theByte&0xF0) == MIDI_CONTROL)
						{
							switch (m_ReadInfo.data[1])
							{
								//main volume
								case 0x07:
									m_nVolume[theByte&0x0F]=m_ReadInfo.data[2];
									if (m_nVolume[theByte&0x0F] > m_nMaxVolume[theByte&0x0F])
										m_nMaxVolume[theByte&0x0F]=m_nVolume[theByte&0x0F];
								break;
								//bank switch MSB
								case 0x00:
									TRACEIT2("Bank Change MSB channel %d - %02X - CMIDIFileLoader::Load\n",theByte&0x0F,m_ReadInfo.data[2]);
									SetChannelBank(theByte&0x0F,m_ReadInfo.data[2],1);
									switch(m_ReadInfo.data[2])
									{
										case 0x78:
											bFound78=true;
										break;
										case 0x79:
											bFound79=true;
										break;
										default:
											bFound78=false;
											bFound79=false;
									}
								break;
								//bank switch LSB
								case 0x20:
									TRACEIT2("Bank Change LSB channel %d  - %02X - CMIDIFileLoader::Load\n",theByte&0x0F,m_ReadInfo.data[2]);
									SetChannelBank(theByte&0x0F,m_ReadInfo.data[2],0);
									switch(m_ReadInfo.data[2])
									{
										case 0x00:
											if ((theByte&0x0F) == 0x0A)
											{
												if (bFound79)
													EnablePercussionChannel(false);
												if (bFound78)
													EnablePercussionChannel(true);
												iVibraRecogState[theByte&0x0F]=0;
											}
										break;
										case 0x06:
											if (bFound79)
												iVibraRecogState[theByte&0x0F]=3;
											else
												iVibraRecogState[theByte&0x0F]=0;
										break;
										default:
											iVibraRecogState[theByte&0x0F]=0;
									}
									bFound78=false;
									bFound79=false;
								break;
							}
						}
						if ((unsigned int)m_ReadInfo.quanta < m_nFirstOutTime)
						{
							if (theByte != MIDI_TEMPO)
								m_nFirstOutTime=m_ReadInfo.quanta;
							//else
							//	m_nTempo = (((ULONG)m_ReadInfo.data[1])<<16) | (((ULONG)m_ReadInfo.data[2])<<8) | ((ULONG)m_ReadInfo.data[3]);
						}
						if (m_nLastOutTime < (unsigned int)m_ReadInfo.quanta)
							m_nLastOutTime=m_ReadInfo.quanta;
						bAddEvent=TRUE;
					break;
					default:
						switch (theByte)
						{
							case MIDI_SYSEXCL:			
							break;
							case CMIDIFile::copyright:
								if (m_strCopyright.empty())
								{
									TRACEIT2("setting copyright\n");
									SetCopyright((LPCTSTR)&m_ReadInfo.data[1]);
								}
							break;
							case CMIDIFile::sequenceOrTrackName:
								if (m_strSongName.empty())
								{
									TRACEIT2("setting songname\n");
									SetSongName((LPCTSTR)&m_ReadInfo.data[1]);
								}
								TRACEIT2("setting trackname\n");
								m_Tracks[m_iCurrentTrack]->m_strName = (TCHAR *)&m_ReadInfo.data[1];
								bAddEvent=TRUE;
							break;
							case CMIDIFile::songName:
								if (m_strSongName.empty())
								{
									TRACEIT2("setting songname\n");
									SetSongName((LPCTSTR)&m_ReadInfo.data[1]);
								}
								bAddEvent=TRUE;
							break;
							case CMIDIFile::trackChange:
								if (++m_iCurrentTrack >= MIDI_MAXTRACKS)
									iError=CMIDIFile::fileCorrupt;
								else
								{
									m_nCurrentTime=0;
									if (ReadTrackHeader (ar) == CMIDIFile::ok)
									{
										CMIDITrack *pTrack=new CMIDITrack;
										m_Tracks.push_back(pTrack);
									}
									else
									{
										--m_iCurrentTrack;
										goto FinishedParsing;
									}
								}
							break;
							default:
							{
								int i;
								TRACEIT2 ("track: %02d delta: %07d   data:",m_iCurrentTrack,m_ReadInfo.quanta);
								for (i=0;i < (m_ReadInfo.nData > 0 ? m_ReadInfo.nData : 1);i++)
									TRACEIT2 ("%02X ",m_ReadInfo.data[i]);
								TRACEIT2 (" - loadMidi\n");
								bAddEvent=TRUE;
							}
						}
					}
			}
			if (bAddEvent)
			{
				if (!m_Tracks[m_iCurrentTrack]->AddEvent(&m_ReadInfo))
					iError=CMIDIFile::internalError;
			}
		};
	}

FinishedParsing:
	if (bGotMip)
	{
		if (GetBrokenChannels() > 0)
			bGotMip=false;
	}
	//initialize MIP-values with some default
	/*
	if (!bGotMip)
	{
		InitPresetPolyphonyWithMaxPolyphony();
		InitPriosWithDefaults();
	}
	*/
	//check if we finally found any content for any channel
	BOOL bContent=FALSE;
	for (i=0;i < 16;i++)
	{
		if (IsChannelUsed(i))
			bContent=TRUE;
	}
	//
	if (!bContent)
		iError=CMIDIFile::fileCorrupt;
	else
		CalcSequencePolyphony();
	return iError;
}













/*\
 *<------------ Load ------------>
 @m read a midi from file
 *--> I N <-- @p
 * const char *szFile - pointer to path
 */
int CMIDIFileLoader::Load(const char *szFile)
{
	ASSERT(szFile);
	int iError=ok;
	ifstream is;
	is.exceptions ( ifstream::eofbit | ifstream::failbit | ifstream::badbit );
	is.open(szFile,ios::binary);
	m_strSongName.erase();
#ifdef WIN32
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char ext[_MAX_EXT];
	char fname[_MAX_FNAME];
	_splitpath(szFile,drive,dir,fname,ext);
#else
	char *fname;
	if ((fname=strrchr(szFile,'/')) == NULL)
		fname=(char *)szFile;
#endif
	SetSongName(fname);						//preinit with filename
	iError=Load(is);
	is.close();
	return iError;
}


/*\
 *<------------ ReadPreamble ------------>
 @m read file header
 *--> I N <-- @p
 * CArchive &ar - reference of the opened archive stream
 *<-- OUT --> @r
 * int - midi-status (ok,endOfStream,unrecognized)
 */
int CMIDIFileLoader::ReadPreamble (istream &ar)
{
	char		typebuf[8];
	long int	size;
	short		fmt, tracks, div;

    if (!ReadChunkType (ar,(char *)typebuf))		//
		return endOfStream;
	if (strcmp (typebuf,"MThd"))            //must be MThd !!
		return endOfStream;
	
	ar.read((char *)&size,4);
	size=ntohl(size);
	TRACEIT ("size:%d - ReadPreamble\n",size);

	ar.read((char *)&fmt,2);
	fmt=ntohs(fmt);
    TRACEIT ("format-level:%d - ReadPreamble\n",fmt);
    if (fmt < 0 || fmt > 2)             //
		return endOfStream;

	ar.read((char *)&tracks,2);
	tracks=ntohs(tracks);
	TRACEIT ("tracks:%d - ReadPreamble\n",tracks);

	ar.read((char *)&div,2);
	div=ntohs(div);
	TRACEIT ("div:%d - ReadPreamble\n",div);

	size -= 6;
	while (size)
	{
		ar.read((char *)typebuf,1);
		--size;
	};

	//m_nTracks=fmt ? tracks-1 : 1;
	m_nFormat=fmt;

    if (div < 0)                        //time code encoding ?
	{
		// for now, we undo the effect of the time code
		// we may want to eventually pass the time code up ?
		short SMPTEformat,ticksPerFrame;

		TRACEIT ("used TIME ENCODING !\n");
        ticksPerFrame = div & 0xff;         //
		// SMPTEformat is one of 24, 25, 29, or 30.
		// it's stored negative !
        SMPTEformat = -(div >> 8);          //
        div = ticksPerFrame * SMPTEformat;  //
	}
    m_nDivision = div;                     //
	m_fTimeScale = 60000000.0 / (double)(m_nDivision * m_nQuantaSize);
    return ReadTrackHeader (ar);
}

/*\
 *<------------ ReadTrackHeader ------------>
 @m read track header
 *--> I N <-- @p
 * CArchive &ar - reference of the opened archive stream
 *<-- OUT --> @r
 * int - midi-status (ok,endOfStream,unrecognized)
 */
int CMIDIFileLoader::ReadTrackHeader (istream &ar)
{
	char typebuf[8];
    signed long int size;
    double n;

	// tempo in file is in micro-seconds per quarter note
    n = 60000000.0/(double)DEFAULTTEMPO;      //
    // division is the number of delta time "ticks" that make up a
    // quarter note. Quanta size is in micro seconds.
    m_fTimeScale = n / (double)(m_nDivision * m_nQuantaSize);

    TRACEIT2("checking Track header %d - readTrackHeader\n",m_iCurrentTrack);

    m_nCurrentTime=0;

    if (!ReadChunkType (ar,(char *)typebuf))
    {
		TRACEIT ("failed when reading chunk !\n");
        return endOfStream;
	}
	if (strcmp(typebuf,"MTrk"))
    {
    	TRACEIT2 ("failed when comparing \"%4s\" with MTrk !\n",typebuf);
        TRACEIT2 ("trying it with another trackchange !\n");
		return endOfStream;
		/*
		//some midis have multiple track-change commands in a row
		//that is ILLEGAL but we try to get it all right
 		fseek (p->mfp,-4,SEEK_CUR);
		if (MIDIFILEReadEvent (p) == ok && p->m->data[0] == MIDIFILE_trackChange)
			return readTrackHeader (p);
		else
		*/		
	}
	ar.read((char *)&size,4);
	size=ntohl(size);
    if (!size)
    {
		TRACEIT2 ("failed as size is ZERO/Nara/NULL/Nix !\n");
        return endOfStream;
    }
    TRACEIT2 ("track seems to be ok\n");
    return ok;
}

/*
static int readVariableQuantity (FILE *mfp, int *n)
{
	int m = 0;
	unsigned char temp;
	while (fread (&temp,1,1,mfp) > 0)
    {
    	if (temp & 0x80)
        	m = (m<<7) + (temp & 0x7F);
		else
        {
			*n = (m<<7) + (temp & 0x7F);
            return ok;
		}
	}
    return endOfStream;
}
*/

/*\
 @citate
 *We do not support multi-packet system exclusive messages with different
 *timings. When such a beast occurs, it is concatenated into a single
 * event and the time stamp is that of the last piece of the event.
\*/

/*\
 *<------------ AddEvent ------------>
 @m add an event at the tail of this track
 *--> I N <-- @p
 * ReadStruct *pRS - event data from file
 *<-- OUT --> @r
 * BOOL - TRUE=done
 */
BOOL CMIDITrack::InsertEvent(ReadStruct *pRS)
{
	CMIDIEvent *pEvent=new CMIDIEvent(	pRS->quanta,
										pRS->data,
										pRS->nData);
	return InsertEvent(pEvent);
}

/*
BOOL CMIDITrack::InsertEvent(CMIDIEvent *pEvent)
{
	BOOL bRet=TRUE;

	ASSERT (pEvent);
	if (pEvent)
	{
		ASSERT (pEvent->GetSize() < 0xFFFF);
		if (pEvent->GetSize() < 0xFFFF)
			m_Events.InsertAt(0,pEvent);
		else
			bRet=FALSE;
	}
	else
		bRet=FALSE;

	return bRet;
}
*/
BOOL CMIDITrack::InsertEvent(CMIDIEvent *pEvent)
{
	iterEventArray iter;
	for (iter=m_Events.begin();iter != m_Events.end();iter++)
	{
		if ((*iter)->GetAt() >= pEvent->GetAt())
		{
			m_Events.insert(iter,pEvent);
			return TRUE;
		}
	}
	return FALSE;
}



/*\
 *<------------ AddEvent ------------>
 @m add an event at the tail of this track
 *--> I N <-- @p
 * ReadStruct *pRS - event data from file
 *<-- OUT --> @r
 * BOOL - TRUE=done
 */
BOOL CMIDITrack::AddEvent(ReadStruct *pRS)
{
	BOOL bRet=TRUE;

	CMIDIEvent *pEvent=new CMIDIEvent(	pRS->quanta,
										pRS->data,
										pRS->nData,
										pRS->bMetaEvent);

	ASSERT (pEvent);
	if (pEvent)
	{
		ASSERT (pEvent->GetSize() < 0xFFFF);
		if (pEvent->GetSize() < 0xFFFF)
		{
			//event in correct sequence ?
			if (m_nLastQuanta <= (uint32_t)pEvent->GetAt())
			{	//yes -> add to tail
				m_nLastQuanta=pEvent->GetAt();
				m_Events.push_back(pEvent);
				bRet=TRUE;
			}
			else
			{	//no -> insert at right place
				bRet=InsertEvent(pEvent);
			}
		}
		else
			bRet=FALSE;
	}
	else
		bRet=FALSE;

	return bRet;
}

/*\
 *<------------ GetSongName ------------>
 @m retrieve song name
 *<-- OUT --> @r
 * CString & - name
 */
tstring &CMIDIFileLoader::GetSongName()
{
	return m_strSongName;
}

tstring &CMIDIFileLoader::GetCopyright()
{
	return m_strCopyright;
}

/*\
 *<------------ SetSongName ------------>
 @m set name of this song
 *--> I N <-- @p
 * CString &str - song name
 */
void CMIDIFile::SetSongName(const char *pszName)
{
	m_strSongName=pszName;
}

void CMIDIFile::SetCopyright(const char *pszName)
{
	m_strCopyright=pszName;
}

void CMIDIFileLoader::SetSongPath(const char *pszPath)
{
	m_strSongPath=pszPath;
}

tstring &CMIDIFileLoader::GetSongPath(void)
{
	return m_strSongPath;
}

/*\
 *<------------ GetSize ------------>
 @m get playtime for this song
 *<-- OUT --> @r
 * int - ???
 */
int CMIDIFileLoader::GetSize()
{
	if (m_nFirstOutTime  == 0xFFFFFFFF)
		return 0;
	else if (m_nFirstOutTime < m_nLastOutTime)
		return m_nLastOutTime-m_nFirstOutTime;
	else
		return 0;
}

/*\
 *<------------ GetChannelUsage ------------>
 @m get pointer to usage booleans
 *<-- OUT --> @r
 * BOOL * - pointer to boolean array
 */
/*
BOOL *CMIDIFileLoader::GetChannelUsage()
{
	return m_bChannelUsed;
}
*/

/*\
 *<------------ GetTempo ------------>
 @m get tempo in 6000000/BPM for this song
 *<-- OUT --> @r
 * int - tempo value
\*/
int CMIDIFileLoader::GetTempo()
{
	return m_nTempo;
}

/*\
 *<------------ GetTracks ------------>
 @m get pointer to track array
 *<-- OUT --> @r
 * CTypedPtrArray <CPtrArray, CMIDITrack *> - pointer to track array
\*/
vector<CMIDITrack *> *CMIDIFile::GetTracks()
{
	return &m_Tracks;
}

/*\
 *<------------ SetTempo ------------>
 @m set midi tempo
 *--> I N <-- @p
 * int nTempo - tempo value (60000000/BPM)
 */
void CMIDIFileLoader::SetTempo(int nTempo)
{
	m_nTempo=nTempo;
}

/*\
 *<------------ GetEvent ------------>
 @m get event from specified index
 *--> I N <-- @p
 * int iPosition - index
 *<-- OUT --> @r
 * CMIDIEvent * - pointer to event
 */
CMIDIEvent *CMIDITrack::GetEvent(int iPosition)
{
	ASSERT (iPosition < GetSize());
	return m_Events[iPosition];	
}

/*\
 *<------------ GetSize ------------>
 @m get size of event-array (tracksize)
 *<-- OUT --> @r
 * int - number of events
 */
int CMIDITrack::GetSize()
{
	return (int)m_Events.size();
}

/*\
 *<------------ GetAt ------------>
 @m get at ?
 *<-- OUT --> @r
 * unsigned int - at value ?!
 */
unsigned int CMIDIEvent::GetAt()
{
	return m_nAt;
}

/*\
 *<------------ GetSize ------------>
 @m get event size
 *<-- OUT --> @r
 * int - size in bytes
 */
int CMIDIEvent::GetSize()
{
	return m_nSize;
}

/*\
 *<------------ GetData ------------>
 @m get event data
 *<-- OUT --> @r
 * unsigned char * - pointer to event data
 */
unsigned char *CMIDIEvent::GetData()
{
	return m_pcEvent;
}

bool CMIDIEvent::IsMetaevent()
{
	return m_bMetaevent;
}

/*\
 *<------------ GetData ------------>
 @m get event data
 *<-- OUT --> @r
 * unsigned char * - pointer to event data
 */
void CMIDIEvent::ReplaceData(unsigned char *pcEvent,int nSize)
{
	if (m_pcEvent && m_nSize)
	{
		delete [] m_pcEvent;
		m_nSize=0;
	}

	ASSERT (nSize && nSize < 0xFFFF);
	ASSERT (pcEvent);
	if (nSize && pcEvent)
	{
		m_nSize=nSize;
		m_pcEvent=new unsigned char[nSize];
		ASSERT (m_pcEvent);
		CopyMemory(m_pcEvent,pcEvent,nSize);
	}
	else
		TRACEIT ("CMIDIEvent - no event created\n");
}

/*\
 *<------------ Reset ------------>
 @m kill all event data
 */
void CMIDITrack::Reset()
{
	unsigned int i;
	for (i=0;i < m_Events.size();i++)
	{
		ASSERT (m_Events[i]);
		if (m_Events[i] != NULL)
			delete m_Events[i];
	}
	if (m_Events.size())
		m_Events.erase(m_Events.begin(),m_Events.end());
}

bool CMIDIFileEdit::InsertEvent(int iTrack,CMIDIEvent *pEvent)
{
	ASSERT((unsigned int)iTrack < m_Tracks.size());
	return m_Tracks[iTrack]->InsertEvent(pEvent) == TRUE;
}

CMIDIEvent *CMIDIFileEdit::GetEvent(int iTrack,int iPosition)
{
	CMIDIEvent *pEvent=NULL;
	ASSERT((unsigned int)iTrack < m_Tracks.size());
	if ((unsigned int)iTrack < m_Tracks.size())
	{
		ASSERT(iPosition < m_Tracks[iTrack]->GetSize());
		if (iPosition < m_Tracks[iTrack]->GetSize())
			pEvent=m_Tracks[iTrack]->GetEvent(iPosition);
	}
	return pEvent;
}


/*\
 *<------------ RemoveNote ------------>
 @m remove a certain note from song
 *--> I N <-- @p
 * unsigned int nEventTime - start time
 * int iChannel - channel
 *<-- OUT --> @r
 * BOOL - TRUE=removed
 */
BOOL CMIDIFileEdit::RemoveNote(unsigned int nEventTime,int iChannel)
{
	BOOL bRet=FALSE;
#ifdef MIDISCALE
	int nNote;
	int iCommand;
	unsigned int i,o;
	int nDura=0;
	BOOL bAdapt=FALSE;

	for (i=0;i < m_Tracks.size();i++)
	{		
		for (o=0;o < m_Tracks[i]->m_Events.size();)
		{
			iCommand=m_Tracks[i]->m_Events[o]->GetData()[0];
			nNote=m_Tracks[i]->m_Events[o]->GetData()[1];

			if (iCommand == ((int)MIDI_NOTEON|iChannel) && 
				nEventTime == m_Tracks[i]->m_Events[o]->GetAt())
			{
				m_Tracks[i]->m_Events.RemoveAt(o);
				while (o < m_Tracks[i]->m_Events.size() && bRet == FALSE)
				{
					iCommand=m_Tracks[i]->m_Events[o]->GetData()[0];
					if ((iCommand == ((int)MIDI_NOTEOFF|iChannel)) ||
						(((iCommand == ((int)MIDI_NOTEON|iChannel)) &&
						  (m_Tracks[i]->m_Events[o]->GetData()[2] == 0x00))))
					{
						if (nNote == m_Tracks[i]->m_Events[o]->GetData()[1])
						{
							nDura+=m_Tracks[i]->m_Events[o]->GetAt()-nEventTime;
							bAdapt=TRUE;
							if (nDura >= 0)
							{
								m_Tracks[i]->m_Events.RemoveAt(o);
								bRet=TRUE;
							}
						}
					}
				};
			}
			else
			{
				if (bAdapt && nDura)
				{
					if (m_Tracks[i]->m_Events[o]->m_nAt >= nEventTime)
						m_Tracks[i]->m_Events[o]->m_nAt-=nDura;
				}
				++o;
			}
		}
	}
	m_nLastOutTime-=nDura;
#endif
	return bRet;
}

/*\
 *<------------ GetEventAt ------------>
 @m find an event at a certain time
 *--> I N <-- @p
 * unsigned int nQuanta - time value
 *<-- OUT --> @r
 * int - event index or -1 if none found
 */
int CMIDITrack::GetEventAt(unsigned int nQuanta)
{
	unsigned int i;
	for (i=0;(unsigned int)i < m_Events.size();i++)
	{
		ASSERT (m_Events[i]);
		if (m_Events[i] != NULL)
		{
			if (m_Events[i]->GetAt() == nQuanta)
				return i;
			else
			{
				if (m_Events[i]->GetAt() > nQuanta)
					return -1;
			}
		}
	}
	return -1;
}

/*\
 *<------------ RemoveEvent ------------>
 @m remove an event from the track
 *--> I N <-- @p
 * unsigned int nQuanta - time value
 * int iCommandMask - command mask for removal
 *<-- OUT --> @r
 * BOOL - TRUE=removed an event
 */
BOOL CMIDITrack::RemoveEvent(unsigned int nQuanta, int iCommandMask)
{
	BOOL bDone=FALSE;	
#ifdef MIDISCALE
	unsigned int i;
	for (i=0;i < m_Events.size() && !bDone;i++)
	{
		ASSERT (m_Events[i]);
		if (m_Events[i] != NULL)
		{
			if (m_Events[i]->GetAt() == nQuanta)
			{
				if ((m_Events[i]->GetData()[0] & iCommandMask) == iCommandMask)
				{
					TRACEIT ("CMIDITrack::RemoveEvent - removing event (command %d) at %d...\n",m_Events[i]->GetData()[0],i);
					m_Events.RemoveAt(i,1);
					bDone=TRUE;
				}
			}
		}
	}
#endif
	return bDone;
}

/*\
 *<------------ GetTimeScale ------------>
 @m get time scale
 *<-- OUT --> @r
 * double - time scale value
 */
double CMIDIFileLoader::GetTimeScale()
{
	return m_fTimeScale;
}

/*\
 *<------------ GetQuantaSize ------------>
 @m get quanta size
 *<-- OUT --> @r
 * int - quanta size value
 */
int CMIDIFileLoader::GetQuantaSize()
{
	return m_nQuantaSize;
}

int CMIDIFileLoader::GetBrokenChannels()
{
	return m_nBrokenChannels;
}

/*\
 *<------------ GetDivisor ------------>
 @m get timing divisor
 *<-- OUT --> @r
 * int - divisor value
 */
int CMIDIFileLoader::GetDivisor()
{
	return m_nDivision;
}

/*\
 *<------------ Transpose ------------>
 @m transpose a channel
 *--> I N <-- @p
 * int nOffset - halftone steps to transpose (+,-)
 * int iChannel - channel to transpose
 *<-- OUT --> @r
 * BOOL - TRUE=done
 */
BOOL CMIDIFileEdit::Transpose(int nOffset,int iChannel)
{
	BOOL bRet=FALSE;
	int nNote;
	int iCommand;
	unsigned int i,o,iNoteOn,s;
	int nDura=0;

	for (i=0;i < m_Tracks.size();i++)
	{		
		for (o=0;o < m_Tracks[i]->m_Events.size();o++)
		{
			iCommand=m_Tracks[i]->m_Events[o]->GetData()[0];
			nNote=m_Tracks[i]->m_Events[o]->GetData()[1];
			//is it a "note on" on our channel ?
			if (iCommand == ((int)MIDI_NOTEON|iChannel))
			{	//yes->is it a "note on" with zero volume ?
				if (m_Tracks[i]->m_Events[o]->GetData()[2] > 0x00)
				{	//nope->its a real "note on"
					iNoteOn=o;
					s=o;
					//now try to find its "note off" - also needs to be transposed!
					while (s < m_Tracks[i]->m_Events.size())
					{
						iCommand=m_Tracks[i]->m_Events[s]->GetData()[0];

						if ((iCommand == ((int)MIDI_NOTEOFF|iChannel)) ||
							(((iCommand == ((int)MIDI_NOTEON|iChannel)) &&
							  (m_Tracks[i]->m_Events[s]->GetData()[2] == 0x00))))
						{
							if (nNote == m_Tracks[i]->m_Events[s]->GetData()[1])
							{
								nNote+=nOffset;
								m_Tracks[i]->m_Events[iNoteOn]->GetData()[1]=nNote;
								m_Tracks[i]->m_Events[s]->GetData()[1]=nNote;
								bRet=TRUE;
								break;
							}
						}
						++s;
					};
				}
			}
		}
	}
	m_nLastOutTime-=nDura;
	return bRet;
}

void CMIDIFileLoader::InstrumentUsage(uint32_t nAt,unsigned char *pData)
{
	if ((pData[0]&0xF0) == MIDI_NOTEON)
	{
		if (pData[2] > 0x00)
			AddPatchToList(pData[0]&0x0F,pData[1]);	
	}
}

void CMIDIFileLoader::CalcPolyphony(uint32_t nAt,unsigned char *pData)
{
	unsigned char iCommand;
	unsigned char iChannel;
	iCommand=pData[0]&0xF0;
	iChannel=pData[0]&0x0F;
	switch (iCommand)
	{
		case MIDI_NOTEON:	
			if (pData[2] > 0x00)
				IncreasePolyphony(nAt,iChannel);
			else
				DecreasePolyphony(nAt,iChannel);				
		break;
		case MIDI_NOTEOFF:
			DecreasePolyphony(nAt,iChannel);
		break;
	}
}

void CMIDIFileLoader::SetPolySequence(uint32_t nAt,int iChannel,int nPoly)
{
	ASSERT(nPoly >= 0);
	//m_mapPolyphony[iChannel].SetAt(nAt,nPoly);
	m_mapPolyphony[iChannel][nAt]=nPoly;
}

void CMIDIFileLoader::IncreasePolyphony(uint32_t nAt,int iChannel)
{
	++m_nActivePolyphony[iChannel];

	if (m_nActivePolyphony[iChannel] > GetMaxPolyphony(iChannel))
		SetMaxPolyphony(iChannel,m_nActivePolyphony[iChannel]);
	SetPolySequence(nAt,iChannel,m_nActivePolyphony[iChannel]);
}

void CMIDIFileLoader::DecreasePolyphony(uint32_t nAt,int iChannel)
{
	if (m_nActivePolyphony[iChannel])
		--m_nActivePolyphony[iChannel];
	SetPolySequence(nAt,iChannel,m_nActivePolyphony[iChannel]);
}

CPatchSample *CMIDIFile::pGetFirstSample()
{
	m_iCurrentSample=1;
	return pGetNextSample();
}

CPatchSample *CMIDIFile::pGetNextSample()
{
	CPatchSample *pSample=NULL;
	while (m_iCurrentSample < 128 && pSample == NULL)
	{
		if (m_bUsedSample[m_iCurrentSample] == true)
			pSample=m_listSamples[m_iCurrentSample-1];
		++m_iCurrentSample;
	};	
	return pSample;
}

PatchObject *CMIDIFile::pGetFirstPatch()
{
	m_iCurrentPatch=0;
	return pGetNextPatch();
}

PatchObject *CMIDIFile::pGetNextPatch()
{
	PatchObject *pPatch=NULL;
	if (m_iCurrentPatch < (int)m_listPatches.size())
		pPatch=m_listPatches[m_iCurrentPatch++];
	return pPatch;
}

void CMIDIFile::AddPatchToList(int iChannel,unsigned char nNote)
{
	PatchObject *pPatch;
	bool bDoIt=true;
	int nPatch=0,nBank=0;
	unsigned int nCount=(unsigned int)m_listPatches.size();
	
	//channel 10 are drums and notes equal patch numbers
	if (iChannel == 0x09)
	{
		nPatch=nNote;
		nBank=15360;
	}
	else
	{
		if (m_bInstrumentSet[iChannel])
			nPatch=m_iInstrument[iChannel];
		else
			nPatch=0;
		if (m_bBankSet[iChannel][0] && m_bBankSet[iChannel][1])
			nBank=m_iBank[iChannel];
		else
			nBank=0;
		//special treat for channel 11 drums on sp-midi
		if (nBank == 15360 && iChannel == 0x0A)
			nPatch=nNote;
	}
	if (nCount > 0)
	{
		bDoIt=true;
		unsigned int i;
		for (i=0;i < nCount;i++)
		{
			if (m_listPatches[i]->nBank == nBank &&
				m_listPatches[i]->nChannel == iChannel &&
				m_listPatches[i]->nPatch == nPatch)
			{
				bDoIt=false;
				break;
			}
		}
	}
	if (bDoIt)
	{
		int nPatchBank=0,iSample,i,nCount;

		pPatch=new PatchObject();
		pPatch->nChannel=iChannel;
		pPatch->nBank=nBank;
		pPatch->nPatch=nPatch;

		if (nBank == 15360 && (iChannel == 0x09 || iChannel == 0x0A))
			nPatchBank=1;

		//get mapped patch
		pPatch->nMappedPatch=GetMappedInstrument(nPatch,nPatchBank);
		if (nBank == 15494 && nPatch == 125)
			pPatch->anSamples[0]=0;
		else
		{
			//get instrument samples
			nCount=GetInstrumentSamples(nPatchBank,pPatch->nMappedPatch,&(pPatch->nSize),pPatch->anSamples);
			for (i=0;i < nCount;i++)
			{
				iSample=pPatch->anSamples[i];
				//mark sample as being used
				m_bUsedSample[iSample]=true;
			}
		}
		m_listPatches.push_back(pPatch);
	}
}

int CMIDIFile::GetInstrumentSamples (int iPatchBank, int nInstrument, int *nSize, int *piSamples)
{
	InstrumentSampleMap mapMelodicSamples[29]=
	{
		{0,	 3151, {18,19,20,21,22}},		//"Acoustic Grand"
		{9,	 1251, {13}},					//"Glockenspiel"
		{11, 1284, {13}},					//"Vibraphone"
		{12, 1251, {13}},					//"Marimba"
		{16, 1175, {14,15,16,17}},			//"Drawbar Organ"}
		{27, 2408, {9,7,8}},				//"Electric Guitar(clean)"}
		{33,  979, {1}},					//"Electric Bass(finger)"}
		{40,  803, {27,28}},				//"Violin"}
		{48, 9341, {29,30}},				//"String Ensemble 1"}
		{49, 9341, {29,30}},				//"String Ensemble 2"},
		{56, 2772, {33,34,37,38}},			//"Trumpet"}, 
		{68, 1308, {39,40,41,42}},			//"Oboe"}, 
		{73, 2556, {6,5}},					//"Flute"}, 
		{78,  432, {24}},					//"Whistle"}, 
		{80, 3123, {31}},					//"Lead 1 (square)"}, 
		{89, 1556, {6,5}},					//"Pad 2 (warm)"}, 
		{99, 1556, {6,5}},					//"FX 4 (atmosphere)"}, 
		{114,2251, {26}},					//"Steel Drums"}, 
		{115, 388, {2}},					//"Woodblock"}, 
		{117,1440, {36}},					//"Melodic Tom"}, 
		{119,4518, {23}},					//"Reverse Cymbal"}, 
		{120,1331, {36}},					//"Guitar Fret Noise"},
		{121,1438, {6,5}},					//"Breath Noise"}, 
		{122,4612, {23}},					//"Seashore"}, 
		{123, 545, {24}},					//"Bird Tweet"}, 
		{124,2470, {35}},					//"Telephone Ring"}, 
		{125,4563, {23}},					//"Helicopter"}, 
		{126,4657, {23}},					//"Applause"}, 
		{127,2802, {25}}					//"Gunshot"}, 
	};
	InstrumentSampleMap mapPercussionSamples[14]=
	{
		{36,1028, {11}},					//C2,"Kick"},
		{37, 364, {2}},						//C#2,"Rimclick"},
		{38,2778, {25}},					//D2,"Snare"},
		{42,4734, {10}},					//F#2,"Hi Hat"},
		{45,1440, {36}},					//A2,"Mid Tom"},
		{46,4758, {10}},					//A#2,"Open Hi Hat"},
		{48,1440, {36}},					//C3,"High Tom"},
		{49,4518, {23}},					//C#3,"Crash Cymbal"},
		{51,4530, {23}},					//D#3,"Ride Cymbal"},
		{54,2256, {32}},					//F#3,"Tambourine"},
		{57,4575, {23}},					//A3,"Crash Cymbal2"},
		{62, 578, {3}},						//D4,"Hi Mute Conga"},
		{64,1898, {4}},						//E4,"Low Conga"},
		{82, 946, {12}}						//A#5,"Shaker"},
	};
	InstrumentSampleMap *pMap=NULL;
	int nCount,i,o;

	if (iPatchBank == 0)
	{
		pMap=mapMelodicSamples;
		nCount=29;
	}
	else
	{
		pMap=mapPercussionSamples;
		nCount=14;
	}

	for (i=0;i < nCount;i++)
	{
		if (pMap->nPatch == nInstrument)
		{
			for (o=0;pMap->aiSamples[o] > 0;o++)
			{
				*piSamples=pMap->aiSamples[o];
				*nSize=pMap->nSize;
				++piSamples;
			}
			return o;
		}
		++pMap;
	}
	return 0;
}

void CMIDIFile::SetChannelBank(int iIndex,int iBank,int iPart)
{
	if (iPart == 0)
		m_iBank[iIndex]=(m_iBank[iIndex]&0xFFFFFF80)|iBank;
	else
		m_iBank[iIndex]=(m_iBank[iIndex]&0x0000007F)|(iBank << 7);
	m_bBankSet[iIndex][iPart]=true;
}

void CMIDIFile::SetChannelInstrument(int iIndex,unsigned char iInstrument)
{
	//if (!m_bInstrumentSet[iIndex])
	{
		m_iInstrument[iIndex]=iInstrument;
		m_bInstrumentSet[iIndex]=true;
	}
}

unsigned char CMIDIFile::GetChannelInstrument(int iIndex)
{
	return m_iInstrument[iIndex];
}

#ifdef RESOURCES_INCLUDED
tstring CMIDIFile::sGetFormatName(int nFormat)
{
	CMyString strText;
	//LoadString(GetModuleHandle(NULL),IDS_FORMATNAME_MIDI+nFormat,szText, 256);
	strText.Load(IDS_FORMATNAME_MIDI+nFormat);
	return strText.c_str();
}
#endif
int CMIDIFile::GetChannelPriority(int iChannel)
{
	return m_iChannelPriority[iChannel&0x0F];
}

int CMIDIFile::GetPresetMaxPolyphony(int iChannel)
{
	return m_nPresetMaxPolyphony[iChannel&0x0F];
}
/*
void CMIDIFile::MipLocation(int iTrack, int iPosition)
{
	m_iMIPTrack=iTrack;
	m_iMIPPosition=iPosition;
}
*/
void CMIDIFile::CalcSequencePolyphony(void)
{
	uint32_t iChannel;
	uint32_t i;
	uint32_t nMaxLevel=0;

	for (i=0;i < (uint32_t)m_nLastOutTime;i++)
	{
		uint32_t nSumLevel=0;
		uint32_t iEventsFound=0;
		for (iChannel=0;iChannel < 16;iChannel++)
		{
			if (IsChannelPlaybackActive(iChannel))
			{
				if (m_mapPolyphony[iChannel].size())
				{
					map<uint32_t,int>::iterator iter;
					//if (m_mapPolyphony[iChannel].Lookup(i,nLevel))
					if ((iter=m_mapPolyphony[iChannel].find(i)) != m_mapPolyphony[iChannel].end())
					{
						nSumLevel+=iter->second;
						iEventsFound++;
					}
				}
			}
		}
		if (iEventsFound)
		{
			if (nMaxLevel < nSumLevel)
				nMaxLevel=nSumLevel;
		}
	}
	m_nSequencePolyphony=nMaxLevel;
}

BOOL CMIDIFileEdit::DuplicateChannel(int iChannel,int iDestChannel)
{
	BOOL bRet=FALSE;
	int iCommand;
	unsigned int i,o;

	for (i=0;i < m_Tracks.size();i++)
	{		
		for (o=0;o < m_Tracks[i]->m_Events.size();o++)
		{
			iCommand=m_Tracks[i]->m_Events[o]->GetData()[0];
			if ((iCommand&0x0F) == iChannel)
			{
				CMIDIEvent *pEvent=new CMIDIEvent(	m_Tracks[i]->m_Events[o]->GetAt(),
													m_Tracks[i]->m_Events[o]->GetData(),
													m_Tracks[i]->m_Events[o]->GetSize());
				pEvent->GetData()[0]=(pEvent->GetData()[0]&0xF0)|iDestChannel;
				m_Tracks[i]->InsertEvent(pEvent);
				SetChannelUsed(iDestChannel,TRUE);
				++o;
				bRet=TRUE;				
			}
		}
	}
	SetChannelInstrument(iDestChannel,GetChannelInstrument(iChannel));
	m_iBank[iDestChannel]=m_iBank[iChannel];
	m_bBankSet[iDestChannel][0]=m_bBankSet[iChannel][0];
	m_bBankSet[iDestChannel][1]=m_bBankSet[iChannel][1];
	SetMaxPolyphony(iDestChannel,GetMaxPolyphony(iChannel));
	SetPresetMaxPolyphony(iDestChannel,GetPresetMaxPolyphony(iChannel));
	SetChannelPriority(iDestChannel,GetChannelCount());

	return bRet;
}

BOOL CMIDIFileEdit::RemoveChannel(int iChannel)
{
	BOOL bRet=FALSE;
#ifdef MIDISCALE
	int iCommand;
	unsigned int i,o;
	int nDura=0;
	BOOL bAdapt=FALSE;

	for (i=0;i < m_Tracks.size();i++)
	{		
		for (o=0;o < m_Tracks[i]->m_Events.size();o++)
		{
			iCommand=m_Tracks[i]->m_Events[o]->GetData()[0];
			if ((iCommand&0x0F) == iChannel)
			{
				m_Tracks[i]->m_Events.RemoveAt(o,1);
				//m_Tracks[i]->m_Events.erase();
				--o;
				bRet=TRUE;
			}
		}
	}
#endif
	return bRet;
}

int CMIDIFile::GetMappedInstrument(int nInstrument,int iMap)
{
	//according to Beatnik Bank...
	const int listSPMIDIPercussionMap[53]=
	{
		36, //35 ->Kick
		36, //36 Kick
		37, //37 Rimclick
		38, //38 Snare
		54, //39 ->Tambourine
		38, //40 ->Snare
		45, //41 ->Mid Tom
		42, //42 Hi Hat
		45, //43 ->Mid Tom
		42, //44 ->Hi Hat
		45, //45 Mid Tom
		46, //46 Open Hi Hat
		48, //47 ->High Tom
		48, //48 High Tom
		49, //49 Crash Cymbal
		48, //48 ->High Tom
		51, //51 Ride Cymbal
		51, //52 ->Ride Cymbal
		51, //53 ->Ride Cymbal
		54, //54 Tambourine
		46, //55 ->Open Hi Hat
		37, //56 ->Rimclick
		57, //57 Crash Cymbal 2
		46, //58 ->Open Hi Hat
		51, //59 ->Ride Cymbal
		62, //60 ->Hi Mute Conga
		64, //61 ->Low Conga
		62, //62 Hi Mute Conga
		64, //63 ->Low Conga
		64, //64 Low Conga
		62, //65 ->Hi Mute Conga
		64, //66 ->Low Conga
		62, //67 ->Hi Mute Conga
		64, //68 ->Low Conga
		82, //69 ->Shaker
		82, //70 ->Shaker
		42, //71 ->Hi Hat
		46, //72 ->Open Hi Hat
		82, //73 ->Shaker
		46, //74 ->Open Hi Hat
		37, //75 ->Rimclick
		37, //76 ->Rimclick
		37, //77 ->Rimclick
		62, //78 ->Hi Mute Conga
		64, //79 ->Low Conga
		42, //80 ->Hi Hat
		82, //81 ->Shaker
		82, //82 Shaker
		42, //83 ->Hi Hat
		46, //84 ->Open Hi Hat
		37, //85 ->Rimclick
		62, //86 ->Hi Mute Conga
		64	//87 ->Low Conga
	};

	//according to MIDI in Nokia Phones...
	/*
	const int listSPMIDIPercussionMap[128]=
	{
		36, //35 Acoustic Bass Drum -> 36 Bass Drum 1
		36, //36 Bass Drum 1 -> 36 Bass Drum 1
		75, //37 Side Stick -> 75 Claves
		40, //38 Acoustic Snare -> 40 Electric Snare
		45, //39 Hand Clap -> 45 Low Tom
		40, //40 Electric Snare -> 40 Electric Snare
		45, //41 Low Floor Tom -> 45 Low Tom
		42, //42 Closed Hi Hat -> 42 Closed Hi Hat
		45, //43 High Floor Tom -> 45 Low Tom
		42, //44 Pedal Hi-Hat -> 42 Closed Hi Hat
		45, //45 Low Tom -> 45 Low Tom
		46, //46 Open Hi-Hat -> 46 Open Hi-Hat
		48, //47 Low-Mid Tom -> 48 Hi Mid Tom
		48, //48 Hi Mid Tom -> 48 Hi Mid Tom
		49, //49 Crash Cymbal 1 -> 49 Crash Cymbal 1
		48, //50 High Tom -> 48 Hi Mid Tom
		51, //51 Ride Cymbal 1 -> 51 Ride Cymbal 1
		51, //52 Chinese Cymbal -> 51 Ride Cymbal 1
		51, //53 Ride Bell -> 51 Ride Cymbal 1
		54, //54 Tambourine -> 54 Tambourine
		46, //55 Spash Cymbal -> 46 Open Hi-Hat
		75, //56 Cowbell -> 75 Claves
		57, //57 Crash Cymbal 2 -> 57 Crash Cymbal 2
		46, //58 Vibraslap -> 46 Open Hi-Hat
		51, //59 Ride Cymbal 2 -> 51 Ride Cymbal 1
		62, //60 Hi Bongo -> 62 Mute Hi Conga
		64, //61 Low Bongo -> 64 Low Conga
		62, //62 Mute Hi Conga -> 62 Mute Hi Conga
		64, //63 Open Hi Conga -> 64 Low Conga
		64, //64 Low Conga -> 64 Low Conga 
		62, //65 High Timbale -> 62 Mute Hi Conga
		64, //66 Low Timbale -> 64 Low Conga 
		62, //67 High Agogo -> 62 Mute Hi Conga 
		64, //68 Low Agogo -> 64 Low Conga 
		70, //69 Cabasa -> 70 Maracas 
		70, //70 Maracas -> 70 Maracas 
		42, //71 Short Whistle -> 42 Closed Hi Hat 
		46, //72 Long Whistle -> 46 Open Hi-Hat 
		70, //73 Short Guiro -> 70 Maracas 
		46, //74 Long Guiro -> 46 Open Hi-Hat 
		75, //75 Claves -> 75 Claves 
		75, //76 Hi Wood Block -> 75 Claves 
		75, //77 Low Wood Block -> 75 Claves 
		62, //78 Mute Cuica -> 62 Mute Hi Conga 
		64, //79 Open Cuica -> 64 Low Conga 
		42, //80 Mute Triangle -> 42 Closed Hi Hat 
		70  //81 Open Triangle -> 70 Maracas 
	};
	*/
	//according to Beatnik Bank...
	const int listSPMIDIInstrumentMap[128]=
	{
		0,	//0  Acoustic Grand
		0,	//1  ->Acoustic Grand
		0,	//2  ->Acoustic Grand
		0,	//3  ->Acoustic Grand
		0,	//4  ->Acoustic Grand
		0,	//5  ->Acoustic Grand
		0,	//6  ->Acoustic Grand
		0,	//7  ->Acoustic Grand
		11,	//8  ->Vibraphone
		9,	//9  Glockenspiel
		9,	//10 ->Glockenspiel
		11,	//11 Vibraphone
		12, //12 Marimba
		12, //13 ->Marimba
		9,	//14 ->Glockenspiel
		11,	//15 ->Vibraphone
		16, //16 Organ
		16, //17 ->Organ
		16, //18 ->Organ
		16, //19 ->Organ
		16, //20 ->Organ
		16, //21 ->Organ
		16, //22 ->Organ
		16, //23 ->Organ
		27, //24 ->Guitar
		27, //25 ->Guitar
		27, //26 ->Guitar
		27, //27 Guitar
		27, //28 ->Guitar
		27, //29 ->Guitar
		27, //30 ->Guitar
		27, //31 ->Guitar
		33, //32 ->Bass
		33, //33 Bass
		33, //34 ->Bass
		33, //35 ->Bass
		33, //36 ->Bass
		33, //37 ->Bass
		33, //38 ->Bass
		33, //39 ->Bass
		40, //40 Violin
		40, //41 ->Violin
		40, //42 ->Violin
		40, //43 ->Violin
		48, //44 ->Strings
		12, //45 ->Marimba
		9,  //46 ->Glockenspiel
		114,//47 ->Steeldrum
		48, //48 Strings
		49, //49 Slow Strings
		49, //50 ->Slow Strings
		49, //51 ->Slow Strings
		89, //52 ->Warm Pad
		89, //53 ->Warm Pad
		89, //54 ->Warm Pad
		114,//55 ->Steeldrum
		56, //56 Solo Brass
		56, //57 ->Solo Brass
		56, //58 ->Solo Brass
		56, //59 ->Solo Brass
		56, //60 ->Solo Brass
		56, //61 ->Solo Brass
		56, //62 ->Solo Brass
		56, //63 ->Solo Brass
		68, //64 ->Woodwind
		68, //65 ->Woodwind
		68, //66 ->Woodwind
		68, //67 ->Woodwind
		68, //68 Woodwind
		68, //69 ->Woodwind
		68, //70 ->Woodwind
		68, //71 ->Woodwind
		73, //72 ->Flutes
		73, //73 Flutes
		78, //74 ->Whistle
		73, //75 ->Flutes
		73, //76 ->Flutes
		73, //77 ->Flutes
		78, //78 Whistle
		78, //79 ->Whistle
		80, //80 Lead Synth
		80, //81 ->Lead Synth
		80, //82 ->Lead Synth
		80, //83 ->Lead Synth
		80, //84 ->Lead Synth
		80, //85 ->Lead Synth
		80, //86 ->Lead Synth
		80, //87 ->Lead Synth
		89, //88 ->Warm Pad
		89, //89 Warm Pad
		99, //90 ->Athmosphere
		89, //91 ->Warm Pad
		89, //92 ->Warm Pad
		89, //93 ->Warm Pad
		89, //94 ->Warm Pad
		89, //95 ->Warm Pad
		99, //96 ->Athmosphere
		89, //97 ->Warm Pad
		9,	//98 ->Glockenspiel
		99, //99 Athmosphere
		99, //100 ->Athmosphere
		89, //101 ->Warm Pad
		89, //102 ->Warm Pad
		99, //103 ->Athmosphere
		27, //104 ->Guitar
		27, //105 ->Guitar
		27, //106 ->Guitar
		27, //107 ->Guitar
		12, //108 ->Marimba
		16, //109 ->Organ
		40, //110 ->Violin
		68,	//111 ->Woodwind
		114,//112 ->Steeldrum
		115,//113 ->Woodblock
		114,//114 Steeldrum
		115,//115 Woodblock
		117,//116 ->Melodic Tom
		117,//117 Melodic Tom
		117,//118 ->Melodic Tom
		119,//119 Reverse Cymbal
		120,//120 Guitar Fret Noise
		121,//121 Breath Noise
		122,//122 Seashore
		123,//123 Bird
		124,//124 Telephone
		125,//125 Helicopter
		126,//126 Applause
		127 //127 Gunshot
	};

	//according to MIDI in Nokia Phones...
	/*
	const int listSPMIDIInstrumentMap[128]=
	{
		0,	//Acoustic Grand
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		9,	//Glockenspiel
		9,			
		11,	//Vibraphone
		12,	//Marimba
		12,
		9,
		11,		
		16,	//Drawbar Organ
		16,
		16,
		16,
		16,
		16,
		16,
		16,
		27,	//Electric Guitar (clean)
		27,
		27,
		27,
		27,
		27,
		27,
		27,
		33,	//Electric Bass(finger)
		33,
		33,
		33,
		33,
		33,
		33,
		33,
		40,	//Violin
		40,
		40,
		40,
		48,	
		12,
		9,
		114,	//Steel Drums
		48,	//String Ensemble 1
		49,	//String Ensemble 2
		49,
		49,
		89,	
		89,
		89,
		114,	
		56,	//Trumpet
		56,
		56,
		56,
		56,
		56,
		56,
		56,
		66,	//Tenor Sax
		66,
		66,
		66,
		66,
		66,
		66,
		66,
		73,	//Flute
		73,
		78,	//Whistle
		73,
		73,
		73,
		78,
		33,
		81,	//Lead 2 (sawtooth)
		81,
		81,
		81,
		81,
		81,
		81,
		81,
		89,	//Pad 2 (warm)
		89,
		99,	//FX 4 (atmosphere)
		89,
		89,
		89,
		89,
		89,
		89,
		89,
		9,
		99,
		99,
		89,
		89,
		99,
		27,
		27,
		27,
		27,
		12,
		16,
		40,
		66,
		114,
		115,
		114,
		115,
		117,
		117,
		117,
		119,	//Reverse Cymbal
		120,	//Guitar Fret Noise
		121,	//Breath Noise
		122,
		123,
		124,
		125,
		126,
		127
	};
	*/
	if (iMap == 0)
	{
		TRACEIT("patched instrument %d -> %d\n",nInstrument,listSPMIDIInstrumentMap[nInstrument]);
		nInstrument=listSPMIDIInstrumentMap[nInstrument];
	}
	if (iMap == 1)
	{
		if (nInstrument >= 35 && nInstrument <= 87)
		{
			TRACEIT("patched instrument %d -> %d\n",nInstrument,listSPMIDIPercussionMap[nInstrument-35]);
			nInstrument=listSPMIDIPercussionMap[nInstrument-35];
		}
	}
	return nInstrument;
}

void CMIDIFile::UpdateChannelMips()
{
	int iChannel,iPrio,nUsedVoices=0;
	for (iPrio=1;iPrio <= 16;iPrio++)
	{
		for (iChannel=0;iChannel < 16;iChannel++)
		{
			if (IsChannelPlaybackActive(iChannel))
			{
				if (IsChannelUsed(iChannel) && iPrio == m_iChannelPriority[iChannel])
				{
					if (m_iVibraChannel != iChannel)
						nUsedVoices+=m_nPresetMaxPolyphony[iChannel];
					m_nMIPValue[iChannel]=nUsedVoices;
				}
			}
			else
				m_nMIPValue[iChannel]=0;
		}
	}
}

void CMIDIFileEdit::UpdateVibra(void)
{
}

CMIDIEvent *CMIDIFileEdit::LocateMIP(void)
{
	unsigned int i,o;
	for (i=0;i < m_Tracks.size();i++)
	{		
		for (o=0;o < m_Tracks[i]->m_Events.size();o++)
		{
			unsigned char iCommand=m_Tracks[i]->m_Events[o]->GetData()[0] & 0xF0;
			if (iCommand == MIDI_SYSEXCL)
			{
				if (m_Tracks[i]->m_Events[o]->GetSize() > 7 &&
					m_Tracks[i]->m_Events[o]->GetData()[1] == 0x7F)
				{
					if (m_Tracks[i]->m_Events[o]->GetData()[3] == 0x0B &&	//S1 sub-ID#1 = 0B (Scalable Polyphony MIDI) 
						m_Tracks[i]->m_Events[o]->GetData()[4] == 0x01)		//S2 sub-ID#2 = 01 (MIP Message) 
					{
						return m_Tracks[i]->m_Events[o];
					}
				}
			}
		}
	}
	return NULL;
}

void CMIDIFileEdit::UpdateMip(void)
{
	CMIDIEvent *pEvent;
	if (ContainedMip())
	{
		pEvent=LocateMIP();
		//GetEvent(GetMipTrack(),GetMipPosition());

		int nSize=0;
		unsigned char *pData=NULL;
		RenderMip(&pData,&nSize);
		if (nSize)
		{
			pEvent->ReplaceData(pData,nSize);
			delete [] pData;
		}
	}
	else
	{
		pEvent=GenerateMip();
		InsertEvent(0,pEvent);
		//MipLocation(0,0);
	}
}

/*\
 *<------------ RenderMip ------------>
 @m TODO: put function description here
 *--> I N <-- @p
 * UBYTE *pcEvent - 
 * int nEventSize - 
 *<-- OUT --> @r
 * bool - 
 */
bool CMIDIFileEdit::RenderMip(unsigned char **pcData,int *pnEventSize)
{
	unsigned char *pcEvent;
	int nSize,iChannel,iPrio,nUsedVoices=0,iUsedChannel=0;

	nSize=(GetChannelCount()*2)+6;
	pcEvent=new unsigned char[nSize];
	pcEvent[0]=MIDI_SYSEXCL;
	pcEvent[1]=0x7F;
	pcEvent[2]=0x7F;
	pcEvent[3]=0x0B;
	pcEvent[4]=0x01;
	for (iPrio=1;iPrio <= 16;iPrio++)
	{
		for (iChannel=0;iChannel < 16;iChannel++)
		{
			if (IsChannelPlaybackActive(iChannel))
			{
				if (IsChannelUsed(iChannel) && iPrio == m_iChannelPriority[iChannel])
				{
					if (m_iVibraChannel != iChannel)
						nUsedVoices+=m_nPresetMaxPolyphony[iChannel];
					pcEvent[5+(iUsedChannel*2)]=iChannel;
					pcEvent[6+(iUsedChannel*2)]=nUsedVoices;
					iUsedChannel++;
				}
			}
		}
	}
	pcEvent[nSize-1]=MIDI_EOX;

	*pcData=pcEvent;
	*pnEventSize=nSize;
	return true;
}

CMIDIEvent *CMIDIFileEdit::GenerateMip(void)
{
	int nEventSize;
	unsigned char *pcEvent=NULL;
	CMIDIEvent *pEvent;
	
	RenderMip(&pcEvent,&nEventSize);
	
	pEvent=new CMIDIEvent(0,pcEvent,nEventSize);
	return pEvent;
}

/*\
 *<------------ GenerateVibra ------------>
 @m TODO: put function description here
 *--> I N <-- @p
 * int iChannel - 
 * CMIDIEvent **pEvent - 
 */
void CMIDIFileEdit::GenerateVibra(uint32_t dwAt,int iChannel,CMIDIEvent **pEvent,int *pnEventCount)
{
	ASSERT(pnEventCount);
	ASSERT((iChannel&0x0F) == iChannel);
	unsigned char pcEvent[5];

	*pnEventCount=0;

	pcEvent[0]=MIDI_CONTROL|iChannel;							//
	pcEvent[1]=0x07;											//volume change to small value
	pcEvent[2]=0x01;											//
	pEvent[(*pnEventCount)++]=new CMIDIEvent(dwAt,pcEvent,3);		//

	pcEvent[0]=MIDI_CONTROL|iChannel;							//
	pcEvent[1]=0x00;											//
	pcEvent[2]=0x79;											//
	pEvent[(*pnEventCount)++]=new CMIDIEvent(dwAt,pcEvent,3);		//

	pcEvent[0]=MIDI_CONTROL|iChannel;		//
	pcEvent[1]=0x20;						//
	pcEvent[2]=0x06;						//
	pEvent[(*pnEventCount)++]=new CMIDIEvent(dwAt,pcEvent,3);	//

	pcEvent[0]=MIDI_PROGRAM|iChannel;		//
	pcEvent[1]=0x7D;						//
	pEvent[(*pnEventCount)++]=new CMIDIEvent(dwAt,pcEvent,2);	//
}

CPatchSample::CPatchSample(int nIndex,const char *szName,int nSize)
{
	CPatchSample::nIndex=nIndex;
	CPatchSample::nSize=nSize;
	if (szName != NULL)
		strcpy(CPatchSample::szName,szName);
	else
		strcpy(CPatchSample::szName,"<unknown>");
}

CPatchSample::~CPatchSample()
{
}
