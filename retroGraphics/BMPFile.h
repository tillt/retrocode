#ifndef BMPFILEincluded
#define BMPFILEincluded
class CBMPFile : public CMobileContent
{
public:
	DYNOBJECT(CBMPFile)
	DYNDEFPROPERTY

	CBMPFile(void);
	virtual ~CBMPFile(void);

	void Read(istream &ar);

	static tstring sGetFormatName (int nFormat);
	static tstring sGetCompressionName (int nCompression);

	int nGetPlaytime();
	
	int nGetWidth();
	int nGetHeight();
	int nGetBitsPerPixel();
	int nGetColors();
	int nGetSubFormat(){return m_nSubFormat;};
	int nGetResolutionX(){return m_nResolutionX;};
	int nGetResolutionY(){return m_nResolutionY;};
	int nGetCompression(){return m_nCompression;};

protected:
	tstring sBMPError(void);

	int		m_nSubFormat;
	int		m_nCompression;
	int		m_nBitsPerPixel;
	int		m_nColors;
	int		m_nWidth;
	int		m_nHeight;
	int		m_nResolutionX;
	int		m_nResolutionY;
};
#endif
