#ifndef EightToSixteenBit_included
#define EightToSixteenBit_included
class CEightToSixteenBit : public CFilter
{
public:
	CEightToSixteenBit();
	virtual ~CEightToSixteenBit();
	
	virtual void Init(int nChannels);
	virtual void Process (signed short int *ibuf, signed short int *obuf, uint32_t nSampleCount);
	uint32_t nGetDestSize(uint32_t nSourceSize);

protected:
	int m_nChannels;
};
#endif
