#ifndef RMFSOUND_Defined
#define RMFSOUND_Defined
class CRMFSound : public CAdpcm
{
public:
	CRMFSound(const TCHAR *pszName=NULL);
	~CRMFSound();

	void Serialize(istream &ar,bool bEncrypted);
	void Serialize(ostream &ar);

	uint32_t nRender(rmfCACH *pCache,int nType,int nChannels,int nSampleRate,uint32_t nSourceSize,bool bLoop=false,unsigned char *pcSource=NULL,unsigned char *pcDest=NULL);

	void ExportRaw(void);
	void Decompress(void);
	uint32_t nCompress(int nType,int nChannels,uint32_t nSourceSize,unsigned char *pcSource=NULL, unsigned char *pcDest=NULL, uint32_t nDestSize=0);

	uint32_t nGetPlaytime(void);
	uint32_t nGetBitsPerSample(void){return m_nBitsPerSample;};
	uint32_t nGetStoredBitsPerSample(void){return m_nStoredBitsPerSample;};
	uint32_t nGetChannels(void){return m_nChannels;};
	uint32_t nGetBitRate(void){return m_nBitRate;};
	uint32_t nGetSamplesPerSecond(void){return m_nSampleRate;};
	uint32_t nGetSize(void){return m_nSampleSize;};
	uint32_t nGetType(void){return m_nCompressionType;};
	signed short *pcGetSample(void){return (signed short *)m_pcSampleBuffer;};

	enum SoundEncoding {	compNone,
							compAlaw,
							compUlaw,
							compAdpcm,
							compMpeg	};

protected:
	void CompressAdpcm(unsigned char *pcDest,unsigned char *pcSource,uint32_t nSize,int nChannels,uint32_t nDestSize);
	uint32_t nDecompressedSize(int nType,uint32_t nSampleSize);
	uint32_t nCompressedSize(int nType,uint32_t nSampleSize);

	tstring m_sName;

	uint32_t m_nCompressionType;
	uint32_t m_nChannels;
	uint32_t m_nBitsPerSample;
	uint32_t m_nStoredBitsPerSample;
	uint32_t m_nBitRate;
	uint32_t m_nSampleRate;

	char *m_pcSampleBuffer;
	uint32_t m_nSampleSize;

	static const int32_t m_nIndexTable[16];
	static const int32_t m_nStepsizeTable[89];
};
#endif
