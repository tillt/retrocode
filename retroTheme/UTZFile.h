#ifndef UTZFILEincluded
#define UTZFILEincluded
class CUTZFile : public CThemeBaseContent
{
public:
	DYNOBJECT(CUTZFile)
	DYNDEFPROPERTY

	CUTZFile(void);
	virtual ~CUTZFile(void);

	void Read(istream &ar);

	int nGetSubFormat(void);
	tstring sGetFormatName(void);

	tstring sGetTitle(void) {return m_strName;};
	tstring sGetAuthor(void) {return m_strAuthor;};
	tstring sGetCompany(void) {return m_strCompany;};

	bool bMagicHead(std::istream &ar,uint32_t nSize);

protected:
	void ParseXmlData(char *pcDest,unsigned int nSize);

	tstring m_strName;
	tstring m_strAuthor;
	tstring m_strCompany;
	tstring m_strScreenWidth;
	tstring m_strScreenHeight;
	tstring m_strDeviceName;

	tstring m_strVersion;
};
#endif
