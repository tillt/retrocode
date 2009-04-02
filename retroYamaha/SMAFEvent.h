#ifndef SMAFEVENT_Defined
#define SMAFEVENT_Defined
class CSMAFEvent
{
public:
	CSMAFEvent(uint32_t dwAt,uint32_t nChannel,uint32_t nNote,uint32_t nDuration,uint32_t nVolume);
	CSMAFEvent(uint32_t dwAt,uint32_t nChannel,uint8_t cCommand,uint8_t *pcData,uint32_t nSize);
	~CSMAFEvent();

	uint32_t dwGetAt(void){return m_dwAt;};
	uint32_t nGetChannel(void){return m_nChannel;};
	
	int32_t nGetNote(void){return m_nNote;};
	int32_t nGetDuration(void){return m_nDuration;};
	int32_t nGetVolume(void){return m_nVolume;};
	
	bool bIsControlEvent(void){return m_bIsControlEvent;};

	uint8_t cGetCommand(void){return m_cCommand;};
	uint8_t *pcGetData(void){return m_pcData;};
	uint32_t nGetSize(void){return m_nSize;};

protected:
	bool m_bIsControlEvent;

	uint32_t m_dwAt;
	uint32_t m_nChannel;

	//note specific
	uint32_t m_nNote;
	uint32_t m_nDuration;
	uint32_t m_nVolume;

	//control specific
	uint8_t m_cCommand;
	uint8_t *m_pcData;
	uint32_t m_nSize;
};
#endif
