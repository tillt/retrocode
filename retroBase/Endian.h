#ifndef ENDIANincluded
#define ENDIANincluded

#ifdef WIN32
#ifndef RETROBASE_EXPORTS
#undef DllExport
#define DllExport 
#else
#undef DllExport
#define DllExport   __declspec( dllexport )
#endif
#else
#undef DllExport
#define DllExport 
#endif


class Endian
{
public:
	DllExport Endian();
	~Endian() {};

	DllExport void init(void);

	DllExport bool bIsBigEndian(void);

	DllExport void HostFromBigShortArray(unsigned short int *pcDest,unsigned short int *pcSource,uint32_t nSize);
	DllExport void BigFromHostShortArray(unsigned short int *pcDest,unsigned short int *pcSource,uint32_t nSize);
	DllExport void HostFromLittleShortArray(unsigned short int *pcDest,unsigned short int *pcSource,uint32_t nSize);
	#define LittleFromHostShortArray HostFromLittleShortArray

	int16_t (*wToBig)(int16_t s);
	#define wFromBig wToBig
	int16_t (*wToLittle)(int16_t s);
	#define wFromLittle wToLittle
	
	int32_t (*lToBig)(int32_t i);
	#define lFromBig lToBig
	int32_t (*lToLittle)(int32_t i);
	#define lFromLittle lToLittle

	float (*fToBig)(float f );
	#define fFromBig fToBig
	float (*fToLittle)(float f );
	#define fFromLittle fToLittle

protected:
	static int16_t wSwap(int16_t s);
	static int16_t wSwap(int16_t *s);
	static int16_t wNoSwap(int16_t s);

	static int32_t lSwap (int32_t i);
	static int32_t lSwap (int32_t *i);
	static int32_t lNoSwap(int32_t i);

	static float fSwap(float f);
	static float fNoSwap(float f);

	bool bBigEndianSystem;
};

#endif
