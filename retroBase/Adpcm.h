#ifndef ADPCMincluded
#define ADPCMincluded

#ifndef RETROBASE_EXPORTS
#undef DllExport
#define DllExport 
#else
#undef DllExport
#define DllExport   __declspec( dllexport )
#endif

#define imaStateAdjust(c) (((c)<4)? -1:(2*(c)-6))
/* +0 - +3, decrease step size */
/* +4 - +7, increase step size */
/* -0 - -3, decrease step size */
/* -4 - -7, increase step size */ 

#define ISSTMAX 88
class CAdpcm
{
public:
	//MS-ADPCM de/coder state structure
	typedef struct MSSTATE 
	{
		uint32_t step;
		int16_t iCoef[2];
	}MsState;

	enum{adpcmMS,adpcmIMA};

	static const int32_t m_nIMAStepSizeTable[89];
	static const int16_t m_nMSAdpcmCoef[7][2]; 
	static const uint32_t m_nMSStepAdjustTable[16];
	unsigned char m_cIMAStateAdjustTable[ISSTMAX+1][8];
	static const int32_t m_nIMAIndexTable[16];

	DllExport CAdpcm(void);
	DllExport ~CAdpcm(void);

	DllExport void init(bool bBraindead,unsigned char cFill);
	DllExport void initImaTable(void);

	DllExport int16_t *pwIMAAdpcmDecompress(int16_t *pwOut,unsigned char *pIn,uint32_t nSize,uint32_t *pnOutSize,uint32_t nBlockAlign,uint32_t nSamplesPerBlock,uint32_t nChannels);
	DllExport unsigned char *pcIMAAdpcmCompress(int16_t *pIn,uint32_t nSize,uint32_t *pnOutSize,uint32_t nBlockAlign,uint32_t nSamplesPerBlock,uint32_t nChannels);

	DllExport int16_t nMSAdpcmDecompressSample(signed char c,MsState *state,int16_t sample1,int16_t sample2);
	DllExport void MSAdpcmDecompressBlock(uint32_t nChannels,uint32_t nCoef,const int16_t *iCoef,const unsigned char *ibuff,int16_t *obuff,int32_t n);

	DllExport void MSAdpcmCompressBlock(uint32_t nChannels,const int16_t *ip,int32_t n,int32_t *st,unsigned char *obuff,int32_t blockAlign);
	DllExport void MSAdpcmCompressChannel(uint32_t iChannel,uint32_t nChannels,const int16_t *ip,int32_t n,int32_t *st,unsigned char *obuff);
	DllExport int MSAdpcmCompressSample(uint32_t iChannel,uint32_t nChannels,int16_t v[2],const int16_t iCoef[2],const int16_t *ibuff,int32_t n,int32_t *iostep,unsigned char *obuff);

	DllExport void IMAAdpcmDecompressSample(uint32_t iChannel,uint32_t nChannels,const unsigned char *ibuff,int16_t *obuff,int32_t n);
	DllExport void IMAAdpcmDecompressBlock(uint32_t nChannels,const unsigned char *ibuff,int16_t *obuff,int32_t n);

	//DllExport int IMAAdpcmCompressSample(int ch,int chans,signed short v0,const signed short int *ibuff,int n,int *iostate,unsigned char *obuff);
	DllExport void IMAAdpcmCompressBlock(uint32_t nChannels,const int16_t *ip,uint32_t n,int32_t *iostate,unsigned char *obuff);
	DllExport void IMAAdpcmCompressChannel(uint32_t iChannel,uint32_t nChannels,const int16_t *ip,int32_t n,int32_t *st,unsigned char *obuff);

	DllExport unsigned int RMFAdpcmDecompress(unsigned char *pcDest,unsigned char *pcSource,uint32_t nSize,uint32_t nChannels);
	DllExport void RMFAdpcmCompress(unsigned char *pcDest,unsigned char *pcSource,uint32_t nSize,uint32_t nChannels,uint32_t nDestSize);

	DllExport const int *pnGetIMAStepSizeTable(void);
	DllExport const int16_t *pnGetMSCoef(void);
 
protected:
	bool m_bBraindead;				//set for SAGEM ADPCM to signal the strange way of storing IMA ADPCM data
	unsigned char m_cFillByte;		//used in SAGEM ADPCM 

	int32_t IMAAdpcmCompressChannelBlock(uint32_t iChannel,uint32_t nChannels,int16_t wPredictionValue,const int16_t *pwInput,uint32_t nSamplesPerBlock,int32_t *pwIOState,unsigned char *pcOutput);
	int32_t nIMAOptimizeState(int32_t nLevels,uint32_t iChannel,uint32_t nChannels,const int16_t *pwInput,uint32_t nSamplesPerBlock,int32_t nIOState);
};
#endif
