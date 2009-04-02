#ifndef MONOFILEincluded
#define MONOFILEincluded
class CMonoContent : public CMobileContent
{
public:
	CMonoContent(void);
	virtual ~CMonoContent(void);
	
	int nGetEncoding(void) {return m_nEncoding;};
	int nGetEventCount(void) {return m_nEventCount;};
	int nGetSMSCount(void);
	int nGetPlaytime(void);

	unsigned char cReadHexByte(std::istream &ar);
	
	static tstring sGetEncodingName(int nEncoding);
protected:
	int m_nEventCount;
	int m_nEncoding;
	int m_nLastQuanta;
	int m_nTempo;
	int m_nSize;
	int m_nMaxSize;

	unsigned char *m_pcData;

	void HexToBin(unsigned char *pcDest,unsigned char *pcSrc,int nSize);
	bool bIsText(unsigned char ch);
	bool bIsHex(unsigned char ch);

	static const int encodeBinary=0;
	static const int encodeHex=1;
	static const int encodeText=2;
};
#endif

