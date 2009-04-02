#ifndef THMSAMSUNGFILEincluded
#define THMSAMSUNGFILEincluded
class CTHMSamsungFile : public CThemeBaseContent
{
public:
	DYNOBJECT(CTHMSamsungFile)
	DYNDEFPROPERTY

	CTHMSamsungFile(void);
	virtual ~CTHMSamsungFile(void);

	void Read(istream &ar);

	int nGetSubFormat(void);
	tstring sGetFormatName(void);

	tstring sGetTitle(void) {return m_strName;};
	tstring sGetVersion(void ){return m_strVersion;};

	bool bMagicHead(std::istream &ar,uint32_t nSize);

protected:
	void ParseXmlData(char *pcDest,unsigned int nSize);

	tstring sGetDeviceName(void);

	tstring m_strName;
	tstring m_strVersion;
	tstring m_strProfile;
};
#endif
