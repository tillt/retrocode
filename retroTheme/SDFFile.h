#ifndef SDFFILEincluded
#define SDFFILEincluded
class CSDFFile : public CThemeBaseContent
{
public:
	DYNOBJECT(CSDFFile)
	DYNDEFPROPERTY

	CSDFFile(void);
	virtual ~CSDFFile(void);

	void Read(istream &ar);

	tstring sGetTitle(void) {return m_strResMeta[resmetaName];};
	tstring sGetAuthor(void) {return m_strResMeta[resmetaAuthor];};
	tstring sGetVendor(void) {return m_strResMeta[resmetaVendor];};
	tstring sGetPublished(void) {return m_strResMeta[resmetaPublished];};
	tstring sGetUrl(void) {return m_strResMeta[resmetaUrl];};
	tstring sGetCopyright(void) {return m_strResMeta[resmetaCopyright];};
	tstring sGetDescription(void) {return m_strResMeta[resmetaDescription];};
		
	bool bMagicHead(std::istream &ar,uint32_t nSize);

protected:
	void ParseXmlData(char *pcDest,unsigned int nSize);

	tstring sGetDeviceName(void);

	enum 
	{
		resmetaName,
		resmetaAuthor,
		resmetaPublished,
		resmetaVersion,
		resmetaResolution,
		resmetaVendor,
		resmetaPrice,
		resmetaUrl,
		resmetaCopyright,
		resmetaDescription,

		resmetaLast
	};
	tstring m_strResMeta[resmetaLast];
};
#endif
