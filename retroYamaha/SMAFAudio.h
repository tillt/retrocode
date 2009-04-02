#ifndef SMAFAudio_Included
#define SMAFAudio_Included
class CSMAFSample;
class CSMAFAudio : public CSMAFTrack
{
public:
	CSMAFAudio();
	~CSMAFAudio();

	void Decode(unsigned char **pcBuffer,uint32_t nSize);

	enum AudioChunkIDs {	acidAspI,
							acidAwa,
							acidAtsq	};

	uint32_t nGetSampleCount(){return (uint32_t)m_Samples.size();};
	CSMAFSample *pGetSample(int iIndex);
	uint32_t nGetSampleRate(){return m_nSampleRate;};
	uint32_t nGetChannelCount(){return m_nChannels;};
	uint32_t nGetMaxPlaytime(int *pnIndex=NULL);
	uint32_t nGetPlaytime(void);
	void DecodeAtsq(unsigned char *pcBuffer,uint32_t nSize);

protected:
	uint32_t m_nFormat;
	uint32_t m_nBitsPerSample;
	uint32_t m_nSampleRate;
	uint32_t m_nChannels;
	vector<CSMAFSample *> m_Samples;
public:
	static unsigned char cEncodeSampleRate(uint32_t nSampleRate);
};
#endif
