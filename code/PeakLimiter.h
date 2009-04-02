#ifndef PEAKLIMIT_included
#define PEAKLIMIT_included
class CPeaklimit : public CFilter
{
public:
	CPeaklimit();
	virtual ~CPeaklimit();

	enum FilterMode 
	{
		peakVolume,
		peakLimit,
	};
	
	void Init(int nMode,float fGain, float fThreshhold);
	virtual void Process (signed short int *ibuf, signed short int *obuf, uint32_t nSampleCount);

protected:
	int m_nMode;
	int m_nLimited;
	float m_dGain;
	float m_dLimitGain;
	float m_dThreshhold;		
};
#endif
