#ifndef QCELPFILEincluded
#define QCELPFILEincluded
#ifdef USE_QUALCOMM_LIBRARY
class IQsclCodec;
#else
class CANSI733Codec;
#endif

LPCTSTR szGetQcelpVersion(void);
class CQCELPFile : public CMobileSampleContent
{
public:
	struct type_qpl_variable_rate 
	{
		uint32_t	numOfRates;         // # of rates
		// high byte = rate, low byte = size of following packet
		uint16_t	bytesPerPacket[8]; 
	};

	struct QCELPHEADER
	{
		int8_t		RIFF[4];                // 'R','I','F','F'                 
		uint32_t	nSize;                  // size of QCELP file from here on 
		int8_t		QCELPfmt[8];			// 'Q','L','C','M','f','m','t',' ' 
		uint32_t	nFormatLength;          // The length of the TAG format     
		uint16_t	wFormatTag;             // 02 = SMV, 01 = QCELP 13K and EVRC
		int8_t		format[16];				// codec-guid 
		uint16_t	wCodecVersion;			// codec version 1 or 2 for QCELP, 1 for EVRC and SMV
		int8_t		QCELPcodecname[80];		// "Qcelp 13K"
		uint16_t	wAverageBps;
		uint16_t	wPacketSize;
		uint16_t	wBlockSize;
		uint16_t	wSamplingRate;
		uint16_t	wBitsPerSample;
		type_qpl_variable_rate vRate;
		uint32_t	reserved[5];            // reserved
	}; 

	DYNOBJECT(CQCELPFile)
	DYNDEFPROPERTY

	CQCELPFile(void);
	virtual ~CQCELPFile(void);
	virtual void Read(istream &ar);
	virtual void Write(ostream &out);
	bool bMagicHead(std::istream &ar,uint32_t nSize);

	static uint32_t nGetPlaytime(uint32_t nSize,uint32_t nBitPerSecond);
	static tstring sGetFormatName(int nFormat);

	uint32_t nGetBitsPerSample(void) {return m_Header.wBitsPerSample;};
	uint32_t nGetAvgBps(void) {return m_Header.wAverageBps;};
	uint32_t nGetSamplesPerSecond(void) {return m_Header.wSamplingRate;};
	int nGetSubFormat(void) {return m_nSubFormat;};

	void *pInitLibrary(void);
	#ifdef USE_QUALCOMM_LIBRARY
	void DeinitLibrary(void);
	#else
	void DeinitLibrary(CANSI733Codec *codec);
	#endif

	#ifdef USE_QUALCOMM_LIBRARY
	void InitDecode(IQsclCodec *codec,bool bVariableRate);
	uint32_t nDecode(IQsclCodec *codec,void *pSource,void *pDest,uint32_t nSourceBlocks,uint32_t nDestSize);
	#else
	#endif

	#ifdef USE_QUALCOMM_LIBRARY
	void InitEncode(IQsclCodec *codec,bool bVariableRate);
	uint32_t nEncode(IQsclCodec *codec,int nChannel,int nBitsPerSample,int nSamplesPerSecond,void *pSource,void *pDest,uint32_t nSourceSize);
	#else
	#endif

	//Pulse code modulation codec 
	static const unsigned char guidCodecPCM[16];
	//IS-733 Qualcomm PureVoice 13K
	static const unsigned char guidCodecQ13K[16];
	//IS-733 Qualcomm PureVoice 13K (SmartRate)
	static const unsigned char guidCodecQ13KS[16];
	//G.711 (mu-law) codec 
	static const unsigned char guidCodecULAW[16];

	static LPCTSTR szGetQcelpVersion(void);

protected:
	uint32_t nReadHeader(istream &ar, QCELPHEADER *pWave);
#ifdef USE_QUALCOMM_LIBRARY
	void *m_qscl;
#else
#endif
	static char m_szQcelpVersion[255];
	int m_nSubFormat;
	QCELPHEADER m_Header;
};
#endif
