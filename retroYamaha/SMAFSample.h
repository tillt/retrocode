#ifndef SMAFSAMPLE_Defined
#define SMAFSAMPLE_Defined
class CSMAFSample
{
public:
	CSMAFSample(uint32_t nSampleRate, uint32_t nChannels);
	~CSMAFSample();

	//void Encode(const unsigned char **pcBuffer,int *pnSize);
#ifndef YAMAHA_NOENCODE
	int nEncode(signed short int *pWave,uint32_t nWaveSize,char *pDest);
#endif
#ifndef YAMAHA_NODECODE
	void Decode(const unsigned char *pcBuffer,uint32_t nSize);
#endif
	uint32_t nGetPlaytime();

	uint32_t nGetRawSize(void){return m_nRawSize;};
	uint32_t nGetAdpcmSize(void){return m_nAdpcmSize;};
	uint32_t nGetSamplesPerSecond(void){return m_nSampleRate;};
	uint32_t nGetChannels(void){return m_nChannels;};
	
	int16_t *pcGetRawSample(void){return m_pnRawData;};
	char *pcGetAdpcmSample(void){return m_pnAdpcmData;};

protected:
	int16_t *m_pnRawData;
	char *m_pnAdpcmData;

	uint32_t m_nRawSize;
	uint32_t m_nAdpcmSize;

	uint32_t m_nSampleRate;
	uint32_t m_nChannels;

	static const int32_t m_nStepTable[8];
};
#endif
