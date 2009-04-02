#ifndef AVIFILEincluded
#define AVIFILEincluded

class CAVIStream
{
public:
	CAVIStream(uint32_t nFramePeriod) {m_nFramePeriod=nFramePeriod;};
	virtual ~CAVIStream(void) {};

	bool bReadHeader(istream &ar,uint32_t nSize);
	bool bRead(istream &ar,uint32_t nSize);
	tstring sGetCodecName(uint32_t nCodecId);

	uint32_t nGetType(void) {	return m_nType;	};
	uint32_t nGetCodecId(void)	{	return m_nVideoCodecId;	};
	uint32_t nGetCodecTag(void) {	return m_nVideoCodecTag;	};

	uint32_t nGetWidth(void)	{	return m_nVideoWidth;			};
	uint32_t nGetHeight(void)	{	return m_nVideoHeight;			};
	uint64_t llGetPlaytime(void){	return m_llDuration;			};

	uint32_t nGetBitPerPixel(void)	{	return m_nVideoBitsPerSample;	};
	uint32_t nGetFrameRate(void)		{	return m_nFrameRate;		};
	uint32_t nGetFrameCount(void)		{	return m_nFrames;			};

	WAVEHEADER *pGetWaveHeader(void) {	return &m_WaveHeader;	};

	enum AVICODECS
	{
		AVICODEC_H264,
		AVICODEC_H263,
		AVICODEC_H263P,
		AVICODEC_H263I,
		AVICODEC_H261,
		AVICODEC_MPEG4,
		AVICODEC_MSMPEG4V1,
		AVICODEC_MSMPEG4V2,
		AVICODEC_MSMPEG4V3,
		AVICODEC_WMV1,
		AVICODEC_WMV2,
		AVICODEC_DVVIDEO,
		AVICODEC_MPEG1VIDEO,
		AVICODEC_MPEG2VIDEO,
		AVICODEC_MJPEG,
		AVICODEC_LJPEG,
		AVICODEC_JPEGLS,
		AVICODEC_HUFFYUV,
		AVICODEC_FFVHUFF,
		AVICODEC_CYUV,
		AVICODEC_RAWVIDEO,
		AVICODEC_INDEO3,
		AVICODEC_VP3,
		AVICODEC_VP5,
		AVICODEC_VP6,
		AVICODEC_ASV1,
		AVICODEC_ASV2,
		AVICODEC_VCR1,
		AVICODEC_FFV1,
		AVICODEC_XAN_WC4,
		AVICODEC_MSRLE,
		AVICODEC_MSVIDEO1,
		AVICODEC_CINEPAK,
		AVICODEC_TRUEMOTION1,
		AVICODEC_MSZH,
		AVICODEC_ZLIB,
		AVICODEC_SNOW,
		AVICODEC_4XM,
		AVICODEC_FLV1,
		AVICODEC_FLASHSV,
		AVICODEC_VP6F,
		AVICODEC_SVQ1,
		AVICODEC_TSCC,
		AVICODEC_ULTI,
		AVICODEC_VIXL,
		AVICODEC_QPEG,
		AVICODEC_WMV3,
		AVICODEC_VC1,
		AVICODEC_LOCO,
		AVICODEC_WNV1,
		AVICODEC_AASC,
		AVICODEC_INDEO2,
		AVICODEC_FRAPS,
		AVICODEC_THEORA,
		AVICODEC_TRUEMOTION2,
		AVICODEC_CSCD,
		AVICODEC_ZMBV,
		AVICODEC_KMVC,
		AVICODEC_CAVS,
		AVICODEC_JPEG2000,
		AVICODEC_VMNC		};

protected:
	uint32_t	m_nType;
	uint32_t	m_nHandler;
	
	uint32_t	m_nVideoCodecTag;
	uint32_t	m_nVideoCodecId;

	uint32_t	m_nScale;
	uint32_t	m_nFrameRate;
	uint32_t	m_nFramePeriod;
	uint32_t	m_nCumLen;
	uint32_t	m_nFrames;

	WAVEHEADER	m_WaveHeader;
	uint32_t	m_nStartTime;
	uint64_t	m_llDuration;
	uint32_t	m_nSampleSize;
	uint32_t	m_nFrameOffset;
	bool		m_bInterleaved;
	uint32_t	m_nExtraDataSize;
	uint32_t	m_nVideoHeight;
	uint32_t	m_nVideoWidth;
	uint16_t	m_nVideoBitsPerSample;
};


class CAVIFile : public CMobileSampleContent
{
public:
	DYNOBJECT(CAVIFile)
	DYNDEFPROPERTY

	CAVIFile(void);
	~CAVIFile(void);

	void Read(istream &ar);
	bool bMagicHead(std::istream &ar,uint32_t nSize);

	int iGetFirstVideoStream(void);
	int iGetFirstAudioStream(void);

	CAVIStream *pGetStream(int i);
	unsigned int nGetBitRate(void) { return m_nBitRate;	};
	uint64_t llGetTotalStreamDataSize(void) { return m_nTotalStreamDataSize;};

protected:
	void ReadMetaTag(istream &ar,int nId,unsigned int nSize);

	vector<CAVIStream *> m_Streams;

	uint64_t		m_llMoviListPos;
	uint64_t		m_llMoviEndPos;

	unsigned int	m_nFramePeriod;
	unsigned int	m_nFrameBitRate;
	unsigned int	m_nBitRate;

	unsigned int	m_iCurrentStream;
	unsigned int	m_iNextStream;
	bool			m_bInterleaved;
	bool			m_bODML;

	uint64_t		m_nTotalStreamDataSize;
};
#endif
