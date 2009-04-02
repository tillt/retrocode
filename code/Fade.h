#ifndef FADER_included
#define FADER_included
class CFade : public CFilter
{
public:
	CFade();
	virtual ~CFade();
	
	void Init(int nSampleRate,int nBitsPerSample, int nChannels,unsigned int nFadeTime,int nDirection);
	virtual void Process (signed short int *ibuf, signed short int *obuf, uint32_t nSampleCount);

	enum {	directionIn, directionOut	};

protected:
	int m_nDirection;
	int m_nFadeTime;
	int m_nSampleRate;
	int m_nChannels;
	int m_nBitsPerSample;
};
#endif
