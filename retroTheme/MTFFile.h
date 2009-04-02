#ifndef MTFFILEincluded
#define MTFFILEincluded
class CMTFFile : public CThemeBaseContent
{
public:
	DYNOBJECT(CMTFFile)
	DYNDEFPROPERTY

	CMTFFile(void);
	virtual ~CMTFFile(void);

	void Read(istream &ar);

	tstring sGetFormatName(void);
	int nGetSubFormat(void);

protected:
	tstring sReadWideString(istream &ar);
	void UpdateCheckSum(const char *pcIn,unsigned long nSize);
	void Stream(istream &ar,char *pcIn,unsigned long nSize);

	unsigned long m_nOffset;
	unsigned long m_nCheckSum;

	unsigned char m_cVersion;
};
#endif
