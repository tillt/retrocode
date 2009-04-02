#ifndef SMAFDecoderIncluded
#define SMAFDecoderIncluded
class CSMAFDecoder
{
public:
	CSMAFDecoder();
	~CSMAFDecoder();

	void DecodeChunk(unsigned char **pcBuffer,unsigned char *pcLimit,tstring &sIdentifier,unsigned int *pnSize,unsigned char *pcChunkAttribute);
};
#endif
