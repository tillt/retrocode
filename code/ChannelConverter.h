#ifndef CHANNELCONVERTER_included
#define CHANNELCONVERTER_included
class CChannelConverter : public CFilter
{
public:
	CChannelConverter();
	virtual ~CChannelConverter();
	
	void Init(int nSourceChannels, int nDestChannels);
	virtual void Process (signed short int *ibuf, signed short int *obuf, uint32_t nSampleCount);
	uint32_t nGetDestSize(uint32_t nSourceSize);

protected:
	int m_nSourceChannels;
	int m_nDestChannels;
};
#endif
