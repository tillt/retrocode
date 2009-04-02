#ifndef PngMemoryBitmapIncluded
#define PngMemoryBitmapIncluded
/*
#include "windows.h"
*/
#include "../retroBase/MemoryBitmap.h"

class CPngMemoryBitmap : public CMemoryBitmap
{
public:
	CPngMemoryBitmap(unsigned int width,unsigned int height,unsigned int frame_pix,MyCol *pColBack,MyCol *pColPen);
	~CPngMemoryBitmap(void);

	bool bWritePng(const char *file_name);

	//HBITMAP hGetDDB(HDC hDeviceContext);

protected:
	//HBITMAP CreateAccessableRGBBitmap(HDC hdc,unsigned int nWidth,unsigned int nHeight,void **pDirect);
};
#endif
