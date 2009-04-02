/*\
 * MFMSample.cpp
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
#include <string>
#ifndef WIN32
#include <netinet/in.h>
#endif
#include "../retroBase/MyString.h"
#include "../retroBase/Integer.h"
#include "../retroBase/Adpcm.h"
#include "../retroBase/MobileContent.h"
#include "MFMSample.h"

static signed short okiSTEPSIZE[49] = 
{ 
	16,  17,  19,  21,  23,  25,  28, 
	31,  34,  37,  41,  45,  50,  55, 
	60,  66,  73,  80,  88,  97,  107, 
	118, 130, 143, 157, 173, 190, 209, 
	230, 253, 279, 307, 337, 371, 408, 
	449, 494, 544, 598, 658, 724, 796, 
	876, 963, 1060,1166,1282,1411,1552 
};

static signed short okiSTEPADJUST[8] = { -1,-1,-1,-1,2,4,6,8 }; 

CMFMSample::CMFMSample(unsigned char *pData,uint32_t nSize)
{
	m_nSize=nSize;
	m_pcAdpcm=NULL;
	if (nSize)
	{
		m_pcAdpcm=new unsigned char [nSize];
		CopyMemory(m_pcAdpcm,pData,nSize);
	}
	m_nCodecLast=0;
	m_nCodecIndex=0;
}

CMFMSample::~CMFMSample(void)
{
	if (m_pcAdpcm != NULL)
		delete [] m_pcAdpcm;
}

/*\ 
 * <----------  ----------> 
 * @m 
 * --> I N <-- @p
 * void - 
 * <-- OUT --> @r
 * void CCMXSample::ExportRaw  - 
 \*/ 
void CMFMSample::ExportRaw (void)
{
	const char cName[255]="sound.raw";
	FILE *fp;

	TRACEIT2("exporting sound data as \"%s\"\n",cName);

	if ((fp=fopen(cName,"wb")) != NULL)
	{
		fwrite(m_pcAdpcm,m_nSize,1,fp);
		fclose(fp);
	}
	else
	{
		Log2(verbLevErrors,"couldnt create output file \"%s\"\n",cName);
		throw new CFormatException(CFormatException::formaterrUnknown);
	}
}

void CMFMSample::Decode(signed short int *pcDest,uint32_t  nSize)
{
	DecodeOKI(pcDest,nSize);
}

void CMFMSample::Encode(signed short int *pcSource,uint32_t nSize)
{
	EncodeOKI(pcSource,nSize);
}

unsigned char CMFMSample::cEncodeOKISample(signed short int sample)
{ 
	unsigned char code;
    short   dn;
    short   ss;
    ss   = okiSTEPSIZE[m_nCodecIndex];
    code = 0x00;
    if ((dn = sample - m_nCodecLast) < 0)
    { 
		code = 0x08;
        dn   = -dn;
    }
    if (dn >= ss) 
    { 
		code = code | 0x04;
        dn  -= ss;
    }
    if (dn >= ss/2)
    { 
		code = code | 0x02;
	    dn  -= ss/2;
    }
    if (dn >= ss/4)
        code = code | 0x01;
    // ... use decoder to set the estimate of last sample and adjust the step index
    m_nCodecLast = nDecodeOKISample(code);
    return code;
}

signed short CMFMSample::nDecodeOKISample(unsigned char code)
{
	signed short dn;
	signed short ss;
	signed short sample;

	ss = okiSTEPSIZE[m_nCodecIndex];
	dn = ss/8;
	if (code & 0x01)
		dn += ss/4;
	if (code & 0x02)
		dn += ss/2;
	if (code & 0x04)
		dn += ss;
	if (code & 0x08)
		dn = -dn;
	sample = m_nCodecLast + dn;
	// ... clip to 12 bits
	if (sample > 2047)
		sample = 2047;
	if (sample < -2048)
		sample = -2048;
	// ... adjust step size
	m_nCodecLast   = sample;
	m_nCodecIndex += okiSTEPADJUST[code & 0x07];
	if (m_nCodecIndex < 0) 
		m_nCodecIndex = 0;
	if (m_nCodecIndex > 48) 
		m_nCodecIndex = 48;
	// ... done
	return sample;
}

void CMFMSample::EncodeOKI(signed short int *pcSrc,uint32_t nSize)
{
	uint32_t nLeft;
	unsigned char *p;
	m_nSize=nSize/4;
	if (nSize%4)
		++m_nSize;
	m_pcAdpcm=new unsigned char [m_nSize];
	p=m_pcAdpcm;
	nLeft=m_nSize;
	while(nLeft)
	{
		*p=cEncodeOKISample((*(pcSrc++))>>4)<<4;
		*p|=cEncodeOKISample((*(pcSrc++))>>4);
		--nLeft;
		++p;
	};
}

void CMFMSample::DecodeOKI(signed short int *pcDest,uint32_t nSize) 
{ 
	signed short int *pnOut;
	unsigned char *p;
	unsigned char code;

	ASSERT (pcDest);
	ASSERT (nSize);
	if (pcDest && nSize)
	{	
		p=m_pcAdpcm;
		pnOut=pcDest;

		m_nCodecLast=0;
		m_nCodecIndex=0;
		uint32_t nLeft=m_nSize;

		//as long as there are bytes left...
		while(nLeft)
		{
			//decode upper nibble
			code=(*p>>4)&0x0F;
			*(pnOut++)=nDecodeOKISample(code)<<4;
			//decode lower nibble
			code=*p&0x0F;
			*(pnOut++)=nDecodeOKISample(code)<<4;
			--nLeft;
			++p;
		};
	}
}
