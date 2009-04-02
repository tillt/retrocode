/*\
 * AVIFile.cpp
 * Copyright (C) 2004-2009, MMSGURU - written by Till Toenshoff
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
#ifdef WIN32
#include <malloc.h>
#else
#include <stdlib.h>
#endif
#include "../include/Resource.h"
#include "../retroBase/Basics.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/Adpcm.h"
#include "../retroBase/Endian.h"
#include "../retroBase/Packer.h"
#include "WaveFile.h"
#include "AVIFile.h"
#include "AVIProperty.h"
#include "../retroBase/PacketCollection.h"

DYNIMPPROPERTY(CAVIFile,CAVIProperty)

CAVIFile::CAVIFile(void)
{
	m_pcMagic="RIFF????AVI";
	m_nMagicSize=11;
	m_sFormatName="AVI";
	m_sDefaultExtension=_T("avi");
	TRACEIT2("AVI constructed\n");
	m_sFormatDescription.Load(IDS_FORMDESC_AVI);
	m_encodingCaps.freqs[	CSampleCompatibility::supports8Bit|
							CSampleCompatibility::supports16Bit|
							CSampleCompatibility::supportsMono|
							CSampleCompatibility::supportsStereo].setRange(1,192000);
}

CAVIFile::~CAVIFile(void)
{
	TRACEIT2("AVI destructed\n");
	while (!m_Streams.empty())
		m_Streams.erase(m_Streams.begin());
}

int CAVIFile::iGetFirstVideoStream(void)
{
	unsigned int i;
	for(i=0;i < m_Streams.size();i++)
	{
		if (m_Streams[i]->nGetType() == nMakeID4("vids"))
			return i;
	}
	return -1;
}

int CAVIFile::iGetFirstAudioStream(void)
{
	unsigned int i;
	for(i=0;i < m_Streams.size();i++)
	{
		if (m_Streams[i]->nGetType() == nMakeID4("auds"))
			return i;
	}
	return -1;
}

CAVIStream *CAVIFile::pGetStream(int i) 
{
	CAVIStream *pRet=NULL;

	if (i != -1 && (unsigned int)i < m_Streams.size())
		pRet=m_Streams[i];

	return pRet;
}

tstring CAVIStream::sGetCodecName(uint32_t nCodecId)
{
	int nResId=0;
	map<uint32_t,int> :: const_iterator iterCodec;
	map<uint32_t,int> mapCodecId;
	
	mapCodecId[CAVIStream::AVICODEC_H264]=IDS_FORMATNAME_H264;
	mapCodecId[AVICODEC_H263]=IDS_FORMATNAME_H263;
	mapCodecId[AVICODEC_H263P]=IDS_FORMATNAME_H263P;
	mapCodecId[AVICODEC_H263I]=IDS_FORMATNAME_H263I;
	mapCodecId[AVICODEC_H261]=IDS_FORMATNAME_H261;
	mapCodecId[AVICODEC_MPEG4]=IDS_FORMATNAME_MPEG4;
	mapCodecId[AVICODEC_MSMPEG4V1]=IDS_FORMATNAME_MSMPEG4V1;
	mapCodecId[AVICODEC_MSMPEG4V2]=IDS_FORMATNAME_MSMPEG4V2;
	mapCodecId[AVICODEC_MSMPEG4V3]=IDS_FORMATNAME_MSMPEG4V3;
	mapCodecId[AVICODEC_WMV1]=IDS_FORMATNAME_WMV1;
	mapCodecId[AVICODEC_WMV2]=IDS_FORMATNAME_WMV2;
	mapCodecId[AVICODEC_DVVIDEO]=IDS_FORMATNAME_DVVIDEO;
	mapCodecId[AVICODEC_MPEG1VIDEO]=IDS_FORMATNAME_MPEG1VIDEO;
	mapCodecId[AVICODEC_MPEG2VIDEO]=IDS_FORMATNAME_MPEG2VIDEO;
	mapCodecId[AVICODEC_MJPEG]=IDS_FORMATNAME_MJPEG;
	mapCodecId[AVICODEC_LJPEG]=IDS_FORMATNAME_LJPEG;
	mapCodecId[AVICODEC_JPEGLS]=IDS_FORMATNAME_JPEGLS;
	mapCodecId[AVICODEC_HUFFYUV]=IDS_FORMATNAME_HUFFYUV;
	mapCodecId[AVICODEC_FFVHUFF]=IDS_FORMATNAME_FFVHUFF;
	mapCodecId[AVICODEC_CYUV]=IDS_FORMATNAME_CYUV;
	mapCodecId[AVICODEC_RAWVIDEO]=IDS_FORMATNAME_RAWVIDEO;
	mapCodecId[AVICODEC_VP3]=IDS_FORMATNAME_VP3;
	mapCodecId[AVICODEC_VP5]=IDS_FORMATNAME_VP5;
	mapCodecId[AVICODEC_VP6]=IDS_FORMATNAME_VP6;
	mapCodecId[AVICODEC_ASV1]=IDS_FORMATNAME_ASV1;
	mapCodecId[AVICODEC_ASV2]=IDS_FORMATNAME_ASV2;
	mapCodecId[AVICODEC_VCR1]=IDS_FORMATNAME_VCR1;
	mapCodecId[AVICODEC_FFV1]=IDS_FORMATNAME_FFV1;
	mapCodecId[AVICODEC_XAN_WC4]=IDS_FORMATNAME_XANWC4;
	mapCodecId[AVICODEC_MSRLE]=IDS_FORMATNAME_MSRLE;
	mapCodecId[AVICODEC_MSVIDEO1]=IDS_FORMATNAME_MSVIDEO1;
	mapCodecId[AVICODEC_CINEPAK]=IDS_FORMATNAME_CINEPAK;
	mapCodecId[AVICODEC_TRUEMOTION1]=IDS_FORMATNAME_TRUEMOTION1;
	mapCodecId[AVICODEC_TRUEMOTION2]=IDS_FORMATNAME_TRUEMOTION2;
	mapCodecId[AVICODEC_MSZH]=IDS_FORMATNAME_MSZH;
	mapCodecId[AVICODEC_ZLIB]=IDS_FORMATNAME_ZLIB;
	mapCodecId[AVICODEC_SNOW]=IDS_FORMATNAME_SNOW;
	mapCodecId[AVICODEC_4XM]=IDS_FORMATNAME_4XM;
	mapCodecId[AVICODEC_FLV1]=IDS_FORMATNAME_FLV1;
	mapCodecId[AVICODEC_FLASHSV]=IDS_FORMATNAME_FLASHSV;
	mapCodecId[AVICODEC_VP6F]=IDS_FORMATNAME_VP6F;
	mapCodecId[AVICODEC_SVQ1]=IDS_FORMATNAME_SVQ1;
	mapCodecId[AVICODEC_TSCC]=IDS_FORMATNAME_TSCC;
	mapCodecId[AVICODEC_ULTI]=IDS_FORMATNAME_ULTI;
	mapCodecId[AVICODEC_VIXL]=IDS_FORMATNAME_VIXL;
	mapCodecId[AVICODEC_QPEG]=IDS_FORMATNAME_QPEG;
	mapCodecId[AVICODEC_WMV3]=IDS_FORMATNAME_WMV3;
	mapCodecId[AVICODEC_VC1]=IDS_FORMATNAME_VC1;
	mapCodecId[AVICODEC_LOCO]=IDS_FORMATNAME_LOCO;
	mapCodecId[AVICODEC_WNV1]=IDS_FORMATNAME_WNV1;
	mapCodecId[AVICODEC_AASC]=IDS_FORMATNAME_AASC;
	mapCodecId[AVICODEC_INDEO2]=IDS_FORMATNAME_INDEO2;
	mapCodecId[AVICODEC_INDEO3]=IDS_FORMATNAME_INDEO3;
	mapCodecId[AVICODEC_FRAPS]=IDS_FORMATNAME_FRAPS;
	mapCodecId[AVICODEC_THEORA]=IDS_FORMATNAME_THEORA;
	mapCodecId[AVICODEC_CSCD]=IDS_FORMATNAME_CSCD;
	mapCodecId[AVICODEC_ZMBV]=IDS_FORMATNAME_ZMBV;
	mapCodecId[AVICODEC_KMVC]=IDS_FORMATNAME_KMVC;
	mapCodecId[AVICODEC_CAVS]=IDS_FORMATNAME_CAVS;
	mapCodecId[AVICODEC_JPEG2000]=IDS_FORMATNAME_JPEG2000;
	mapCodecId[AVICODEC_VMNC]=IDS_FORMATNAME_VMNC;
	if ((iterCodec=mapCodecId.find(nCodecId)) == mapCodecId.end())
		nResId=IDS_FORMATNAME_UNKNOWN;
	else
		nResId=iterCodec->second;
	CMyString str;
	
	str.Load(nResId);
	return str;
}

bool CAVIStream::bRead(istream &ar,uint32_t nSize)
{
	bool bRet=true;
	uint32_t nSubTag;

	map<unsigned long,int> :: const_iterator iterChunk;
	map<unsigned long,int> mapChunkId;

	Unpacker up(ar, true); 

	mapChunkId[nMakeID4("H264")]=AVICODEC_H264;
	mapChunkId[nMakeID4("h264")]=AVICODEC_H264;
	mapChunkId[nMakeID4("X264")]=AVICODEC_H264;
	mapChunkId[nMakeID4("x264")]=AVICODEC_H264;
	mapChunkId[nMakeID4("avc1")]=AVICODEC_H264;
	mapChunkId[nMakeID4("VSSH")]=AVICODEC_H264;

	mapChunkId[nMakeID4("H263")]=AVICODEC_H263;
	mapChunkId[nMakeID4("H263")]=AVICODEC_H263P;
	mapChunkId[nMakeID4("U263")]=AVICODEC_H263P;
	mapChunkId[nMakeID4("viv1")]=AVICODEC_H263P;
	mapChunkId[nMakeID4("I263")]=AVICODEC_H263I;
	
	mapChunkId[nMakeID4("H261")]=AVICODEC_H261;

	mapChunkId[nMakeID4("FMP4")]=AVICODEC_MPEG4;
	mapChunkId[nMakeID4("DIVX")]=AVICODEC_MPEG4;
	mapChunkId[nMakeID4("DX50")]=AVICODEC_MPEG4;
	mapChunkId[nMakeID4("XVID")]=AVICODEC_MPEG4;
	mapChunkId[nMakeID4("MP4S")]=AVICODEC_MPEG4;
	mapChunkId[nMakeID4("M4S2")]=AVICODEC_MPEG4;
	mapChunkId[MAKEID4(0x04,0x00,0x00,0x00)]=AVICODEC_MPEG4;
    mapChunkId[nMakeID4("DIV1")]=AVICODEC_MPEG4;
	mapChunkId[nMakeID4("BLZ0")]=AVICODEC_MPEG4;
	mapChunkId[nMakeID4("mp4v")]=AVICODEC_MPEG4;
	mapChunkId[nMakeID4("UMP4")]=AVICODEC_MPEG4;
	mapChunkId[nMakeID4("WV1F")]=AVICODEC_MPEG4;
    mapChunkId[nMakeID4("SEDG")]=AVICODEC_MPEG4;
	mapChunkId[nMakeID4("RMP4")]=AVICODEC_MPEG4;

	mapChunkId[nMakeID4("DIV3")]=AVICODEC_MSMPEG4V3;
	mapChunkId[nMakeID4("MP43")]=AVICODEC_MSMPEG4V3;
	mapChunkId[nMakeID4("MPG3")]=AVICODEC_MSMPEG4V3;
	mapChunkId[nMakeID4("DIV4")]=AVICODEC_MSMPEG4V3;
	mapChunkId[nMakeID4("DIV5")]=AVICODEC_MSMPEG4V3;
	mapChunkId[nMakeID4("DIV6")]=AVICODEC_MSMPEG4V3;
	mapChunkId[nMakeID4("AP41")]=AVICODEC_MSMPEG4V3;
	mapChunkId[nMakeID4("COL1")]=AVICODEC_MSMPEG4V3;
	mapChunkId[nMakeID4("COL0")]=AVICODEC_MSMPEG4V3;
	
	mapChunkId[nMakeID4("MP42")]=AVICODEC_MSMPEG4V2;
	mapChunkId[nMakeID4("DIV2")]=AVICODEC_MSMPEG4V2;

	mapChunkId[nMakeID4("MPG4")]=AVICODEC_MSMPEG4V1;

	mapChunkId[nMakeID4("WMV1")]=AVICODEC_WMV1;
	mapChunkId[nMakeID4("WMV2")]=AVICODEC_WMV2;

	mapChunkId[nMakeID4("dvsd")]=AVICODEC_DVVIDEO;
	mapChunkId[nMakeID4("dvhd")]=AVICODEC_DVVIDEO;
	mapChunkId[nMakeID4("dvsl")]=AVICODEC_DVVIDEO;
	mapChunkId[nMakeID4("dv25")]=AVICODEC_DVVIDEO;

	mapChunkId[nMakeID4("mpg1")]=AVICODEC_MPEG1VIDEO;
	mapChunkId[nMakeID4("mpg2")]=AVICODEC_MPEG2VIDEO;
	mapChunkId[nMakeID4("MPEG")]=AVICODEC_MPEG2VIDEO;

	mapChunkId[nMakeID4("PIM1")]=AVICODEC_MPEG1VIDEO;
	mapChunkId[nMakeID4("VCR2")]=AVICODEC_MPEG1VIDEO;

	mapChunkId[MAKEID4(0x10,0x00,0x00,0x01)]=AVICODEC_MPEG1VIDEO;
	mapChunkId[MAKEID4(0x10,0x00,0x00,0x02)]=AVICODEC_MPEG2VIDEO;

	mapChunkId[nMakeID4("DVR ")]=AVICODEC_MPEG2VIDEO;
	mapChunkId[nMakeID4("MJPG")]=AVICODEC_MJPEG;
	mapChunkId[nMakeID4("JPGL")]=AVICODEC_MJPEG;
	mapChunkId[nMakeID4("LJPG")]=AVICODEC_LJPEG;

	mapChunkId[nMakeID4("MJLS")]=AVICODEC_MJPEG;
	mapChunkId[nMakeID4("HFYU")]=AVICODEC_MJPEG;

	mapChunkId[nMakeID4("FFVH")]=AVICODEC_FFVHUFF;

	mapChunkId[nMakeID4("CYUV")]=AVICODEC_CYUV;
	mapChunkId[0]=AVICODEC_RAWVIDEO;
	mapChunkId[nMakeID4("I420")]=AVICODEC_RAWVIDEO;
	mapChunkId[nMakeID4("YUY2")]=AVICODEC_RAWVIDEO;
	mapChunkId[nMakeID4("Y422")]=AVICODEC_RAWVIDEO;
	mapChunkId[nMakeID4("YV12")]=AVICODEC_RAWVIDEO;
	mapChunkId[nMakeID4("UYVY")]=AVICODEC_RAWVIDEO;
	mapChunkId[nMakeID4("IYUV")]=AVICODEC_RAWVIDEO;
	mapChunkId[nMakeID4("Y800")]=AVICODEC_RAWVIDEO;

	mapChunkId[nMakeID4("IV31")]=AVICODEC_INDEO3;
	mapChunkId[nMakeID4("IV32")]=AVICODEC_INDEO3;

	mapChunkId[nMakeID4("VP30")]=AVICODEC_VP3;
	mapChunkId[nMakeID4("VP31")]=AVICODEC_VP3;
	mapChunkId[nMakeID4("VP50")]=AVICODEC_VP5;
	mapChunkId[nMakeID4("VP60")]=AVICODEC_VP6;
	mapChunkId[nMakeID4("VP61")]=AVICODEC_VP6;
	mapChunkId[nMakeID4("VP62")]=AVICODEC_VP6;

	mapChunkId[nMakeID4("ASV1")]=AVICODEC_ASV1;
	mapChunkId[nMakeID4("ASV2")]=AVICODEC_ASV2;

	mapChunkId[nMakeID4("VCR1")]=AVICODEC_VCR1;
	mapChunkId[nMakeID4("FFV1")]=AVICODEC_FFV1;

	mapChunkId[nMakeID4("Xxan")]=AVICODEC_XAN_WC4;

	mapChunkId[nMakeID4("mrle")]=AVICODEC_MSRLE;
	mapChunkId[MAKEID4(0x01,0x00,0x00,0x00)]=AVICODEC_MSRLE;

	mapChunkId[nMakeID4("MSVC")]=AVICODEC_MSVIDEO1;
	mapChunkId[nMakeID4("msvc")]=AVICODEC_MSVIDEO1;
	mapChunkId[nMakeID4("CRAM")]=AVICODEC_MSVIDEO1;
	mapChunkId[nMakeID4("cram")]=AVICODEC_MSVIDEO1;
	mapChunkId[nMakeID4("WHAM")]=AVICODEC_MSVIDEO1;
	mapChunkId[nMakeID4("wham")]=AVICODEC_MSVIDEO1;

	mapChunkId[nMakeID4("cvid")]=AVICODEC_CINEPAK;
	mapChunkId[nMakeID4("DUCK")]=AVICODEC_TRUEMOTION1;
	mapChunkId[nMakeID4("MSZH")]=AVICODEC_MSZH;
	mapChunkId[nMakeID4("ZLIB")]=AVICODEC_ZLIB;
	mapChunkId[nMakeID4("SNOW")]=AVICODEC_SNOW;
	mapChunkId[nMakeID4("4XMV")]=AVICODEC_4XM;
	mapChunkId[nMakeID4("FLV1")]=AVICODEC_FLV1;
	mapChunkId[nMakeID4("FSV1")]=AVICODEC_FLASHSV;
	mapChunkId[nMakeID4("VP6F")]=AVICODEC_VP6F;
	mapChunkId[nMakeID4("svq1")]=AVICODEC_SVQ1;
	mapChunkId[nMakeID4("tscc")]=AVICODEC_TSCC;
	mapChunkId[nMakeID4("ULTI")]=AVICODEC_ULTI;
	mapChunkId[nMakeID4("VIXL")]=AVICODEC_VIXL;

	mapChunkId[nMakeID4("QPEG")]=AVICODEC_QPEG;
	mapChunkId[nMakeID4("Q1.0")]=AVICODEC_QPEG;
	mapChunkId[nMakeID4("Q1.1")]=AVICODEC_QPEG;

	mapChunkId[nMakeID4("WMV3")]=AVICODEC_WMV3;
	mapChunkId[nMakeID4("WVC1")]=AVICODEC_VC1;

	mapChunkId[nMakeID4("LOCO")]=AVICODEC_LOCO;
	mapChunkId[nMakeID4("WNV1")]=AVICODEC_WNV1;
	mapChunkId[nMakeID4("AASC")]=AVICODEC_AASC;
	mapChunkId[nMakeID4("RT21")]=AVICODEC_INDEO2;
	mapChunkId[nMakeID4("FPS1")]=AVICODEC_FRAPS;
	mapChunkId[nMakeID4("theo")]=AVICODEC_THEORA;
	mapChunkId[nMakeID4("TM20")]=AVICODEC_TRUEMOTION2;
	mapChunkId[nMakeID4("CSCD")]=AVICODEC_CSCD;
	mapChunkId[nMakeID4("ZMBV")]=AVICODEC_ZMBV;
	mapChunkId[nMakeID4("KMVC")]=AVICODEC_KMVC;
	mapChunkId[nMakeID4("CAVS")]=AVICODEC_CAVS;
	mapChunkId[nMakeID4("MJ2C")]=AVICODEC_JPEG2000;
	mapChunkId[nMakeID4("VMnc")]=AVICODEC_VMNC;

	m_nExtraDataSize=0;

	switch(m_nType)
	{
		case MAKEID4('v', 'i', 'd', 's'):
			ar.seekg(4,ios_base::cur);
			up.read("l",&m_nVideoWidth);
			Log2(verbLevDebug3," width: %d\n",m_nVideoWidth);
			up.read("l",&m_nVideoHeight);
			Log2(verbLevDebug3," height: %d\n",m_nVideoHeight);
			ar.seekg(2,ios_base::cur);												//panes
			up.read("s",&m_nVideoBitsPerSample);
			Log2(verbLevDebug3," bits per sample: %d\n",m_nVideoBitsPerSample);
			up.read("l",&nSubTag);
			ar.seekg(20,ios_base::cur);
			if (nSize > 40 && nSize < (1<<30))
			{
				m_nExtraDataSize=nSize - 40;
				ar.seekg(m_nExtraDataSize,ios_base::cur);
			}
			if (m_nExtraDataSize & 1)
				ar.seekg(1,ios_base::cur);
			m_nVideoCodecTag = nSubTag;
			if ((iterChunk=mapChunkId.find(nSubTag)) == mapChunkId.end())
			{
				Log2(verbLevErrors,"unknown codec tag");
				throw new CFormatException(CFormatException::formaterrInvalid,"unknown codec tag");
			}
			else
			{
				m_nVideoCodecId=iterChunk->second;
				Log2(verbLevDebug3," id: %d\n",m_nVideoCodecId);
			}
			Log2(verbLevDebug3,"tag: %c%c%c%c - (%s)\n",nSubTag&0xFF,(nSubTag>>8)&0xFF,(nSubTag>>16)&0xFF,(nSubTag>>24)&0xFF,sGetCodecName(nGetCodecId()).c_str());
		break;
		case MAKEID4('a', 'u', 'd', 's'):
			up.read("s",&m_WaveHeader.wFormatTag);
			Log2(verbLevDebug3," format tag %d (%s)\n",m_WaveHeader.wFormatTag,CWaveFile::sGetFormatName(m_WaveHeader.wFormatTag).c_str());
			up.read("s",&m_WaveHeader.wChannels);
			Log2(verbLevDebug3," channels %d\n",m_WaveHeader.wChannels);
			up.read("l",&m_WaveHeader.nSamplesPerSec);
			Log2(verbLevDebug3," rate %d\n",m_WaveHeader.nSamplesPerSec);
			up.read("l",&m_WaveHeader.nAvgBytesPerSec); 
			Log2(verbLevDebug3," avg byte per sec %d\n",m_WaveHeader.nAvgBytesPerSec);
			up.read("s",&m_WaveHeader.wBlockAlign);
			Log2(verbLevDebug3," block align %d\n",m_WaveHeader.wBlockAlign);
			up.read("s",&m_WaveHeader.wBitsPerSample);
			Log2(verbLevDebug3," bits per sample %d\n",m_WaveHeader.wBitsPerSample);
			ar.seekg((nSize-16)+(nSize&0x01),ios_base::cur);
		break;
		default:
			ar.seekg(nSize,ios_base::cur);
	}
	return bRet;
}

bool CAVIStream::bReadHeader(istream &ar,uint32_t nSize)
{
	bool bRet=true;

	Unpacker up(ar, true); 

	up.read("l",&m_nType);
	Log2(verbLevDebug3,"sub-tag: %c%c%c%c\n",(m_nType)&0x0FF,(m_nType>>8)&0x0FF,(m_nType>>16)&0x0FF,(m_nType>>24)&0x0FF);
	up.read("l",&m_nHandler);
	Log2(verbLevDebug3,"handler: %c%c%c%c\n",(m_nHandler)&0x0FF,(m_nHandler>>8)&0x0FF,(m_nHandler>>16)&0x0FF,(m_nHandler>>24)&0x0FF);

	//type 1 AVI?
	if (m_nType == MAKEID4('i', 'a', 'v', 's') || m_nType  == MAKEID4('i', 'v', 'a', 's'))
	{	//yes->...
		ar.seekg(12,ios_base::cur);
		up.read("l",&m_nScale);
		up.read("l",&m_nFrameRate);
		ar.seekg(nSize-20+(nSize&0x01),ios_base::cur);
	}
	else
	{	//nope->...
		ar.seekg(12,ios_base::cur);
		up.read("l",&m_nScale);
		Log2(verbLevDebug3,"scale %d\n",m_nScale);
		up.read("l",&m_nFrameRate);
		Log2(verbLevDebug3,"rate %d\n",m_nFrameRate);
		if(!m_nScale || !m_nFrameRate)
		{
			if(m_nFramePeriod)
			{
				m_nFrameRate = 1000000;
				m_nScale = m_nFramePeriod;
			}
			else
			{
				m_nFrameRate = 25;
				m_nScale = 1;
			}
		}
		up.read("l",&m_nCumLen);
		Log2(verbLevDebug3,"cumlen %d\n",m_nCumLen);
		up.read("l",&m_nFrames);
		Log2(verbLevDebug3,"frames %d\n",m_nFrames);
		m_nStartTime = 0;
		m_llDuration = (m_nFrames*((uint64_t)m_nScale*1000)) / m_nFrameRate;
		Log2(verbLevDebug3,"duration %d\n",(uint32_t)m_llDuration);
		m_nFrameRate=m_nFrameRate/m_nScale;
		
		ar.seekg(8,ios_base::cur);

		up.read("l",&m_nSampleSize);				//sample size
		if (m_nSampleSize > 1)
			m_nCumLen = m_nCumLen * m_nSampleSize;		//
		m_nFrameOffset=m_nCumLen;						//
		
		Log2(verbLevDebug3,"%d %d %d %d\n", m_nFrameRate, m_nScale, m_nStartTime, m_nSampleSize);

		switch(m_nType) 
		{
			case MAKEID4('v', 'i', 'd', 's'):
				m_nSampleSize=0;
			break;
			case MAKEID4('a', 'u', 'd', 's'):
			break;
			case MAKEID4('t', 'x', 't', 's'):
			break;
			case MAKEID4('p', 'a', 'd', 's'):
			break;
			default:
				TRACEIT2("unknown stream type\n");
		}
		ar.seekg(nSize-48+(nSize&0x01),ios_base::cur);
	}
	return bRet;
}

bool CAVIFile::bMagicHead(std::istream &ar,uint32_t nSize)
{
	unsigned char buffer[12];
	bool bRet=false;

	m_nMagicSize=sizeof(buffer);
	
	if (m_nMagicSize > 0 && (int)nSize >= m_nMagicSize)
	{
		try 
		{
			ar.read((char *)buffer,m_nMagicSize);
			if (memcmp(m_pcMagic,buffer,4) == 0 && memcmp(m_pcMagic+8,buffer+8,3) == 0)
				bRet=true;
		}
		catch(istream::failure const &e)
		{
			Log2(verbLevErrors,"read error on bit-stream magic head (%s)\n",e.what());
			TRACEIT("catching exception in bMagicHead\n");
			bRet=false;
		}
	}
	return bRet;
}

void CAVIFile::ReadMetaTag(istream &ar,int nId,unsigned int nSize)
{
	uint32_t nRead;											//
	char buffer[256];										//

	nRead=min(nSize,255);									//a meta string may not exceed 255 byte internally
	ar.read(buffer,nRead);									//read the string data
	buffer[nRead]=0;										//terminate string

	m_strInfo[nId]=buffer;									//store the meta string

	ar.seekg((nSize - nRead) + (nSize & 1),ios_base::cur);	//seek any extra data that was not read
}

void CAVIFile::Read(istream &ar)
{
	uint32_t nTag;
	uint32_t nSubTag;
	uint32_t nSize;
	bool bDone=false;

	Unpacker up(ar, true);

	m_iCurrentStream=0;
	m_iNextStream=0;
	m_nFramePeriod=0;
	m_nBitRate=0;
	m_bODML=false;
	try
	{
		ar.seekg(12,ios_base::beg);						//skip first 12 bytes
		while (!bDone)
		{
			up.read("l",&nTag);					//read the tag
			up.read("l",&nSize);				//read its size
			Log2(verbLevDebug3,"tag: %c%c%c%c, size: %d\n",nTag&0x0FF,(nTag>>8)&0x0FF,(nTag>>16)&0x0FF,(nTag>>24)&0x0FF,nSize);
			switch(nTag)
			{
				case MAKEID4('L', 'I', 'S', 'T'):
					// ignored, except when start of video packets 
					up.read("l",&nSubTag);
					Log2(verbLevDebug3,"sub-tag: %c%c%c%c\n",(nSubTag)&0x0FF,(nSubTag>>8)&0x0FF,(nSubTag>>16)&0x0FF,(nSubTag>>24)&0x0FF);
					if (nSubTag == MAKEID4('m', 'o', 'v', 'i')) 
					{
						m_llMoviListPos = (uint64_t)ar.tellg() - 4;
						if(nSize) 
							m_llMoviEndPos = m_llMoviListPos + nSize + (nSize & 1);
						else     
							m_llMoviEndPos = m_nFileSize;
						bDone=true;
						m_nTotalStreamDataSize=nSize-8;
					}
				break;
				case MAKEID4('a', 'v', 'i', 'h'):
					//avi header 
					//using frame_period is bad idea 
					up.read("l",&m_nFramePeriod);
					Log2(verbLevDebug3,"frame period: %d\n",m_nFramePeriod);
					up.read("l",&m_nBitRate);
					m_nBitRate=m_nBitRate*8;
					Log2(verbLevDebug3,"bit rate: %d\n",m_nBitRate);
					
					ar.seekg(4,ios_base::cur);
					uint32_t nFlag;
					up.read("l",&nFlag);
					m_bInterleaved=(nFlag & 0x20) != 0x20;

					ar.seekg(8,ios_base::cur);
					uint32_t n,i;
					up.read("l",&n);
					Log2(verbLevDebug3,"number of streams: %d\n",n);

					for(i=0;i < n;i++) 
					{	//stream header
						CAVIStream *pHead=new CAVIStream(m_nFramePeriod);
						m_Streams.push_back(pHead);
					}
					ar.seekg((nSize-28)+(nSize&1),ios_base::cur);
				break;
				case MAKEID4('s', 't', 'r', 'h'):
					m_iCurrentStream=m_iNextStream;
					++m_iNextStream;
					m_Streams[m_iCurrentStream]->bReadHeader(ar,nSize);
				break;
				case MAKEID4('s', 't', 'r', 'f'):
					if (m_iCurrentStream >= m_Streams.size()) 
						ar.seekg(nSize,ios_base::cur);
					else 
						m_Streams[m_iCurrentStream]->bRead(ar,nSize);
				break;				
				case MAKEID4('I', 'N', 'A', 'M'):	ReadMetaTag(ar,infoTitle,nSize);		break;
				case MAKEID4('I', 'A', 'R', 'T'):	ReadMetaTag(ar,infoArtist,nSize);		break;
				case MAKEID4('I', 'C', 'O', 'P'):	ReadMetaTag(ar,infoCopyright,nSize);	break;
				case MAKEID4('I', 'C', 'M', 'T'):	ReadMetaTag(ar,infoComments,nSize);		break;
				case MAKEID4('I', 'G', 'N', 'R'):	ReadMetaTag(ar,infoGenre,nSize);		break;
				case MAKEID4('I', 'P', 'R', 'D'):	ReadMetaTag(ar,infoAlbum,nSize);		break;
				case MAKEID4('o','d','m','l'):		
					m_bODML=true;							
				default:
					//skip tag
					ar.seekg(nSize + (nSize & 1),ios_base::cur);
			}
		};
	}
	catch(istream::failure const &e)
	{
		Log2(verbLevErrors,"read error on bit-stream (%s)\n",e.what());
	}
}
