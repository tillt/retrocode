#ifndef MSEQFILEincluded
#define MSEQFILEincluded
typedef struct 
{
	char sID[4];					//".SEQ"
	int nClass;						//class allways is 1

	bool bEms;						//EMS content
	int nTmode;						//0 multiprocessor
									//1 streaming
									//2 packet
									//3 reserved

	unsigned int nFilesize;			//filesize

	int nCopyStatus;				//0 global copyprotect
									//1 global copyprotect after copy countdown
									//2 see tracks copy status
									//3 global unprotected
	int nCopyCountdown;				//64-0
	
	int nResendStatus;				//
	int nResendCountdown;			//

	int nLoopDef1;					//loop define part 1
									//0 loop not defined
									//1 loop defined
									//2 reserved
									//3 infinite loop (without regarding NBRepeat
	int nLoopPoint1;				//end of loop 1
	int nStartPoint1;				//start of loop 1
	int nNBRepeat1;					//number of loops for part 1

	int nLoopDef2;					//loop define part 2
									//0 loop not defined
									//1 loop defined
									//2 reserved
									//3 infinite loop (without regarding NBRepeat
	int nLoopPoint2;				//end of loop 2
	int nStartPoint2;				//start of loop 2
	int nNBRepeat2;					//number of loops for part 2

	int nTimebase;					//
	int nTracks;					//
}MSEQHeader;

class CMSEQFile : public CMobileContent
{
public:
	DYNOBJECT(CMSEQFile)
	DYNDEFPROPERTY

	CMSEQFile(void);
	virtual ~CMSEQFile(void);
	
	void Read(istream &ar);

	int nGetTrackCount(void);
	int nGetCopyStatus(void);
	int nGetTimebase(void);
	int nGetLoopCount(void);
	int nGetTmode(void);

protected:
	MSEQHeader m_Header;

	void ReadHeader(istream &ar, MSEQHeader *pWave);
};
#endif
