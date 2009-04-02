#ifndef SWFFileincluded
#define SWFFileincluded
class CSWFFile : public CMP3File
{
public:
	DYNOBJECT(CSWFFile)
	DYNDEFPROPERTY

	CSWFFile(void);
	virtual ~CSWFFile(void);
	virtual void Serialize(istream &ar);
	virtual void Serialize(ostream &ar);

	void Read(istream &ar);
	void Write(ostream &ar);

	bool bMagicHead(std::istream &ar,uint32_t nSize);

	int nGetFormat(void);
	static tstring sGetFormatName(int nFormat);
	bool bContainsMP3(void){return m_bContainsMP3;};
	uint32_t nGetLoopCount(void){return m_nLoops;};
	uint32_t nGetFrameRate(void) {return m_nFrameRate / 256;};
	uint32_t nGetFrameWidth(void) {return m_rcMovieSize.right-m_rcMovieSize.left;};
	uint32_t nGetFrameHeight(void) {return m_rcMovieSize.bottom-m_rcMovieSize.top;};

protected:
	#define unsignedvalue(a)	(unsigned int)(a < 0 ? (-a) : a)

	// Tag IDs (adopted from J. C. Kessels' Form2Flash)
	#define ST_END                  0
	#define ST_SHOWFRAME            1
	#define ST_DEFINESHAPE          2
	#define ST_FREECHARACTER        3
	#define ST_PLACEOBJECT          4
	#define ST_REMOVEOBJECT         5
	#define ST_DEFINEBITS           6
	#define ST_DEFINEBITSJPEG       6 
	#define ST_DEFINEBUTTON         7
	#define ST_JPEGTABLES           8
	#define ST_SETBACKGROUNDCOLOR   9
	#define ST_DEFINEFONT           10
	#define ST_DEFINETEXT           11
	#define ST_DOACTION             12
	#define ST_DEFINEFONTINFO       13
	#define ST_DEFINESOUND          14 /* Event sound tags. */
	#define ST_STARTSOUND           15
	#define ST_DEFINEBUTTONSOUND    17
	#define ST_SOUNDSTREAMHEAD      18
	#define ST_SOUNDSTREAMBLOCK     19
	#define ST_DEFINEBITSLOSSLESS   20 /* A bitmap using lossless zlib compression. */
	#define ST_DEFINEBITSJPEG2      21 /* A bitmap using an internal JPEG compression table. */
	#define ST_DEFINESHAPE2         22
	#define ST_DEFINEBUTTONCXFORM   23
	#define ST_PROTECT              24 /* This file should not be importable for editing. */
	#define ST_PLACEOBJECT2         26 /* The new style place w/ alpha color transform and name. */
	#define ST_REMOVEOBJECT2        28 /* A more compact remove object that omits the character tag (just depth). */
	#define ST_FREEALL              31 /* ? */
	#define ST_DEFINESHAPE3         32 /* A shape V3 includes alpha values. */
	#define ST_DEFINETEXT2          33 /* A text V2 includes alpha values. */
	#define ST_DEFINEBUTTON2        34 /* A button V2 includes color transform, alpha and multiple actions */
	#define ST_DEFINEBITSJPEG3      35 /* A JPEG bitmap with alpha info. */
	#define ST_DEFINEBITSLOSSLESS2  36 /* A lossless bitmap with alpha info. */
	#define ST_DEFINEEDITTEXT       37
	#define ST_DEFINEMOVIE          38
	#define ST_DEFINESPRITE         39 /* Define a sequence of tags that describe the behavior of a sprite. */
	#define ST_NAMECHARACTER        40 /* Name a character definition, character id and a string, (used for buttons, bitmaps, sprites and sounds). */
	#define ST_SERIALNUMBER         41
	#define ST_GENERATORTEXT        42 /* contains an id */
	#define ST_FRAMELABEL           43 /* A string label for the current frame. */
	#define ST_SOUNDSTREAMHEAD2     45 /* For lossless streaming sound, should not have needed this... */
	#define ST_DEFINEMORPHSHAPE     46 /* A morph shape definition */
	#define ST_DEFINEFONT2          48
	#define ST_TEMPLATECOMMAND      49
	#define ST_GENERATOR3           51
	#define ST_EXTERNALFONT         52
	#define ST_EXPORTASSETS			56
	#define ST_IMPORTASSETS			57
	#define ST_ENABLEDEBUGGER		58
	#define ST_DOINITACTION         59
	#define ST_DEFINEVIDEOSTREAM    60
	#define ST_VIDEOFRAME           61
	#define ST_DEFINEFONTINFO2		62
	#define ST_MX4					63 /*(?) */
	#define ST_SCRIPTLIMITS			65 /* version 7- u16 maxrecursedepth, u16 scripttimeoutseconds */
	#define ST_SETTABINDEX			66 /* version 7- u16 depth(!), u16 tab order value */

	#define SI_HASINPOINT	0x01
	#define SI_HASOUTPOINT	0x02
	#define SI_HASLOOPS		0x04
	#define SI_HASENVELOPE	0x08
	#define SI_NOMULTIPLE	0x10
	#define SI_STOP			0x20

	enum FormatIndex {	swfcodecUnknown=0,
						swfcodecRaw=1,
						swfcodecAdpcm=2,
						swfcodecMP3,
						swfcodecNellyMoser	};

	typedef struct
	{
		int left;
		int top;
		int right;
		int bottom;
	} swf_rectangle;

	typedef struct
	{
		unsigned int nID;
		unsigned int nDataSize;
		unsigned char *pcData;
		unsigned int nLen;
	} swf_tag;

	void readRect(swf_rectangle *rc);
	unsigned char readBit(void);
	int readBits(int nSize);
	bool readShort(unsigned short int *pn);
	bool readLong(uint32_t *pn);
	bool readBytes(void *pDest,unsigned int nSize);
	swf_tag *readTag(void);

	void writeFlush(void);
	void writeBit(bool bBit);
	void writeBits(int nBits,unsigned int nSize);
	unsigned int writeRect(swf_rectangle *pRect);
	unsigned int writeShort(short int nValue);
	unsigned int writeLong(unsigned int nValue);
	unsigned int writeBytes(const void *pSource,unsigned int nSize);

	bool bParseDefineSound(swf_tag *pTag);
	bool bParseStreamHead(swf_tag *pTag);
	bool bParseStreamBlock(swf_tag *pTag);
	bool bParseStartSound(swf_tag *pTag);

	void DecompressMP3(void *pMpeg,uint32_t nSize);
	void CompressMP3(CMobileSampleContent *pSource,unsigned char **ppDest,uint32_t *pnSize);
	
	void readInit(void *pSource,uint32_t nSize);
	void writeInit(void *pSource,uint32_t nSize);

	unsigned int nRenderHeader(CMobileSampleContent *pSource,swf_rectangle *pRect,unsigned int nFileSize=0);
	unsigned int nRenderBody(CMobileSampleContent *pSource,uint32_t nSampleCount,uint32_t nBackgroundRGB,unsigned char *pMpeg,uint32_t nMpegSize);
	unsigned int nRenderTag(unsigned int nId, unsigned int nSize=0, void *pData=NULL);
	
	unsigned int nRenderDefineSound(unsigned int nSoundId,uint32_t nSampleCount,CMobileSampleContent *pSource,unsigned char *pMpeg,uint32_t nMpegSize);
	unsigned int nRenderShowFrame(void);
	unsigned int nRenderEnd(void);
	unsigned int nRenderBackgroundColor(unsigned char cRed,unsigned char cGreen,unsigned char cBlue);
	unsigned int nRenderStartSound(unsigned int nSoundId,unsigned short int nLoops=0);

	swf_rectangle m_rcMovieSize;
	unsigned short int m_nFrameRate;
	unsigned short int m_nFrameCount;

	int m_nFormatVersion;
	bool m_bCompressed;
	int m_nSoundCompression;
	bool m_bContainsSound;
	bool m_bContainsPicture;
	bool m_bContainsMP3;
	bool m_bUsesStreaming;
	unsigned short int m_nLoops;
	uint32_t m_nCueIn;
	uint32_t m_nCueOut;

	Endian endian;

	CBufferCollector m_bc;

	unsigned int m_nBitpos;
	unsigned int m_nBitCount;
	unsigned char m_cBitBuffer;
	unsigned char *m_pc;
	unsigned char *m_pcStart;
	unsigned char *m_pcEnd;
};
#endif
