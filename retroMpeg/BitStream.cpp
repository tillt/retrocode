#include "stdafx.h"
#include <map>
#include <iostream>
#include <strstream>
#include <fstream>
#include "Basics.h"
#include "BitStream.h"

unsigned char CBitStream::cReverse(unsigned char cIn,int iBits)
{
	int iBit;
	unsigned char cOut=0x00;
	const unsigned char cMask[]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};

	for (iBit=0;iBit < iBits;iBit++)
	{
		if ((cIn & cMask[iBit]) == cMask[iBit])
			cOut|=cMask[(iBits-1)-iBit];
	}
	return cOut;
}

void CBitStream::Stream(unsigned char *pcDest,int *iOutByte,int *iOutBit,unsigned char cIn,int nBits)
{
	static unsigned char cOutByte;
	if (*iOutByte == 0 && *iOutBit == 0)
		cOutByte=0;
	while (nBits)
	{
		cOutByte=cOutByte << *iOutBit;
		cOutByte=cOutByte | (cIn >> *iOutBit);
		*iOutBit+=nBits;
		if (*iOutBit >= 8)
		{
			*(pcDest+(*iOutByte))=cOutByte;
			nBits=(*iOutBit)-8;

			(*iOutByte)++;
			*iOutBit=0;

			cIn=cIn&(0xFF>>(8-nBits));			
			cOutByte=0;
		}
		else
			nBits=0;
	};
}
