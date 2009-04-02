#if !defined(AFX_MIDIFILEWRITER_H__F03799A0_94C9_11D4_90DB_C4AF7A16D36A__INCLUDED_)
#define AFX_MIDIFILEWRITER_H__F03799A0_94C9_11D4_90DB_C4AF7A16D36A__INCLUDED_

#include "MIDIFile.h"

class CMIDIFileWriter : virtual CMIDIFile, public CMIDIFileEdit
{
public:
	DllExport CMIDIFileWriter();
	DllExport virtual ~CMIDIFileWriter();
	DllExport void SetTiming (int nDivisor,int nQuantaSize);
	
	DllExport int Save(ostream &os,int iChannel=-1);
	DllExport virtual int Save(ostream &os,bool bPlayChannel[]);

	BOOL SetEnd (uint32_t nShift);

	void SetIncludeMIP(bool bEnable=true);
	void SetIncludeText(bool bEnable=true);

	int WriteEnding (ostream &os);
	int WriteEndOfTrack (ostream &os);
	int WritePreamble(ostream &os);
	BOOL WriteChunkType (ostream &os,char *pBuffer);
	int WriteTrackHeader (ostream &os,int iTrack);
	int WriteEvent (ostream &os,CMIDIEvent *pOut);
	int WriteVariableQuantity (ostream &os,int value);

	int Save(const char *szFile);

protected:
	bool GetFirstEvent (CMIDIEvent **pOut);
	bool GetNextEvent (CMIDIEvent **pOut);
	bool GetFirstEvent (CMIDIEvent **pOut,int iChannel);
	bool GetNextEvent (CMIDIEvent **pOut,int iChannel);
	int GetTrackSize(int iTrack);

	bool m_bIncludeMIP;
	bool m_bIncludeText;
	bool m_bPlaybackSave;

	uint64_t m_llLastTrackHeaderOffset;
	unsigned char m_cRunningStatus;
};

#endif // !defined(AFX_MIDIFILEWRITER_H__F03799A0_94C9_11D4_90DB_C4AF7A16D36A__INCLUDED_)
