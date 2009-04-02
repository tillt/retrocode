#ifndef WAVEFILEincluded
#define WAVEFILEincluded

#define MAKEID4(a,b,c,d)	( (uint32_t)a | ((uint32_t)b<<8) | ((uint32_t)c<<16) | ((uint32_t)d<<24) )

typedef struct tagWAVEHEADER 
{ 
    char				RIFF[4];                // 'R','I','F','F'                 
    uint32_t			nSize;                  // size of wave file from here on 
    char				WAVEfmt[8];             // 'W','A','V','E','f','m','t',' ' 
    uint32_t			nFormatLength;          // The length of the TAG format     
    uint16_t			wFormatTag;             // should be 1 for PCM type data   
    uint16_t			wChannels;              // should be 1 for MONO type data 
    uint32_t			nSamplesPerSec;         // should be 11025, 22050, 44100   
    uint32_t			nAvgBytesPerSec;        // Average Data Rate               
    uint16_t			wBlockAlign;            // 1 for 8 bit data, 2 for 16 bit 
    uint16_t			wBitsPerSample;		    // 8 for 8 bit data, 16 for 16 bit 
} WAVEHEADER; 

typedef struct tagADPCMCOEFFSET 
{
	int16_t	iCoef1;
	int16_t	iCoef2;
} ADPCMCOEFSET;

typedef struct tagADPCMFORMAT
{
	WAVEHEADER		*wave;
	uint16_t		wSamplesPerBlock;
	uint16_t		wNumCoef;
	ADPCMCOEFSET	*paCoeff;
}ADPCMWAVEFORMAT;

class CWaveFile : public CMobileSampleContent
{
public:
	DYNOBJECT(CWaveFile)
	DYNDEFPROPERTY

	CWaveFile(void);
	virtual ~CWaveFile(void);

	virtual void Read(istream &ar);
	virtual void Write(ostream &out);

	bool Load(char *pcBuffer,uint32_t nSize);
	bool Load(const char *pszPath);

	int nGetFormat(void) {return m_Header.wFormatTag;};
	static tstring sGetFormatName (int nFormat);
	
	bool bMagicHead(std::istream &ar,uint32_t nSize);

	enum {	compPCM=0x00,
			compIMAADPCM=0x01,
			compMSADPCM=0x02,
			compQCELP=0x03,
			compSAGEM=0x04		};
protected:
	void ReadData(istream &ar);
	void ReadHeader(istream &ar);

	uint32_t nReadLIST(istream &ar,uint32_t nListSize);
	uint32_t nRenderMetadata(ostream &out,CMobileSampleContent *pSource,bool bRender=true);

	int16_t *pwMSAdpcmDecompress(unsigned char *pIn,uint32_t nSize,uint32_t *pnOutSize);
	unsigned char *pcMSAdpcmCompress(int16_t *pIn,uint32_t nSize,uint32_t *pnOutSize);

	CAdpcm m_codec;
	bool m_bHeaderRead;
	uint32_t m_nAdpcmOffset;

	WAVEHEADER m_Header;
	ADPCMWAVEFORMAT m_Adpcm;
};
#endif
