#ifndef CMXFILEincluded
#define CMXFILEincluded
struct CMXHEADER
{
    int8_t	cmid[4];                // 'c','m','i','d'                 
    uint32_t  size;                   // size of CMX file from here on 
	uint16_t	wHeaderLength;			// size of the CMX header
	uint8_t	cMode;					// 01 = melody, 02 = song
	uint8_t	cInstrumentsMask;		// instruments = OCTET ; 0VMFPTWS
									//	V=1 contains other vocal parts ; 
									//	M=1 contains male vocal parts ; 
									//	F=1 contains female vocal parts ; 
									//	P=1 contains picture data ; 
									//	T=1 contains text data ; 
									//	W=1 contains wave data ; 
									//	S=1 contains musical event
	uint8_t	cTracks;				// number of tracks
	uint16_t	wVersion;				// version
	uint8_t	cSourceInfo;			// source-info = 
									// 00 = No copyright, downloaded (from the net) 
									// 01 = Copyrighted, downloaded (from the net) 
									// 02 = No copyright, mobile originated 
									// 03 = Copyrighted, mobile originated 
									// 04 = No copyright, from desktop 
									// 05 = Copy righted, from desktop
	int8_t	sCopyright[256];		// copyright note (text)
	int8_t	sTitle[256];			// title (text)
	int8_t	sProvider[256];			// author / provider (text)
	int8_t	sDate[9];				// date
	uint8_t	cCharset;				// code-value = 
									// %b00000000 ANSI CHARSET / 
									// %b00000001 ISO8859-1 
									// %b00000010 ISO8859-2 
									// %b00000011 ISO8859-3 
									// %b00000100 ISO8859-4 
									// %b00000101 ISO8859-5 
									// %b00000110 ISO8859-6 
									// %b00000111 ISO8859-7 
									// %b00001000 ISO8859-8 
									// %b00001001 ISO8859-9 
									// %b00001010 ISO8859-10 
									// %b10000001 HANGUL CHARSET 
									// %b10000010 Chinese Simplified 
									// %b10000011 Chinese Traditional 
									// %b10000100 Hindi  
									// Others Reserved
}; 

class CCMXSample;
//class CCMXFile : public CMobileSampleContent, public CMobileProperty
class CCMXFile : public CMobileSampleContent
{
public:
	DYNOBJECT(CCMXFile)
	DYNDEFPROPERTY

	CCMXFile(void);
	virtual ~CCMXFile(void);
	virtual void Read(istream &ar);
	virtual void Write(ostream &out);

	int nGetEncoding(void) {return (int)m_Header.cCharset;};
	int nGetFormat(void);

	static tstring sGetFormatName(int nFormat);
	bool bUsesSamples(void) {return m_bContainsSample;};
	bool bUsesGraphix(void) {return m_bContainsPicture;};
	uint32_t nGetPlaytime(void) {return m_nPlaytime;};

	CMXHEADER m_Header;

	uint32_t	m_nTempo;
	uint32_t	m_nTimebase;
	uint32_t	m_nCueStartAt;
	uint32_t	m_nCueEndAt;
	uint32_t	m_nFinePitch[4];

	enum InstrumentMask {	cmiOtherVocal=0x01,
							cmiMaleVocal=0x02,
							cmiFemaleVocal=0x04,
							cmiPictureData=0x08,
							cmiTextData=0x10,
							cmiWaveData=0x20,
							cmiMusicEvent=0x40	};

	enum SourceMask {		cmsNetFree=0x00,
							cmsNetCopy=0x01,
							cmsMobileFree=0x02,
							cmsMobileCopy=0x03,
							cmsDeskFree=0x04,
							cmsDeskCopy=0x05	};

	enum CharMask {			cmcAnsi=0x00,
							cmcIso1=0x01,
							cmcIso2=0x02,
							cmcIso3=0x03,
							cmcIso4=0x04,
							cmcIso5=0x05,
							cmcIso6=0x06,
							cmcIso7=0x07,
							cmcIso8=0x08,
							cmcIso9=0x09,
							cmcIso10=0x0A,
							cmcHangul=0x81,
							cmcChnSimple=0x82,
							cmcChnTrad=0x83,
							cmcChnHindi=0x84	};
							

protected:
	unsigned char *pAddExtEvent(unsigned char *pOut,unsigned char nDelta,unsigned char nCommand,unsigned char nParameter);

	enum ChunkIDs {	cidVERS,
					cidCODE,
					cidTITL,
					cidDATE,
					cidSORC,
					cidCOPY,
					cidEXSN,
					cidEXSA,
					cidEXSB,
					cidEXSC,
					cidEXST,		//*
					cidTRAC,		//*
					cidPCPI,
					cidCNTS,
					cidPROT,
					cidTOOL,		//*
					cidNOTE,		//*
					cidPOLY,
					cidWAVE	};

	enum cmxEvents {
		eMasterVol=0xB0,
		eMasterTune=0xB3,
		ePartConfig=0xB9,
		ePause=0xBD,
		eStop=0xBE,
		eReset=0xBF,
		eCuepoint=0xD0,
		eJump=0xD1,
		eNOP=0xDE,
		eEOT=0xDF,
		eProgramChg=0xE0,
		eBankChg=0xE1,
		eVolume=0xE2,
		ePanPot=0xE3,
		ePitchBend=0xE4,
		eChannelAssign=0xE5,
		ePitchRange=0xE7,
		eWaveChnVol=0xE8,
		eWaveChnPan=0xE9,
		eTextCtrl=0xEB,
		ePictCtrl=0xEC,
		eLEDCtrl=0xED,
		eVibraCtrl=0xEE,
		eWaveDataLen=0xF1,
		eTextDataLen=0xF2,
		ePictDataLen=0xF3,
		eAnimDataLen=0xF4
	};

	enum cmxPictureFormats {
		pfBmp=0x01,
		pfJpeg=0x02,
		pfPng=0x03,
	};

	void ReadHeader(istream &ar, CMXHEADER *pCmx);
	int ReadSubchunk(istream &ar, CMXHEADER *pCmx);
	bool bParseCNTS(istream &ar, uint32_t wSize,uint32_t *pwSizeRemaining);
	bool bParseTRAC(istream &ar, uint32_t wSize,uint32_t *pwSizeRemaining);
	void ExportPicture(const char *pszPathPrefix,int nPictureId,int nPictureFormatId,unsigned char *p,uint32_t nExtSize);
	//void ExportSample(unsigned char *p,uint32_tint nExtSize);
	uint32_t nRenderTrack(uint32_t nFormat,uint32_t nSampleRate,uint32_t nLoopcount,uint32_t nVolume,uint32_t nSourceSize,char *pcSource=NULL,char *pcDest=NULL);

#define IsChannelFinepitch(a)	(!(a & 0x80))
#define IsTimebaseTempo(a)		((a & 0xF0) == 0xC0)


	bool m_bContainsPicture;
	bool m_bContainsSample;
	uint32_t m_nPlaytime;

	CCMXSample *m_pCurrentSample;
	vector<CCMXSample *> m_Samples;
};
#endif
