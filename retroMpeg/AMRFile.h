#ifndef AMRFILEincluded
#define AMRFILEincluded
typedef struct
{
	int nType;
	char *pcType;
	int nHeader;
	int nFrame;
	int nBytes;
	unsigned char cCode;
}ModeTable;

class CMobileSampleContent;

class CAMRFile : public CMobileSampleContent
{
public:
	DYNOBJECT(CAMRFile)
	DYNDEFPROPERTY

	CAMRFile(void);
	virtual ~CAMRFile(void);

	virtual void Read(istream &ar);
	virtual void Write(ostream &out);

	static uint32_t nGetBitRate(uint32_t nSubFormat,uint32_t nMode);
	uint32_t nGetBitRate(uint32_t nMode);
	uint32_t nGetSamplesPerSecond(void);
	uint32_t nGetChannels(void) {return m_nSubFormat > 2 ? 2 : 1;};
	uint32_t nGetMode(void) {return m_nMode;};
	uint32_t nGetFormat(void) {return m_nSubFormat;};
	static tstring sGetFormatName(uint32_t nFormat);

	uint32_t nGetMode(uint32_t nSubFormat,uint32_t nBitRate);

	static const int subFormatNB=1;
	static const int subFormatWB=2;
	static const int subFormatMC=3;
	static const int subFormatMCWB=4;

	enum 
	{
		AMR475,
		AMR515,
		AMR590,
		AMR670,
		AMR740,
		AMR795,
		AMR102,
		AMR122,
		SID8,
		SID9,
		SIDA,
		SIDB,
		AMRERROR,
		NDAT
	} AMRNBmodes;

	enum 
	{
		AMR660,		//6.60
		AMR885,		//8.85
		AMR126,		//12.65
		AMR142,		//14.25
		AMR158,		//15.85
		AMR182,		//18.25
		AMR198,		//19.85
		AMR230,		//23.05
		AMR238,		//23.85
	} AMRWBmodes;

protected:
	uint32_t m_nSubFormat;
	uint32_t m_nMode;
};

class CAWBFile : public CAMRFile
{
public:
	DYNOBJECT(CAWBFile)
	DYNDEFPROPERTY
	CAWBFile(void);
	virtual ~CAWBFile(void);

	virtual void Read(istream &ar);
	virtual void Write(ostream &out);
};

#endif
