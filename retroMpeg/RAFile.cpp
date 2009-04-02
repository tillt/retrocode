/*\
 * RAFile.cpp
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
#include "RAFile.h"

DYNIMPPROPERTY(CRAFile,CFFMPEGProperty)

CRAFile::CRAFile(void)
{
	m_nMagicSize=4;
	m_pcMagic="";
	m_sFormatName="Real Audio (RA)";
	m_sDefaultExtension=_T("ra");
	m_nFFMPEGDefaultBitrate=128000;
	m_nCodecId=CODEC_ID_AC3;
	m_strFFMPEGFormatName=_T("rm");
	m_sFormatDescription=_T("");
	m_sFormatCredits=_T("The WMA, RA and OGG codecs are entirely based on: \"avcodec\" Copyright (c) 2000-2004 Fabrice Bellard; \"avformat\"  Copyright (c) 2000-2004 Fabrice Bellard.");
	m_encodingPara.addPara(cmdParaNumber,paraNumBitrate);
	m_encodingPara.addPara(cmdParaString,paraStrTitle);
	m_encodingPara.addPara(cmdParaString,paraStrArtist);
	m_encodingPara.addPara(cmdParaString,paraStrNote);
	m_encodingPara.addPara(cmdParaString,paraStrCopyright);
	m_encodingCaps.freqs[	CSampleCompatibility::supports16Bit|
							CSampleCompatibility::supportsMono].setRange(8000,44100);

}

CRAFile::~CRAFile(void)
{
}

bool CRAFile::bMagicHead(std::istream &ar,uint32_t nSize)
{
	char sCompare[5];
	bool bRet=false;

	if (m_nMagicSize == 0 || (int)nSize < m_nMagicSize)
		return false;
	try 
	{
        ar.read(sCompare,4);
		sCompare[4]=0;
		if (!strcmp(sCompare,".ra\375") || !strcmp(sCompare,".RMF"))
			bRet=true;
	}
	catch(istream::failure const &e)
	{
		Log2(verbLevErrors,"read error on bit-stream magic head (%s)\n",e.what());
		TRACEIT("catching exception in bMagicHead\n");
		bRet=false;
	}
	return bRet;
}


int CRAFile::nLocateBestAudioStream(AVFormatContext *pFormatContext)
{
	int audioStream=-1;
	int nMaxBitRate=0;
	unsigned int i;
	for(i=0; i < pFormatContext->nb_streams; i++)
	{
		if(pFormatContext->streams[i]->codec->codec_type == CODEC_TYPE_AUDIO)
		{
			if ((pFormatContext->streams[i]->codec->codec_id == CODEC_ID_COOK ||
				pFormatContext->streams[i]->codec->codec_id == CODEC_ID_AC3 ||
				pFormatContext->streams[i]->codec->codec_id == CODEC_ID_RA_288 ||
				pFormatContext->streams[i]->codec->codec_id == CODEC_ID_RA_144) && 
				pFormatContext->streams[i]->codec->bit_rate > nMaxBitRate)
			{
				nMaxBitRate=pFormatContext->streams[i]->codec->bit_rate;
				audioStream=i;
			}
		}
	}
	return audioStream;
}
