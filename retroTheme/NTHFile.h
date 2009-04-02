#ifndef NTHFILEincluded
#define NTHFILEincluded
class CNTHFile : public CThemeBaseContent
{
public:
	DYNOBJECT(CNTHFile)
	DYNDEFPROPERTY

	CNTHFile(void);
	virtual ~CNTHFile(void);

	void Read(istream &ar);

	static int nGetSubFormat(const char *szVersion);
	static tstring sGetFormatName(int nSubformat);

	tstring sGetTitle(void) {return m_strName;};
	tstring sGetVersion(void ){return m_strVersion;};

	bool bMagicHead(std::istream &ar,uint32_t nSize);
	
protected:
	void ParseXmlData(char *pcDest,unsigned int nSize);

	tstring m_strName;
	tstring m_strVersion;
};
#endif
