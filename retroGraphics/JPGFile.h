#ifndef JPGFILEincluded
#define JPGFILEincluded
LPCTSTR szGetLibJpegVersion(void);

class CJPGFile : public CMobileContent
{
public:
	DYNOBJECT(CJPGFile)
	DYNDEFPROPERTY

	CJPGFile(void);
	virtual ~CJPGFile(void);

	void Read(istream &ar);

	static tstring sGetFormatName (int nFormat);
	int nGetPlaytime();
	
	int nGetWidth();
	int nGetHeight();
	int nGetBitsPerPixel();
	int nGetColors();
	int nGetResolutionX(){return m_nResolutionX;};
	int nGetResolutionY(){return m_nResolutionY;};
	int nGetSubFormat(){return m_nSubFormat;};
	bool bIsProgressive(){return m_bProgressive;};


protected:
	tstring sJPGError(void);

	//static int JPGRead (JPGFileType *JPGFile, JPGByteType *cIn, int nLen);

	int		m_nSubFormat;
	int		m_nBitsPerPixel;
	int		m_nColors;
	int		m_nWidth;
	int		m_nHeight;
	int		m_nResolutionX;
	int		m_nResolutionY;
	bool	m_bProgressive;
};
#endif
