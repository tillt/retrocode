#ifndef RMFINSTRUMENT_Defined
#define RMFINSTRUMENT_Defined
class CRMFInstrument;
class CRMFInstrument
{
public:
	CRMFInstrument(const TCHAR *pszName=NULL);
	~CRMFInstrument();

	uint32_t nRender(rmfCACH *pCache=NULL,unsigned char *pDest=NULL);

	void Serialize(istream &ar);
	void Serialize(ostream &ar);

	tstring m_sName;
protected:
	
	
};
#endif
