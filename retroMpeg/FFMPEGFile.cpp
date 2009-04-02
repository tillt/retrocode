/*\
 * FFMPEGFile.cpp
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
#include <errno.h>
#include "../include/Resource.h"
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
#include "lame/lame.h"

/* max frame size, in samples */
#define MPA_FRAME_SIZE 1152 
#define BUFFER_SIZE (2*MPA_FRAME_SIZE)
typedef struct Mp3AudioContext {
        lame_global_flags *gfp;
        int stereo;
        uint8_t buffer[BUFFER_SIZE];
        int buffer_index;
} Mp3AudioContext;

const char *pcGetProgress(char *pcOut,unsigned int nTotal,unsigned int nDone)
{
	int nPercent;
	ASSERT(nTotal);
	nPercent=(int)(((uint64_t)nDone*100)/nTotal);
	sprintf(pcOut,"processed %d%%\015",nPercent);
	return (const char *)pcOut;
}

tstring CFFMPEGFile::sGetError(int err)
{
	tstring strRet;
    switch(err) 
	{
		case AVERROR_NUMEXPECTED:
			strRet="Incorrect image filename syntax.\nUse '%%d' to specify the image number:\n for img1.jpg, img2.jpg, ..., use 'img%%d.jpg';\n for img001.jpg, img002.jpg, ..., use 'img%%03d.jpg'.\n";
		break;
		case AVERROR_INVALIDDATA:
			strRet="Error while parsing header\n";
		break;
		case AVERROR_NOFMT:
			strRet="Unknown format\n";
		break;
		case AVERROR_IO:
			strRet="I/O error occured\nUsually that means that input file is truncated and/or corrupted.\n";
		break;
		case AVERROR_NOMEM:
			strRet="memory allocation error occured\n";
		break;
		default:
			strRet="Error while opening file\n";
    }
	return strRet;
}

/*\
 * <---------- CFFMPEGFile :: FFMPEGread ----------> 
 * @m ffmpeg read callback
 * --> I N <-- @p
 * void *opaque - 
 * uint8_t *buf - 
 * int buf_size - 
 * <-- OUT --> @r
 * 
\*/
int CFFMPEGFile::FFMPEGread(void *opaque, uint8_t *buf, int buf_size)
{
	int nRead=0;
	int nReadable=0;
	istream *pArchive=(istream *)opaque;
	ASSERT(pArchive);
	ASSERT(buf);
	TRACEIT2("my read %d bytes\n",buf_size);
	offset_t pos=0;
	pos=pArchive->tellg();
	Log2(verbLevDebug3,"read from position %d - %d\n",(unsigned long)pos,pos+buf_size);
	if (buf_size > 0)
	{
		try
		{
			pArchive->read((char *)buf,buf_size);
			nRead=buf_size;
		}
		catch (istream::failure const &e)
		{
			if (!pArchive->eof())
				Log2(verbLevErrors,"File Access Exception\n\t%s\n",e.what());
			nRead=pArchive->gcount();
		}
	}
	else
		nRead=0;
	TRACEIT2("%d bytes done\n",nRead);
	pArchive->clear();
	return nRead;
}

/*\
 * <---------- CFFMPEGFile :: FFMPEGwrite ----------> 
 * @m ffmpeg write callback
 * --> I N <-- @p
 * void *opaque - 
 * uint8_t *buf - 
 * int buf_size - 
 * <-- OUT --> @r
 * 
\*/
int CFFMPEGFile::FFMPEGwrite(void *opaque, uint8_t *buf, int buf_size)
{
	ostream *pArchive=(ostream *)opaque;
	int nWritten=0;
	TRACEIT2("my write\n");
	ASSERT(buf);
	ASSERT(buf_size);
	try
	{
		pArchive->write((const char *)buf,buf_size);
		nWritten=buf_size;
	}
	catch (ostream::failure const &e)
	{
		Log2(verbLevErrors,"File Access Exception\n\t%s\n",e.what());
		nWritten=0;
	}
	pArchive->clear();
	return nWritten;
}

/*\
 * <---------- CFFMPEGFile :: FFMPEGseek_out ----------> 
 * @m ffmpeg (output-)seek callback
 * --> I N <-- @p
 * void *opaque    - 
 * offset_t offset - 
 * int whence      - 
 * <-- OUT --> @r
 * 
\*/
offset_t CFFMPEGFile::FFMPEGseek_out(void *opaque, offset_t offset, int whence)
{
	offset_t pos=0;
	ostream *pArchive=(ostream *)opaque;
	ASSERT(pArchive);
	TRACEIT2("my seeking\n");
	if (whence != SEEK_CUR && whence != SEEK_SET)
	{
		Log2(verbLevErrors,"Invalid seek request (%04Xh)\n",whence);
		return AVERROR(EINVAL);
	}
	try
	{
		pArchive->seekp((unsigned long)offset,(ios_base::seekdir) whence);
		pos=pArchive->tellp();
	}
	catch (ostream::failure const &e)
	{
		Log2(verbLevErrors,"File Access Exception\n\t%s\n",e.what());
		pos=0;
	}
	pArchive->clear();
	return pos;
}

/*\
 * <---------- CFFMPEGFile :: FFMPEGseek_in ----------> 
 * @m ffmpeg (input-)seek callback
 * --> I N <-- @p
 * void *opaque    - 
 * offset_t offset - 
 * int whence      - 
 * <-- OUT --> @r
 * 
\*/
offset_t CFFMPEGFile::FFMPEGseek_in(void *opaque, offset_t offset, int whence)
{
	offset_t pos=0;
	istream *pArchive=(istream *)opaque;
	TRACEIT2("my seeking\n");
	if (whence == 0x10000)
		return -1;
	if (whence != SEEK_CUR && whence != SEEK_SET && whence != SEEK_END)
		return AVERROR(EINVAL);
	try
	{
		pArchive->seekg((unsigned long)offset,(ios_base::seekdir) whence);
		pos=pArchive->tellg();
	}
	catch (ostream::failure const &e)
	{
		Log2(verbLevErrors,"File Access Exception\n\t%s\n",e.what());
		pos=-1;
	}
	Log2(verbLevDebug3,"seeked to position %d\n",(unsigned long)pos);
	pArchive->clear();
	return pos;
}

LPCTSTR szGetFFMPEGVersion(void)
{
	static char str[255];
	sprintf(str,"libAVcodec %s\n\tlibAVformat %s",AV_STRINGIFY(LIBAVCODEC_VERSION),AV_STRINGIFY(LIBAVFORMAT_VERSION));
    return str;
}

CFFMPEGFile::CFFMPEGFile(void)
{
	m_sFormatName="FFMPEG";
	m_nCodecId=-1;
}

CFFMPEGFile::~CFFMPEGFile(void)
{
}

 
/*
 * add an audio output stream
 */
AVStream *CFFMPEGFile::add_audio_stream(AVFormatContext *oc, int codec_id)
{
    AVCodecContext *c;
    AVStream *st;

    st = av_new_stream(oc, 1);
    if (!st) 
	{
        Log2(verbLevErrors,"could not alloc stream\n");
        return NULL;
    }
    c = st->codec;
    c->codec_id = (CodecID)codec_id;
    c->codec_type = CODEC_TYPE_AUDIO;

    /* put sample parameters */
	c->bit_rate = m_pParameters->m_nParameter[paraNumBitrate];
	c->sample_rate = m_pCSSource->m_nCSSamplesPerSecond;
	c->channels = m_pCSSource->m_nCSChannels;
	c->bits_per_sample = m_pCSSource->m_nCSBitsPerSample;
    return st;
}

void CFFMPEGFile::close_audio(AVFormatContext *oc, AVStream *st)
{
    avcodec_close(st->codec);
    //av_free(audio_outbuf);
}

void CFFMPEGFile::Read(istream &ar)
{
	CBufferCollector bc;
	AVInputFormat *fmt;
	AVFormatContext *ic;
	AVCodecContext *cc;
	AVCodec *pCodec;
	
	int nDecodedAudioBufferSize;
    int16_t *pnDecodedAudioBuffer;
	int audioStream;

	AVFormatParameters params, *ap = &params;
	ByteIOContext pb1,*pb=&pb1;
	bool bFailed=false;
 
	m_nIOBufferSize=0x7FFF;
	m_pIOBuffer=malloc(m_nIOBufferSize);
	ZeroMemory(m_pIOBuffer,m_nIOBufferSize);

	av_log_set_level(AV_LOG_QUIET); 
	av_register_all();
	//find the input format parser
	if ((fmt=av_find_input_format(m_strFFMPEGFormatName.c_str())) == NULL)
	{
		bFailed=true;
		Log2(verbLevErrors,"failed to find the input format\n");
		throw new CFormatException(CFormatException::formaterrSource,"failed to identify the input format");
	}
	
    if (!bFailed && init_put_byte(pb,(unsigned char *)m_pIOBuffer,m_nIOBufferSize,URL_RDONLY,&ar,FFMPEGread,FFMPEGwrite,FFMPEGseek_in) < 0)
	{
		bFailed=true;
		Log2(verbLevErrors,"failed to assign IO functions\n");
		throw new CFormatException(CFormatException::formaterrSource,"failed to assign IO functions");
	}
	if (!bFailed && av_open_input_stream(&ic,pb,"",fmt,NULL) < 0)
	{
		bFailed=true;
		Log2(verbLevErrors,"failed to open input stream\n");
		throw new CFormatException(CFormatException::formaterrSource,"failed to open input stream");
	}
	if (!bFailed && av_find_stream_info(ic) < 0)
	{
		bFailed=true;
		Log2(verbLevErrors,"could not find codec parameters\n");
		throw new CFormatException(CFormatException::formaterrSource,"could not find codec parameters");
    }
	if (!bFailed)
	{
		//dump_format(ic, 0, "", false);
		// Find the first video stream
		audioStream=nLocateBestAudioStream(ic);
	}
	if(!bFailed && audioStream == -1)
	{
		bFailed=true;
		Log2(verbLevErrors,"no audio streams found\n");
		throw new CFormatException(CFormatException::formaterrSource,"no audio streams found");
	}
	if (!bFailed)
	{
		cc=ic->streams[audioStream]->codec;
		Log2(verbLevDebug1,"got a stream, codec: \"%s\"...\n",sGetFormatName(cc->codec_id).c_str());
		if (ic->title)
			m_strInfo[infoTitle]=ic->title;
		if (ic->author)
			m_strInfo[infoArtist]=ic->author;
		if (ic->comment)
			m_strInfo[infoComments]=ic->comment;
		if (ic->copyright)
			m_strInfo[infoCopyright]=ic->copyright;
		if (ic->album)
			m_strInfo[infoAlbum]=ic->album;
		//if (ic->timestamp)
			//m_strInfo[infoDateCreated]=ic->timestamp;		
	}
	// Get a pointer to the codec context for the video stream
	if(!bFailed)
	{
		// Find the decoder for the video stream
		if ((pCodec=avcodec_find_decoder(cc->codec_id)) == NULL)
		{
			bFailed=true;
			Log2(verbLevErrors,"no codec found\n");
			throw new CFormatException(CFormatException::formaterrSource,"no codec found");
		}
	}
	if(!bFailed)
	{
		Log2(verbLevDebug1,"got a codec...\n");
		// Inform the codec that we can handle truncated bitstreams -- i.e.,
		// bitstreams where frame boundaries can fall in the middle of packets
		if(pCodec->capabilities & CODEC_CAP_TRUNCATED)
			cc->flags|=CODEC_FLAG_TRUNCATED;
		// Open codec
		if (avcodec_open(cc, pCodec) < 0)
		{
			bFailed=true;
			Log2(verbLevErrors,"could not open codec\n");
			throw new CFormatException(CFormatException::formaterrSource,"could not open codec");
		}
	}
	static AVPacket packet;
	int bytesRemaining=0;
	int bytesDecoded;
	uint8_t  *rawData;

	Mp3AudioContext *priv=(Mp3AudioContext *)cc->priv_data;

	if(!bFailed)
	{
		int nBytesRendered;
		Log2(verbLevDebug1,"decoding...\n");
		nDecodedAudioBufferSize = AVCODEC_MAX_AUDIO_FRAME_SIZE;
		pnDecodedAudioBuffer = (int16_t *)malloc(nDecodedAudioBufferSize + FF_INPUT_BUFFER_PADDING_SIZE);

		m_nCSBitsPerSample=16;
		m_nCSChannels=cc->channels;
		m_nCSSamplesPerSecond=cc->sample_rate;
		m_nCodecId=cc->codec_id;
		m_nBitRate=cc->bit_rate;
		TRACEIT2("codec id: %d\n",m_nCodecId);

		//decode packets until we have decoded a complete stream
		while(av_read_packet(ic,&packet) == 0)
		{
			Log2(verbLevDebug3,"stream id #%d...\n",packet.stream_index);
			if (packet.stream_index == audioStream)
			{
				Log2(verbLevDebug3,"packet #%d...\n",packet.pos);
				bytesRemaining=packet.size;
				rawData=packet.data;
				//work on the current packet until we have decoded all of it
				while(bytesRemaining > 0) 
				{
					// Decode the next chunk of data
					nBytesRendered=nDecodedAudioBufferSize;
					hexdump("",rawData,min(0xFFFF,bytesRemaining));
					bytesDecoded=avcodec_decode_audio2(cc, pnDecodedAudioBuffer, &nBytesRendered, rawData, bytesRemaining);
					// Was there an error?
					if(bytesDecoded < 0)
					{
						Log2(verbLevErrors,"error while decoding frame - trying to resync\n");
						--bytesRemaining;
						++rawData;
					}
					else
					{
						if (nBytesRendered)
						{  
							ASSERT(pnDecodedAudioBuffer);
							ASSERT(nBytesRendered);
							bc.CreateCopyPacket((unsigned char *)pnDecodedAudioBuffer,nBytesRendered);
						}
						bytesRemaining-=bytesDecoded;
						rawData+=bytesDecoded;
					}
				};
			}
			if (packet.data)
				av_free_packet(&packet);
		};
		TRACEIT2("end of packet read loop\n");
	}
	if(!bFailed)
	{
		if ((m_pcCSBuffer=CMobileSampleContent::Alloc(bc.nGetSize())) != NULL)
			m_nCSSize=bc.nCopyLinear((unsigned char *)m_pcCSBuffer,bc.nGetSize());
		Log2(verbLevDebug1,"pcm data size=%d\n",m_nCSSize);
		m_nPlayTime=nGetSamplePlaytime();
	}
}

/*\
 * <---------- CFFMPEGFile :: Write ----------> 
 * @m taken from "ffmpeg/output_example.c"
 * --> I N <-- @p
 * ostream &out - reference of output stream object
\*/
void CFFMPEGFile::Write(ostream &out)
{
	uint8_t *audio_outbuf;
	int audio_outbuf_size;
	int audio_input_frame_size;

	AVOutputFormat *fmt;
	AVFormatContext *oc;
	AVStream *audio_st;
	int err;
	unsigned int i;

	if (m_pParameters->m_nParameter[paraNumBitrate] == 0)
	{
		m_pParameters->m_nParameter[paraNumBitrate]=m_pCSSource->m_nCSChannels * m_nFFMPEGDefaultBitrate;
		Log2(verbLevWarnings,"bitrate not set, using: %d\n",m_pParameters->m_nParameter[paraNumBitrate]);
	}
	Log2(verbLevDebug1,"bitrate: %dbps\n",m_pParameters->m_nParameter[paraNumBitrate]);
	if (m_pCSSource->m_nCSBitsPerSample != 16 && m_pCSSource->m_nCSBitsPerSample != 8)
	{
		TRACEIT2("sample width incompatible\n");
		throw new CFormatException(CFormatException::formaterrIncompatibleSampleFormat,"sample width incompatible");
	}
	m_nEncBytesLeft=m_pCSSource->m_nCSSize;
	m_pcEnc=(char *)m_pCSSource->m_pcCSBuffer;
	m_nIOBufferSize=32768;
	m_pIOBuffer=new char[m_nIOBufferSize];
	//initialize libavcodec, and register all codecs and formats
	av_register_all();
	//auto detect the output format from the name. default is mpeg.
	if ((fmt = guess_format(m_strFFMPEGFormatName.c_str(), NULL, NULL)) == NULL)
	{
		Log2(verbLevErrors,"could not find suitable output format\n");
		throw new CFormatException(CFormatException::formaterrUnknown,"could not find suitable output format");
	}
	//patch FFMPEG audio codec with RetroCode's default value
	fmt->audio_codec=(CodecID)m_nCodecId;
	//allocate the output media context
	oc = av_alloc_format_context();
	if (!oc) 
	{
		Log2(verbLevErrors,"memory error\n");
		throw new CFormatException(CFormatException::formaterrUnknown,"memory error");
	}
	oc->oformat = fmt;
	if (!m_pParameters->m_strParameter[paraStrTitle].empty())
		strcpy (oc->title,m_pParameters->m_strParameter[paraStrTitle].c_str());
	if (!m_pParameters->m_strParameter[paraStrArtist].empty())
		strcpy (oc->author,m_pParameters->m_strParameter[paraStrArtist].c_str());
	if (!m_pParameters->m_strParameter[paraStrCopyright].empty())
		strcpy (oc->copyright,m_pParameters->m_strParameter[paraStrCopyright].c_str());
	if (!m_pParameters->m_strParameter[paraStrNote].empty())
		strcpy (oc->comment,m_pParameters->m_strParameter[paraStrNote].c_str());
	if (!m_pParameters->m_strParameter[paraStrCategory].empty())
		strcpy (oc->album,m_pParameters->m_strParameter[paraStrCategory].c_str());
	//add the audio stream using the default format codec and initialize the codec
	audio_st = NULL;
	if (fmt->audio_codec != CODEC_ID_NONE) 
		audio_st = add_audio_stream(oc, fmt->audio_codec);
	//set the output parameters (must be done even if no parameters)
	if (av_set_parameters(oc, NULL) < 0) 
	{
		Log2(verbLevErrors,"Invalid output format parameters\n");
		throw new CFormatException(CFormatException::formaterrUnknown,"invalid output format parameters");
	}
//#ifdef _DEBUG
//	dump_format(oc, 0, "out.*", 1);
//#endif
	//now that all the parameters are set, we can open the audio and video codecs and allocate the necessary encode buffers
	if (audio_st)
	{
		AVCodecContext *c;
		AVCodec *codec;
	    c = audio_st->codec;
		//find the audio encoder
		codec = avcodec_find_encoder(c->codec_id);
		if (!codec) 
		{
			Log2(verbLevErrors,"codec not found\n");
			throw new CFormatException(CFormatException::formaterrUnknown,"codec not found");
		}
		//open it
		if ((err = avcodec_open(c, codec)) < 0) 
		{
			tstring strErr=sGetError(err);
			Log2(verbLevErrors,"could not open codec: %s",strErr.c_str());
			throw new CFormatException(CFormatException::formaterrUnknown,"could not open codec");
		}
		//audio_outbuf_size = m_m_pCSSource->m_nCSSize;
		audio_outbuf_size = AVCODEC_MAX_AUDIO_FRAME_SIZE;
		audio_outbuf = (uint8_t *)malloc(audio_outbuf_size + FF_INPUT_BUFFER_PADDING_SIZE);

		//ugly hack for PCM codecs (will be removed ASAP with new PCM support to compute the input frame size in samples
		if (c->frame_size <= 1) 
		{
			audio_input_frame_size = audio_outbuf_size / c->channels;
			switch(audio_st->codec->codec_id) 
			{
				case CODEC_ID_PCM_S16LE:
				case CODEC_ID_PCM_S16BE:
				case CODEC_ID_PCM_U16LE:
				case CODEC_ID_PCM_U16BE:
					audio_input_frame_size >>= 1;
				break;
			}
		} 
		else 
		{
			audio_input_frame_size = c->frame_size;
		}
	}
	
	oc->pb=av_alloc_put_byte((unsigned char *)m_pIOBuffer,m_nIOBufferSize,URL_WRONLY,&out,FFMPEGread,FFMPEGwrite,FFMPEGseek_out);

	//write the stream header, if any 
	av_write_header(oc);

	AVPacket pkt;
	av_init_packet(&pkt);

	AVCodecContext *c=audio_st->codec;
	uint32_t nLeFrame=(unsigned long)audio_input_frame_size*(2*c->channels);
	//encode all complete frames
	while(m_nEncBytesLeft > nLeFrame)
	{
		ASSERT(audio_input_frame_size);
		ASSERT(audio_outbuf_size);
		ASSERT(m_pcEnc);
		ASSERT(audio_outbuf);
		pkt.size=avcodec_encode_audio(c, audio_outbuf, audio_outbuf_size, (const short *)m_pcEnc);
		pkt.pts=av_rescale_q(c->coded_frame->pts, c->time_base, audio_st->time_base);
		pkt.flags |= PKT_FLAG_KEY;
		pkt.stream_index=audio_st->index;
		pkt.data=audio_outbuf;
		//write the compressed frame in the media file
		if (av_write_frame(oc, &pkt) != 0) 
		{
			Log2(verbLevErrors,"error while writing audio frame\n");
			throw new CFormatException(CFormatException::formaterrUnknown,"error while writing audio frame");
		}
		m_pcEnc+=nLeFrame;
		m_nEncBytesLeft-=nLeFrame;

		//char pcPrg[25];
		//pcGetProgress(pcPrg,m_pCSSource->m_nCSSize,m_pCSSource->m_nCSSize-m_nEncBytesLeft);
		//cout << pcPrg;
	};
	//incomplete frame left?
	if (m_nEncBytesLeft)
	{	//yes->...
		char *pcLastFrame=NULL;
		pcLastFrame=new char[nLeFrame];
		ZeroMemory(pcLastFrame,nLeFrame);
		CopyMemory(pcLastFrame,m_pcEnc,m_nEncBytesLeft);

		ASSERT(audio_input_frame_size);
		ASSERT(audio_outbuf_size);
		ASSERT(m_pcEnc);
		ASSERT(audio_outbuf);
		pkt.size=avcodec_encode_audio(c, audio_outbuf, audio_outbuf_size, (const short *)pcLastFrame);
		pkt.pts=av_rescale_q(c->coded_frame->pts, c->time_base, audio_st->time_base);
		pkt.flags |= PKT_FLAG_KEY;
		pkt.stream_index=audio_st->index;
		pkt.data=audio_outbuf;
		//write the compressed frame in the media file
		if (av_write_frame(oc, &pkt) != 0) 
		{
			Log2(verbLevErrors,"error while writing audio frame\n");
			throw new CFormatException(CFormatException::formaterrUnknown,"error while writing audio frame");
		}
	}

	//close codec
	if (audio_st)
		close_audio(oc, audio_st);
	//write the trailer, if any
	av_write_trailer(oc);
	// free the streams
	for(i = 0; i < oc->nb_streams; i++) 
	{
		av_freep(&oc->streams[i]->codec);
		av_freep(&oc->streams[i]);
	}
	//free the stream
	av_free(oc);

	m_nCSBitsPerSample=0;
	m_nCSChannels=m_pCSSource->m_nCSChannels;
	m_nCSSamplesPerSecond=m_pCSSource->m_nCSSamplesPerSecond;
	out.seekp(0,ios_base::end);
	m_nFileSize=out.tellp();
	m_nCSSize=m_nFileSize;
}

tstring CFFMPEGFile::sGetFormatName(int nCodec)
{
	CMyString strText;

	map<int,int> :: const_iterator iterCodec;
	map<int,int> myMap;

	myMap[CODEC_ID_RA_144]=IDS_FORMATNAME_RA_144;
    myMap[CODEC_ID_RA_288]=IDS_FORMATNAME_RA_288;
	myMap[CODEC_ID_COOK]=IDS_FORMATNAME_RA_COOK;
	myMap[CODEC_ID_AC3]=IDS_FORMATNAME_RA_AC3;
	myMap[CODEC_ID_MP3]=IDS_FORMATNAME_MPEG;
	myMap[CODEC_ID_MP2]=IDS_FORMATNAME_MPEG;
	myMap[CODEC_ID_VORBIS]=IDS_FORMATNAME_VORBIS;
	myMap[CODEC_ID_WMAV1]=IDS_FORMATNAME_WMV1;
	if ((iterCodec=myMap.find(nCodec)) == myMap.end())
		strText.Load(IDS_FORMATNAME_RA_UNKNOWN);
	else
		strText.Load(iterCodec->second);
	return strText;
}

int CFFMPEGFile::nLocateBestAudioStream(AVFormatContext *pFormatContext)
{
	int audioStream=-1;
	int nMaxBitRate=0;
	unsigned int i;
	for(i=0; i < pFormatContext->nb_streams; i++)
	{
		if(pFormatContext->streams[i]->codec->codec_type == CODEC_TYPE_AUDIO)
		{
			if (pFormatContext->streams[i]->codec->bit_rate > nMaxBitRate)
			{
				nMaxBitRate=pFormatContext->streams[i]->codec->bit_rate;
				audioStream=i;
			}
		}
	}
	return audioStream;
}
