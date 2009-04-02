#ifndef MYFILTER
#define MYFILTER
class CFilter
{
public:
	CFilter();
	~CFilter();
	void Process (signed short int *ibuf, signed short int *obuf, uint32_t nSampleCount);
	
protected:
	signed short int nClip(const int32_t nSample);
	signed int ndBToAmp(const double dB);
	double fAmpTodB(const signed int nAmp);

	int m_nClipped;
};
#endif
