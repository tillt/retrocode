#include "stdafx.h"
#include "../retroBase/Basics.h"
#include "SMAFEvent.h"

CSMAFEvent::CSMAFEvent(uint32_t dwAt,uint32_t nChannel,uint32_t nNote,uint32_t nDuration,uint32_t nVolume=100) : m_bIsControlEvent(false),m_dwAt(dwAt),m_nChannel(nChannel),m_nDuration(nDuration),m_nNote(nNote),m_nVolume(nVolume)
{
}

CSMAFEvent::CSMAFEvent(uint32_t dwAt,uint32_t nChannel,uint8_t cCommand,uint8_t *pcData,uint32_t nSize) : m_bIsControlEvent(true),m_dwAt(dwAt),m_nChannel(nChannel),m_cCommand(cCommand),m_pcData(pcData),m_nSize(nSize)
{
}

CSMAFEvent::~CSMAFEvent()
{
}

