#ifndef MemoryBitmapIncluded
#define MemoryBitmapIncluded
#include "Basics.h"
#include <memory.h>

#ifndef RETROBASE_EXPORTS
#undef DllExport
#define DllExport 
#else
#undef DllExport
#define DllExport   __declspec( dllexport )
#endif

typedef struct 
{
	unsigned char cRed;
	unsigned char cGreen;
	unsigned char cBlue;
}MyCol;

class CMemoryBitmap
{
public:
	DllExport CMemoryBitmap(unsigned int width,unsigned int height,unsigned int frame_pix,MyCol *pColBack,MyCol *pColPen);
	DllExport ~CMemoryBitmap(void);

	DllExport void SetPenCol(MyCol *pCol) { CopyMemory(&m_colPen,pCol,sizeof(MyCol)); };

	DllExport unsigned int nGetHeight(void) { return m_nHeight; };
	DllExport unsigned int nGetWidth(void) { return m_nWidth; };

	DllExport unsigned int nGetClientHeight(void) { return m_nHeight-m_nFramePix*2; };
	DllExport unsigned int nGetClientWidth(void) { return m_nWidth-m_nFramePix*2; };

	DllExport void Erase(void);

	DllExport void FadeFill(MyCol *pColFrom,MyCol *pColTo);

	DllExport void Plot(unsigned int x,unsigned int y);

	DllExport void LineVert(unsigned int x,unsigned int y,unsigned int yy,MyCol *pColLineSweep);
	DllExport void Line(unsigned int x,unsigned int y,unsigned int xx,unsigned int yy);

	DllExport void Gloss(unsigned int x,unsigned int y,unsigned int xx,unsigned int yy,double dAmpFrom,double dAmpTo);

protected:
	unsigned char *m_pcImage;
	unsigned int m_nSize;
	unsigned int m_nWidth;
	unsigned int m_nHeight;
	unsigned int m_nFramePix;
	MyCol m_colPen;
	MyCol m_colBack;
};
#endif
