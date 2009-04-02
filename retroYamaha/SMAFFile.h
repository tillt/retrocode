#ifndef SMAFFILEincluded
#define SMAFFILEincluded
class CSMAFAudio;
class CSMAFTrack;
class CSMAFProperty;
class CSMAFGraph;

// CSMAFFile document
class CSMAFFile : public CSMAFDecoder, public CMobileSampleContent
{
public:
	DYNOBJECT(CSMAFFile)
	DYNDEFPROPERTY

	vector<CSMAFTrack *> m_Tracks;

	CSMAFFile();
	virtual ~CSMAFFile();
#ifdef _AFX
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
#else
	virtual void Write(ostream &ar);
#endif
	virtual void Read(std::istream &ar);
	tstring sGetErrorMsg();

	bool bGetStatusCopy(){return m_bStatusCopy;};
	bool bGetStatusSave(){return m_bStatusSave;};
	bool bGetStatusEdit(){return m_bStatusEdit;};
	bool bUsesSamples(){return m_bContainsSamples;};
	bool bUsesGraphix(){return m_bContainsGraphix;};
	bool bUsesHumanVoice(){return m_bContainsHumanVoice;};
	bool bUsesSynthesizer(){return m_bContainsSynthesizer;};
	uint32_t nGetPlaytime(void){return m_nPlaytime;};
	uint32_t nGetSamplePlaytime(void){return m_nSamplePlaytime;};
	uint32_t nGetSampleRate(void){return m_nCSSamplesPerSecond;};
	uint32_t nGetChannels(void){return m_nCSChannels;};

	void SetStatusCopy(bool bStatus){m_bStatusCopy=bStatus;};
	void SetStatusSave(bool bStatus){m_bStatusSave=bStatus;};
	void SetStatusEdit(bool bStatus){m_bStatusEdit=bStatus;};

	int nGetFormat(){return m_nFormat;};
	int nGetSequenceFormat(){return m_nSequenceFormat;};
	int nGetEncoding(){return m_nEncoding;};
	CSMAFAudio *pGetAudio(){return m_pAudio;};
	CSMAFTrack *pGetTrack(int i){return m_Tracks[i];};

	int nGetTrackCount(){return (int)m_Tracks.size();};

	static string sGetEncodingName(int nEncoding);
	static string sGetFormatName(int nFormat);
	
	string sGetInfoText(int iIndex){return m_strInfo[iIndex];};
	void SetInfoText(int iIndex,LPCSTR pszInfo){m_strInfo[iIndex]=pszInfo;};

	static const int smafencShiftJis = 0x00;
	static const int smafencLatin1 = 0x01;
	static const int smafencEucKr = 0x02;
	static const int smafencHzGb = 0x03;
	static const int smafencBig5 = 0x04;
	static const int smafencKoi8 = 0x05;
	static const int smafencTcVn = 0x06;
	static const int smafencUSC2 = 0x20;
	static const int smafencUSC4 = 0x21;
	static const int smafencUTF7 = 0x22;
    static const int smafencUTF8 = 0x23;
	static const int smafencUTF16 = 0x24;
	static const int smafencUTF32 = 0x25;
	static const int smafencOctet = 0xFF;

	enum {
		smaffmtUnknown=0,
		smaffmtMA1=1,
		smaffmtMA2,
		smaffmtMA3,
		smaffmtMA5,
		smaffmtMA7
	};


protected:
	int nRenderSysEx(char *pcDest,int nDeviceId,const char *pcMsg,int nSize);
	bool bVerifyCheckSum(void);
	unsigned short int nCalcCheckSum(unsigned char *buf, unsigned nbytes);
	void DecodeStatusByte(unsigned char cStatus);
	unsigned char EncodeStatusByte(unsigned char cStatus);

	void Decode(void);

#ifndef YAMAHA_NOENCODE
	void Encode();

	void RenderDestinationStatic(ostream &out,CMobileSampleContent *pSource);
	void RenderDestinationStreaming(ostream &out,CMobileSampleContent *pSource,int nInternalFormatId);
	int nRenderCNTI(CMobileSampleContent *pSource,unsigned char *pcBuffer=NULL);
	int nRenderOPDA(CMobileSampleContent *pSource,unsigned char *pDest=NULL);
#endif
	unsigned short int nRenderAXCheckSum(unsigned char *pcSouce, uint32_t nLen);
	void DecodeAX(const TCHAR *pcBuffer);

	//void DecodeChunk(unsigned char **pcBuffer,unsigned char *pcLimit,tstring &sIdentifier,unsigned int *pnSize, unsigned char *pcChunkAttribute);
	void DecodeCNTI(unsigned char **pcBuffer,unsigned int nSize);
	void DecodeOPDA(unsigned char **pcBuffer,unsigned int nSize);

	void DecodeCNTISubChunk(unsigned char **pcBuffer,unsigned char *pcLimit,tstring &sIdentifier,unsigned int *pnSize,unsigned char **pcChunkAttribute);
	void DecodeOPDASubChunk(unsigned char **pcBuffer,unsigned char *pcLimit,tstring &sIdentifier,unsigned int *pnSize,unsigned char **pcChunkAttribute);

	int nEncodePlaytime (unsigned int nValue,unsigned char *pcBuffer);

	uint32_t m_nSize;
	int m_nCheckSumOffset;

	uint32_t m_nPlaytime;
	uint32_t m_nSamplePlaytime;

	int m_nFormat;
	int m_nEncoding;

	uint32_t m_nHighestLocatedDeviceId;

	CSMAFAudio *m_pAudio;
	CSMAFGraph *m_pGraph;
	
	unsigned char *m_pcSMAF;
	
	unsigned char *m_pcStatusByte;

	int m_nLastError;
	bool m_bStatusSave;
	bool m_bStatusCopy;
	bool m_bStatusEdit;
	bool m_bContainsSamples;
	bool m_bContainsGraphix;
	bool m_bContainsHumanVoice;
	bool m_bContainsSynthesizer;
	bool m_bContainsCustomInstruments;
	int m_nSequenceFormat;
	bool m_bSampleFromTracks;


	tstring m_strLastError;
	enum ChunkIDs {	cidMMMD,
					cidCNTI,
					cidOPDA,
					cidMTR,
					cidMSTR,
					cidMtsu,
					cidMtsq,
					cidMspl,
					cidATR,
					cidDch,
					cidPro,
					cidGTR,
					cidAtsq	};

	enum SubChunkIDs {	scidM2,
						scidL2,
						scidST,
						scidES,
						scidRF,
						scidHV,
						scidA0,
						scidA1,
						scidA2,
						scidSW,
						scidVN,
						scidGR,
						scidMI,
						scidCD,
						scidUD,
						scidCN,
						scidCA,
						scidWW,
						scidAW,
						scidAN,
						scidCR,
						scidLC,
						scidAS,
						scidMD,
						scidLast	};

	static const unsigned short int m_wCRCTable[256];
};
#endif
