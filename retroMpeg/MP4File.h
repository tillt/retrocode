#ifndef MP4FILEincluded
#define MP4FILEincluded
class CBufferCollector;
class CFAACBase: public CMobileSampleContent
{
public:
	//DYNOBJECT(CFAADBase)
	//DYNDEFPROPERTY(CFAADProperty)
	CFAACBase(void);
	virtual ~CFAACBase(void);

	static tstring sGetFormatName(int nFormat);
	bool bMagicHead(std::istream &ar,uint32_t nSize);

	virtual void Write(ostream &out);
	virtual void Read(istream &ar);

	void RenderAAC(CBufferCollector *pbc,CMobileSampleContent *pSource);
	int GetAACTrack(void *infile);
	int GetAMRTrack(void *infile);
	int GetVideoTrack(void *infile);
	void InitFromAMRTrack(void *infile,int iTrack);
	void InitFromAACTrack(void *infile,int iTrack);
	int GetTrackInfo(void *infile,int iTrack);
	int nGetPlaytime(int nBitRate,int nSize);

	tstring sGetInfoText(int iIndex){return m_strInfo[iIndex];};
	void SetInfoText(int iIndex,LPCTSTR pszInfo){m_strInfo[iIndex]=pszInfo;};

	uint32_t m_nObjectType;
	uint32_t m_nVersion;
	uint32_t m_nBitRate;
	uint32_t m_nWidth;
	uint32_t m_nHeight;
	uint32_t m_nFrameRate;
	//int m_nSubFormat;
	uint32_t m_nPlayTime;

protected:
	static const uint32_t m_cnSampleRates[16];
	//static const char *m_pszProfileName[6];

	char m_pszFileName[1024];

	char m_pcMP4Type[6][4];
	bool m_bUse3GPEncoding;

	bool GetConfiguration(unsigned char **ppConfig,uint32_t *pConfigLength,unsigned char profile,uint32_t samplingRate,unsigned char channels);
	unsigned char cFindSamplingRateIndex(uint32_t samplingRate);

	static unsigned int read_callback(void *user_data, void *buffer, unsigned int length);
	static unsigned int seek_callback(void *user_data, int64_t position);	
};

class CMP4File : public CFAACBase
{
public:
	DYNOBJECT(CMP4File)
	DYNDEFPROPERTY

	CMP4File(void);
	virtual ~CMP4File(void);

	virtual void CreateFile(ofstream &ar,const TCHAR *pszFileName);
	virtual void CloseFile(ofstream &ar);
	
protected:
};

class C3GPPFile : public CMP4File
{
public:
	DYNOBJECT(C3GPPFile)
	DYNDEFPROPERTY

	C3GPPFile(void);
	virtual ~C3GPPFile(void);
	
protected:
};

class C3GP2File : public CFAACBase
{
public:
	DYNOBJECT(C3GP2File)
	DYNDEFPROPERTY

	C3GP2File(void);
	virtual ~C3GP2File(void);
	
protected:
};
#endif
