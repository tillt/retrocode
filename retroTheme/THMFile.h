#ifndef THMFILEincluded
#define THMFILEincluded
class CTHMFile : public CThemeBaseContent
{
public:
	DYNOBJECT(CTHMFile)
	DYNDEFPROPERTY

	CTHMFile(void);
	virtual ~CTHMFile(void);

	void Read(istream &ar);

	int nGetSubFormat(void);
	tstring sGetFormatName(void);

	tstring sGetTitle(void) {return m_strName;};
	tstring sGetVersion(void ){return m_strVersion;};
	tstring sGetDevice(void){return m_strDeviceName;};
	tstring sGetAuthor(void);
	tstring sGetCompany(void){return m_strCompany;};
	tstring sGetEmail(void){return m_strEmail;};

protected:
	void ParseXmlData(char *pcDest,unsigned int nSize);

	tstring m_strDeviceName;
	tstring m_strName;
	tstring m_strFirstName;
	tstring m_strLastName;
	tstring m_strCompany;
	tstring m_strEmail;
	tstring m_strVersion;
};
#endif
