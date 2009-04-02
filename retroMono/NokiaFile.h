#ifndef NOKIAFILEincluded
#define NOKIAFILEincluded
class CNokiaProperty;
class CNokiaFile : public CMonoContent
{
public:
	DYNOBJECT(CNokiaFile)
	DYNDEFPROPERTY

	CNokiaFile(void);
	virtual ~CNokiaFile(void);
	
	void Read(istream &ar);

	bool bMagicHead(std::istream &ar,uint32_t nSize);

	static tstring sGetLoopName(int nLoop);

	int bGetLoop(void) {return m_nLoop > 0;};
	tstring sGetName(void) {return m_strName;};

protected:
	int m_nLoop;
	tstring m_strName;

	unsigned char DecodeBits(const unsigned char *pBuffer, int &iByte, int &iBit, int nBits);
	bool Decode();
};

#define OTT_HEADID_PATTERN	0x00
#define OTT_HEADID_NOTE		0x01
#define OTT_HEADID_SCALE	0x02
#define OTT_HEADID_STYLE	0x03
#define OTT_HEADID_TEMPO	0x04
#define OTT_HEADID_VOLUME	0x05

#define OTT_COMMAND_CANCEL		05
#define OTT_COMMAND_RINGTONE	37
#define OTT_COMMAND_SOUND		29
#define OTT_COMMAND_UNICODE		34

#define OTT_SONGTYPE_BASIC		0x01
#define OTT_SONGTYPE_TEMP		0x02
#define OTT_SONGTYPE_MIDI		0x03
#define OTT_SONGTYPE_SAMPLE		0x04

#endif

