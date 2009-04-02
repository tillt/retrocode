#ifndef BUTTERWORTH_included
#define BUTTERWORTH_included
class CButterWorth : public CFilter
{
public:
	CButterWorth();
	virtual ~CButterWorth();
	
	void Init(int nMode,double nFrequency,int nRate,double nBandwidth);
	virtual void Process (signed short int *ibuf, signed short int *obuf, uint32_t nSampleCount);

	enum FilterMode {
		butterLowpass,
		butterHighpass,
		butterBandpass,
	};

	#ifndef M_PI
	#define	M_PI		3.14159265358979323846
	#endif 

protected:
	double m_dX [2];
	double m_dY [2];
	double m_dA [3];
	double m_dB [2];
	double m_dFrequency;		//cut off frequency for low-pass and high-pass, center frequency for band-pass
	double m_dBandwidth;
};
#endif
