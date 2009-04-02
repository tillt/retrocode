#ifndef CMXSAMPLE_INCLUDED
#define CMXSAMPLE_INCLUDED
class CCMXPacket
{
public:
	CCMXPacket(unsigned char *pBuffer, uint32_t nSize);
	~CCMXPacket(void);

	unsigned char *pGetData(void){return m_pData;};
	uint32_t nGetSize(void){return m_nSize;};
	uint32_t nGetDelta(uint32_t nFormat,uint32_t nSampleRate,uint32_t nSampleDelta,uint32_t nDoneDelta,uint32_t nDoneSize);

	enum cmxSAMPLEFORMATS {
		fmtWave=0,fmtQCELP=4,fmtIMA=5,fmtUnknown=6
	};

protected:
	unsigned char *m_pData;
	uint32_t m_nSize;
};

class CCMXSample : public CAdpcm
{
public:
	CCMXSample(uint32_t nFormat,uint32_t nSampleRate);
	~CCMXSample(void);

	void Decode(signed short int **pcDest,uint32_t *pSize);
	void Encode(signed short int *pcSource,uint32_t nSize);

	void AddPacket(unsigned char *pData,uint32_t nSize);
	
	uint32_t nGetSize(void){return m_nSizeSum;};
	uint32_t nGetPacketCount(void){return (uint32_t)m_Packets.size();};
	CCMXPacket *pGetPacket(unsigned int nIndex) {return m_Packets[nIndex];};
	uint32_t nGetSampleRate(void) {return m_nSampleRate;};
	uint32_t nGetFormat(void) {return m_nFormat;};
	int nGetDelta(void) {return m_nDelta;};
	uint32_t nGetFragmentDelta(uint32_t nFragmentPosition,uint32_t nFragmentTime);
	void ExportRaw (void);

protected:
	int m_nDelta;
	uint32_t m_nSampleRate;
	uint32_t m_nFormat;
	uint32_t m_nSizeSum;
	vector<CCMXPacket *> m_Packets;

	void EncodeQCELP(signed short int *pcSource,uint32_t nSize);
	void EncodeIMA(signed short int *pcSource,uint32_t nSize);

	void DecodeQCELP(signed short int **pcDest,uint32_t *pSize);
	void DecodeIMA(signed short int **pcDest,uint32_t *pSize);
};
#endif
