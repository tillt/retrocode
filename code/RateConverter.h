#ifndef RATECONVERT_included
#define RATECONVERT_included

class CRateConverter : public CFilter
{
public:
	CRateConverter ();
	virtual ~CRateConverter ();

	void Init(int nInChannels,int nInRate,int nOutRate);
	virtual void Process (signed short int *ibuf, signed short int *obuf, uint32_t nSampleCount);
	uint32_t nGetDestSize(uint32_t nSourceSize);

protected:
	enum rate
	{
		rateIn,
		rateOut
	};
	int m_nSamplesPerSecond[2];
	int m_nChannels;
	long long m_llOposIncFrac;
	long long m_llOposInc;
};
#endif