/*\
 * OGGFile.cpp
 * Copyright (C) 2004-2008, MMSGURU - written by Till Toenshoff
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
#include <map>
#include <math.h>
#include <iostream>
#include <strstream>
#include <fstream>
#include <stdlib.h>

#include "../retroBase/Basics.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MyString.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/PacketCollection.h"
#ifdef __cplusplus
extern "C"{
#endif 
#include "ffmpeg/avformat.h"
#include "ffmpeg/avcodec.h"
#ifdef __cplusplus
}
#endif
#include "FFMPEGFile.h"
#include "FFMPEGProperty.h"
#include "OGGFile.h"

DYNIMPPROPERTY(COGGFile,CFFMPEGProperty)

COGGFile::COGGFile(void)
{
	m_nMagicSize=4;
	m_pcMagic="OggS";
	m_sFormatName="OGG";
	m_nCodecId=CODEC_ID_VORBIS;
	m_sDefaultExtension=_T("ogg");
	m_strFFMPEGFormatName=_T("ogg");
	m_encodingPara.addPara(cmdParaNumber,paraNumBitrate);
	m_encodingCaps.freqs[	CSampleCompatibility::supports16Bit|
							CSampleCompatibility::supportsMono|
							CSampleCompatibility::supportsStereo].setRange(8000,192000);
}

COGGFile::~COGGFile(void)
{
}
