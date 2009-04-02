#ifndef JADFILEincluded
#define JADFILEincluded
class CJADFile : public CMobileContent
{
public:
	DYNOBJECT(CJADFile)
	DYNDEFPROPERTY

	CJADFile(void);
	virtual ~CJADFile(void);

	void Read(istream &ar) throw();

	int nGetSubFormat(void);
	tstring sGetFormatName(int nFormatId);

	tstring sGetTitle(void) {return m_strName;};
	tstring sGetCompany(void) {return m_strCompany;};

	bool bMagicHead(std::istream &ar,uint32_t nSize);

protected:
	void ParseData(char *pcDest,unsigned int nSize);

	tstring m_strName;
	tstring m_strCLDC;
	tstring m_strMIDP;
	tstring m_strCompany;
	tstring m_strVersion;
};
#endif
