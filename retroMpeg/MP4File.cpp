/*\
 * MP4File.cpp
 * Copyright (C) 2004-2007, MMSGURU - written by Till Toenshoff
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program i	s distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
\*/
#include "stdafx.h"
#include <sys/timeb.h> 
#include <map>
#include <fstream>
#include <stdlib.h>
#include "../include/Resource.h"
#include "../retroBase/Basics.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MyString.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/BitStream.h"
#include "../retroBase/PacketCollection.h"
#include "MP4File.h"
#include "AACFile.h"
#include "AMRFile.h"
#include "faac.h"
#undef MAIN
#undef SSR
#undef LTP
#include "neaacdec.h"
#include "mp4ff.h"

#define AAC_DEFAULT_BITRATE 64000

#ifdef __cplusplus
extern "C"{
#endif
#include "amrnb/interf_dec.h"
#ifdef __cplusplus
}
#endif
#undef LANG_ENGLISH

#ifndef WIN32
#define _ftime ftime
#define _timeb timeb
#endif

#include "mpeg4ip.h"
#include "mp4.h"
//#include "ismacryp/ismacryplib.h"
#include "MP4Property.h"

const uint32_t CFAACBase::m_cnSampleRates[16] = 
{
	96000, 88200, 64000, 48000, 44100, 32000,	
	24000, 22050, 16000, 12000, 11025, 8000,	
	7350, 0, 0, 0								
};

int gettimeofday (struct timeval *t, void *foo)
{
	struct _timeb temp;
	_ftime(&temp);
	t->tv_sec = (uint32_t)temp.time;
	t->tv_usec = temp.millitm * 1000;
	return (0);
} 

LPCTSTR szGetMPEG4IPVersion(void)
{
	static char str[255];
	sprintf(str,"mpeg4ip %s",MPEG4IP_VERSION);
    return str;
}

LPCTSTR szGetFAACVersion(void)
{
	static char str[255];
	char *faac_id_string=NULL;
	char *faac_copyright_string=NULL; 
	if (faacEncGetVersion(&faac_id_string, &faac_copyright_string) == FAAC_CFG_VERSION)
	{
		sprintf(str,"libFAAC %s",faac_id_string);
	}
    return str;
}

LPCTSTR szGetFAADVersion(void)
{
	static char str[255];
	//uint32_tcap = NeAACDecGetCapabilities(); 
	//sprintf(str,"libFAAD2 %s (%s)",FAAD2_VERSION, cap & FIXED_POINT_CAP ? "Fixed point version" : "Floating point version");
	sprintf(str,"libFAAD2 %s",FAAD2_VERSION);
    return str;
}

CFAACBase::CFAACBase(void)
{
	m_sFormatName="FAAD2BASE";
	m_nCSChannels=0;
	m_nObjectType=0;
	m_nCSSamplesPerSecond=0;
	m_nVersion=0;
	m_nBitRate=0;
	strcpy(m_pcMP4Type[0],"");
	strcpy(m_pcMP4Type[1],"");
	strcpy(m_pcMP4Type[2],"");
	strcpy(m_pcMP4Type[3],"");
	m_bUse3GPEncoding=false;
	m_nFrameRate=0;
	m_nWidth=0;
	m_nHeight=0;
	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono|CSampleCompatibility::supportsStereo].addDigit(7350);
	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono|CSampleCompatibility::supportsStereo].addDigit(8000);
	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono|CSampleCompatibility::supportsStereo].addDigit(11025);
	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono|CSampleCompatibility::supportsStereo].addDigit(12000);
	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono|CSampleCompatibility::supportsStereo].addDigit(16000);
	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono|CSampleCompatibility::supportsStereo].addDigit(22050);
	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono|CSampleCompatibility::supportsStereo].addDigit(24000);
	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono|CSampleCompatibility::supportsStereo].addDigit(32000);
	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono|CSampleCompatibility::supportsStereo].addDigit(44100);
	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono|CSampleCompatibility::supportsStereo].addDigit(48000);
	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono|CSampleCompatibility::supportsStereo].addDigit(64000);
	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono|CSampleCompatibility::supportsStereo].addDigit(88200);
	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono|CSampleCompatibility::supportsStereo].addDigit(96000);
}

CFAACBase::~CFAACBase(void)
{
}

/*\
 * <---------- sGetFormatName ---------->
 * @m get profile name
 * --> I N <-- @p
 * int nFormat - 
 * <-- OUT --> @r
 * tstring - profile name
\*/
tstring CFAACBase::sGetFormatName(int nFormat)
{	
	CMyString sAudio="";
	CMyString sVideo="";
	CMyString sOut="unknown";
	int nAudioFormat=nFormat&0x0F;
	int nVideoFormat=(nFormat&0xFFF0)>>4;
	int nAudioProfileName[]=	
	{
		IDS_FORMATNAME_UNKNOWN,
		IDS_FORMATNAME_AAC_MAIN,
		IDS_FORMATNAME_AAC_LC,
		IDS_FORMATNAME_AAC_SSR,
		IDS_FORMATNAME_AAC_LTP,
		IDS_FORMATNAME_AAC_HE,
		IDS_FORMATNAME_AMR_NB,
		IDS_FORMATNAME_AMR_WB
	};
	int nVideoProfileName[]=	
	{
		IDS_FORMATNAME_UNKNOWN,
		IDS_FORMATNAME_H263,
		IDS_FORMATNAME_H264,
		IDS_FORMATNAME_MPEG4
	};
	if (nAudioFormat > 0 && nAudioFormat < 9)
		sAudio.Load(nAudioProfileName[nAudioFormat-1]);
	if (nVideoFormat > 0 && nVideoFormat < 5)
		sVideo.Load(nVideoProfileName[nVideoFormat-1]);
	if (!sAudio.empty())
		sOut=sAudio;
	if (!sVideo.empty())
		sOut=sVideo+CMyString(_T(", "))+sOut;
	return sOut;
}

/*\
 * <---------- bMagicHead ---------->
 * @m 
 * --> I N <-- @p
 * std::istream &ar - 
 * uint32_tnSize - 
 * <-- OUT --> @r
 * bool - 
\*/
bool CFAACBase::bMagicHead(std::istream &ar,uint32_t nSize)
{
	bool bRet=false;
	char buffer[12];
	unsigned char adxx_id[5];
 	uint32_t tagsize;

	m_nFileSize=nSize;

	try 
	{
		ar.read((char *)buffer,10);
		if (memcmp(buffer, "ID3", 3) == 0)
		{
			// high bit is not used 
			tagsize = (buffer[6] << 21) | (buffer[7] << 14) | (buffer[8] <<  7) | (buffer[9] <<  0);
			ar.seekg(tagsize,ios_base::cur);
			tagsize += 10;
		}
		else
		{
			tagsize = 0;
			ar.seekg(0,ios_base::beg);
		}
		adxx_id[4]=0;
		ar.seekg(4,ios_base::cur);
		ar.read((char *)adxx_id, 4);
		TRACEIT("tag: %s\n",adxx_id);
		if(memcmp(adxx_id,"ftyp", 4) == 0)
		{
			ar.read((char *)adxx_id, 4);
			TRACEIT("tag: %s\n",adxx_id);
			if(memcmp(adxx_id, m_pcMP4Type[0], 3) == 0)
				bRet=true;
			else if(strlen(m_pcMP4Type[1]) && memcmp(adxx_id, m_pcMP4Type[1], 3) == 0)
				bRet=true;
			else if(strlen(m_pcMP4Type[2]) && memcmp(adxx_id, m_pcMP4Type[2], 3) == 0)
				bRet=true;
			else if(strlen(m_pcMP4Type[3]) && memcmp(adxx_id, m_pcMP4Type[3], 3) == 0)
				bRet=true;
		}
	}
	catch(istream::failure const &e)
	{
		Log2(verbLevErrors,"File Access Exception\n\t%s\n",e.what());
		TRACEIT("catching exception in bMagicHead\n");
		bRet=false;
	}
	return bRet;
}

/*\
 * <---------- read_callback ---------->
 * @m 
 * --> I N <-- @p
 * void *user_data - 
 * void *buffer - 
 * unsigned int length - 
 * <-- OUT --> @r
 * unsigned int - 
\*/
unsigned int CFAACBase::read_callback(void *user_data, void *buffer, unsigned int length)
{
	istream *ar=(istream *)user_data;
	try 
	{
		ar->read((char *)buffer, length);
	}
	catch(istream::failure const &e)
	{
		Log2(verbLevDebug1,"File Access Exception\n\t%s\n",e.what());
		TRACEIT("catching exception in read_callback (tried %d - got %d)\n",length,ar->gcount());
	}
	ar->clear();
	return ar->gcount();
}

/*\
 * <---------- seek_callback ---------->
 * @m 
 * --> I N <-- @p
 * void *user_data - 
 * LONGLONG position - 
 * <-- OUT --> @r
 * uint32_t - 
\*/
uint32_t CFAACBase::seek_callback(void *user_data, int64_t position)
{
	istream *ar=(istream *)user_data;
	try 
	{
		ar->seekg((unsigned int)position,istream::beg);
	}
	catch(istream::failure const &e)
	{
		Log2(verbLevErrors,"File Access Exception\n\t%s\n",e.what());
		TRACEIT("catching exception in read_callback\n");
		position=0;
	}
	ar->clear();
	return (unsigned int)position;
}

/*\
 * <---------- GetAACTrack ---------->
 * @m 
 * --> I N <-- @p
 * void *infile - 
 * <-- OUT --> @r
 * int - 
\*/
int CFAACBase::GetAACTrack(void *infile)
{
    /* find AAC track */
    int i, rc;
	int nTrackType=0;
    int numTracks = mp4ff_total_tracks((mp4ff_t *)infile);

	Log2(verbLevDebug2,"tracks in file: %d\n",numTracks);
    for (i = 0; i < numTracks; i++)
    {
        unsigned char *buff = NULL;
        int buff_size = 0;

		nTrackType=mp4ff_get_track_type((mp4ff_t *)infile,i);
		Log2(verbLevDebug2,"track %d - Track Type: %d\n",i,nTrackType);
		if (nTrackType != 2)
		{
			mp4ff_get_decoder_config((mp4ff_t *)infile, i, &buff,(unsigned int*)&buff_size);
			if (buff)
			{
				mp4AudioSpecificConfig mp4ASC;
				rc = NeAACDecAudioSpecificConfig(buff, buff_size, &mp4ASC);
				free(buff);
				Log2(verbLevDebug2,"track %d - objectType: %d, sampleFreq: %d\n",i,mp4ASC.objectTypeIndex,mp4ASC.samplingFrequencyIndex);
				if (rc < 0)
					continue;
				return i;
			}
		}
    }

    /* can't decode this */
    return -1;
}


int CFAACBase::GetVideoTrack(void *infile)
{
    int i;
	int nTrackType=0;
    int numTracks = mp4ff_total_tracks((mp4ff_t *)infile);

    for (i = 0; i < numTracks; i++)
    {
        unsigned char *buff = NULL;
        int buff_size = 0;
		nTrackType=mp4ff_get_track_type((mp4ff_t *)infile,i);
		if (nTrackType == 2)
		{
			Log2(verbLevDebug2,"track %d - TrackType: %d\n",i,nTrackType);
			return i;
		}
    }

    // can't decode this 
    return -1;
}

/*\
 * <---------- GetAMRTrack ---------->
 * @m 
 * --> I N <-- @p
 * void *infile - 
 * <-- OUT --> @r
 * int - 
\*/
int CFAACBase::GetAMRTrack(void *infile)
{
    /* find AMR track */
    int i;
	unsigned char *pBuffer;
	unsigned int nBytes;
    int numTracks = mp4ff_total_tracks((mp4ff_t *)infile);
    for (i = 0; i < numTracks; i++)
    {
		if (mp4ff_get_track_type((mp4ff_t *)infile,i) == 1 && mp4ff_get_audio_type((mp4ff_t *)infile,i) == 0xFF)
		{
			mp4ff_read_sample((mp4ff_t *)infile,i,0,&pBuffer,&nBytes);
			Log2(verbLevDebug2,"track: %d  -- amr - mode: %d\n",i,((*pBuffer)>>3)&0x0F);
			free(pBuffer);
            return i;
		}
    }
    /* can't decode this */
    return -1;
}


/*\
 * <---------- GetTrackInfo ---------->
 * @m 
 * --> I N <-- @p
 * void *infile - 
 * int iTrack - 
 * <-- OUT --> @r
 * int - 
\*/
/*
int CFAACBase::GetTrackInfo(void *infile,int iTrack)
{
    unsigned char *buff = NULL;
    int buff_size = 0;
	unsigned char *pBuffer;
	unsigned int nBytes;
    mp4AudioSpecificConfig mp4ASC;

    mp4ff_get_decoder_config((mp4ff_t *)infile, iTrack, &buff,(unsigned int *)&buff_size);

	static const char *pszCodecName[]=
	{	
		"NULL",
		"AAC Main",
		"AAC LC",
		"AAC SSR",
		"AAC LTP",
		"SBR",
		"AAC Scalable",
		"TwinVQ",
		"CELP",
		"HVXC",
		"Reserved",
		"Reserved",
		"TTSI",
		"Main synthetic",
		"Wavetable synthesis",
		"General MIDI",
		"Algorithmic Synthesis and Audio FX",
		"ER AAC LC",
		"(Reserved)",
		"ER AAC LTP",
		"ER AAC scalable",
		"ER TwinVQ",
		"ER BSAC",
		"ER AAC LD",
		"ER CELP",
		"ER HVXC",
		"ER HILN",
		"ER Parametric",
		"(Reserved)",
		"(Reserved)",
		"(Reserved)",
		"(Reserved)"
	};
	Log2(verbLevMessages,"track: %d  type: %d - audioType: %d\n",iTrack,mp4ff_get_track_type((mp4ff_t *)infile,iTrack),mp4ff_get_audio_type((mp4ff_t *)infile,iTrack));
    if (buff)
    {
		int nObjectType,nSampleFreq,nChannels;
		
		nObjectType=(*buff&0xF8)>>3;		//5	4	1	3
		nSampleFreq=(*buff&0x07)<<1;		//3	4	1	0
		nSampleFreq|=(*(buff+1)&0x80)>>7;	//1		2	7
		nChannels=(*(buff+1)&0x78)>>3;		//4	4	2	

		free(buff);

		if (nSampleFreq < 12 && nObjectType < 32)
			Log2(verbLevMessages,"track: %d  -- audio: \"%s\" (%d) - channels: %d - freq: %d (%d)\n",iTrack,pszCodecName[nObjectType],nObjectType,nChannels,m_cnSampleRates[nSampleFreq],nSampleFreq);
		else
			Log2(verbLevErrors,"track: %d  -- audio: invalid frequency\n",iTrack);
    }
	else
	{
		if (mp4ff_get_track_type((mp4ff_t *)infile,iTrack) == 1)
		{
			mp4ff_read_sample((mp4ff_t *)infile,iTrack,0,&pBuffer,&nBytes);
			Log2(verbLevMessages,"track: %d  -- amr - mode: %d\n",iTrack,((*pBuffer)>>3)&0x0F);
		}
	}
	return 1;
}
*/

/*\
 * <---------- nGetPlaytime ---------->
 * @m 
 * --> I N <-- @p
 * int nBitRate - 
 * int nSize - 
 * <-- OUT --> @r
 * int - 
\*/
int CFAACBase::nGetPlaytime(int nBitRate,int nSize)
{
	int nRet=0;
	int nNom,nDiv;
	nNom=nSize*8;
	nDiv=nBitRate;
	ASSERT(nDiv);
	if (nDiv)
		nRet=round(nNom,nDiv);
	return nRet;
}

/*\
 * <---------- InitFromAMRTrack ---------->
 * @m 
 * --> I N <-- @p
 * void *infile - 
 * int iTrack - 
\*/
void CFAACBase::InitFromAMRTrack(void *infile,int iTrack)
{
	unsigned char *pBuffer;
	unsigned int nBytes;
	TRACEIT2("mp4ff_read_sample..\n");
	mp4ff_read_sample((mp4ff_t *)infile,iTrack,0,&pBuffer,&nBytes);
	Log2(verbLevDebug2,"track: %d  -- amr - mode: %d\n",iTrack,((*pBuffer)>>3)&0x0F);
	m_nBitRate=CAMRFile::nGetBitRate(1,((*pBuffer)>>3)&0x0F);
	TRACEIT2("bitrate: %d\n",m_nBitRate);
	m_nCSSamplesPerSecond=8000;
	m_nCSChannels=1;
	m_nObjectType=(m_nObjectType & 0xFFF0) | 7;
}

/*\
 * <---------- InitFromAACTrack ---------->
 * @m 
 * --> I N <-- @p
 * void *infile - 
 * int iTrack - 
\*/
void CFAACBase::InitFromAACTrack(void *infile,int iTrack)
{
	unsigned char *pBuffer;
	unsigned int nBytes;

    mp4ff_get_decoder_config((mp4ff_t *)infile, iTrack, &pBuffer, (unsigned int *)&nBytes);

    if (pBuffer)
    {
		int iSampleFreq;
		m_nObjectType=(m_nObjectType & 0xFFF0)|(((*pBuffer&0xF8)>>3)+2);		//5	4	1	3
		Log2(verbLevDebug3,"object type: %d\n",m_nObjectType);
		iSampleFreq=(*pBuffer&0x07)<<1;			//3	4	1	0
		iSampleFreq|=(*(pBuffer+1)&0x80)>>7;	//1		2	7
		m_nCSChannels=(*(pBuffer+1)&0x78)>>3;	//4	4	2	
		Log2(verbLevDebug3,"channels: %d\n",m_nCSChannels);
		m_nCSSamplesPerSecond=m_cnSampleRates[iSampleFreq];
		Log2(verbLevDebug3,"sample freq: %d\n",m_nCSSamplesPerSecond);
		free(pBuffer);
    }
}

/*\
 * <---------- CFAACBase :: Read ----------> 
 * @m 
 * --> I N <-- @p
 * istream &ar - 
\*/
void CFAACBase::Read(istream &ar)
{
    int track;
    uint32_t samplerate;
    unsigned char channels;

    mp4ff_t *infile;

    NeAACDecHandle hDecoder;
	NeAACDecFrameInfo frameInfo;
    NeAACDecConfigurationPtr config;
    mp4AudioSpecificConfig mp4ASC;
	CBufferCollector bc;
    unsigned char *buffer;
    int buffer_size;
	unsigned char *pcBuffer;
	unsigned int nSize;
	unsigned int nFrameIndex=0;
	int nSampleToDo;

    int first_time = 1;

    /* for gapless decoding */
    unsigned int useAacLength = 1;
    unsigned int initial = 1;
    unsigned int framesize;
    uint32_t timescale;

	int outputFormat = FAAD_FMT_16BIT;

    /* initialise the callback structure */
    mp4ff_callback_t *mp4cb =(mp4ff_callback_t *) malloc(sizeof(mp4ff_callback_t));

    //mp4File = fopen(mp4file, "rb");
    mp4cb->read = read_callback;
    mp4cb->seek = (uint32_t (*) (void *,uint64_t))seek_callback;
    mp4cb->user_data = &ar;

    hDecoder = NeAACDecOpen();

    /* Set configuration */
    config = NeAACDecGetCurrentConfiguration(hDecoder);
    config->outputFormat = FAAD_FMT_16BIT;
    config->downMatrix = 0;
    NeAACDecSetConfiguration(hDecoder, config);

	m_nPlayTime=0;

	infile = mp4ff_open_read(mp4cb);
    if (!infile)
    {
		Log2(verbLevErrors,"Error opening file\n");
        return;
    }
	if ((track = GetVideoTrack(infile)) >= 0)
	{
		m_nWidth = (int)mp4ff_get_frame_width(infile, track);
		m_nHeight = (int)mp4ff_get_frame_height(infile, track);
		m_nFrameRate = (unsigned int)mp4ff_get_frame_rate(infile, track);
		m_nObjectType = m_nObjectType | ((mp4ff_get_video_type(infile, track)+1)<<4);
		Log2(verbLevDebug2,"Video resulting nObjectType = %d\n",m_nObjectType);
		m_nPlayTime = (unsigned int)mp4ff_get_track_playtime(infile, track);
	}
    if ((track = GetAACTrack(infile)) < 0)
    {
		Log2(verbLevWarnings,"Unable to find AAC sound track in the MP4 file.\n");
		if ((track = GetAMRTrack(infile)) >= 0)
		{
			void *st;
			short synth[160];
			nSampleToDo=mp4ff_num_samples(infile,track);
			InitFromAMRTrack(infile,track);
			TRACEIT2("init decoder\n");
			if ((st = Decoder_Interface_init()) != NULL)
			{
				TRACEIT2("processing sample...\n");
				while (nSampleToDo--)
				{
					mp4ff_read_sample(infile,track,nFrameIndex++,&pcBuffer,(unsigned int *)&nSize);
					Decoder_Interface_Decode(st, pcBuffer, synth, 0);
					Log2(verbLevDebug1,"*");
					bc.CreateCopyPacket((unsigned char *)synth,160*sizeof(signed short));
					nSize=0;
				};
				TRACEIT2("decoder deinit\n");
				Decoder_Interface_exit(st);

				Log2(verbLevDebug1,"\n");
				//if ((m_pcCSBuffer=calloc(bc.nGetSize(),1)) != NULL)
				if ((m_pcCSBuffer=CMobileSampleContent::Alloc(bc.nGetSize())) != NULL)
					m_nCSSize=bc.nCopyLinear((unsigned char *)m_pcCSBuffer,bc.nGetSize());

				m_nCSBitsPerSample=16;

				if (!m_nPlayTime)
					m_nPlayTime=nGetSamplePlaytime();
			}
		}
		else
		{
			Log2(verbLevWarnings,"Unable to find AMR sound track in the MP4 file.\n");
			Log2(verbLevErrors,"No decodable sound track found in the MP4 file.\n");
		}
    }
	else
	{	
		InitFromAACTrack(infile,track);

		buffer = NULL;
		buffer_size = 0;
		mp4ff_get_decoder_config(infile, track, &buffer, (unsigned int *)&buffer_size);

		//initialize decoder library from MP4 file
		if(NeAACDecInit2(hDecoder, buffer, buffer_size, (unsigned long *)&samplerate, &channels) < 0)
		{
			/* If some error initializing occured, skip the file */
			Log2(verbLevErrors,"Error initializing decoder library.\n");
			NeAACDecClose(hDecoder);
			mp4ff_close(infile);
			free(mp4cb);
			return;
		}

		timescale = mp4ff_time_scale(infile, track);
		framesize = 1024;
		useAacLength = 0;

		if (buffer)
		{
			if (NeAACDecAudioSpecificConfig(buffer, buffer_size, &mp4ASC) >= 0)
			{
				if (mp4ASC.frameLengthFlag == 1) framesize = 960;
				if (mp4ASC.sbr_present_flag == 1) framesize *= 2;
			}
			free(buffer);
		}

		/* print some mp4 file info */
		TRACEIT2("file info:\n\n");
		{
			char *tag = NULL, *item = NULL;
			int k, j;
			
			char *ot[6] = { "NULL", 
							"MAIN AAC", 
							"LC AAC", 
							"SSR AAC", 
							"LTP AAC", 
							"HE AAC" };
			long samples = mp4ff_num_samples(infile, track);
			float f = 1024.0;
			float seconds;
			if (mp4ASC.sbr_present_flag == 1)
				f = f * (float)2.0;
			seconds = ((float)samples*(float)(f-1.0))*1000/(float)mp4ASC.samplingFrequency;
			TRACEIT2("%s\t%.3f msecs, %d ch, %d Hz\n\n", ot[(mp4ASC.objectTypeIndex > 5) ? 0 : mp4ASC.objectTypeIndex],seconds, mp4ASC.channelsConfiguration, mp4ASC.samplingFrequency);

			m_nObjectType=(m_nObjectType&0xFFF0)|(mp4ASC.objectTypeIndex+1);
			Log2(verbLevDebug1,"type: %d\n",mp4ASC.objectTypeIndex);

			//m_nVersion=;
			m_nBitRate=mp4ff_get_avg_bitrate(infile, track);
			m_nCSSamplesPerSecond=mp4ASC.samplingFrequency;
			m_nCSBitsPerSample=16;
			m_nCSChannels=mp4ASC.channelsConfiguration;
			if (!m_nPlayTime)
				m_nPlayTime=(int)(seconds+.5);
			//Log2(verbLevMessages,"audio type: %d\n",mp4ff_get_audio_type(infile,track));

			uint32_t nTotalSamples;
			nTotalSamples=mp4ff_num_samples(infile,track);
			nSampleToDo=nTotalSamples;

			//GetAACData(infile,track,&m_pcCSBuffer,&m_nCSSize);
			Log2(verbLevDebug1,"decoding aac: $");
			while (nSampleToDo--)
			{
				void *sample_buffer;

				if (mp4ff_read_sample(infile,track,nFrameIndex++,&pcBuffer,(unsigned int *)&nSize) == 0)
					break;
				//TRACEIT2("ripping frame %d\n",nFrameIndex-1);
				//if (nSampleToDo > 0)
					//Log2(verbLevMessages,"%3d\b\b\b\b",100-((nSampleToDo*100)/nTotalSamples));
				//bc.CreateCopyPacket(pcBuffer,nSize);
				//free(pcBuffer);
				if((sample_buffer = NeAACDecDecode(hDecoder, &frameInfo, pcBuffer, nSize)) == NULL)
				{
					break;
				}
				//AdvanceBuffer(&b, frameInfo.bytesconsumed);
				free(pcBuffer);
				if (frameInfo.error > 0)
				{
					Log2(verbLevErrors,"Error: %s\n",NeAACDecGetErrorMessage(frameInfo.error));
				}
				if ((frameInfo.error == 0) && (frameInfo.samples > 0))
					bc.CreateCopyPacket((unsigned char *)sample_buffer,frameInfo.samples*(m_nCSBitsPerSample>>3));
			};
			Log2(verbLevDebug1,"\n");

			if ((m_pcCSBuffer=CMobileSampleContent::Alloc(bc.nGetSize())) != NULL)
				m_nCSSize=bc.nCopyLinear((unsigned char *)m_pcCSBuffer,bc.nGetSize());

			char *pszMeta;
			if (mp4ff_meta_get_title(infile, &pszMeta))
			{
				SetInfoText(infoTitle,pszMeta);
				free(pszMeta);
			}
			if (mp4ff_meta_get_artist(infile, &pszMeta))
			{
				SetInfoText(infoArtist,pszMeta);
				free(pszMeta);
			}
			if (mp4ff_meta_get_writer(infile, &pszMeta))
			{
				SetInfoText(infoWords,pszMeta);
				free(pszMeta);
			}
			if (mp4ff_meta_get_album(infile, &pszMeta))
			{
				SetInfoText(infoKeywords,pszMeta);
				free(pszMeta);
			}
			if (mp4ff_meta_get_tool(infile, &pszMeta))
			{
				SetInfoText(infoSoftware,pszMeta);
				free(pszMeta);
			}
			if (mp4ff_meta_get_comment(infile, &pszMeta))
			{
				SetInfoText(infoComments,pszMeta);
				free(pszMeta);
			}
			if (mp4ff_meta_get_genre(infile, &pszMeta))
			{
				SetInfoText(infoCategory,pszMeta);
				free(pszMeta);
			}
			if (mp4ff_meta_get_date(infile, &pszMeta))
			{
				SetInfoText(infoDateCreated,pszMeta);
				free(pszMeta);
			}
			if (mp4ff_meta_get_tempo(infile, &pszMeta))
			{
				SetInfoText(infoTempoDescription,pszMeta);
				free(pszMeta);
			}
			
			j = mp4ff_meta_get_num_items(infile);
			for (k = 0; k < j; k++)
			{
				if (mp4ff_meta_get_by_index(infile, k, &item, &tag))
				{
					if (item != NULL && tag != NULL)
					{
						Log2(verbLevDebug3,"%s: %s\n", item, tag);
						free(item); item = NULL;
						free(tag); tag = NULL;
					}
				}
			}
			
			if (j > 0) 
				Log2(verbLevDebug1,"\n");

			//DecodeAACData();
		}
	}
	NeAACDecClose(hDecoder);
    mp4ff_close(infile);
    free(mp4cb);
}

DYNIMPPROPERTY(CMP4File,CMP4Property)

CMP4File::CMP4File(void) 
{
	m_pcMagic="";
	m_nMagicSize=5;
	m_sFormatName="MPEG-4 (MP4)";
	strcpy(m_pcMP4Type[0],"mp4");
	strcpy(m_pcMP4Type[1],"MP4");
	strcpy(m_pcMP4Type[2],"m4a");
	strcpy(m_pcMP4Type[3],"m4v");
	strcpy(m_pcMP4Type[4],"M4A");
	strcpy(m_pcMP4Type[5],"M4V");
	m_sDefaultExtension=_T("mp4");
	m_listAltExtensions.push_back("m4a");
	m_listAltExtensions.push_back("m4r");
	//m_sFormatDescription
	m_sFormatCredits=_T("The MP4 encoder is entirely based on: \"mpeg4ip\", Copyright (c) 2001 by Cisco Systems Inc.");
	/*
	m_encodingPara.addPara(cmdParaNumber,paraNumBitrate);
	m_encodingPara.addPara(cmdParaBool,paraBoolAllowID3);
	m_encodingPara.addPara(cmdParaNumber,paraNumEncode);
	m_encodingPara.addPara(cmdParaNumber,paraNumAac);
	m_encodingPara.addPara(cmdParaNumber,paraNumAdts);
	m_encodingPara.addPara(cmdParaString,paraStrTitle);
	m_encodingPara.addPara(cmdParaString,paraStrNote);
	m_encodingPara.addPara(cmdParaString,paraStrCategory);
	m_encodingPara.addPara(cmdParaString,paraStrCopyright);
	*/
}

CMP4File::~CMP4File(void)
{
}

C3GPPFile::C3GPPFile(void) 
{
	m_pcMagic="";
	m_nMagicSize=5;
	m_sFormatName="3GPP File Format (3GP)";
	strcpy(m_pcMP4Type[0],"3gp");
	strcpy(m_pcMP4Type[1],"3GP");
	m_sDefaultExtension=_T("3gp");
	m_sFormatCredits=_T("");
	m_bUse3GPEncoding=true;
	/*
	m_encodingPara.addPara(cmdParaNumber,paraNumBitrate);
	m_encodingPara.addPara(cmdParaBool,paraBoolAllowID3);
	m_encodingPara.addPara(cmdParaNumber,paraNumEncode);
	m_encodingPara.addPara(cmdParaNumber,paraNumAac);
	m_encodingPara.addPara(cmdParaNumber,paraNumAdts);
	m_encodingPara.addPara(cmdParaString,paraStrTitle);
	m_encodingPara.addPara(cmdParaString,paraStrNote);
	m_encodingPara.addPara(cmdParaString,paraStrCategory);
	m_encodingPara.addPara(cmdParaString,paraStrCopyright);
	*/
}

DYNIMPPROPERTY(C3GPPFile,CMP4Property)

C3GPPFile::~C3GPPFile(void)
{
}

DYNIMPPROPERTY(C3GP2File,CMP4Property)

C3GP2File::C3GP2File(void) 
{
	m_pcMagic="";
	m_nMagicSize=5;
	m_sFormatName="3GP2 File Format (3G2)";
	strcpy(m_pcMP4Type[0],"3g2");
	strcpy(m_pcMP4Type[1],"3G2");
}

C3GP2File::~C3GP2File(void)
{
}

/*\
 * <---------- cFindSamplingRateIndex ---------->
 * @m 
 * --> I N <-- @p
 * uint32_tsamplingRate - 
 * <-- OUT --> @r
 * unsigned char - 
\*/
unsigned char CFAACBase::cFindSamplingRateIndex(uint32_t samplingRate)
{
	unsigned char i;
	for (i=0;i < 16;i++) 
	{
		if (samplingRate == m_cnSampleRates[i]) 
			return i;
	}
	return 16 - 1;
} 

/*\
 * <---------- CFAACBase :: GetConfiguration ----------> 
 * @m 
 * --> I N <-- @p
 * unsigned char **ppConfig     - 
 * uint32_t*pConfigLength - 
 * unsigned char profile        - 
 * uint32_tsamplingRate   - 
 * unsigned char channels       - 
 * <-- OUT --> @r
 * bool - 
\*/
bool CFAACBase::GetConfiguration(unsigned char **ppConfig,uint32_t *pConfigLength,unsigned char profile,uint32_t samplingRate,unsigned char channels)
{
	unsigned char *pConfig = (unsigned char *)calloc(2,1);

	if (pConfig == NULL) 
		return false;

	unsigned char samplingRateIndex = cFindSamplingRateIndex(samplingRate);

	pConfig[0] = ((profile + 1) << 3) | ((samplingRateIndex & 0xe) >> 1);
	pConfig[1] = ((samplingRateIndex & 0x1) << 7) | (channels << 3);
	*ppConfig = pConfig;
	*pConfigLength = 2;
	return true;
}

/*\
 * <---------- CFAACBase :: Write ----------> 
 * @m 
 * --> I N <-- @p
 * ostream &out - output stream reference
\*/
void CFAACBase::Write(ostream &out)
{
	char *p3gppSupportedBrands[2] = {"3gp5", "3gp4"}; 
	CBufferCollector bc;
	bool bRet=false;
	int nOffset=0;
//    uint32_tinputSamples;
//    uint32_tmaxOutputBytes;
	uint32_t numberOfBytesWritten = 0;
	uint32_t samplesPerSecond;
	uint8_t mpegVersion=0;
	uint8_t profile;
	uint8_t channelConfig;

	Log2(verbLevDebug1,"enable ID3 encoding: %s\n",m_pParameters->m_bParameter[paraBoolAllowID3] ? "true" : "false");

	//ismacryp_session_id_t ismaCrypSId;
	mp4v2_ismacrypParams *icPp =  (mp4v2_ismacrypParams *) malloc(sizeof(mp4v2_ismacrypParams));
	memset(icPp, 0, sizeof(mp4v2_ismacrypParams));

	samplesPerSecond = m_pCSSource->m_nCSSamplesPerSecond;
	if (m_pParameters->m_nParameter[paraNumEncode])
		mpegVersion = m_pParameters->m_nParameter[paraNumEncode] == 4 ? 0 : 1;
	profile = m_pParameters->m_nParameter[paraNumAac];
	channelConfig = m_pCSSource->m_nCSChannels;

	u_int8_t audioType = MP4_INVALID_AUDIO_TYPE;
	
	switch (mpegVersion) 
	{
		case 0:	audioType = MP4_MPEG4_AUDIO_TYPE;	break;
		case 1:
			switch (profile) 
			{
				case 0:		audioType = MP4_MPEG2_AAC_MAIN_AUDIO_TYPE;	break;
				case 1:		audioType = MP4_MPEG2_AAC_LC_AUDIO_TYPE;	break;
				case 2:		audioType = MP4_MPEG2_AAC_SSR_AUDIO_TYPE;	break;
				default:	Log2(verbLevErrors,	"unknown profile supplied %d\n",profile);
			}
		break;
	}
	// add the new audio track
	MP4TrackId trackId;
	MP4FileHandle mp4File;

	if (m_bUse3GPEncoding)
		mp4File=MP4CreateEx(m_pszFileName,0,0,1,0,p3gppSupportedBrands[0],0x0001,p3gppSupportedBrands,sizeof(p3gppSupportedBrands) / sizeof(p3gppSupportedBrands[0]));
	else
		mp4File=MP4Create(m_pszFileName,0,0);
	trackId = MP4AddAudioTrack(mp4File,samplesPerSecond, 1024, audioType);

	if (trackId == MP4_INVALID_TRACK_ID) 
	{
		Log2(verbLevErrors,"can't create audio track\n");
		return;
	}
	if (MP4GetNumberOfTracks(mp4File, MP4_AUDIO_TRACK_TYPE) == 1) 
	{
		MP4SetAudioProfileLevel(mp4File, 0x0F);
	}

	u_int8_t* pConfig = NULL;
	u_int32_t configLength = 0;

	GetConfiguration(&pConfig,(uint32_t*)&configLength,profile,samplesPerSecond,channelConfig);

	if (!MP4SetTrackESConfiguration(mp4File, trackId, pConfig, configLength)) 
	{
		Log2(verbLevErrors,"can't write audio configuration\n");
		MP4DeleteTrack(mp4File, trackId);
		return;
	}

	MP4SampleId sampleId = 1;
	//encode aac data
	RenderAAC(&bc,m_pCSSource);
	//write aac data to file
	for (unsigned int i=0;i < bc.nGetCount();i++)
	{
		if (bc.pbeGetPacket(i)->nGetSize())
		{
			if (!MP4WriteSample(mp4File, trackId, bc.pbeGetPacket(i)->pGetData(), bc.pbeGetPacket(i)->nGetSize())) 
			{
				Log2(verbLevErrors,"can't write audio frame %u\n",sampleId);
				MP4DeleteTrack(mp4File, trackId);
				return;
			}
			sampleId++;
		}
	};
	Log2(verbLevDebug1,"enable 3GPP encoding: %s\n",m_bUse3GPEncoding ? "true" : "false");
	MP4Close(mp4File);
	if (m_bUse3GPEncoding)
	{
		MP4Make3GPCompliant(m_pszFileName);
	}
	Log2(verbLevDebug3,"done creating mp4\n");
	ofstream ar(m_pszFileName,ios_base::binary | ios_base::app |ios_base::out);
	//write ID3 V1 tag and data..
	ar.seekp(0,ios::end);
	if (m_pParameters->m_bParameter[paraBoolAllowID3])
		RenderID3V1(ar,m_pCSSource);
	m_nFileSize=ar.tellp();
}

/*\
 * <---------- RenderAAC ---------->
 * @m encode into AAC
 * --> I N <-- @p
 * CBufferCollector *pbc - pointer to buffer collector object
 * CMobileSampleContent *pSource - pointer to source sample
\*/
void CFAACBase::RenderAAC(CBufferCollector *pbc,CMobileSampleContent *pSource)
{
	faacEncHandle hEncoder;
	bool bRet=false;
	int nOffset=0;
    uint32_t inputSamples;
    uint32_t maxOutputBytes;
	uint32_t numberOfBytesWritten = 0;

	if (pSource->m_nCSBitsPerSample != 16)
	{
		Log2(1,"sample width incompatible\n");
		throw new CFormatException(CFormatException::formaterrSource,"sample width incompatible");
	}
	if (m_pParameters->m_nParameter[paraNumBitrate] == 0)
	{
		m_pParameters->m_nParameter[paraNumBitrate]=AAC_DEFAULT_BITRATE;
		Log2(verbLevWarnings,"no bitrate given, using default %dbps\n",m_pParameters->m_nParameter[paraNumBitrate]);
	}
	//printf("%d\n",m_pParameters->m_nParameter[paraNumAac]);
	Log2(verbLevDebug1,"bitrate: %dbps\n",m_pParameters->m_nParameter[paraNumBitrate]);
	Log2(verbLevDebug1,"profile: %s\n",sGetFormatName(1 + MAIN + m_pParameters->m_nParameter[paraNumAac]).c_str());
	Log2(verbLevDebug1,"use MPEG%d encoding\n",m_pParameters->m_nParameter[paraNumEncode]);
	Log2(verbLevDebug1,"enable ADTS packet headers: %s\n",m_pParameters->m_nParameter[paraNumAdts] ? "true" : "false");

	//open and setup the encoder
	if ((hEncoder = faacEncOpen(pSource->m_nCSSamplesPerSecond,pSource->m_nCSChannels,(unsigned long int *)&inputSamples,(unsigned long int *)&maxOutputBytes)) == NULL)
	{
		Log2(1,"failed to open aac encoder\n");
		throw new CFormatException(CFormatException::formaterrUnknown,"failed to init AAC encoder");
	}

	//set encoder configuration
	faacEncConfigurationPtr config = faacEncGetCurrentConfiguration(hEncoder);
	config->inputFormat=FAAC_INPUT_16BIT;	//
	config->useLfe=0;						//
	config->useTns=0;						//
    config->allowMidside=1;					//
	config->outputFormat=m_pParameters->m_nParameter[paraNumAdts];		//0=raw, 1=ADTS
	if (m_pParameters->m_nParameter[paraNumEncode] == 4)
		config->mpegVersion=MPEG4;			//MPEG4
	else
		config->mpegVersion=MPEG2;			//MPEG2
	config->aacObjectType=MAIN + m_pParameters->m_nParameter[paraNumAac];		//MAIN, LOW, ...
	config->bitRate=m_pParameters->m_nParameter[paraNumBitrate]/pSource->m_nCSChannels;
	if (config->aacObjectType == SSR)		//set to LTP 
		config->aacObjectType = LTP;		//
	//config->quantqual=100;
	//config->bandWidth = 0;
	if (!faacEncSetConfiguration(hEncoder, config))
	{
		faacEncClose(hEncoder);
		throw new CFormatException(CFormatException::formaterrParameters,"failed to configure AAC encoder");
	}
	Log2(verbLevDebug3,"aac profile: %d\n", config->aacObjectType);
	Log2(verbLevDebug3,"quantqual: %ld\n", config->quantqual);
	Log2(verbLevDebug3,"bandwidth: %d\n", config->bandWidth);

	uint32_t totalBytesRead = 0;
	unsigned int nBytesToDo=pSource->m_nCSSize;
	unsigned int bytesInput = 0;
	short *pcmbuf;
	unsigned char *bitbuf;

	//bitbuf=(unsigned char *)calloc(maxOutputBytes,sizeof(unsigned char));
	bitbuf=new unsigned char[maxOutputBytes];
	memset(bitbuf,0,maxOutputBytes);
	pcmbuf=(signed short *)pSource->m_pcCSBuffer;

	Log2(verbLevDebug2,"encoding aac: ");
	while(nBytesToDo)
	{
		int bytesEncoded;
		bytesInput=min(nBytesToDo,inputSamples*sizeof(short));
		TRACEIT2("%3d\n",(unsigned long)((((uint64_t)pSource->m_nCSSize-nBytesToDo)*100)/pSource->m_nCSSize));
		totalBytesRead += bytesInput;
		//call the actual encoding routine
		bytesEncoded = faacEncEncode(hEncoder,(int32_t *)pcmbuf,bytesInput/2,bitbuf,maxOutputBytes);
		//all done
		if (!bytesInput && !bytesEncoded)
			break;
		if (bytesEncoded < 0)
		{
			faacEncClose(hEncoder);
			throw new CFormatException(CFormatException::formaterrUnknown);
		}
		if (bytesEncoded)
			pbc->CreateCopyPacket((unsigned char *)bitbuf,bytesEncoded);
		numberOfBytesWritten+=bytesEncoded;
		nBytesToDo-=bytesInput;
		pcmbuf+=bytesInput/2;
	};
	Log2(verbLevDebug2,"\n");
	m_nBitRate=config->bitRate;

	if (bitbuf) 
		free(bitbuf);
	faacEncClose(hEncoder);
	m_nCSBitsPerSample=0;
	m_nCSChannels=pSource->m_nCSChannels;
	m_nCSSamplesPerSecond=pSource->m_nCSSamplesPerSecond;
	m_nCSSize=numberOfBytesWritten;
}

/*\
 * <---------- CreateFile ---------->
 * @m 
 * --> I N <-- @p
 * ofstream &ar - 
 * const TCHAR *pszFileName - 
\*/
void CMP4File::CreateFile(ofstream &ar,const TCHAR *pszFileName)
{
	strcpy(m_pszFileName,pszFileName);
}

/*\
 * <---------- CloseFile ---------->
 * @m 
 * --> I N <-- @p
 * ofstream &ar - 
\*/
void CMP4File::CloseFile(ofstream &ar)
{
}
