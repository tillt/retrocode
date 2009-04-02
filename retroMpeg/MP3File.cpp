/*\
 * MP3File.cpp
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
/*\
 * MP3 identification is kind of based on the parser of MPG123 by Michael Hipp et al
\*/
#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <map>
#ifdef WIN32
#include "Winsock2.h"
#else
#include <netinet/in.h>
#endif
#include "id3.h"
#include "id3/tag.h"
#include "id3/utils.h"
#include "id3/misc_support.h"
#include "id3/readers.h"
#include "id3/writers.h"
#include "mad.h"
#include "lame/lame.h"
#include "../include/Resource.h"
#include "../retroBase/Basics.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MyString.h"
#include "../retroBase/Endian.h"
#include "../retroBase/Packer.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/PacketCollection.h"
#include "MP3File.h"
//#include "FFMPEGFile.h"
#include "MP3Property.h"
#define HAVE_MEMCPY
//#include "interface.h"		//mpglib
#include "Mp4Export.h"

#define MadErrorString(x) mad_stream_errorstr(x) 

//extern DllImport Endian gEndian;
mad_fixed_t	Filter[32]; 

/*
 * Layer 2 Alloc tables .. 
 * most other tables are calculated on program start (which is (of course)
 * not ISO-conform) .. 
 * Layer-3 huffman table is in huffman.h
 */
typedef struct
{
	short bits;
	short d;
}al_table;


al_table alloc_0[] = 
{
	{4,0},{5,3},{3,-3},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},{9,-255},{10,-511},
	{11,-1023},{12,-2047},{13,-4095},{14,-8191},{15,-16383},{16,-32767},
	{4,0},{5,3},{3,-3},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},{9,-255},{10,-511},
	{11,-1023},{12,-2047},{13,-4095},{14,-8191},{15,-16383},{16,-32767},
	{4,0},{5,3},{3,-3},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},{9,-255},{10,-511},
	{11,-1023},{12,-2047},{13,-4095},{14,-8191},{15,-16383},{16,-32767},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{16,-32767},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{16,-32767},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{16,-32767},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{16,-32767},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{16,-32767},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{16,-32767},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{16,-32767},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{2,0},{5,3},{7,5},{16,-32767},
	{2,0},{5,3},{7,5},{16,-32767},
	{2,0},{5,3},{7,5},{16,-32767},
	{2,0},{5,3},{7,5},{16,-32767} };

al_table alloc_1[] = {
	{4,0},{5,3},{3,-3},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},{9,-255},{10,-511},
	{11,-1023},{12,-2047},{13,-4095},{14,-8191},{15,-16383},{16,-32767},
	{4,0},{5,3},{3,-3},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},{9,-255},{10,-511},
	{11,-1023},{12,-2047},{13,-4095},{14,-8191},{15,-16383},{16,-32767},
	{4,0},{5,3},{3,-3},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},{9,-255},{10,-511},
	{11,-1023},{12,-2047},{13,-4095},{14,-8191},{15,-16383},{16,-32767},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{16,-32767},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{16,-32767},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{16,-32767},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{16,-32767},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{16,-32767},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{16,-32767},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{16,-32767},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
	{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{3,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{16,-32767},
	{2,0},{5,3},{7,5},{16,-32767},
	{2,0},{5,3},{7,5},{16,-32767},
	{2,0},{5,3},{7,5},{16,-32767},
	{2,0},{5,3},{7,5},{16,-32767},
	{2,0},{5,3},{7,5},{16,-32767},
	{2,0},{5,3},{7,5},{16,-32767},
	{2,0},{5,3},{7,5},{16,-32767} };

al_table alloc_2[] = {
	{4,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},{9,-255},
	{10,-511},{11,-1023},{12,-2047},{13,-4095},{14,-8191},{15,-16383},
	{4,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},{9,-255},
	{10,-511},{11,-1023},{12,-2047},{13,-4095},{14,-8191},{15,-16383},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63} };

al_table alloc_3[] = {
	{4,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},{9,-255},
	{10,-511},{11,-1023},{12,-2047},{13,-4095},{14,-8191},{15,-16383},
	{4,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},{9,-255},
	{10,-511},{11,-1023},{12,-2047},{13,-4095},{14,-8191},{15,-16383},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63} };

al_table alloc_4[] = {
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
		{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{14,-8191},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
		{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{14,-8191},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
		{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{14,-8191},
	{4,0},{5,3},{7,5},{3,-3},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},{8,-127},
		{9,-255},{10,-511},{11,-1023},{12,-2047},{13,-4095},{14,-8191},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{3,0},{5,3},{7,5},{10,9},{4,-7},{5,-15},{6,-31},{7,-63},
	{2,0},{5,3},{7,5},{10,9},
	{2,0},{5,3},{7,5},{10,9},
	{2,0},{5,3},{7,5},{10,9},
	{2,0},{5,3},{7,5},{10,9},
	{2,0},{5,3},{7,5},{10,9},
	{2,0},{5,3},{7,5},{10,9},
	{2,0},{5,3},{7,5},{10,9},
	{2,0},{5,3},{7,5},{10,9},
	{2,0},{5,3},{7,5},{10,9},
	{2,0},{5,3},{7,5},{10,9},
	{2,0},{5,3},{7,5},{10,9},
    {2,0},{5,3},{7,5},{10,9},
    {2,0},{5,3},{7,5},{10,9},
    {2,0},{5,3},{7,5},{10,9},
    {2,0},{5,3},{7,5},{10,9},
    {2,0},{5,3},{7,5},{10,9},
    {2,0},{5,3},{7,5},{10,9},
    {2,0},{5,3},{7,5},{10,9},
    {2,0},{5,3},{7,5},{10,9}  };

LPCTSTR szGetLameVersion(void)
{
	static char str[255];
	sprintf(str,"LAME %s",get_lame_version());
    return str;
}


LPCTSTR szGetMadVersion(void)
{
	static char str[255];
	sprintf(str,"libMAD %s",MAD_VERSION);
    return str;
}


DYNIMPPROPERTY(CMP3File,CMP3Property)

CMP3File::CMP3File(void)
{
	m_nMagicSize=1;
	m_pcMagic="\0FF";
	m_sFormatName="MPEG-1,2 Level 3 (MP3)";
	m_sDefaultExtension=_T("mp3");
	m_sFormatDescription.Load(IDS_FORMDESC_MP3);
	m_sFormatCredits=_T("The MP3 codec is entirely based on: \"LAME\", Copyright (c) 1999 by A.L. Faber; \"MPG123\", Copyright (c) 1995-1997 by Michael Hipp, \"libmad\", Copyright (c) 2000-2004 by Underbit Technologies, Inc.");

	m_nOldHead = 0;
	m_nFirstHead=0;
	m_nPlayTime=0;
	m_bVBR=false;
	m_bCRC=false;
	m_nFramSizeSum=0;
	m_bJointStereo=false;
	m_bCopyrightBit=false;

	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono|CSampleCompatibility::supportsStereo].addDigit(11025);
	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono|CSampleCompatibility::supportsStereo].addDigit(16000);
	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono|CSampleCompatibility::supportsStereo].addDigit(22050);
	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono|CSampleCompatibility::supportsStereo].addDigit(24000);
	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono|CSampleCompatibility::supportsStereo].addDigit(32000);
	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono|CSampleCompatibility::supportsStereo].addDigit(44100);
	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono|CSampleCompatibility::supportsStereo].addDigit(48000);

	m_encodingPara.addPara(cmdParaNumber,paraNumBitrate);
	m_encodingPara.addPara(cmdParaNumber,paraNumQuality);
	m_encodingPara.addPara(cmdParaBool,paraBoolAllowID3);
	m_encodingPara.addPara(cmdParaBool,paraBoolAllowVBR);
	m_encodingPara.addPara(cmdParaBool,paraBoolAllowCRC);
	m_encodingPara.addPara(cmdParaBool,paraBoolAllowJointStereo);
	m_encodingPara.addPara(cmdParaBool,paraBoolCopyrighted);
	m_encodingPara.addPara(cmdParaString,paraStrTitle);
	m_encodingPara.addPara(cmdParaString,paraStrSubTitle);
	m_encodingPara.addPara(cmdParaString,paraStrArtist);
	m_encodingPara.addPara(cmdParaString,paraStrNote);
	m_encodingPara.addPara(cmdParaString,paraStrCategory);
	m_encodingPara.addPara(cmdParaString,paraStrCopyright);
	m_encodingPara.addPara(cmdParaString,paraStrEncoder);
}

CMP3File::~CMP3File(void)
{
}

/*
DWORD CStream::GetFramesToPlay (void)
{
	//bytes = (fsize+8)*(num+2);
	return ((DWORD)m_pFile->fsize/(m_nFrameSize+8))-2;
}
*/


void CMP3File::InitInfoFromFile(std::istream &ar)
{
	ID3_IStreamReader isr(ar); 
	ID3_Tag myTag;

	myTag.Link(isr, ID3TT_ALL); 
	const Mp3_Headerinfo* mp3info;
	
	mp3info = myTag.GetMp3HeaderInfo(); 
	if (mp3info)
	{
		m_nSubFormat=0;
		switch (mp3info->version)
		{
			case MPEGVERSION_1:		m_nSubFormat=4;			break;
			case MPEGVERSION_2:		m_nSubFormat=8;			break;
			case MPEGVERSION_2_5:	m_nSubFormat=12;		break;
			default:				m_nSubFormat=0;
		}
		if (m_nSubFormat)
		{
			switch (mp3info->layer)
			{
				case MPEGLAYER_III:		m_nSubFormat+=1;		break;
				case MPEGLAYER_II:		m_nSubFormat+=2;		break;
				case MPEGLAYER_I:		m_nSubFormat+=3;		break;
				case MPEGLAYER_FALSE:							break;
				case MPEGLAYER_UNDEFINED:						break;
			}
			m_nBitRate=mp3info->bitrate;
			m_nCSSamplesPerSecond=mp3info->frequency;
			m_nCSChannels=mp3info->channelmode == MP3CHANNELMODE_SINGLE_CHANNEL ? 1 : 2;
			m_bJointStereo=mp3info->channelmode == MP3CHANNELMODE_JOINT_STEREO;
			m_nPlayTime=mp3info->time*1000;
			m_nFrameCount=mp3info->frames;
			m_bCRC=mp3info->crc != MP3CRC_NONE;
			m_bOriginalBit=mp3info->original;
			m_bPrivateBit=mp3info->privatebit;
			m_bCopyrightBit=mp3info->copyrighted;
			//m_bVBR=mp3info->vbr_bitrate;
			TRACEIT2("framesize: %d\n",mp3info->framesize);
		}
	}
	using namespace dami;

	ID3_Tag::Iterator *iter = myTag.CreateIterator();
	const ID3_Frame* frame = NULL;
	while (NULL != (frame = iter->GetNext()))
	{
		map<ID3_FrameID,int> mapID3Info;
		map<ID3_FrameID,int>::iterator iter;

		const char *desc = frame->GetDescription();
		mapID3Info[ID3FID_ALBUM]=infoAlbum;
		mapID3Info[ID3FID_COMPOSER]=infoComposer;
		mapID3Info[ID3FID_COPYRIGHT]=infoCopyright;
		mapID3Info[ID3FID_COMMENT]=infoComments;
		mapID3Info[ID3FID_DATE]=infoDate;
		mapID3Info[ID3FID_ENCODEDBY]=infoEncodedBy;
		mapID3Info[ID3FID_LYRICIST]=infoWriter;
		mapID3Info[ID3FID_TITLE]=infoTitle;
		mapID3Info[ID3FID_SUBTITLE]=infoSubTitle;
		mapID3Info[ID3FID_YEAR]=infoYear;
		mapID3Info[ID3FID_CONTENTTYPE]=infoGenre;
		if (!desc) 
			desc = "";
		TRACEIT ("=== %s (%s)\n",frame->GetTextID(),desc);
		ID3_FrameID eFrameID = frame->GetID();
		if ((iter=mapID3Info.find(eFrameID)) != mapID3Info.end())
		{
			switch(eFrameID)
			{
				case ID3FID_CONTENTTYPE:
				{
					int nNum;
					char *sText =ID3_GetString(frame, ID3FN_TEXT);
					tstring strText = sText;
					if ((nNum=(int)ParseGenreNum(strText)) != 0xFF)
					{
						do
						{
							if (nNum < ID3_NR_OF_V1_GENRES)
							{
								if (!m_strInfo[iter->second].empty())
									m_strInfo[iter->second]+=" / ";
								m_strInfo[iter->second]+=ID3_v1_genre_description[nNum];
							}
						}while ((nNum=(int)ParseGenreNum(strText)) != 0xFF);
					}
					else
						m_strInfo[iter->second]=sText;
					delete [] sText;
				}
				break;
				case ID3FID_COMMENT:
				{
					char *sText = ID3_GetString(frame, ID3FN_TEXT), *sDesc = ID3_GetString(frame, ID3FN_DESCRIPTION), *sLang = ID3_GetString(frame, ID3FN_LANGUAGE);
					CMyString sOut;
					sOut.Format("(%s)[%s]: %s",sDesc,sLang,sText);
					m_strInfo[iter->second]=sOut;
					delete [] sText;
					delete [] sDesc;
					delete [] sLang;
				}
				default:
					char *sText = ID3_GetString(frame, ID3FN_TEXT);
					m_strInfo[iter->second]=sText;
					delete [] sText;
			}
		}
	};
	//delete iter; 
}
//***************************************************************************
// Applies a frequency-domain filter to audio data in the subband-domain.	*
//***************************************************************************
/*
void CMP3File::ApplyFilter(struct mad_frame *Frame)
{
	int	Channel,Sample,Samples,SubBand;

	//There is two application loops, each optimized for the number
	//of audio channels to process. The first alternative is for
	//two-channel frames, the second is for mono-audio.
	Samples=MAD_NSBSAMPLES(&Frame->header);
	if(Frame->header.mode!=MAD_MODE_SINGLE_CHANNEL)
		for(Channel=0;Channel<2;Channel++)
			for(Sample=0;Sample<Samples;Sample++)
				for(SubBand=0;SubBand<32;SubBand++)
					Frame->sbsample[Channel][Sample][SubBand]=mad_f_mul(Frame->sbsample[Channel][Sample][SubBand],Filter[SubBand]);
	else
		for(Sample=0;Sample<Samples;Sample++)
			for(SubBand=0;SubBand<32;SubBand++)
				Frame->sbsample[0][Sample][SubBand]=mad_f_mul(Frame->sbsample[0][Sample][SubBand],Filter[SubBand]);
}
 */

void CMP3File::Read(std::istream &ar)
{
	size_t nConsumed=0;
	bool bDoFilter=false;
	CBufferCollector bc;

	//get ID3 tag info - that library also gives us some basic info on the MP3 itself
	InitInfoFromFile(ar);
	//make sure the stream is back in original state
	ar.clear();
	ar.seekg(0,ios_base::beg);
		
#define INPUT_BUFFER_SIZE	(5*8192)
#define OUTPUT_BUFFER_SIZE	8192 /* Must be an integer multiple of 4. */

	struct mad_stream	Stream;
	struct mad_frame	Frame;
	struct mad_synth	Synth;
	mad_timer_t			Timer;
	unsigned char		InputBuffer[INPUT_BUFFER_SIZE+MAD_BUFFER_GUARD],
						OutputBuffer[OUTPUT_BUFFER_SIZE],
						*GuardPtr=NULL;
	signed short int	*OutputPtr=(signed short int *)OutputBuffer;
	const unsigned char	*OutputBufferEnd=OutputBuffer+OUTPUT_BUFFER_SIZE;
	int					Status=0,
						i;
	unsigned long		FrameCount=0;
//	bstdfile_t			*BstdFile;

	/* First the structures used by libmad must be initialized. */
	mad_stream_init(&Stream);
	mad_frame_init(&Frame);
	mad_synth_init(&Synth);
	mad_timer_reset(&Timer);

	/* Decoding options can here be set in the options field of the
	 * Stream structure.
	 */

	/* {1} When decoding from a file we need to know when the end of
	 * the file is reached at the same time as the last bytes are read
	 * (see also the comment marked {3} bellow). Neither the standard
	 * C fread() function nor the POSIX read() system call provides
	 * this feature. We thus need to perform our reads through an
	 * interface having this feature, this is implemented here by the
	 * bstdfile.c module.
	 */
	//BstdFile=NewBstdFile(InputFp);
	Log2(verbLevDebug3,"decoding mpeg: ");
	/* This is the decoding loop. */
	do
	{
		if (FrameCount && m_nFrameCount)
			LogLineSynth(verbLevDebug1,".");
		/* The input bucket must be filled if it becomes empty or if
		 * it's the first execution of the loop.
		 */
		if(Stream.buffer==NULL || Stream.error==MAD_ERROR_BUFLEN)
		{
			size_t ReadSize,Remaining;
			unsigned char	*ReadStart;

			/* {2} libmad may not consume all bytes of the input
			 * buffer. If the last frame in the buffer is not wholly
			 * contained by it, then that frame's start is pointed by
			 * the next_frame member of the Stream structure. This
			 * common situation occurs when mad_frame_decode() fails,
			 * sets the stream error code to MAD_ERROR_BUFLEN, and
			 * sets the next_frame pointer to a non NULL value. (See
			 * also the comment marked {4} bellow.)
			 *
			 * When this occurs, the remaining unused bytes must be
			 * put back at the beginning of the buffer and taken in
			 * account before refilling the buffer. This means that
			 * the input buffer must be large enough to hold a whole
			 * frame at the highest observable bit-rate (currently 448
			 * kb/s). XXX=XXX Is 2016 bytes the size of the largest
			 * frame? (448000*(1152/32000))/8
			 */
			if(Stream.next_frame != NULL)
			{
				Remaining=Stream.bufend-Stream.next_frame;
				memmove(InputBuffer,Stream.next_frame,Remaining);
				ReadStart=InputBuffer+Remaining;
				ReadSize=INPUT_BUFFER_SIZE-Remaining;
			}
			else
			{
				ReadSize=INPUT_BUFFER_SIZE;
				ReadStart=InputBuffer;
				Remaining=0;
				TRACEIT2("next-frame is empty\n");
			}

			/* Fill-in the buffer. If an error occurs print a message
			 * and leave the decoding loop. If the end of stream is
			 * reached we also leave the loop but the return status is
			 * left untouched.
			 */
			//ar.seekg(ReadStart,ios_base::beg);
			try
			{
				ar.read((char *)ReadStart,(std::streamsize)ReadSize);
			}
			catch(istream::failure const &e)
			{
				Log2(verbLevDebug2,"read error on bit-stream (%s)\n",e.what());
				ReadSize=ar.gcount();
			}
			nConsumed += ReadSize;
			if(ReadSize == 0)
			{
				GuardPtr=ReadStart+ReadSize;
				memset(GuardPtr,0,MAD_BUFFER_GUARD);
				ReadSize+=MAD_BUFFER_GUARD;
				TRACEIT2("file seems to end at this point (%d)\n",nConsumed);
				break;
			}

			/* Pipe the new buffer content to libmad's stream decoder
             * facility.
			 */
			mad_stream_buffer(&Stream,InputBuffer,(unsigned long)ReadSize+Remaining);
			Stream.error=(mad_error)0;
		}
		if(mad_frame_decode(&Frame,&Stream))
		{
			//Frame.
			if(MAD_RECOVERABLE(Stream.error))
			{
				/* Do not print a message if the error is a loss of
				 * synchronization and this loss is due to the end of
				 * stream guard bytes. (See the comments marked {3}
				 * supra for more informations about guard bytes.)
				 */
				if(Stream.error!=MAD_ERROR_LOSTSYNC || Stream.this_frame != GuardPtr)
					Log2(verbLevWarnings,"recoverable frame level error (%s)\n",MadErrorString(&Stream));
				continue;
			}
			else
				if(Stream.error==MAD_ERROR_BUFLEN)
					continue;
				else
				{
					Log2(verbLevErrors,"unrecoverable frame level error (%s).\n",MadErrorString(&Stream));
					break;
				}
		}

		if(FrameCount == 0)
		{
			m_nBitRate=Frame.header.bitrate;
			m_nMinBitRate=m_nBitRate;
			m_nMaxBitRate=m_nBitRate;
			m_bVBR=false;
		}
		else
		{
			//bool lsf=(Frame.header.flags&MAD_FLAG_LSF_EXT)==MAD_FLAG_LSF_EXT;
			if ((unsigned int)Frame.header.bitrate != (unsigned int)m_nBitRate)
			{
				TRACEIT2("bitrate changed from %d to %d\n",m_nBitRate,Frame.header.bitrate);
				m_nBitRate=Frame.header.bitrate;
				m_bVBR=true;
				if (m_nBitRate < m_nMinBitRate)
					m_nMinBitRate=m_nBitRate;
				if (m_nBitRate > m_nMaxBitRate)
					m_nMaxBitRate=m_nBitRate;
			}
		}
		//m_nFramSizeSum+=nGetFrameSize(&Stream);
		if (Stream.next_frame)
			m_nFramSizeSum+=(unsigned int)(Stream.next_frame-Stream.this_frame);

		/* Accounting. The computed frame duration is in the frame
		 * header structure. It is expressed as a fixed point number
		 * whole data type is mad_timer_t. It is different from the
		 * samples fixed point format and unlike it, it can't directly
		 * be added or subtracted. The timer module provides several
		 * functions to operate on such numbers. Be careful there, as
		 * some functions of libmad's timer module receive some of
		 * their mad_timer_t arguments by value!
		 */
		FrameCount++;
		mad_timer_add(&Timer,Frame.header.duration);

		/* Between the frame decoding and samples synthesis we can
		 * perform some operations on the audio data. We do this only
		 * if some processing was required. Detailed explanations are
		 * given in the ApplyFilter() function.
		 */
		//if(bDoFilter)
		//	ApplyFilter(&Frame);

		/* Once decoded the frame is synthesized to PCM samples. No errors
		 * are reported by mad_synth_frame();
		 */
		mad_synth_frame(&Synth,&Frame);

		/* Synthesized samples must be converted from libmad's fixed
		 * point number to the consumer format. Here we use unsigned
		 * 16 bit big endian integers on two channels. Integer samples
		 * are temporarily stored in a buffer that is flushed when
		 * full.
		 */
	
		for(i=0;i < Synth.pcm.length;i++)
		{
			/* Left channel */
			*(OutputPtr++)=MadFixedToSshort(Synth.pcm.samples[0][i]);

			/* Right channel. If the decoded stream is monophonic then
			 * the right output channel is the same as the left one.
			 */
			if(MAD_NCHANNELS(&Frame.header)==2)
				*(OutputPtr++)=MadFixedToSshort(Synth.pcm.samples[1][i]);

			/* Flush the output buffer if it is full. */
			if(OutputPtr == (short *)OutputBufferEnd)
			{
				bc.CreateCopyPacket(OutputBuffer,OUTPUT_BUFFER_SIZE);
				OutputPtr=(short *)OutputBuffer;
			}
		}
	}while(1);
	Log2(verbLevDebug1,"decoding mpeg done - frame-size sum: %d byte (extra info: %d byte)\n",m_nFramSizeSum,m_nFileSize-m_nFramSizeSum);
	
	/* Mad is no longer used, the structures that were initialized must
     * now be cleared.
	 */
	mad_synth_finish(&Synth);
	mad_frame_finish(&Frame);
	mad_stream_finish(&Stream);

	if (!FrameCount)
	{
		throw new CFormatException(CFormatException::formaterrInvalid,"zero frames decoded, possibly not an MPEG file");
	}
	m_nFrameCount=FrameCount;

	if(OutputPtr != (short *)OutputBuffer)
	{
		size_t	BufferSize=(unsigned char *)OutputPtr-OutputBuffer;
		bc.CreateCopyPacket(OutputBuffer,(unsigned long)BufferSize);
	}

	/* Accounting report if no error occurred. */
	if(!Status)
	{
		char	Buffer[80];

		/* The duration timer is converted to a human readable string
		 * with the versatile, but still constrained mad_timer_string()
		 * function, in a fashion not unlike strftime(). The main
		 * difference is that the timer is broken into several
		 * values according some of it's arguments. The units and
		 * fracunits arguments specify the intended conversion to be
		 * executed.
		 *
		 * The conversion unit (MAD_UNIT_MINUTES in our example) also
		 * specify the order and kind of conversion specifications
		 * that can be used in the format string.
		 *
		 * It is best to examine libmad's timer.c source-code for details
		 * of the available units, fraction of units, their meanings,
		 * the format arguments, etc.
		 */
		mad_timer_string(Timer,Buffer,"%lu:%02lu.%03u",MAD_UNITS_MINUTES,MAD_UNITS_MILLISECONDS,0);
		Log2(verbLevDebug1,"%lu frames decoded (%s).\n",FrameCount,Buffer);
	}
	m_nCSSize=bc.nGetSize();
	m_nCSBitsPerSample=16;
	if (m_bVBR && (m_nCSSamplesPerSecond*m_nCSChannels))
	{
		int nPlaytime=m_nCSSize / ((m_nCSSamplesPerSecond*m_nCSChannels)*2);
		if (nPlaytime)
			m_nBitRate=(m_nFileSize*8) / nPlaytime;
	}
	if ((m_pcCSBuffer=CMobileSampleContent::Alloc(m_nCSSize)) != NULL)
		bc.nCopyLinear((unsigned char *)m_pcCSBuffer,m_nCSSize);
}

size_t CMP3File::ParseGenreNum(tstring &sIn)
{
	tstring sGenre = sIn;
	size_t ulGenre = 0xFF;
	size_t size = sGenre.size();

	// If the genre string begins with "(ddd)", where "ddd" is a number, then
	// "ddd" is the genre number---get it
	size_t i = 0;
	if (i < size && size && sGenre[i] == '(')
	{
		++i;
		while (i < size && isdigit(sGenre[i]))
			++i;
		if (i < size && sGenre[i] == ')')
		{
			// if the genre number is greater than 255, its invalid.
			ulGenre = min(0xFF, atoi(&sGenre[1]));
			++i;
		}
	}
	sIn=sIn.substr(i);
	return ulGenre;
} 

/*
int CMP3File::HeadRead (std::istream &ar,unsigned char *hbuf,uint32_t*newhead)
{
	ar.read((char *)hbuf,4);
	*newhead = ((unsigned long) hbuf[0] << 24) |
	           ((unsigned long) hbuf[1] << 16) |
	           ((unsigned long) hbuf[2] << 8)  |
	            (unsigned long) hbuf[3];
	return TRUE;
}
*/

void CMP3File::ReadFrameInit (void)
{
	m_nOldHead = 0;
	m_nFirstHead = 0;
	m_nInvalidFrameCount=0;
	m_nFrameCount=0;
}

bool CMP3File::bMagicHead(std::istream &ar,uint32_t nSize)
{
	CMP3Frame frame;
	m_nMagicSize=4;
	bool bRet=false;

	m_nFileSize=nSize;
	
	if (m_nMagicSize == 0 || (int)nSize < m_nMagicSize)
		return false;
	try 
	{
		ReadFrameInit();
		bRet = ReadFrame(ar,&frame) == 1;
	}
	//
	//catch(istream::failure const *e)
	catch(istream::failure const &e)
	{
		//Log2(verbLevErrors,"read error on bit-stream magic head (%s)\n",e.what());
		TRACEIT2("catching exception in bMagicHead\n");
		bRet=false;
	}
	return bRet;
}

bool CMP3File::HeadCheck(uint32_t newhead) 
{
	if( (newhead & 0xffe00000) != 0xffe00000)
		return FALSE;
	if(!((newhead>>17)&3))
		return FALSE;
	if( ((newhead>>12)&0xf) == 0xf)
		return FALSE;
	if( ((newhead>>10)&0x3) == 0x3 )
		return FALSE;
	return TRUE;
}

int CMP3File::ReadFrame(std::istream &ar,CMP3Frame *fr)
{
	int nTabSel123[2][3][16] = 
	{
		{ 
			{0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,},
			{0,32,48,56, 64, 80, 96,112,128,160,192,224,256,320,384,},
			{0,32,40,48, 56, 64, 80, 96,112,128,160,192,224,256,320,} 
		},
		{ 
			{0,32,48,56,64,80,96,112,128,144,160,176,192,224,256,},
			{0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,},
			{0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,}			//
		}
	};
	uint32_t nFreqs[7] = { 44100, 48000, 32000, 22050, 24000, 16000 , 11025 };
	static uint32_t newhead;
	static uint32_t framesize;

read_again:
	ar.read((char *)&newhead,4);
	newhead=ntohl(newhead);
	//uint32_tccNewHead=ntohs(newhead);
	TRACEIT2("newhead=0x%08lX\n",newhead);
	//same procedure as last year, madame ?
	if ((m_nOldHead != newhead) | !m_nOldHead)	
	{
		TRACEIT2 ("header change...\n");
		fr->header_change = 1;						//new header !!!! 
	//init_resync:
		if(!m_nFirstHead & !HeadCheck(newhead))		//is it the first header and illegal ?
		{											//yes...
			//I even saw RIFF headers at the beginning of MPEG streams ;(
			//me too, thats life....8o}
			TRACEIT2 ("junk at the beginning\n");
			uint32_t ccNewHead=newhead;
			if (TOFOURCC(ccNewHead) == MAKEFOURCC('R','I','F','F'))
			{
				uint32_t cs=0;
				TRACEIT2("there is a freakin RIFF header at the beginning of this MP3\n");
				if (ProcessChunk (ar,0,TOFOURCC(RIFFtag),0,&cs))
				{
					TRACEIT2 ("skipped RIFF header\n");
					goto read_again;
				}
				else
				{
					TRACEIT2 ("failed to process RIFF header, try standard size\n");
					ar.seekg(0x0046,ios_base::beg);
					goto read_again;
				}					
			}
			else
			{
				TRACEIT2("that junk is not a RIFF header, that is for sure\n");
			}
		}

		if ((newhead & 0xffe00000) != 0xffe00000) 
		{
			TRACEIT2 ("illegal Audio-MPEG-Header 0x%08lx at offset 0x%08lX (0x%08lX).\n",newhead,(int)ar.tellg()-4,('I'<<24)+('D'<<16)+('3'<<8));
			// and those ugly ID3 tags
			if((newhead & 0xffffff00) == ('T'<<24)+('A'<<16)+('G'<<8)) 
			{
				TRACEIT2 ("skipping old ID3-tag\n");
				ar.seekg((int)ar.tellg()+124,ios_base::beg);
				goto read_again;
			}
			else if((newhead & 0xffffff00) == ('I'<<24)+('D'<<16)+('3'<<8)) 
			{
				/*
				ID3v2/file identifier   "ID3" 
				ID3v2 version           $03 00
				ID3v2 flags             %abc00000
				ID3v2 size              4 * %0xxxxxxx
				*/
				bool bUnsyncUsed=false;
				bool bExtHeader=false;
				bool bExperimental=false;				
				unsigned char cId3Size=0;
				uint32_t nId3Size=0;
				unsigned char cId3Flags=0;
				unsigned short int nId3Version=0;
				unsigned char *acOut=(unsigned char *)&nId3Version;

				acOut[0]=newhead&0xFF;
				ar.read((char *)&acOut[1],1);
				Log2(verbLevDebug2,"ID3-version %04X\n",nId3Version);
				//if (nId3Version == 0x03)
				switch (nId3Version)
				{
					case 0x02:
					case 0x03:
						ar.read((char *)&cId3Flags,1);
						TRACEIT2 ("ID3-flags\n");
			 			Log2(verbLevDebug2,"ID3-flags %02X\n",cId3Flags);
						bUnsyncUsed = (cId3Flags&0x80) == 0x80;
						bExtHeader = (cId3Flags&0x40) == 0x40;
						bExperimental= (cId3Flags&0x20) == 0x20;
						ar.read((char *)&cId3Size,1);
						nId3Size=((unsigned int)cId3Size&0x7F)<<21;
						ar.read((char *)&cId3Size,1);
						nId3Size|=((unsigned int)cId3Size&0x7F)<<14;
						ar.read((char *)&cId3Size,1);
						nId3Size|=((unsigned int)cId3Size&0x7F)<<7;
						ar.read((char *)&cId3Size,1);
						nId3Size|=((unsigned int)cId3Size&0x7F);
						Log2(verbLevDebug2,"ID3-size %08X)\n",nId3Size);
						ar.seekg(nId3Size,ios_base::cur);
						//ar.read((char *)&cId3Size,1);
						//Log2(verbLevDebug2,"first byte after Id3Header %02X\n",cId3Size);
					break;
					default:
						Log2(verbLevDebug2,"skipping new ID3-tag\n");
						TRACEIT2 ("skipping new ID3-tag\n");
						ar.seekg((int)ar.tellg()+(0x1000-4),ios_base::beg);
				}
				
				goto read_again;
			}
			else
			{
				TRACEIT2 ("it wasnt an ID3 tag, so what the hell does this have to do with MP3?\n");
				return 0;
			}
			/* Read more bytes until we find something that looks
			   reasonably like a valid header.  This is not a
			   perfect strategy, but it should get us back on the
			   track within a short time (and hopefully without
			   too much distortion in the audio output).  */
			   /*
			do 
			{
				++m_nInvalidFrameCount;
				++rtry;
				memmove (&hbuf[0], &hbuf[1], 7);
				ar.read((char *)&(hbuf[3]),1);
				//this is faster than combining newhead from scratch
				newhead = ((newhead << 8) | hbuf[3]) & 0xffffffff;
				if (!m_nOldHead)
					goto init_resync;       //"considered harmful", eh?
			}while (	(newhead & HDRCMPMASK) != (m_nOldHead & HDRCMPMASK)
					&&	(newhead & HDRCMPMASK) != (m_nFirstHead & HDRCMPMASK));
			TRACEIT2 ( "skipped %d bytes in file input.\n", rtry);
			*/
		}
		if (!m_nFirstHead)
			m_nFirstHead = newhead;

		if (newhead & (1<<20))
		{
			fr->lsf = (newhead & (1<<19)) ? 0x0 : 0x1;
			fr->mpeg25 = 0;
		}
		else 
		{
			fr->lsf = 1;
			fr->mpeg25 = 1;
		}
    	if (!m_nOldHead) 
		{
			fr->lay = 4-((newhead>>17)&3);
			fr->bitrate_index = ((newhead>>12)&0xf);
			if( ((newhead>>10)&0x3) == 0x3) 
			{
				TRACEIT2 ("stream error, aborting....\n");
				return 0;
			}
			if(fr->mpeg25) 
				fr->sampling_frequency = 6 + ((newhead>>10)&0x3);
			else
				fr->sampling_frequency = ((newhead>>10)&0x3) + (fr->lsf*3);
			fr->error_protection = ((newhead>>16)&0x1)^0x1;
		}
		//allow Bitrate change for 2.5 ...
		if(fr->mpeg25)
			fr->bitrate_index = ((newhead>>12)&0xf);
		fr->padding   = ((newhead>>9)&0x1);
		fr->extension = ((newhead>>8)&0x1);
		fr->mode      = ((newhead>>6)&0x3);
		fr->mode_ext  = ((newhead>>4)&0x3);
		fr->copyright = ((newhead>>3)&0x1);
		fr->original  = ((newhead>>2)&0x1);
		fr->emphasis  = newhead & 0x3;
		fr->stereo    = (fr->mode == MPG_MD_MONO) ? 1 : 2;
		m_nOldHead = newhead;
		if(!fr->bitrate_index)
		{
			TRACEIT2 ("free format not supported, aborting...\n");
			return 0;
		}
		switch(fr->lay)
		{
			case 1:			
				fr->jsbound = (fr->mode == MPG_MD_JOINT_STEREO) ? (fr->mode_ext<<2)+4 : 32;
				if (nFreqs[fr->sampling_frequency])
				{
					framesize  = (long)nTabSel123[fr->lsf][0][fr->bitrate_index] * 12000;
					framesize /= nFreqs[fr->sampling_frequency];
					framesize  = ((framesize+fr->padding)<<2)-4;
				}
				else
				{
					throw new CFormatException(CFormatException::formaterrInvalid,"sample frequency is invalid");
				}
			break;
			case 2:
				GetIIStuff(fr);
				fr->jsbound = (fr->mode == MPG_MD_JOINT_STEREO) ? (fr->mode_ext<<2)+4 : fr->II_sblimit;
				if (nFreqs[fr->sampling_frequency])
				{
					framesize = (long)nTabSel123[fr->lsf][1][fr->bitrate_index] * 144000;
					framesize /= nFreqs[fr->sampling_frequency];
					framesize += fr->padding - 4;
				}
				else
				{
					throw new CFormatException(CFormatException::formaterrInvalid,"sample frequency is invalid");
				}
			break;
			case 3:
				if(fr->lsf)
					fr->m_nSsize = (fr->stereo == 1) ? 9 : 17;
				else
					fr->m_nSsize = (fr->stereo == 1) ? 17 : 32;
				if(fr->error_protection)
					fr->m_nSsize += 2;

				if (nFreqs[fr->sampling_frequency])
				{
					framesize  = (long)nTabSel123[fr->lsf][2][fr->bitrate_index] * 144000;
					framesize /= nFreqs[fr->sampling_frequency]<<(fr->lsf);
					framesize = framesize + fr->padding - 4;
				}
				else
				{
					throw new CFormatException(CFormatException::formaterrInvalid,"sample frequency is invalid");
				}
			break; 
			default:
				TRACEIT2 ("unknown layer type, aborting...\n"); 
				return 0;
		}
	}
	else
		fr->header_change = 0;					//same procedure as every year, james...

	//m_lNumFrames=STREAM->GetFramesToPlay()
	//m_nFrameSize=framesize;
	m_nCSChannels=fr->stereo ? 2 : 1;
	m_nCSSamplesPerSecond=nFreqs[fr->sampling_frequency] << fr->lsf;
	m_nBitRate=nTabSel123[fr->lsf][fr->lay-1][fr->bitrate_index];
	return 1;
} 

CMP3Frame::CMP3Frame ()
{	
	alloc=NULL;
	//both channels 
    single=-1;			
	//no downsampling
    down_sample=0;
	sampling_frequency=0;
	bitrate_index=0;
	ZeroMemory (&m_ID3,sizeof(id3tag));
}

CMP3Frame::~CMP3Frame()
{
}

//adopted from SHOWRIFF.C (c)94 UP-Vision Computergrafik for c't
BOOL CMP3File::ProcessChunk (	std::istream &ar,
								uint32		dwSeekPos,
								uint32		DesiredTag,
								int			nRekDepth,
								uint32		*pdwChunkSize)
{
	char	tagstr[5];				//FOURCC of chunk converted to string
	uint32	chunkid;					//read FOURCC of chunk
	long	datapos;					//position of data in file to process

	/*
	if (dwSeekPos > m_pFile->fsize-1) 
	{										//Oops. Must be something wrong!
		TRACEIT ("RIFF-ERROR: data would be behind end of file!\n");
		return FALSE;
	}
	*/
	ar.seekg (dwSeekPos,ios_base::beg);		//go to desired file position !
	//read chunk header
	ReadChunkHead(ar,&chunkid,pdwChunkSize);
	//do we have to test identity ?
	if (DesiredTag) 
	{
		//yepp..
		if (DesiredTag != chunkid) 
		{
			char ds[5];
			FOURCC2Str (chunkid,tagstr);			//now we can PRINT the chunkid
			FOURCC2Str (DesiredTag,ds);				//
			TRACEIT2 ("RIFF-ERROR: expected chunk '%s', found '%s'\n",ds,tagstr);
			return FALSE;
		}
	}
	//here is the data
	datapos=dwSeekPos+sizeof(uint32)+sizeof(uint32);	
	//too long?
	/*
	if (datapos + ((*pdwChunkSize+1)&~1) > m_pFile->fsize)	
	{												
		TRACEIT ("RIFF-ERROR: chunk exceeds file (starts at 0x%x)\n",dwSeekPos);
		return FALSE;
	}
	*/
	//switch (TOFOURCC(chunkid)) 
	switch (chunkid) 
	{
		case RIFFtag:
		case LISTtag: 
			{
				uint32  datashowed;
				uint32  formtype;       //format of chunk
				uint32  subchunksize;   //size of a read subchunk
				//read the form type
				ar.read ((char *)&formtype,sizeof(uint32));
				datashowed=sizeof(uint32);	//we showed the form type
				datapos+=datashowed;		//for the rest of the routine
				while (datashowed < *pdwChunkSize) 
				{							//while not showed all:
					long subchunklen;       //complete size of a subchunk

					//recurse for subchunks of RIFF and LIST chunks:
					if (!ProcessChunk (ar,datapos,0,nRekDepth+1,&subchunksize)) 
						return FALSE;
					subchunklen = sizeof(uint32) +		//this is the complete..
								  sizeof(uint32)  +		//.. size of the subchunk
								  ((subchunksize+1) & ~1);
					datashowed += subchunklen;			//we showed the subchunk
					datapos    += subchunklen;			//for the rest of the loop
				};
			} 
		break;
		case DATAtag:
			datapos+=sizeof(uint32);
		break;
	}
	TRACEIT2("datapos: %Xh\n",datapos);
	return TRUE;
}

// Build a string from a FOURCC number
// (s must have room for at least 5 chars)
void CMP3File::FOURCC2Str (uint32 fcc, char *s)
{
	s[0]=(char)((fcc      ) & 0xFF);
	s[1]=(char)((fcc >>  8) & 0xFF);
	s[2]=(char)((fcc >> 16) & 0xFF);
	s[3]=(char)((fcc >> 24) & 0xFF);
	s[4]=0;
}

// Reads a chunk ID and the chunk's size from file f at actual
// file position :
void CMP3File::ReadChunkHead (std::istream &ar,uint32_t *ID,uint32 *size)
{
	Unpacker up(ar,true);
	up.read("4b",ID);
	up.read("l",size);
}

//MPEG2 Layer3
void CMP3File::GetIIStuff (CMP3Frame *fr)
{
	static const int translate[3][2][16] = 
	{ 
		{ 
			{ 0,2,2,2,2,2,2,0,0,0,1,1,1,1,1,0 } ,	//mono, 
			{ 0,2,2,0,0,0,1,1,1,1,1,1,1,1,1,0 }		//stereo
		} ,
		{ 
			{ 0,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0 } ,
			{ 0,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0 } 
		} ,
		{ 
			{ 0,3,3,3,3,3,3,0,0,0,1,1,1,1,1,0 } ,
			{ 0,3,3,0,0,0,1,1,1,1,1,1,1,1,1,0 } 
		} 
	};
	static const al_table *tables[5] = { alloc_0, alloc_1, alloc_2, alloc_3 , alloc_4 };
	static const int sblims[5] = { 27 , 30 , 8, 12 , 30 };
	int table,sblim;

	if(fr->lsf)
		table = 4;
	else
		table = translate[fr->sampling_frequency][2-fr->stereo][fr->bitrate_index];
	sblim = sblims[table];
	fr->alloc = (void *)tables[table];
	fr->II_sblimit = sblim;
}

tstring CMP3File::sGetFormatName(int nFormat)
{
	CMyString strText;
	strText.Load(IDS_FORMATNAME_MPEG+nFormat);
	return strText;
}

/*\
 * <---------- bIsLegalSampleRate ---------->
 * @m mpeg compatible sample rate?
 * --> I N <-- @p
 * int nSampleRate - source sample rate
 * <-- OUT --> @r
 * bool - TRUE=legal
\*/
bool CMP3File::bIsLegalSampleRate(uint32_t nSampleRate)
{
	int i;
	uint32_t nFreqs[] = { 44100, 48000, 32000, 22050, 24000, 16000 , 11025 ,0};
	if (nSampleRate)
	{
		for (i=0;nFreqs[i] != 0;i++)
		{
			if (nFreqs[i] == nSampleRate)
				return true;
		}
	}
	return false;
}

/*\
 * <---------- bIsLegalBitrate ---------->
 * @m mpeg compatible bitrate?
 * --> I N <-- @p
 * int nBitrate - source bitrate
 * <-- OUT --> @r
 * bool - TRUE=legal
\*/
bool CMP3File::bIsLegalBitrate(uint32_t nBitrate)
{
	int i;
	const int bitrate_table[] = {8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0};
	if (nBitrate)
	{
		for (i=0;bitrate_table[i] != 0;i++)
		{
			if (bitrate_table[i] == nBitrate/1000)
				return true;
		}
	}
	return false;
}

/*\
 * <---------- RenderDestination ---------->
 * @m create a MP3 out of this 
 * --> I N <-- @p
 * ostream &out - ouput stream data
 * CMobileSampleContent *pSource - source data
\*/
void CMP3File::Write(ostream &out)
{
	int ret_code;
	uint32_t nSourceSamples=m_pCSSource->m_nCSSize/(m_pCSSource->m_nCSBitsPerSample/8);
	uint32_t nDestSize=2*nSourceSamples+7200;
	uint32_t nUsedSize;

	ASSERT(nDestSize);
	char *pcDest=new char [nDestSize];
	ASSERT(pcDest);
	memset(pcDest,0,nDestSize);

	lame_global_flags *gfp;

	if (m_pParameters->m_nParameter[paraNumBitrate] == 0)
	{
		m_pParameters->m_nParameter[paraNumBitrate]=m_pCSSource->m_nCSChannels == 1 ? 32000 : 64000;
		Log2(verbLevWarnings,"bitrate not set, using: %d\n",m_pParameters->m_nParameter[paraNumBitrate]);
	}

	Log2(verbLevDebug1,"bitrate: %dbps\n",m_pParameters->m_nParameter[paraNumBitrate]);

	if (m_pCSSource->m_nCSBitsPerSample != 16 && m_pCSSource->m_nCSBitsPerSample != 8)
	{
		TRACEIT2("sample width incompatible\n");
		throw new CFormatException(CFormatException::formaterrIncompatibleSampleFormat,"sample width incompatible");
	}
	if (!bIsLegalBitrate(m_pParameters->m_nParameter[paraNumBitrate]))
	{
		TRACEIT2("bitrate incompatible\n");
		throw new CFormatException(CFormatException::formaterrParameters,"bitrate incompatible");
	}

	gfp = lame_init();
	if (m_pCSSource->m_nCSChannels == 1)
		lame_set_mode( gfp, MONO );
	else if (m_pParameters->m_bParameter[paraBoolAllowJointStereo])
		lame_set_mode( gfp, JOINT_STEREO);
	else
		lame_set_mode( gfp, STEREO);
	lame_set_num_channels(gfp,m_pCSSource->m_nCSChannels);

	lame_set_in_samplerate(gfp,m_pCSSource->m_nCSSamplesPerSecond);
	lame_set_brate(gfp,m_pParameters->m_nParameter[paraNumBitrate]/1000);
	lame_set_mode(gfp,m_pCSSource->m_nCSChannels > 1 ? JOINT_STEREO : MONO);
	lame_set_bWriteVbrTag( gfp, 0 );
	if (!m_pParameters->m_bParameter[paraBoolAllowVBR])
		lame_set_VBR(gfp,vbr_off);
	else
		lame_set_VBR(gfp,vbr_default);
	lame_set_error_protection( gfp,m_pParameters->m_bParameter[paraBoolAllowCRC] ? 1 : 0);

	lame_set_quality(gfp,m_pParameters->m_nParameter[paraNumQuality]);		// 2=high  5 = medium  7=low
	lame_set_copyright(gfp,m_pParameters->m_bParameter[paraBoolCopyrighted]);

	if ((ret_code = lame_init_params(gfp)) == 0)
	{
		Log2(verbLevDebug1,"compressing sample data...");
		if (m_pCSSource->m_nCSChannels == 1)
			nUsedSize=lame_encode_buffer(gfp,(const short *)m_pCSSource->m_pcCSBuffer,(const short *)m_pCSSource->m_pcCSBuffer,nSourceSamples,(unsigned char *)pcDest,nDestSize);
		else
			nUsedSize=lame_encode_buffer_interleaved(gfp,(short *)m_pCSSource->m_pcCSBuffer,nSourceSamples/2,(unsigned char *)pcDest,nDestSize);
		Log2(verbLevDebug1,"\n");
		nUsedSize+=lame_encode_flush(gfp,(unsigned char *)pcDest+nUsedSize,nDestSize);

		out.write((char *)pcDest,nUsedSize);
		m_nCSSize=nUsedSize;
		//write ID3 V1 tag and data..
		if (m_pParameters->m_bParameter[paraBoolAllowID3])
			RenderID3V1(out,m_pCSSource);
		m_nFileSize=out.tellp();
		m_nCSSamplesPerSecond=m_pCSSource->m_nCSSamplesPerSecond;
		m_nCSChannels=m_pCSSource->m_nCSChannels;
		m_nCSBitsPerSample=0;
	}
	else
	{
		TRACEIT2("failed to init parameters at codec\n");
		throw new CFormatException(CFormatException::formaterrParameters);
	}
	lame_close(gfp);
	delete [] pcDest;
}

/****************************************************************************
 * Print human readable informations about an audio MPEG frame.				*
 ****************************************************************************/
void CMP3File::PrintFrameInfo(struct mad_header *Header)
{
	const char	*Layer,*Mode,*Emphasis;

	/* Convert the layer number to it's printed representation. */
	switch(Header->layer)
	{
		case MAD_LAYER_I:
			Layer="I";
		break;
		case MAD_LAYER_II:
			Layer="II";
		break;
		case MAD_LAYER_III:
			Layer="III";
		break;
		default:
			Layer="(unexpected layer value)";
	}

	/* Convert the audio mode to it's printed representation. */
	switch(Header->mode)
	{
		case MAD_MODE_SINGLE_CHANNEL:
			Mode="single channel";
		break;
		case MAD_MODE_DUAL_CHANNEL:
			Mode="dual channel";
		break;
		case MAD_MODE_JOINT_STEREO:
			Mode="joint (MS/intensity) stereo";
		break;
		case MAD_MODE_STEREO:
			Mode="normal LR stereo";
		break;
		default:
			Mode="(unexpected mode value)";
	}

	/* Convert the emphasis to it's printed representation. Note that
	 * the MAD_EMPHASIS_RESERVED enumeration value appeared in libmad
	 * version 0.15.0b.
	 */
	switch(Header->emphasis)
	{
		case MAD_EMPHASIS_NONE:
			Emphasis="no";
		break;
		case MAD_EMPHASIS_50_15_US:
			Emphasis="50/15 us";
		break;
		case MAD_EMPHASIS_CCITT_J_17:
			Emphasis="CCITT J.17";
		break;
#if (MAD_VERSION_MAJOR>=1) || \
	((MAD_VERSION_MAJOR==0) && (MAD_VERSION_MINOR>=15))
		case MAD_EMPHASIS_RESERVED:
			Emphasis="reserved(!)";
		break;
#endif
		default:
			Emphasis="(unexpected emphasis value)";
		break;
	}

	Log2(verbLevDebug1,"MPEG %s, %lu kb/s audio MPEG layer %s stream %s CRC, %s with %s emphasis at %d Hz sample rate\n",
						Header->flags&MAD_FLAG_MPEG_2_5_EXT ?"2.5":"1/2",
						Header->bitrate,
						Layer,
						Header->flags&MAD_FLAG_PROTECTION ? "with":"without",
						Mode,
						Emphasis,
						Header->samplerate);
} 

/****************************************************************************
 * Converts a sample from libmad's fixed point number format to a signed	*
 * short (16 bits).															*
 ****************************************************************************/
signed short CMP3File::MadFixedToSshort(mad_fixed_t sample)
{
	/* A fixed point number is formed of the following bit pattern:
	 *
	 * SWWWFFFFFFFFFFFFFFFFFFFFFFFFFFFF
	 * MSB                          LSB
	 * S ==> Sign (0 is positive, 1 is negative)
	 * W ==> Whole part bits
	 * F ==> Fractional part bits
	 *
	 * This pattern contains MAD_F_FRACBITS fractional bits, one
	 * should alway use this macro when working on the bits of a fixed
	 * point number. It is not guaranteed to be constant over the
	 * different platforms supported by libmad.
	 *
	 * The signed short value is formed, after clipping, by the least
	 * significant whole part bit, followed by the 15 most significant
	 * fractional part bits. Warning: this is a quick and dirty way to
	 * compute the 16-bit number, madplay includes much better
	 * algorithms.
	 */

	/*
	// Clipping
	if(Fixed >= MAD_F_ONE)
		return(SHRT_MAX);
	if(Fixed<=-MAD_F_ONE)
		return(-SHRT_MAX);

	// Conversion.
	Fixed=Fixed>>(MAD_F_FRACBITS-15);
	return((signed short)Fixed);
	*/

  /* round */
	sample += (1L << (MAD_F_FRACBITS - 16));

	/* clip */
	if (sample >= MAD_F_ONE)
		sample = MAD_F_ONE - 1;
	else if (sample < -MAD_F_ONE)
		sample = -MAD_F_ONE;

	/* quantize */
	return sample >> (MAD_F_FRACBITS + 1 - 16);
}
