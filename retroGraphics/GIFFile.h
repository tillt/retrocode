#ifndef GIFFILEincluded
#define GIFFILEincluded

LPCTSTR szGetLibGifVersion(void);

class CGIFFile : public CMobileContent
{
public:
	DYNOBJECT(CGIFFile)
	DYNDEFPROPERTY

	CGIFFile(void);
	virtual ~CGIFFile(void);

	void Read(istream &ar);

	static tstring sGetFormatName (int nFormat);
	int nGetPlaytime();
	
	bool bMagicHead(std::istream &ar,uint32_t nSize);

	int nGetWidth();
	int nGetPlayTime();
	int nGetHeight();
	int nGetBitsPerPixel();
	int nGetColors();
	int nGetAverageInterFrameDelay();
	int nGetFrameCount();
	int nGetSubFormat();
	int nGetLoopCount(){return m_nLoopCount;};
	bool bIsInterlaced(){return m_bInterlaced;};
	bool bIsTransparent(){return m_bTransparent;};
	tstring sGetComment(){return m_sComment;};


protected:
	tstring sGifError(void);

	static int gifRead (void *GifFile,void *cIn, int nLen);

	tstring m_sComment;
	int		m_nFormatVersion;
	int     m_nPlaytime;
	int		m_nBitsPerPixel;
	int		m_nColors;
	int		m_nAverageInterFrameDelay;
	int		m_nFrames;
	int		m_nWidth;
	int		m_nLoopCount;
	int		m_nHeight;
	bool	m_bInterlaced;
	bool	m_bTransparent;
};
#endif
