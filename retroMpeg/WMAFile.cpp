/*\
 * WMAFile.cpp
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
#include "WMAFile.h"

 
DYNIMPPROPERTY(CWMAFile,CFFMPEGProperty)

CWMAFile::CWMAFile(void)
{
	m_nMagicSize=16;
	m_pcMagic="\060\046\262\165\216\146\317\021\246\331\000\252\000\142\316\154";
	m_sFormatName="Windows Media Audio (WMA)";
	m_sDefaultExtension=_T("wma");
	m_strFFMPEGFormatName=_T("asf");
	m_nCodecId=CODEC_ID_MP3;

	m_encodingPara.addPara(cmdParaString,paraStrCopyright);
	m_encodingPara.addPara(cmdParaNumber,paraNumBitrate);
	m_encodingPara.addPara(cmdParaString,paraStrTitle);
	m_encodingPara.addPara(cmdParaString,paraStrArtist);
	m_encodingPara.addPara(cmdParaString,paraStrNote);

	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono|CSampleCompatibility::supportsStereo].addDigit(11025);
	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono|CSampleCompatibility::supportsStereo].addDigit(16000);
	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono|CSampleCompatibility::supportsStereo].addDigit(22050);
	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono|CSampleCompatibility::supportsStereo].addDigit(24000);
	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono|CSampleCompatibility::supportsStereo].addDigit(32000);
	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono|CSampleCompatibility::supportsStereo].addDigit(44100);
	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono|CSampleCompatibility::supportsStereo].addDigit(48000);
}

CWMAFile::~CWMAFile(void)
{
}
