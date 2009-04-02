#ifndef RINGBACK_included
#define RINGBACK_included
class CRingback : public CFilter
{
public:
	CRingback();
	virtual ~CRingback();
	
	void Init(int nSampleRate,int nBitsPerSample, int nChannels, const char *pszCountryConfigFile,const char *pszCountryId);
	virtual void Process (signed short int *ibuf, signed short int *obuf, uint32_t nSampleCount);

	typedef struct tagITU
	{
		double dSineAmplification_dB;	//sine gain
		int nComponentCount;			//number of sine-frequencies to mix
		int nPhaseCount;				//number of active/pause phases
		int nLoopStart;					//cycle index (usually 0)
		bool bMixTones;					//mix=true, modulate=false
		double dFrequency[2];			//frequency/s to mix
		int nActivePhaseDuration[8];	//duration/s of the active phase/s
		int nPausePhaseDuration[8];		//duration/s of the pause phase/s
	}ituSet;

protected:
	bool ParseSignal(const char *pcSignal);
	bool SelectSignal(const char *pcDest,const char *pcCountryId);

	ituSet set;
	int m_nSampleRate;
	int m_nChannels;
	int m_nBitsPerSample;
	int m_iSelectedSet;
};
#endif
