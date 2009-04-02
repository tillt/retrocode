#ifndef FFMPEGFILEincluded
#define FFMPEGFILEincluded
LPCTSTR szGetFFMPEGVersion(void);
class CFFMPEGFile : public CMobileSampleContent
{
public:
	CFFMPEGFile(void);
	virtual ~CFFMPEGFile(void);

	virtual void Read(istream &ar);
	virtual void Write(ostream &out);

	uint32_t nGetChannels(void) {return m_nCSChannels;};
	uint32_t nGetFormat(void) {return m_nCodecId;};	
	uint32_t nGetPlaytime(void) {return m_nPlayTime;};	
	static tstring sGetFormatName(int nCodec);
	static tstring sGetError(int err);

protected:
	void get_audio_frame(int16_t *samples, int frame_size, int nb_channels);
	AVStream *add_audio_stream(AVFormatContext *oc, int codec_id);

	void write_audio_frame(AVFormatContext *oc, AVStream *st);
	void close_audio(AVFormatContext *oc, AVStream *st);

	static int FFMPEGread(void *opaque, uint8_t *buf, int buf_size);
	static int FFMPEGwrite(void *opaque, uint8_t *buf, int buf_size);
	static offset_t FFMPEGseek_in(void *opaque, offset_t offset, int whence);
	static offset_t FFMPEGseek_out(void *opaque, offset_t offset, int whence);

	virtual int nLocateBestAudioStream(AVFormatContext *pFormatContext);

	uint32_t m_nEncBytesLeft;
	char *m_pcEnc;
	//CMobileSampleContent *m_pSource;
	void *m_pIOBuffer;
	uint32_t m_nIOBufferSize;
	uint32_t m_nFFMPEGDefaultBitrate;
	CMyString m_strFFMPEGFormatName;

	uint32_t m_nBitRate;
	uint32_t m_nPlayTime;
	uint32_t m_nFrameSize;
	uint32_t m_nSubFormat;
	uint32_t m_nFrameCount;
	uint32_t m_nCodecId;

	friend class CFFMPEGProperty;
};
#endif
