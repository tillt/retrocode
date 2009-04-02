#ifndef PACKET_COLLECTION_HEADER
#define PACKET_COLLECTION_HEADER
#ifndef RETROBASE_EXPORTS
#undef DllExport
#define DllExport 
#else
#undef DllExport
#define DllExport   __declspec( dllexport )
#endif

class CBufferEntry
{
public:
	CBufferEntry(unsigned char *pBuffer, uint32_t nSize);
	~CBufferEntry(void);

	unsigned char *pGetData(void){return m_pData;};
	uint32_t nGetSize(void){return m_nSize;};

protected:
	unsigned char *m_pData;
	uint32_t m_nSize;
};

class CBufferCollector
{
public:
	DllExport CBufferCollector(void);
	DllExport ~CBufferCollector(void);

	DllExport void CreateCopyPacket(unsigned char *pData,uint32_t nSize);
	DllExport uint32_t nCopyLinear(unsigned char *pDest,uint32_t nSize);
	
	DllExport CBufferEntry *pbeGetPacket(unsigned int iIndex) {return m_Packets[iIndex];};
	
	DllExport uint32_t nGetSize(void) {	return m_nTotalSize;		};
	DllExport uint32_t nGetCount(void) {	return (uint32_t)m_Packets.size();	};

protected:
	uint32_t m_nTotalSize;
	vector<CBufferEntry *>m_Packets;
};

#endif
