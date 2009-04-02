#if !defined(AFX_MIDIFILE_H__85E42880_8776_11D4_90DB_DCF680CEAB7A__INCLUDED_)
#define AFX_MIDIFILE_H__85E42880_8776_11D4_90DB_DCF680CEAB7A__INCLUDED_
#include "Basics.h"
#include <map>
#include <vector>
#include <iostream>
#include "MyString.h"

//typedef unsigned char UBYTE;

#define DESCALEQUANTA(quanta) ((int) ( ((double)quanta/m_fTimeScale) + 0.5) )
#define SCALEQUANTA(quanta) ((int)(0.5+(m_fTimeScale * (double)quanta)))

class CMIDITrack;


typedef struct _ReadStruct
{
	int                 quanta;
   	bool 				bMetaEvent;
	int                 nData;
	unsigned char       *data;
}ReadStruct;


/*

	attribute channel

	attribute quanta
	attribute polyphony
	
*/


#ifndef RETROBASE_EXPORTS
#undef DllExport
#define DllExport 
#else
#undef DllExport
#define DllExport   __declspec( dllexport )
#endif

DllExport typedef map<uint32_t,int> CMapQuantaToPoly;


class CMIDIEvent
{
public:
	DllExport CMIDIEvent();
	DllExport CMIDIEvent(CMIDIEvent &src);
	DllExport CMIDIEvent(int nAt,unsigned char *pcEvent,int nSize,bool bMeta=false);
	DllExport ~CMIDIEvent();

	DllExport bool IsMetaevent();
	DllExport unsigned char * GetData ();
	DllExport int GetSize ();
	DllExport unsigned int GetAt ();
	DllExport void ReplaceData(unsigned char *pcEvent,int nSize);

protected:
	unsigned int	m_nAt;
	unsigned char *m_pcEvent;
	bool	m_bMetaevent;
	int		m_nSize;
	friend class CMIDIFileEdit; 
	friend class CMIDIConverter;
};

class CMIDITrack
{
public:
	DllExport CMIDITrack();
	DllExport CMIDITrack(CMIDITrack &rc);
	DllExport ~CMIDITrack();

	DllExport BOOL RemoveEvent (unsigned int nQuanta,int iCommandMask);
	DllExport void Reset ();
	DllExport int GetSize ();
	DllExport CMIDIEvent *GetEvent (int iPosition);

	DllExport BOOL AddEvent(ReadStruct *pRS);
	DllExport BOOL InsertEvent(ReadStruct *pRS);
	DllExport BOOL InsertEvent(CMIDIEvent *pEvent);
	DllExport int GetEventAt(unsigned int nQuanta);

protected:
	tstring m_strName;
	uint32_t m_nLastQuanta;
	//CTypedPtrArray <CPtrArray, CMIDIEvent *> m_Events;
	typedef vector<CMIDIEvent *> CEventArray;
	typedef vector<CMIDIEvent *>::iterator iterEventArray;
	CEventArray m_Events;

	friend class CMIDIFileLoader; 
	friend class CSPMIDIFileLoader; 
	friend class CMIDIFileEdit;
	friend class CMIDIConverter;
};

typedef struct tagPATCHOBJECT
{
	int nChannel;
	int nPatch;
	int nMappedPatch;
	int nBank;
	char szName[64];
	int nSize;
	int anSamples[16];
}PatchObject;

typedef vector<PatchObject *> CPatchArray;

class CPatchSample
{
public:
	DllExport CPatchSample(int nIndex=0,const char *szName=NULL,int nSize=0);
	DllExport ~CPatchSample();

	int nIndex;
	char szName[64];
	int nSize;
};

typedef vector<CPatchSample *> CSampleArray;

class CMIDIFile
{
public:
	DllExport CMIDIFile();
	DllExport ~CMIDIFile();
		
	struct InstrumentMap
	{
		int iInstrument;
		const char *pszName;
	};

	void Init(void);
	void Reset(void);

	DllExport vector<CMIDITrack *> *GetTracks();
	DllExport void SetChannelUsed(int iIndex,bool bUsed);
	DllExport uint32_t GetPlaytime(void);
	DllExport static tstring sGetFormatName(int nFormat);
	DllExport tstring GetInstrumentName(int nBank,int iInstrument,int iChannel);
	DllExport PatchObject *pGetFirstPatch();
	DllExport PatchObject *pGetNextPatch();
	DllExport CPatchSample *pGetFirstSample();
	DllExport CPatchSample *pGetNextSample();

	bool IsChannelUsed(int iChannel);
	int GetChannelCount();

	int GetInstrumentSamples (int iPatchBank, int nInstrument, int *nSize, int *piSamples);
	int GetFormat(void){return m_nFormat+1;};


	void SetCopyright(const char *pszCopy);
	DllExport void SetSongName(const char *pszName);

	void SetChannelPriority(int iChannel,int iPrio);
	int GetChannelPriority(int iChannel);

	void AddPatchToList(int iChannel,unsigned char nNote);

	bool ContainedMip(void){return m_bContainedMIP;};
	bool ContainedVibra(void){return m_iVibraChannel != -1;};
	bool IsDP2Vibra(void){return m_nVibraMode == 2;};
	bool ContainedExtraPerc(void){return m_bAdditionalPercussion;};

	void SetVibraMode(int nMode) {m_nVibraMode=nMode;};
	int GetMaxPolyphony(int iChannel){	return m_nMaxPolyphony[iChannel];};
	int GetSequencePolyphony(){	return m_nSequencePolyphony;};
	void SetMaxPolyphony(int iChannel,int nLevel){	m_nMaxPolyphony[iChannel]=nLevel;};
	void SetChannelPlayback(int iChannel,bool bOn){m_bPlayChannel[iChannel&0x0F]=bOn;};
	bool IsChannelPlaybackActive(int iChannel){return m_bPlayChannel[iChannel&0x0F];};	

	//void MipLocation (int iTrack,int iPosition);
	void SetChannelInstrument(int iIndex,unsigned char iInstrument);
	void SetChannelBank(int iIndex,int iBank,int iPart);
	unsigned char GetChannelInstrument(int iIndex);
	int GetChannelBank(int iIndex){return m_iBank[iIndex];};
	
	int GetMappedInstrument(int nInstrument,int iMap);

	void SetPresetMaxPolyphony(int iChannel,int nMip);
	int GetPresetMaxPolyphony(int iChannel);

	void CalcSequencePolyphony(void);
	void UpdateChannelMips();
	int GetChannelMIPValue(int iChannel){return m_nMIPValue[iChannel];};

	enum {  unrecognized=-1,endOfStream=0,ok=1,undefined,   //normal event
			firstISysExcl,middleISysExcl,endISysExcl,       //multi-packet sys excl
			sysExcl,fileCorrupt,internalError};				//single-packet sys excl

	typedef enum _MetaEvent
	{
		seqNumber = 0,			//data[1] and data[2] contain high and low order bits of number
		text = 1,				//data[1]* specifies null-terminated text
		copyright = 2,			//
		sequenceOrTrackName,	//
		instrumentName,			//
		lyric,					//
		songName,				//
		cuePoint,				//should be implemented by midifile.c
		channelPrefix,			//parameters ???
		/* Track change metaevent: data[1] and data[2] contain high/low order bits,
		 * respectively, containing the track number. These events can only be
		 * encountered when reading a level-1 file. */
		trackChange,			//
		tempoChange,			//tempo change metaevent: data[1:4] contain 4 bytes of data
		smpteOffset,			//data[1:5] are the 5 numbers hr mn sec fr ff
		timeSig,				//data is a single int, where 1-byte fields are nn dd cc bb
		keySig,					//data is a single short, where 1-byte fields are sf mi
		seqMeta					//
	}Metaevent;

	typedef struct tagInstrumentSampleMap
	{
		const int nPatch;
		const int nSize;
		const int aiSamples[16];
	}InstrumentSampleMap;

	//CTypedPtrArray <CPtrArray, CMIDITrack *> m_Tracks;	//midi tracks
	CMapQuantaToPoly m_mapPolyphony[16];		

	//CTypedPtrArray <CPtrArray, CMIDITrack *> m_Tracks;	//midi tracks
	vector<CMIDITrack *> m_Tracks;
	int				m_nFormat;							//format level
	unsigned int	m_nFirstOutTime;					//first audible event time
	unsigned int	m_nLastOutTime;						//last event time
	int				m_nDivision;						//# of delta-time ticks per quarter
	int				m_nQuantaSize;						//in micro-seconds.
	int				m_nTempo;							//60000000/m_nTempo = tempo in BPM
	double			m_fTimeScale;						//timeScale * currentTime gives time in seconds	
	int				m_iCurrentTrack;					//
	tstring			m_strSongName;						//name of our song
	tstring			m_strCopyright;

	tstring			m_strSongPath;

	bool			m_bDidTitle;	
	int				m_nCurrentTime;				//current time in quanta.
	unsigned char	m_cRunningStatus;			//
    ReadStruct		m_ReadInfo;					//info for current event

	int m_iTrackPos[64];

	bool			m_bChannelUsed[16];
	int				m_nMaxPolyphony[16];
	int				m_nActivePolyphony[16];
	bool			m_bInstrumentSet[16];
	int				m_iInstrument[16];
	bool			m_bBankSet[16][2];
	int				m_iBank[16];
	int				m_nEventCount[16];
	int				m_nVelocities[16];
	int				m_nVolume[16];
	int				m_nMaxVolume[16];

	CPatchArray m_listPatches;
	CSampleArray m_listSamples;
	bool m_bUsedSample[128];

	int	m_nMIPValue[16];

	bool m_bContainedMIP;
    //int m_iMIPTrack;
	//int m_iMIPPosition;
	int	m_iChannelPriority[16];
	int	m_nPresetMaxPolyphony[16]; 
	bool m_bPlayChannel[16];
	bool m_bAdditionalPercussion;
	int m_iVibraChannel;
	int m_iVibraTrack;
	int m_nVibraMode;
	int	m_nSequencePolyphony;				//

	friend class CMIDIConverter;

private:
	int m_iCurrentPatch;
	int m_iCurrentSample;
	int m_iCurrentPatchBank;
};

class CMIDIFileEdit : virtual public CMIDIFile
{
public:
	typedef struct tagMIDIViewEvent
	{
		unsigned int nTime;
		BOOL	bTempo;
		int		nTempo;
		
		BOOL	bNoteOn;
		int		nChannel;
		int		nNote;
		int		nDuration;
	}MIDIViewEvent;

public:
	CMIDIFileEdit();
	~CMIDIFileEdit();

	BOOL Transpose(int nOffset,int iChannel);

	CMIDIEvent *LocateMIP(void);
	void UpdateMip(void);
	void UpdateVibra(void);

	unsigned int GetFirstOutTime();
	BOOL GetFirstEvent (MIDIViewEvent *pOut);
	BOOL GetNextEvent (MIDIViewEvent *pOut);

	BOOL RemoveNote(unsigned int nEventTime,int iChannel);
	bool InsertEvent(int iTrack,CMIDIEvent *pEvent);
	BOOL RemoveChannel(int iChannel);
	BOOL DuplicateChannel(int iChannel,int iDestChannel);

	CMIDIEvent *GetEvent(int iTrack, int iPosition);

	DllExport int GetTrackCount (void);
	int GetActiveChannel(void){return m_iChannel;};
	void SetActiveChannel(int iChannel){m_iChannel=iChannel;};

protected:
	CMIDIEvent *GenerateMip(void);
	void GenerateVibra(uint32_t dwAt,int iChannel,CMIDIEvent **pEvent,int *pnEventCount);
	bool RenderMip(unsigned char **pcData,int *pnEventSize);

	int m_iTrackPos[64];		
	int m_iEventPos;
	int m_iChannel;
};

class CMIDIFileLoader : virtual public CMIDIFile
{
public:
	DllExport CMIDIFileLoader();
	DllExport virtual ~CMIDIFileLoader();

	DllExport int Load (istream &ar);
	DllExport int Load (const char *szFile);
	DllExport tstring &GetSongName();
	DllExport tstring &GetCopyright();

	int GetDivisor();
	int GetQuantaSize();
	double GetTimeScale();

	void SetTempo (int nTempo);

	//CTypedPtrArray <CPtrArray, CMIDITrack *> *GetTracks();

	int GetTempo (void);
	
	//int GetChannelCount();
	//bool IsChannelUsed(int iChannel);
	//void SetChannelUsed(int iIndex,bool bUsed);

	bool ProcessMip(void);

	void InitPresetPolyphonyWithMaxPolyphony(void);
	void InitPriosWithDefaults(void);

	void SetSongPath(const char *pszPath);
	tstring &GetSongPath(void);
    
	int GetVibraChannel(void){return m_iVibraChannel;};
	void SetVibraChannel(int iChannel){m_iVibraChannel=iChannel;};

	void EnablePercussionChannel(bool bEnable){m_bAdditionalPercussion=bEnable;};
	bool HasAdditionalPercussionChannel(void){return m_bAdditionalPercussion;};

	int GetBrokenChannels();

	int GetSize (void);


//#ifdef _AFX
//#endif
	void CalcPolyphony(uint32_t nAt,unsigned char *pData);
	void InstrumentUsage(uint32_t nAt,unsigned char *pData);

	#define DEFAULTQUANTASIZE	(1000)
	#define DEFAULTTEMPO		(120.0)


protected:
	int ReadSysExclEvent (istream &ar,int oldState);
	int ReadMetaevent (istream &ar);
	int ReadEscapeEvent (istream &ar);
	int ReadTrackHeader (istream &ar);
	int ReadPreamble (istream &ar);
	int ReadEvent (istream &ar);
	bool ReadChunkType (istream &ar,char *pBuffer);
	int ReadVariableQuantity (istream &ar,int *n);
	void SetPolySequence(uint32_t nAt,int iChannel,int nPoly);

	void IncreasePolyphony(uint32_t nAt,int nChannel);
	void DecreasePolyphony(uint32_t nAt,int nChannel);
protected:
	int m_nBrokenChannels;

};

#endif // !defined(AFX_MIDIFILE_H__85E42880_8776_11D4_90DB_DCF680CEAB7A__INCLUDED_)
