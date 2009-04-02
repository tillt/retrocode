/*\
 * MemoryBitmap.cpp
 * Copyright (C) 2004-2007, MMSGURU - written by Till Toenshoff
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
\*/
#include "stdafx.h"
#include "Integer.h"
#include "MemoryBitmap.h"
#include "Basics.h"

CMemoryBitmap::CMemoryBitmap(unsigned int nWidth,unsigned int nHeight,unsigned int nFramePix,MyCol *pColBack,MyCol *pColPen) :	m_nWidth(nWidth+(nFramePix*2)),
																																m_nHeight(nHeight+(nFramePix*2)),
																																m_nFramePix(nFramePix)
{
	m_nSize=(m_nWidth*sizeof(MyCol))*m_nHeight;
	m_pcImage=new unsigned char[m_nSize];
	CopyMemory(&m_colBack,pColBack,sizeof(MyCol));
	CopyMemory(&m_colPen,pColPen,sizeof(MyCol));
}

CMemoryBitmap::~CMemoryBitmap(void)
{
	if (m_pcImage != NULL)
		delete [] m_pcImage;
	m_pcImage=NULL;
}

/*\
 * <---------- Gloss ---------->
 * @m draw a fading highlite over the image
 * --> I N <-- @p
 * unsigned int y - top
 * unsigned int yy - bottom
 * double dAmpFrom - alpha from
 * double dAmpTo - alpha to
\*/
void CMemoryBitmap::Gloss(unsigned int x,unsigned int y,unsigned int xx,unsigned int yy,double dAmpFrom,double dAmpTo)
{
	double dAmpDelta=(dAmpTo-dAmpFrom)/(yy-y);
	double dAmp=dAmpFrom;
	double dAlphaGrayLevel;
	MyCol *p;

	if (m_pcImage)
	{
		p=(MyCol *)m_pcImage+(y * nGetWidth());									//skip top gloss frame

		for (;y < yy;y++)
		{
			dAlphaGrayLevel=255.0*(1.0-dAmp);												//calc alpha level result for the mixed color component
			p+=x;																			//skip left frame
			for (unsigned int lx=x ; lx < xx; lx++)
			{
				p->cBlue=(unsigned char)min(((double)p->cBlue*dAmp) + dAlphaGrayLevel,255.0);		//calc effect on individual 
				p->cGreen=(unsigned char)min(((double)p->cGreen*dAmp) + dAlphaGrayLevel,255.0);		//pixel and color components
				p->cRed=(unsigned char)min(((double)p->cRed*dAmp) + dAlphaGrayLevel,255.0);			//
				p++;
			}
			p+=nGetWidth() - xx;															//skip right frame
			dAmp+=dAmpDelta;																//ramp the amplification level / alpha level
		}
	}
	else
	{
		TRACEIT2("no bitmap in memory!\n");
	}
}

void CMemoryBitmap::Line(unsigned int x0,unsigned int y0,unsigned int x1,unsigned int y1)
{
	unsigned int nBuffer;
	bool steep = abs((signed int)y1 - (signed int)y0) > abs((signed int)x1 - (signed int)x0);
	if (steep)
	{
		nBuffer=x0;
		x0=y0;
		y0=nBuffer;

		nBuffer=x1;
		x1=y1;
		y1=nBuffer;
	}
	if (x0 > x1)
	{
		nBuffer=x0;
		x0=x1;
		x1=nBuffer;

		nBuffer=y0;
		y0=y1;
		y1=nBuffer;
	}
	int deltax = x1 - x0;
	int deltay = abs((signed int)y1 - (signed int)y0);
    int error = 0;
    int y = y0;
	int ystep;

	if (y0 < y1)
		ystep = 1;
	else 
		ystep = -1;

    for (unsigned int x=x0;x < x1;x++)
	{
		if (steep)
			Plot(y,x);
		else 
			Plot(x,y);
		error += deltay;
		if (2*error >= deltax)
		{
             y += ystep;
             error -= deltax;
		}
	}
}

/*\
 * <---------- Erase ---------->
 * @m fill image with background color
\*/
void CMemoryBitmap::Erase(void)
{
	if (m_pcImage)
	{
		unsigned char *p=m_pcImage;
		for (unsigned int y=0;y < m_nHeight;y++)
		{
			for (unsigned int x=0;x < m_nWidth;x++)
			{
				CopyMemory(p,&m_colBack,sizeof(MyCol));
				p+=sizeof(MyCol);
			}
		}
	}
	else
	{
		TRACEIT2("no bitmap in memory!\n");
	}
}

/*\
 * <---------- FadeFill ---------->
 * @m fill background with a vertical fade
 * --> I N <-- @p
 * MyCol *pFrom - 
 * MyCol *pTo - 
\*/
void CMemoryBitmap::FadeFill(MyCol *pFrom,MyCol *pTo)
{
	double r,g,b;
	double dr,dg,db;
	MyCol col;

	r=pFrom->cRed;
	g=pFrom->cGreen;
	b=pFrom->cBlue;

	dr=((double)pTo->cRed - r) / m_nHeight;
	dg=((double)pTo->cGreen - g) / m_nHeight;
	db=((double)pTo->cBlue - b) / m_nHeight;
	
	if (m_pcImage)
	{
		MyCol *p=(MyCol *)m_pcImage+(m_nWidth*m_nFramePix);
		for (unsigned int y=m_nFramePix;y < m_nHeight-m_nFramePix;y++)
		{
			col.cRed=(unsigned char)min(r,255.0);
			col.cGreen=(unsigned char)min(g,255.0);
			col.cBlue=(unsigned char)min(b,255.0);
			p+=m_nFramePix;
			for (unsigned int x=m_nFramePix;x < m_nWidth-m_nFramePix;x++)
			{
				CopyMemory(p,&col,sizeof(MyCol));
				p++;
			}
			p+=m_nFramePix;

			r+=dr;
			g+=dg;
			b+=db;
		}
	}
	else
	{
		TRACEIT2("no bitmap in memory!\n");
	}
}

/*\
 * <---------- LineVert ---------->
 * @m draw a vertical line using the pen color
 * --> I N <-- @p
 * unsigned int x - horizontal coordinate
 * unsigned int y - top
 * unsigned int yy - bottom
\*/
void CMemoryBitmap::LineVert(unsigned int x,unsigned int y,unsigned int yy,MyCol *pColArray)
{
	if (m_pcImage)
	{
		x+=m_nFramePix;
		y+=m_nFramePix;
		yy+=m_nFramePix;
		if (x >= m_nWidth)
		{
			TRACEIT2("x cropped in width !\n");
			x=m_nWidth-(m_nFramePix-1);
		}
		if (y >= m_nHeight)
		{
			TRACEIT2("y cropped in height !\n");
			y=m_nHeight-(m_nFramePix-1);
		}
		if (yy >= m_nHeight)
		{
			TRACEIT2("yy cropped in height !\n");
			yy=m_nHeight-(m_nFramePix-1);
		}
		for (unsigned int i=y;i < yy;i++)
			CopyMemory(m_pcImage+(x*sizeof(MyCol))+(i*(m_nWidth*sizeof(MyCol))),pColArray+i,sizeof(MyCol));
	}
	else
	{
		TRACEIT2("no bitmap in memory!\n");
	}
}

/*\
 * <---------- Plot ---------->
 * @m draw one pixel using the pen color
 * --> I N <-- @p
 * unsigned int x - horizontal coordinate
 * unsigned int y - vertical coordinate
\*/
void CMemoryBitmap::Plot(unsigned int x,unsigned int y)
{
	if (m_pcImage)
	{
		x+=m_nFramePix;
		y+=m_nFramePix;
		if (x >= m_nWidth)
		{
			TRACEIT2("x cropped in width !\n");
			x=m_nWidth-1;
		}
		if (y >= m_nHeight)
		{
			TRACEIT2("y cropped in height !\n");
			y=m_nHeight-1;
		}
		CopyMemory(m_pcImage+(x*sizeof(MyCol))+(y*(m_nWidth*sizeof(MyCol))),&m_colPen,sizeof(MyCol));
	}
	else
	{
		TRACEIT2("no bitmap in memory!\n");
	}
}
