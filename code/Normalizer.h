#ifndef NORMALIZER_included
#define NORMALIZER_included
class CNormalizer : public CFilter
{
public:
	CNormalizer();
	virtual ~CNormalizer();
	
	//void Init(int nSourceChannels, int nDestChannels);
	virtual void Process (signed short int *ibuf, signed short int *obuf, uint32_t nSampleCount);

protected:
};
#endif
