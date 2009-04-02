#ifndef MP3FILEincluded
#define MP3FILEincluded
LPCTSTR szGetLameVersion(void);
LPCTSTR szGetMadVersion(void);
class CMP3Frame
{	
public:
	CMP3Frame ();
	virtual ~CMP3Frame ();

	typedef struct 
	{
		char tag[3];
		char title[30];
		char artist[30];
		char album[30];
		char year[4];
		char comment[30];
		unsigned char genre;
	}id3tag;

    int			stereo;				//FALSE-mono, TRUE stereo
    int			jsbound;			//
    int			single;				//
    int			II_sblimit;			//
    int			lsf;				//
    int			mpeg25;				//
    int			down_sample;		//	
    int			header_change;		//
    int			block_size;			//
    int			lay;				//layer (1,2,3 and 0 for unknown)
    int			error_protection;	//using CRC
    int			bitrate_index;		//bitrate index
    int			sampling_frequency;	//frequency
    int			padding;			//
    int			extension;			//		
    int			mode;				//acoustic mode
    int			mode_ext;			//
    int			copyright;			//
    int			original;			//
    int			emphasis;			//
    LPVOID		alloc;				//
	//LPVOID		m_pLayer;			//
	//LPVOID		m_pSynth;			//
	int			m_nSsize;
	id3tag		m_ID3;				//
};

class CMP3File : public CMobileSampleContent
{
public:
	DYNOBJECT(CMP3File)
	DYNDEFPROPERTY

	CMP3File(void);
	~CMP3File(void);

	virtual bool bMagicHead(std::istream &ar,uint32_t nSize);
	virtual void Write(ostream &out);
	virtual void Read(istream &ar);

	uint32_t nGetBitRate(void){return m_nBitRate;};
	uint32_t nGetPlaytime(void){return m_nPlayTime;};
	uint32_t nGetSubFormat(void){return m_nSubFormat;};
	bool bGetCRC(void){return m_bCRC;};
	bool bGetVBR(void){return m_bVBR;};
	bool bGetJointStereo(void){return m_bJointStereo;};
	void InitInfoFromFile(std::istream &ar);
	uint32_t nGetMinBitRate(void){return m_nMinBitRate;};
	uint32_t nGetMaxBitRate(void){return m_nMaxBitRate;};
	bool bGetCopyrightBit(void){return m_bCopyrightBit;};
	bool bGetOriginalBit(void){return m_bOriginalBit;};
	bool bGetPrivateBit(void){return m_bPrivateBit;};

	static tstring sGetFormatName(int nFormat);

	tstring sGetInfoText(int iIndex){return m_strInfo[iIndex];};
	void SetInfoText(int iIndex,LPCTSTR pszInfo){m_strInfo[iIndex]=pszInfo;};

protected:
	#define RIFFtag MAKEFOURCC('R','I','F','F')
	#define LISTtag MAKEFOURCC('L','I','S','T')
	#define WAVEtag MAKEFOURCC('W','A','V','E')
	#define FMTtag  MAKEFOURCC('f','m','t',' ')
	#define DATAtag MAKEFOURCC('d','a','t','a')

	#define			AUDIOBUFSIZE			16384
	#define         MAX_NAME_SIZE           81
	#define         SBLIMIT                 32
	#define         SCALE_BLOCK             12
	#define         SSLIMIT                 18
	#define         MPG_MD_STEREO           0
	#define         MPG_MD_JOINT_STEREO     1
	#define         MPG_MD_DUAL_CHANNEL     2
	#define         MPG_MD_MONO             3
	#define			MAXOUTBURST				32768
	#define			HDRCMPMASK				0xfffffd00 	 

	void FOURCC2Str (uint32_t fcc, char *s);
	void ReadChunkHead (std::istream &ar,uint32_t *ID,uint32_t *size);
	BOOL ProcessChunk (std::istream &ar,uint32_t dwSeekPos,uint32_t DesiredTag,int nRekDepth,uint32_t *pdwChunkSize);

	int HeadRead (std::istream &ar,unsigned char *hbuf,uint32_t*newhead);
	bool HeadCheck(uint32_t newhead);

	void ReadFrameInit(void);
	int ReadFrame(std::istream &ar,CMP3Frame *fr);

	void GetIIStuff (CMP3Frame *fr);
	size_t ParseGenreNum(tstring &sIn);

protected:
	//int				filept_opened;
	int				m_nBS;
	uint32_t m_nOldHead;
	uint32_t m_nFirstHead;

	uint32_t m_nBitRate;
	uint32_t m_nMinBitRate;
	uint32_t m_nMaxBitRate;
	uint32_t m_nPlayTime;
	uint32_t m_nFrameSize;
	uint32_t m_nSubFormat;
	uint32_t m_nFrameCount;
	uint32_t m_nInvalidFrameCount;
	bool m_bCopyrightBit;
	bool m_bPrivateBit;
	bool m_bOriginalBit;
	bool m_bVBR;
	bool m_bCRC;
	uint32_t m_nFramSizeSum;
	bool m_bJointStereo;

	bool bIsLegalBitrate(uint32_t nBitrate);
	bool bIsLegalSampleRate(uint32_t nSampleRate);
	void PrintFrameInfo(struct mad_header *Header);
	void ApplyFilter(struct mad_frame *Frame);
	signed short MadFixedToSshort(int Fixed);
	//BOOL			m_bTryResync;
	//int				m_nFrameSize;
	//int				m_nOldFrameSize;

};
#endif
