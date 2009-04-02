#pragma once
#include "DMusic.h"
#include "MIDIFileDoc.h"

class CMIDIPlayer : public CMIDIFileDoc
{
public:
	CMIDIPlayer();
	~CMIDIPlayer();

	bool Play(LPVOID pMem,int iLen);
	void Stop(void);
	bool IsPlaying(void);
	bool GetPlaytime(DWORD *pdwLength,DWORD *pdwCurrent);

protected:
	bool m_bStarted;
	//the MIDI in memory (Music-Segment)
	IDirectMusicSegment *m_pMidi;
	IDirectMusicSegmentState *m_pSegState;
};
