#ifndef JARFILEincluded
#define JARFILEincluded
#include "../retroBase/MobileContent.h"

class CJARFile : public CMobileContent
{
public:
	DYNOBJECT(CJARFile)
	DYNDEFPROPERTY

	CJARFile(void);
	virtual ~CJARFile(void);

	void Read(istream &ar);

	int nGetSubFormat(void);
	tstring sGetFormatName(int nFormatId);

	tstring sGetTitle(void) {return m_strName;};
	tstring sGetCompany(void) {return m_strCompany;};

	bool bMagicHead(std::istream &ar,uint32_t nSize);

protected:
	void ParseManifestData(char *pcDest,unsigned int nSize);

	tstring m_strName;
	tstring m_strCLDC;
	tstring m_strMIDP;
	tstring m_strCompany;
	tstring m_strVersion;
};
#endif
