#ifndef ANSI733Codec_included
#define ANSI733Codec_included
#include "qcelp/typedef_tty.h"

#ifndef USE_QUALCOMM_LIBRARY
#define NUMBYTES(a)	(((NUMBITS[a]+7)/8)+1)

class CANSI733Codec
{
public:
	CANSI733Codec(void);
	~CANSI733Codec(void);

	void Init(bool bModeEncode,bool bVarRate);
	void Deinit(void);

	unsigned long nDecode(short *pDest,unsigned char *pSource,unsigned long nPackets,unsigned long nSize);
	unsigned long nEncode(unsigned char *pDest,uint32_t *pnDestSize,short *pSource,unsigned long nSize);

	static const char *pszGetVersion(void)	{	return "TIA 50 ANSI-733 20040315-025 RetroPatch 0.3"; };

protected:
	tty_settings		*m_pTTY;
	struct CONTROL		*m_pControl;
	struct ENCODER_MEM	*m_pEncoder_memory;
	struct DECODER_MEM	*m_pDecoder_memory;
	struct PACKET		*m_pPacket;
};
#endif

#endif
