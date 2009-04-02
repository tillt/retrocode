#ifndef CROP_included
#define CROP_included
class CCrop : public CFilter
{
public:
	CCrop();
	virtual ~CCrop();
	
	virtual void Init(int nSampleRate,int nBitsPerSample,int nChannels,uint32_t nOffset,uint32_t nDestPlaytime,bool bAutoCrop=false,void *pSource=NULL,uint32_t nSampleCount=0);
	virtual void Process (signed short int *ibuf, signed short int *obuf, uint32_t nSampleCount);
	uint32_t nGetDestSize(uint32_t nSourceSize);

protected:
	int m_nSampleRate;
	int m_nChannels;
	int m_nBitsPerSample;
	
	uint32_t m_nOffsetTime;
	uint32_t m_nDestPlayTime;

	uint32_t m_nDestData;
	uint32_t m_nSourceData;

	uint32_t m_nPosStart;
	uint32_t m_nPosEnd;
};
#endif
