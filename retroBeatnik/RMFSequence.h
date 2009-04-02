#ifndef RMFSequence_Defined
#define RMFSequence_Defined
class CRMFSequence
{
public:
	CRMFSequence(int nEncoding=seqEcmi,const TCHAR *pszName=NULL);
	~CRMFSequence();

	enum SoundEncoding {seqMidi,
						seqEcmi,
						seqCmid,
						seqEmid	};

	const static unsigned short sequenceID=0x0049;

	void ExportRaw(const char *pcpath);
	void Serialize(istream &ar);
	void Serialize(ostream &ar);

	uint32_t nRender(rmfCACH *pCache,unsigned char *pcDest,int nChannels,int nSampleRate,uint32_t nSampleSize,uint32_t nPlaytime=0,uint32_t nFadetime=0);

	unsigned char *pcGetSequence(){return m_pcSequenceBuffer;};
	uint32_t nGetSequenceSize(){return m_nSequenceSize;};

protected:
	uint32_t nRenderMIDI(int nChannels,int nSampleRate,int nSampleSize,uint32_t nPlaytime=0,uint32_t nFadetime=0,unsigned char *pcDest=NULL,uint32_t nDestSize=0);

	tstring m_sName;
	unsigned char *m_pcSequenceBuffer;
	uint32_t m_nSequenceSize;
	uint32_t m_nEncoding;

	void Decompress(uint32_t nUnfoldedSize);
	void Compress(void);
};
#endif
