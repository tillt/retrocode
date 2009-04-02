#ifndef MFMSAMPLE_INCLUDED
#define MFMSAMPLE_INCLUDED
class CMFMSample : public CAdpcm
{
public:
	CMFMSample(unsigned char *pcBuffer,uint32_t nSize);
	~CMFMSample(void);

	void Decode(signed short int *pcDest,uint32_t nSize);
	void Encode(signed short int *pcSource,uint32_t nSize);

	//void AddPacket(unsigned char *pData,uint32_tnSize);
	
	/*
	uint32_tnGetSize(void){return m_nSizeSum;};
	uint32_tnGetPacketCount(void){return (unsigned long)m_Packets.size();};
	CCMXPacket *pGetPacket(unsigned int nIndex) {return m_Packets[nIndex];};
	int nGetSampleRate(void) {return m_nSampleRate;};
	int nGetFormat(void) {return m_nFormat;};
	int nGetDelta(void) {return m_nDelta;};
	uint32_tnGetFragmentDelta(uint32_tint nFragmentPosition,uint32_tint nFragmentTime);
	void ExportRaw (void);
	*/
	void ExportRaw (void);
	uint32_t nGetSize(void){return m_nSize;};
	unsigned char *pcGetAdpcm(void){return m_pcAdpcm;};


protected:
	//int m_nDelta;
	uint32_t m_nSize;
	//vector<CCMXPacket *> m_Packets;
	unsigned char *m_pcAdpcm;

	signed short m_nCodecLast;
	signed short m_nCodecIndex;

	signed short nDecodeOKISample(unsigned char code);
	unsigned char cEncodeOKISample(signed short int sample);

	void DecodeOKI(signed short int *pcDest,uint32_t nSize);
	void EncodeOKI(signed short int *pcSrc,uint32_t nSize);
};
#endif
