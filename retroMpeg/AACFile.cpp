/*\
 * AACFile.cpp
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
/*
 * based on aacinfo (part of the faad2 distribution)
 * Copyright (C) 2003 M. Bakker, Ahead Software AG, http://www.nero.com
*/
#include "stdafx.h"
#include <map>
#include <stdlib.h>
#include <iostream>
#include <strstream>
#include <fstream>
#include "../include/Resource.h" 
#include "../retroBase/Basics.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MyString.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/PacketCollection.h"
#include "faac.h"
#undef MAIN
#undef SSR
#undef LTP
#include "neaacdec.h"
#include "MP4File.h"
#include "AACFile.h"
#include "MP4Property.h"
#include "AACProperty.h"

#define ADIF_MAX_SIZE 30 /* Should be enough */
#define ADTS_MAX_SIZE 10 /* Should be enough */
#define MAX_CHANNELS 6 

DYNIMPPROPERTY(CAACFile,CAACProperty)

CAACFile::CAACFile(void)
{
	m_pcMagic="";
	m_nMagicSize=5;
	m_sFormatName="Advanced Audio Coding (AAC)";
	m_sDefaultExtension=_T("aac");
	m_sFormatDescription.Load(IDS_FORMDESC_AAC);
	m_sFormatCredits=_T("The AAC codec is entirely based on: \"FAAC\", Copyright (c) 2001 by M. Bakker; \"FAAD2\", Copyright (c) 2003 by M. Bakker.");
	m_encodingPara.addPara(cmdParaNumber,paraNumBitrate);
	m_encodingPara.addPara(cmdParaBool,paraBoolAllowID3);
	m_encodingPara.addPara(cmdParaNumber,paraNumEncode);
	m_encodingPara.addPara(cmdParaNumber,paraNumAac);
	m_encodingPara.addPara(cmdParaNumber,paraNumAdts);
	m_encodingPara.addPara(cmdParaString,paraStrTitle);
	m_encodingPara.addPara(cmdParaString,paraStrArtist);
	m_encodingPara.addPara(cmdParaString,paraStrNote);
	m_encodingPara.addPara(cmdParaString,paraStrCategory);
	m_encodingPara.addPara(cmdParaString,paraStrCopyright);
}
/*
The MPEG-4 AAC patent license grants rights for multiple MPEG-4 AAC Object Types, 
including AAC LC (Low Complexity), AAC LTP (Long-Term Prediction), AAC Scalable, 
and ER AAC LD (Low Delay). 
*/
CAACFile::~CAACFile(void)
{
	TRACEIT2("killing AAC");
}

/*\
 * <---------- AdtsParse ---------->
 * @m parse a complete ADTS AAC
 * --> I N <-- @p
 * aac_buffer *b - 
 * int *bitrate - 
 * float *length - 
\*/
void CAACFile::AdtsParse(aac_buffer *b, int *bitrate, float *length)
{
    int frames, frame_length;
    int t_framelength = 0;
    int samplerate;
    float frames_per_sec, bytes_per_frame;

    /* Read all frames to ensure correct time and bitrate */
    for (frames = 0; /* */; frames++)
    {
        nFillBuffer(b);

        if (b->bytes_into_buffer > 7)
        {
            /* check syncword */
            if (!((b->buffer[0] == 0xFF)&&((b->buffer[1] & 0xF6) == 0xF0)))
                break;

            if (frames == 0)
                samplerate = m_cnSampleRates[(b->buffer[2]&0x3c)>>2];

            frame_length = ((((unsigned int)b->buffer[3] & 0x3)) << 11) | (((unsigned int)b->buffer[4]) << 3) | (b->buffer[5] >> 5);

            t_framelength += frame_length;

            if (frame_length > b->bytes_into_buffer)
                break;

            AdvanceBuffer(b, frame_length);
        } 
		else 
		{
            break;
        }
    }
	Log2(verbLevDebug1,"parsed %d bytes containing %d frames\n",b->file_offset,frames);
    frames_per_sec = (float)samplerate/1024.0f;
    if (frames != 0)
        bytes_per_frame = (float)t_framelength/(float)(frames*1000);
    else
        bytes_per_frame = 0;
    *bitrate = (int)(8. * bytes_per_frame * frames_per_sec + 0.5);
    if (frames_per_sec != 0)
        *length = (float)frames/frames_per_sec;
    else
        *length = 1;
} 

/*\
 * <---------- sGetHeaderName ---------->
 * @m get readable string describing the AAC header used
 * --> I N <-- @p
 * int nHeader - header format index
 * <-- OUT --> @r
 * tstring - ADIF or ADTS
\*/
tstring CAACFile::sGetHeaderName(int nHeader)
{
	const int nFormatName[]={IDS_FORMATNAME_AAC_ADIF,IDS_FORMATNAME_AAC_ADTS};
	CMyString sOut="unknown";
	if (nHeader > 0)
		sOut.Load(nFormatName[(nHeader-1)&0x01]);
	return sOut;
}

/*\
 * <---------- CAACFile :: nFillBuffer ----------> 
 * @m 
 * --> I N <-- @p
 * aac_buffer *b - 
 * <-- OUT --> @r
 * 
\*/
int CAACFile::nFillBuffer(aac_buffer *b)
{
    int bread;
    if (b->bytes_consumed > 0)
    {
        if (b->bytes_into_buffer)
        {
            memmove((void*)b->buffer, (void*)(b->buffer + b->bytes_consumed),b->bytes_into_buffer*sizeof(unsigned char));
        }
        if (!b->at_eof)
        {
			bread=b->bytes_consumed;
			try
			{
				b->ar->read((char *)(b->buffer + b->bytes_into_buffer), b->bytes_consumed);
			}
			catch (istream::failure const &e)
			{
				Log2(1,"File Access Exception\n\t%s\n",e.what());
				bread=b->ar->gcount();
				b->ar->clear();
			}
			if (bread != b->bytes_consumed)
                b->at_eof = 1;

            b->bytes_into_buffer += bread;
        }

        b->bytes_consumed = 0;

        if (b->bytes_into_buffer > 3)
        {
            if (memcmp(b->buffer, "TAG", 3) == 0)
                b->bytes_into_buffer = 0;
        }
        if (b->bytes_into_buffer > 11)
        {
            if (memcmp(b->buffer, "LYRICSBEGIN", 11) == 0)
                b->bytes_into_buffer = 0;
        }
        if (b->bytes_into_buffer > 8)
        {
            if (memcmp(b->buffer, "APETAGEX", 8) == 0)
                b->bytes_into_buffer = 0;
        }
    }

    return 1;
}

/*\
 * <---------- CAACFile :: AdvanceBuffer ----------> 
 * @m advance offsets by n bytes
 * --> I N <-- @p
 * aac_buffer *b - 
 * int bytes     - 
 * <-- OUT --> @r
 * 
\*/
void CAACFile::AdvanceBuffer(aac_buffer *b, int bytes)
{
    b->file_offset += bytes;
    b->bytes_consumed = bytes;
    b->bytes_into_buffer -= bytes;
}

/*\
 * <---------- CAACFile :: Read ----------> 
 * @m 
 * --> I N <-- @p
 * istream &ar - 
 * <-- OUT --> @r
 * 
\*/
void CAACFile::Read(istream &ar)
{
	int old_format=0;
	int def_srate=0;
	int object_type = LC;
	int outputFormat = FAAD_FMT_16BIT; 
	int downMatrix=0;

    int tagsize;
    uint32_t samplerate;
    unsigned char channels;
    void *sample_buffer;

    NeAACDecHandle hDecoder;
    NeAACDecFrameInfo frameInfo;
    NeAACDecConfigurationPtr config;

    int old_percent = -1;
    int bread;
    int header_type = 0;
    int bitrate = 0;
    float length = 0;

	aac_buffer b;						//aac data buffer
	CBufferCollector bc;				//pcm data buffer list

    memset(&b, 0, sizeof(aac_buffer));

	if (!(b.buffer = (unsigned char*)calloc(1,FAAD_MIN_STREAMSIZE*MAX_CHANNELS)))
    {
		Log2(verbLevErrors,"Memory allocation error\n");
        return;
    }
	b.buffer_end=b.buffer+(FAAD_MIN_STREAMSIZE*MAX_CHANNELS);
    
	bread=FAAD_MIN_STREAMSIZE*MAX_CHANNELS;
    ar.read((char *)b.buffer,bread);
    
	b.bytes_into_buffer = bread;
    b.bytes_consumed = 0;
    b.file_offset = 0;
	b.ar=&ar;

    if (bread != FAAD_MIN_STREAMSIZE*MAX_CHANNELS)
        b.at_eof = 1;

    tagsize = 0;
    if (!memcmp(b.buffer, "ID3", 3))
    {
        /* high bit is not used */
        tagsize = (b.buffer[6] << 21) | (b.buffer[7] << 14) | (b.buffer[8] <<  7) | (b.buffer[9] <<  0);

        tagsize += 10;
        AdvanceBuffer(&b, tagsize);
        nFillBuffer(&b);
    }

	/*
	uint32_tnCaps=NeAACDecGetCapabilities();
	TRACEIT2("LC capable: %s\n",nCaps & LC_DEC_CAP ? "yes" : "no");
	TRACEIT2("Main capable: %s\n",nCaps & MAIN_DEC_CAP ? "yes" : "no");
	TRACEIT2("LTP capable: %s\n",nCaps & LTP_DEC_CAP ? "yes" : "no");
	TRACEIT2("LD capable: %s\n",nCaps & LD_DEC_CAP ? "yes" : "no");
	TRACEIT2("error resilience capable: %s\n",nCaps & ERROR_RESILIENCE_CAP ? "yes" : "no");
	TRACEIT2("fixed point capable: %s\n",nCaps & FIXED_POINT_CAP ? "yes" : "no");
	*/
    hDecoder = NeAACDecOpen();

    /* Set the default object type and samplerate */
    /* This is useful for RAW AAC files */
    config = NeAACDecGetCurrentConfiguration(hDecoder);
    if (def_srate)
        config->defSampleRate = def_srate;
    config->defObjectType = object_type;
	TRACEIT2("default object type %d\n",config->defObjectType);
    config->outputFormat = outputFormat;
	TRACEIT2("output format %d\n",config->outputFormat);
    config->downMatrix = downMatrix;
	TRACEIT2("downmatrix %d\n",config->downMatrix);
    config->useOldADTSFormat = old_format;
    config->dontUpSampleImplicitSBR = 1;
    if (!NeAACDecSetConfiguration(hDecoder, config))
	{
		NeAACDecClose(hDecoder);
		Log2(verbLevErrors,"couldnt initialize decoder with our configuration\n");
		return;
	}

    //get AAC infos from file
    header_type = 0;
    if ((b.buffer[0] == 0xFF) && ((b.buffer[1] & 0xF6) == 0xF0))
    {
        AdtsParse(&b, &bitrate, &length);
		ar.seekg(tagsize,ios_base::beg);
		bread = FAAD_MIN_STREAMSIZE*MAX_CHANNELS;
		ar.read((char *)b.buffer, bread);
        if (bread != FAAD_MIN_STREAMSIZE*MAX_CHANNELS)
            b.at_eof = 1;
        else
            b.at_eof = 0;

		info.version=(b.buffer[1] & 0x08) == 0 ? 4 : 2;

        b.bytes_into_buffer = bread;
        b.bytes_consumed = 0;
        b.file_offset = tagsize;
        header_type = 1;
    } 
	else if (memcmp(b.buffer, "ADIF", 4) == 0) 
	{
        int skip_size = (b.buffer[4] & 0x80) ? 9 : 0;
        bitrate =	((unsigned int)(b.buffer[4 + skip_size] & 0x0F)<<19) |
					((unsigned int)b.buffer[5 + skip_size]<<11) |
					((unsigned int)b.buffer[6 + skip_size]<<3) |
					((unsigned int)b.buffer[7 + skip_size] & 0xE0);

        length = (float)m_nFileSize;
        if (length != 0)
            length = ((float)length*8.f)/((float)bitrate) + 0.5f;
        bitrate = (int)((float)bitrate/1000.0f + 0.5f);
        header_type = 2;
		info.version=2;
    }


    nFillBuffer(&b);
	TRACEIT2("initializing AAC decoder\n");
	samplerate=44100;
	channels=2;
    if ((bread = NeAACDecInit(hDecoder, b.buffer,b.bytes_into_buffer,(unsigned long *)&samplerate, &channels)) < 0)
    {
		Log2(verbLevErrors,"could not initialize decoder with first package\n");
        if (b.buffer)
            free(b.buffer);
        NeAACDecClose(hDecoder);
		return;
    }
	m_nCSBitsPerSample=16;
	m_nCSChannels=channels;
	m_nCSSamplesPerSecond=samplerate;
	m_nBitRate=bitrate;
	//m_nObjectType=hDecoder->n+1;
	info.bitrate=bitrate*1000;
	info.channels=channels;
	info.object_type=object_type;
	info.sampling_rate=samplerate;
	info.headertype=header_type+1;
	//info.length=(unsigned long)(length*1000000);
	info.length=(unsigned long)(length*1000);
	info.bVariableBitrate=false;
	if (bread < 0)
	{
        NeAACDecClose(hDecoder);
		return;
	}

	TRACEIT2("reading first buffer\n");
	AdvanceBuffer(&b, bread);
	nFillBuffer(&b);

    /* print AAC file info */
    switch (header_type)
    {
		case 0:
			Log2(verbLevDebug1,"RAW\n\n");
        break;
		case 1:
			Log2(verbLevDebug1,"ADTS, %.3f sec, %d kbps, %d Hz\n\n",length, bitrate, samplerate);
        break;
		case 2:
			Log2(verbLevDebug1,"ADIF, %.3f sec, %d kbps, %d Hz\n\n",length, bitrate, samplerate);
        break;
    }
	TRACEIT2("sampling_rate: %d\n", m_nCSSamplesPerSecond);
	Log2(verbLevDebug1,"bitrate: %d\n", bitrate);
	Log2(verbLevDebug1,"length: %.3f\n", length);
	uint32_t nTotalSamples=m_nFileSize;
	uint32_t nSampleToDo=nTotalSamples;

	Log2(verbLevDebug1,"decoding aac: ..");
    do
    {
        sample_buffer = NeAACDecDecode(hDecoder, &frameInfo, b.buffer, b.bytes_into_buffer);
		nSampleToDo-=b.bytes_consumed;
		/* update buffer indices */
        AdvanceBuffer(&b, frameInfo.bytesconsumed);
        if (frameInfo.error > 0)
		{
			Log2(verbLevErrors,"%s\n",NeAACDecGetErrorMessage(frameInfo.error));
			throw new CFormatException(CFormatException::formaterrInvalid,(char *)NeAACDecGetErrorMessage(frameInfo.error));
		}
		else
			info.object_type=frameInfo.object_type+1;

   		if (nSampleToDo > 0)
			Log2(verbLevDebug1,"%3d \b\b\b\b",100-((nSampleToDo*100)/nTotalSamples));
	    if ((frameInfo.error == 0) && (frameInfo.samples > 0))
			bc.CreateCopyPacket((unsigned char *)sample_buffer,frameInfo.samples*(m_nCSBitsPerSample>>3));
        /* fill input buffer */
        nFillBuffer(&b);
        if (b.bytes_into_buffer == 0)
            sample_buffer = NULL; /* to make sure it stops now */
    } while (sample_buffer != NULL);

	TRACEIT("object type: %s\n", sGetFormatName(info.object_type).c_str());
	Log2(verbLevDebug1,"object type: %s\n", sGetFormatName(info.object_type).c_str());

    NeAACDecClose(hDecoder);

    if (b.buffer)
        free(b.buffer);

	if ((m_pcCSBuffer=calloc(bc.nGetSize(),1)) != NULL)
		m_nCSSize=bc.nCopyLinear((unsigned char *)m_pcCSBuffer,bc.nGetSize());
}

/*\
 * <---------- CAACFile :: ReadADIFHeader ----------> 
 * @m 
 * --> I N <-- @p
 * istream &ar - 
 * <-- OUT --> @r
 * 
\*/
int CAACFile::ReadADIFHeader(istream &ar)
{
    int bitstream;
    unsigned char buffer[ADIF_MAX_SIZE];
    int skip_size = 0;
    int sf_idx;

    //Get ADIF header data 
    info.headertype = 1;

	ar.read((char *)buffer,(unsigned int)ADIF_MAX_SIZE);

	// copyright string 
    if(buffer[0] & 0x80)
        skip_size += 9; // skip 9 bytes 

    bitstream = buffer[0 + skip_size] & 0x10;
    info.bitrate = ((unsigned int)(buffer[0 + skip_size] & 0x0F)<<19) |
					((unsigned int)buffer[1 + skip_size]<<11) |
					((unsigned int)buffer[2 + skip_size]<<3) |
					((unsigned int)buffer[3 + skip_size] & 0xE0);

    if (bitstream == 0)
    {
        info.object_type =  ((buffer[6 + skip_size]&0x01)<<1)|((buffer[7 + skip_size]&0x80)>>7);
        sf_idx = (buffer[7 + skip_size]&0x78)>>3;
		info.bVariableBitrate=false;
    } 
	else 
	{
        info.object_type = (buffer[4 + skip_size] & 0x18)>>3;
        sf_idx = ((buffer[4 + skip_size] & 0x07)<<1)|((buffer[5 + skip_size] & 0x80)>>7);
		info.bVariableBitrate=true;
    }
	info.sampling_rate = m_cnSampleRates[sf_idx];

    return 0;
}

/*\
 * <---------- ReadADTSHeader ---------->
 * @m 
 * --> I N <-- @p
 * istream &ar - 
 * uint32_t**seek_table - 
 * int *seek_table_len - 
 * int tagsize - 
 * int no_seek_table - 
 * <-- OUT --> @r
 * int - 
\*/
int CAACFile::ReadADTSHeader(istream &ar,uint32_t**seek_table, int *seek_table_len,int tagsize, int no_seek_table)
{
    // Get ADTS header data 
    unsigned char buffer[ADTS_MAX_SIZE];
    int frames, framesinsec=0, t_framelength = 0, frame_length, sr_idx, ID;
    int second = 0, pos;
    float frames_per_sec = 0;
    uint32_t bytes;
	uint32_t *tmp_seek_table = NULL;

    info.headertype = 2;
	try
	{
		// Read all frames to ensure correct time and bitrate 
		for(frames=0; ; frames++, framesinsec++)
		{
			// If streaming, only go until we hit 5 seconds worth 
			pos = ar.tellg();

			// 12 bit SYNCWORD 
			bytes=ADTS_MAX_SIZE;
			ar.read((char *)buffer, bytes);

			// check syncword 
			if (!((buffer[0] == 0xFF)&&((buffer[1] & 0xF6) == 0xF0)))
				break;

			if(!frames)
			{
				// fixed ADTS header is the same for every frame, so we read it only once 
				// Syncword found, proceed to read in the fixed ADTS header 
				ID = buffer[1] & 0x08;
				info.object_type = (buffer[2]&0xC0)>>6;
				sr_idx = (buffer[2]&0x3C)>>2;
				info.channels = ((buffer[2]&0x01)<<2)|((buffer[3]&0xC0)>>6);
				frames_per_sec = m_cnSampleRates[sr_idx] / 1024.f;
				TRACEIT2("frames_per_sec: %f\n",frames_per_sec);
			}
			// ...and the variable ADTS header 
			if (ID == 0) 
				info.version = 4;
			else
				info.version = 2;

			frame_length = ((((unsigned int)buffer[3] & 0x3)) << 11) | (((unsigned int)buffer[4]) << 3) | (buffer[5] >> 5);
			t_framelength += frame_length;

			if(framesinsec == 43)
				framesinsec = 0;

			if(framesinsec == 0 && seek_table_len)
			{
				tmp_seek_table = (uint32_t*) realloc(tmp_seek_table, (second + 1) * sizeof(unsigned long));
				tmp_seek_table[second] = pos;
			}
			if(framesinsec == 0)
				second++;

			ar.seekg(frame_length - ADTS_MAX_SIZE,ios_base::cur);
		}

		if(seek_table_len)
		{
			*seek_table_len = second;
			*seek_table = tmp_seek_table;
		}
	}
	catch(istream::failure const &e)
	{
		Log2(verbLevErrors,"read error on bit-stream adts header (%s)\n",e.what());
		TRACEIT2("catching exception in read\n");
	}
    info.sampling_rate = m_cnSampleRates[sr_idx];
	info.bitrate = (int)(((t_framelength / frames) * (info.sampling_rate/1024.0)) +0.5)*8;
	info.length = (int)((float)(frames/frames_per_sec))*1000;

    return 0;
}

/*\
 * <---------- GetAACFormat ---------->
 * @m 
 * --> I N <-- @p
 * istream &ar - 
 * uint32_t**seek_table - 
 * int *seek_table_len - 
 * int no_seek_table - 
 * <-- OUT --> @r
 * int - 
\*/
int CAACFile::GetAACFormat(istream &ar,uint32_t**seek_table, int *seek_table_len, int no_seek_table)
{
    memset(&info, 0, sizeof(faadAACInfo));
    info.length = 0;
	ar.seekg(m_nTagSize,ios_base::beg);

	if(m_bADIF)
	{
		ar.seekg(4,ios_base::cur);
		ReadADIFHeader(ar);
	}
	else
        ReadADTSHeader(ar, seek_table, seek_table_len, m_nTagSize, no_seek_table);
    return 0;
}

/*\
 * <---------- CAACFile :: bMagicHead ----------> 
 * @m identify an AAC file as such
 * NOTE: does not work with raw AAC files
 * --> I N <-- @p
 * std::istream &ar    - input stream object reference
 * uint32_tnSize - size of the input stream in bytes
 * <-- OUT --> @r
 * bool -  true=ok, false=failed
\*/
bool CAACFile::bMagicHead(std::istream &ar,uint32_t nSize)
{
	bool bRet=false;
	unsigned char buffer[16];
	int nOffset=0;
	m_nFileSize=nSize;
	try 
	{
		nOffset=0;
		m_nTagSize = 0;
		ar.read((char *)buffer,16);
		//is it starting with an ID3 tag?
		if (memcmp(buffer+nOffset, "ID3", 3) == 0)
		{	//yes->skip it...
			//high bit is not used 
			m_nTagSize = (buffer[6] << 21) | (buffer[7] << 14) | (buffer[8] <<  7) | (buffer[9] <<  0);
			m_nTagSize += 10;
			ar.seekg(m_nTagSize,ios_base::beg);
			ar.read((char *)buffer,4);
		}
		// Determine the header type of the file, check the first two bytes 
		if(memcmp(buffer+nOffset,"AD", 2) == 0)
		{
			// We think its an ADIF header, but check the rest just to make sure 
			if(memcmp(buffer+nOffset, "ADIF", 4) == 0)
			{
				bRet=true;
				m_bADIF=true;
				TRACEIT2("seems to be an ADIF encoded AAC\n");
			}
		}
		else
		{
			// No ADIF, check for ADTS header
			if (buffer[nOffset] == 0xFF)
			{
				if ((buffer[nOffset+1] & 0xF6) == 0xF0)
				{
					bRet=true;
					m_bADIF=false;
					TRACEIT2("its an ADTS encoded AAC\n");
				}

			}
		}
	}	
	catch(istream::failure const &e)
	{
		Log2(verbLevErrors,"read error on bit-stream magic head (%s)\n",e.what());
		TRACEIT2("catching exception\n");
		bRet=false;
	}
	TRACEIT2("finished\n");
	return bRet;
}

/*\
 * <---------- CAACFile :: Write ----------> 
 * @m encode and write aac file
 * --> I N <-- @p
 * ostream &out - output stream
\*/
void CAACFile::Write(ostream &out)
{
	bool bRet=false;
	CBufferCollector bc;

	RenderAAC(&bc,m_pCSSource);

	for (unsigned int i=0;i < bc.nGetCount();i++)
		out.write((const char *)bc.pbeGetPacket(i)->pGetData(),bc.pbeGetPacket(i)->nGetSize());

	//write ID3 V1 tag and data..
	if (m_pParameters->m_bSwitch[paraBoolAllowID3])
		RenderID3V1(out,m_pCSSource);
	m_nFileSize=out.tellp();
}
