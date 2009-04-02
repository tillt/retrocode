#ifndef RMFFILEincluded
#define RMFFILEincluded
class CRMFSequence;
class CRMFInstrument;
class CRMFSound;
class CRMFFile : public CMobileSampleContent
{
public:
	DYNOBJECT(CRMFFile)
	DYNDEFPROPERTY

	CRMFFile(void);
	virtual ~CRMFFile(void);
	string sGetInfoText(int i) {return m_strInfo[i];};
	uint32_t nGetPlaytime(void){return m_nPlaytime;};
	uint32_t nGetSamplePlaytime(void);
	bool bUsesSamples(void){return m_Sounds.size() > 0;};
	uint32_t nGetFormat(void);
	uint32_t nGetSamplesPerSecond(void);
	uint32_t nGetChannels(void);
	uint32_t nGetBitRate(void);
	uint32_t nGetBitsPerSample(void);
	static tstring sGetFormatName (int nFormat);

	virtual void Write(ostream &out);
	virtual void Read(istream &ar);

protected:
	enum FormatIndex {	rmfformatUnknown=0,
						rmfformatSequence=1,
						rmfformatUncompressed=2,
						rmfformatAdpcm,
						rmfformatUlaw,
						rmfformatAlaw,
						rmfformatMpeg,
						rmfformatMultiple,
						rmfformatLast	};

	CRMFSequence *m_pSequence;
	vector<CRMFInstrument *> m_Instruments;
	vector<CRMFSound *> m_Sounds;

	uint32_t m_nPlaytime;

	uint32_t nRenderSONG(CMobileSampleContent *pSource,rmfCACH *pCache,unsigned char *pDest=NULL);
	uint32_t nRenderSONGSubTags(CMobileSampleContent *pSource,unsigned char *pDest);
	uint32_t nRenderINST(rmfCACH *pCache,unsigned char *pDest);
	uint32_t nRenderESND(rmfCACH *pCache,int nType,int nChannels,int nSampleRate,uint32_t nSourceSize,bool bLoop,unsigned char *pcSource,unsigned char *pcDest);
	uint32_t nRenderEMID(rmfCACH *pCache,int nType,int nChannels,int nSampleRate,uint32_t nSourceSize,uint32_t nPlaytime,uint32_t nFadetime,unsigned char *pcDest);
	uint32_t nRenderCACH(int nTags,rmfCACH *pCach,unsigned char *pcDest);

	bool bValidateHeader(istream &ar);
	char *pcReadString(istream &ar);
	void ParseINST(istream &ar);
	void ParseESND(istream &ar,bool bEncrypted);
	void ParseECMI(istream &ar);
	void ParseSONG(istream &ar);

	friend class CRMFProperty;
};
#endif
