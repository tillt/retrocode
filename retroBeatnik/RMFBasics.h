#ifndef RMFBASICS_Defined
#define RMFBASICS_Defined
typedef struct
{
	char sTag[4];
	uint32_t nFirstValue;
	uint32_t nDataSize;
	uint32_t nNameOffset;
	uint32_t nDataOffset;
}rmfCACH;

class CRMFBasics
{
public:
	CRMFBasics(){};
	~CRMFBasics(){};
	static bool bReadTag (istream &ar,uint32_t *pnTag,uint32_t *pnNextOffset);
	
	static unsigned char *pRenderInteger(unsigned char *pcDest,uint32_t nValue);

	static int nEncryptText(const unsigned char *pcIn,unsigned char *pcOut,uint32_t nMaxLen);
	static int nDecryptText(const unsigned char *pcIn,unsigned char *pcOut,uint32_t nMaxLen);

	static void EncryptBinary(unsigned char *pcIn,unsigned char *pcOut,int nLen);
	static void DecryptBinary(unsigned char *pcIn,unsigned char *pcOut,int nLen);

protected:
	static const uint32_t cryptionSeed=0x0000DCE5;
	static const uint32_t cryptionMultiplier=0x0000CE6D;
	static const uint32_t cryptionOffset=0x000058BF;
};
#endif
