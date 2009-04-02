/*\
 * RAWFile.cpp
 * Copyright (C) 2008, MMSGURU - written by Till Toenshoff
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
#include <iostream>
#include <map>
#include <math.h>
#include <strstream>
#include <fstream>
#include "../include/Resource.h"
#include "../retroBase/Basics.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MyString.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/Endian.h"
#include "../retroBase/Packer.h"
#include "RAWFile.h"
#include "RAWProperty.h"

DYNIMPPROPERTY(CRAWFile,CRAWProperty)

CRAWFile::CRAWFile(void)
{
	TRACEIT2("constructing RAW object\n");
	m_pcMagic="";
	m_nMagicSize=0;
	m_sFormatName=_T("Raw PCM");
	m_sDefaultExtension=_T("raw");
	//m_sFormatDescription.Load(IDS_FORMDESC_WAVE);
	m_encodingCaps.freqs[	CSampleCompatibility::supports8Bit|
							CSampleCompatibility::supports16Bit|
							CSampleCompatibility::supportsMono|
							CSampleCompatibility::supportsStereo].setRange(1,192000);
}

CRAWFile::~CRAWFile(void)
{
	TRACEIT2("deleting RAW object\n");
}

/*\ 
 * <---------- CRAWFile::Read ----------> 
 * @m read and parse a wave file
 * --> I N <-- @p
 * istream &ar - reference to the input stream
\*/ 
void CRAWFile::Read(istream &ar)
{
	Endian endian;
	endian.init();
	if (m_nCSSize)
	{
		m_pcCSBuffer=Alloc(m_nCSSize);
		ASSERT(m_pcCSBuffer);
		if(m_pcCSBuffer)
		{
			Log2(5,"trying to read %d bytes...\n",m_nCSSize);
			ar.read((char *)m_pcCSBuffer,m_nCSSize);
			hexdump("input wave: ",(unsigned char *)m_pcCSBuffer,40);
			//Log2(3,"format is pcm (%02Xh)\n",m_Header.wFormatTag);
			//if (m_Header.wBitsPerSample == 16)
			endian.HostFromLittleShortArray((uint16_t *)m_pcCSBuffer,(uint16_t *)m_pcCSBuffer,m_nCSSize);
		}
	}
}

/*\ 
 * <---------- CRAWFile::Serialize ----------> 
 * @m create destination wave format from raw source sample data
 * --> I N <-- @p
 * ostream &out - reference to the binary output stream
\*/ 
void CRAWFile::Write(ostream &out)
{
	Endian endian;
	endian.init();
	m_nCSBitsPerSample=16;
	m_nCSChannels=m_pCSSource->m_nCSChannels;
	m_nCSSamplesPerSecond=m_pCSSource->m_nCSSamplesPerSecond;
	m_nCSSize=m_pCSSource->m_nCSSize;
	m_pcCSBuffer=CMobileSampleContent::Alloc(m_nCSSize);
	if (m_pCSSource->m_nCSBitsPerSample == 16)
		endian.HostFromLittleShortArray((unsigned short *)m_pcCSBuffer,(unsigned short *)m_pCSSource->m_pcCSBuffer,m_nCSSize);
	else
		CopyMemory(m_pcCSBuffer,m_pCSSource->m_pcCSBuffer,m_nCSSize);
	out.write((char *)m_pcCSBuffer,m_nCSSize);
	m_nFileSize=m_nCSSize;
}
