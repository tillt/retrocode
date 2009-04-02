#ifndef LOOP_included
#define LOOP_included
class CLoop : public CCrop
{
public:
	CLoop();
	virtual ~CLoop();

	virtual void Init(int nSampleRate,int nChannels,int nBitsPerSample,uint32_t nDestPlaytime,void *pSource,uint32_t nSourceSampleCount);
	virtual void Process (signed short int *ibuf, signed short int *obuf, uint32_t nSampleCount);
	uint32_t nGetDestSize(uint32_t nSourceSize);

protected:

};
#endif
