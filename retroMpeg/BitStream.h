#ifndef BITSTREAMincluded
#define BITSTREAMincluded

class CBitStream
{
public:
	CBitStream(){};
	~CBitStream(){};

	static unsigned char cReverse(unsigned char cIn,int iBits);
	static void Stream(unsigned char *pcDest,int *iOutByte,int *iOutBit,unsigned char cIn,int nBits);
}; 

#endif
