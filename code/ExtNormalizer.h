#ifndef EXTNORMALIZER_included
#define EXTNORMALIZER_included
class CExtNormalizer : public CPeaklimit
{
public:
	CExtNormalizer();
	virtual ~CExtNormalizer();
	
	void Init(double fRmsNormDb);
	virtual void Process (signed short int *ibuf, signed short int *obuf, uint32_t nSampleCount);

protected:
	bool bUpdateStats(signed short int *ibuf,uint32_t nSampleCount);

	double m_fRMSThreshDb;
	double m_fRMSNormDb;
	double m_fRMSLevelDb;
	uint32_t m_nPeak;
	uint32_t m_nPeakOffset;
};
#endif
