#ifndef PNGFILEincluded
#define PNGFILEincluded
LPCTSTR szGetLibPngVersion(void);
class CPNGFile : public CMobileContent
{
public:
	DYNOBJECT(CPNGFile)
	DYNDEFPROPERTY

	CPNGFile(void);
	virtual ~CPNGFile(void);

	void Read(istream &ar);

	static tstring sGetFormatName (int nFormat);
	int nGetPlaytime();
	
	int nGetWidth();
	int nGetHeight();
	int nGetBitsPerPixel();
	int nGetColors();
	int nGetSubFormat(){return m_nSubFormat;};
	bool bIsInterlaced(){return m_bInterlaced;};
	bool bIsTransparent(){return m_bTransparent;};
	tstring sGetComment(){return m_sComment;};


protected:
	tstring sPNGError(void);

	tstring m_sComment;
	int		m_nSubFormat;
	int		m_nBitsPerPixel;
	int		m_nColors;
	int		m_nWidth;
	int		m_nHeight;
	bool	m_bInterlaced;
	bool	m_bTransparent;
};
#endif
