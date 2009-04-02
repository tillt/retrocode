/*\
 * AMRFile.cpp
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
#include <iostream>
#include <strstream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h> 

#ifdef __cplusplus
extern "C"{
#endif 

#include "amrwb/typedef.h"
#include "amrwb/dec_if.h"
#include "amrwb/enc_if.h"

//float is the best!
#ifdef USEAMR_FLOAT
#include "amrnb/typedef.h"
#include "amrnb/interf_enc.h"
#include "amrnb/interf_dec.h"
#else
#include "amrnbint/typedef.h"
#include "amrnbint/cnst.h"
#include "amrnbint/mode.h"
#include "amrnbint/frame.h"
#include "amrnbint/sp_enc.h"
#include "amrnbint/sp_dec.h"
#include "amrnbint/sid_sync.h"
#include "amrnbint/e_homing.h"
#include "amrnbint/n_proc.h"
#include "amrnbint/strfunc.h"
#include "amrnbint/pre_proc.h"
#include "amrnbint/vadname.h"
#endif

#ifdef __cplusplus
}
#endif

#include "../include/Resource.h"
#include "../retroBase/Basics.h"
#include "../retroBase/Integer.h"
#include "../retroBase/BitStream.h"
#include "../retroBase/PacketCollection.h"
#include "../retroBase/MobileContent.h"
#include "AMRFile.h"
#include "AMRProperty.h"

DYNIMPPROPERTY(CAMRFile,CAMRProperty)
DYNIMPPROPERTY(CAWBFile,CAMRProperty)

CAWBFile::CAWBFile(void)
{
	TRACEIT2("AWB Constructor\n");
	m_pcMagic="#!AMR-WB";
	m_nMagicSize=8;
	m_sFormatName=_T("AMRWB");
	m_nSubFormat=subFormatWB;
	m_sFormatDescription.Load(IDS_FORMDESC_AMRWB);
	m_sFormatCredits=_T("The AMR-WB codec is entirely based on: \"3GPP specification TS 26.201: Speech Codec speech processing functions; AMR Wideband Speech Codec; Frame Structure\", Copyright (c) 2003 by 3GPP; \"TS 26.204: 3GPP AMR Wideband Floating-point Speech Codec\"), Copyright (c) 2003 by 3GPP." );
	m_sDefaultExtension=_T("awb");
	//m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono|CSampleCompatibility::supportsStereo].addDigit(16000);
	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono].addDigit(16000);
	//m_encodingPara.addPara(cmdParaNumber,paraNumBitrate);
	//m_encodingPara.addPara(cmdParaBool,paraBoolAllowDTX);

}

CAWBFile::~CAWBFile(void)
{
	TRACEIT2("AWB destructor\n");
}

CAMRFile::CAMRFile(void)
{
	TRACEIT2("AMR constructor\n");
	m_pcMagic="#!AMR\012";
	m_nMagicSize=6;
	m_sFormatName="AMR";
	m_nSubFormat=subFormatNB;
	m_sDefaultExtension=_T("amr");
	m_sFormatDescription.Load(IDS_FORMDESC_AMRNB);
	m_sFormatCredits=_T("The AMR-NB codec is entirely based on: \"3GPP specification TS 26.101: Mandatory speech processing functions; AMR speech codec frame structure\", Copyright (c) 2003 by 3GPP; \"TS 26.104: 3GPP AMR Floating-point Speech Codec V5.1.0\"), Copyright (c) 2003 by 3GPP." );
	//m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono|CSampleCompatibility::supportsStereo].addDigit(8000);
	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono].addDigit(8000);
	m_encodingPara.addPara(cmdParaNumber,paraNumBitrate);
	m_encodingPara.addPara(cmdParaBool,paraBoolAllowDTX);
}

CAMRFile::~CAMRFile(void)
{
	TRACEIT2("AMR destructor\n");
}

void CAMRFile::Read(istream &ar)
{
	const int8_t block_size[16]= {12, 13, 15, 17, 19, 20, 26, 31, 5, 0, 0, 0, 0, 0, 0, 0};
	unsigned char theByte;
	unsigned char cBuffer[4];
	uint8_t serial[NB_SERIAL_MAX];
	CBufferCollector bc;

	ar.seekg(0,ios_base::beg);

	map<string,int> mapFormat;

	mapFormat["#!AMR\012"]=subFormatNB;
	mapFormat["#!AMR-WB\012"]=subFormatWB;
	mapFormat["#!AMR_MC1.0\012"]=subFormatMC;
	mapFormat["#!AMR-WB_MC1.0\012"]=subFormatMCWB;

	ar.seekg(5,ios_base::beg);
	ar.read((char *)&theByte,1);
	switch (theByte)
	{
		case '\012':	
			m_nSubFormat=subFormatNB;	
		break;
		case '_':
			ar.read((char *)cBuffer,3);
			if(!memcmp(cBuffer,"MC1.0\012",6))
				m_nSubFormat=subFormatMC;
		break;
		case '-':
			ar.read((char *)cBuffer,3);
			if(!memcmp(cBuffer,"WB\012",3))
				m_nSubFormat=subFormatWB;
			else
			{
				if(!memcmp(cBuffer,"WB_MC1.0\012",9))
					m_nSubFormat=subFormatMCWB;
			}
		break;
	}

	void *st;
	short synth[160];

	TRACEIT2("init decoder\n");
#ifdef USEAMR_FLOAT
	if ((st = Decoder_Interface_init()) != NULL)
#else
	char id;
	if (Speech_Decode_Frame_init(&st,&id))
#endif
	{
		TRACEIT2("processing sample...\n");
		bool bDone=false;
		int16_t mode;
		int frame=0;
		while (!bDone)
		{
			try
			{
				ar.read((char *)serial,sizeof (uint8_t));
				mode = (int16_t)((serial[0] >> 3) & 0x0F);

				ar.read((char *)&serial[1], block_size[mode] );
				frame++;

				Log2(verbLevDebug1,"Decoding AMR frame: %ld\n",frame);

#ifdef USEAMR_FLOAT
				Decoder_Interface_Decode(st, serial, synth, 0);
#else
				UnpackBits(st,mode,serial,frame_type,synth);
#endif
				bc.CreateCopyPacket((unsigned char *)synth,320);
			}
			catch(istream::failure const &e)
			{
				Log2(verbLevDebug1,"read error on bit-stream (%s)\n",e.what());
				bDone=true;
			}
		}
		TRACEIT2("decoder deinit\n");
		Decoder_Interface_exit(st);

		Log2(verbLevDebug1,"\n");
		if ((m_pcCSBuffer=CMobileSampleContent::Alloc(bc.nGetSize())) != NULL)
		{
			m_nCSSize=bc.nCopyLinear((unsigned char *)m_pcCSBuffer,bc.nGetSize());
			m_nCSBitsPerSample=16;
			m_nCSChannels=1;
			m_nCSSamplesPerSecond=8000;
		}
	}
}

uint32_t CAMRFile::nGetBitRate(uint32_t nMode)
{
	return nGetBitRate(m_nSubFormat,nMode);
}

uint32_t CAMRFile::nGetBitRate(uint32_t nSubFormat,uint32_t nMode)
{
	uint32_t nRate[2][9]=
	{
		{
			4750,
			5150,
			5900,
			6700,
			7400,
			7950,
			10020,
			12200
		},
		{
			6600,		//00000100
			8850,		//00001100
			12650,		//00010100
			14250,		//00011100 
			15850,		//00100100
			18250,		//00101100
			19850,		//00110100
			23050,		//00111100
			23850		//01000100
		}
	};
	ASSERT(nSubFormat);
	return nRate[(nSubFormat-1) & 0x01][nMode&0x0F];
}

uint32_t CAMRFile::nGetMode(uint32_t nSubFormat,uint32_t nBitRate)
{
	uint32_t nMode=(uint32_t)-1;
	map<uint32_t,uint32_t> :: const_iterator iter;
	map<uint32_t,uint32_t> mapMode;
	if (nSubFormat == subFormatNB)
	{
		mapMode[4750]=0;
		mapMode[5150]=1;
		mapMode[5900]=2;
		mapMode[6700]=3;
		mapMode[7400]=4;
		mapMode[7950]=5;
		mapMode[10020]=6;
		mapMode[12200]=7;
	}
	if (nSubFormat == subFormatWB)
	{
		mapMode[6600]=0;
		mapMode[8850]=1;
		mapMode[12650]=2;
		mapMode[14250]=3;
		mapMode[15850]=4;
		mapMode[18250]=5;
		mapMode[19850]=6;
		mapMode[23050]=7;
		mapMode[23850]=8;
	}
	if ((iter=mapMode.find(nBitRate)) == mapMode.end())
		nMode=-1;
	else
		nMode=iter->second;
	return nMode;
}

uint32_t CAMRFile::nGetSamplesPerSecond(void)
{
	ASSERT(m_nSubFormat);
	if ((m_nSubFormat-1) &  0x01)
		return 16000;
	else
		return 8000;
}

tstring CAMRFile::sGetFormatName(uint32_t nFormat)
{
	CMyString strName="unknown";
	int nFormatName[]=
	{
		IDS_FORMATNAME_AMR_NB,
		IDS_FORMATNAME_AMR_WB
	};
	ASSERT(nFormat > 0);
	if (nFormat > 0)
		strName.LoadString(nFormatName[(nFormat-1)&0x01]);
	return strName;
}

/*\
 * <---------- RenderDestination ---------->
 * @m 
 * --> I N <-- @p
 * ostream &out - 
 * CMobileSampleContent *pSource - 
\*/
void CAMRFile::Write(ostream &out)
{
	//Log("bits per sample: %d, bitrate: %dbps, sample rate: %dHz, channels: %d\n",m_pCSSource->m_nCSBitsPerSample,m_pParameters->m_nParameter[paraNumBitrate],m_pCSSource->m_nCSSamplesPerSecond,m_pCSSource->m_nCSChannels);
	if (m_pParameters->m_nParameter[paraNumBitrate] == 0)
	{
		m_pParameters->m_nParameter[paraNumBitrate]=12200;
		Log2(verbLevWarnings,"bitrate not set, using highest possible: %d\n",m_pParameters->m_nParameter[paraNumBitrate]);
	}
	if (m_pCSSource->m_nCSChannels != 1)
	{
		Log2(verbLevErrors,"channel count incompatible\n");
		throw new CFormatException(CFormatException::formaterrSource,"channel count incompatible");
	}
	if (m_pCSSource->m_nCSBitsPerSample != 16)
	{
		Log2(verbLevErrors,"sample width incompatible\n");
		throw new CFormatException(CFormatException::formaterrSource,"sample width incompatible");
	}
	if (m_pCSSource->m_nCSSamplesPerSecond != 8000)
	{
		Log2(verbLevErrors,"sample rate incompatible\n");
		throw new CFormatException(CFormatException::formaterrSource,"sample rate incompatible");
	}
	int32_t nMode=(int32_t)nGetMode(subFormatNB,m_pParameters->m_nParameter[paraNumBitrate]);
	if (nMode < 0)
	{
		Log2(verbLevErrors,"bitrate incompatible\n");
		throw new CFormatException(CFormatException::formaterrSource,"bitrate incompatible");
	}
	signed short int *pcIn=(signed short int *)m_pCSSource->m_pcCSBuffer;
	uint32_t nSamplesLeft=m_pCSSource->m_nCSSize/2;
	uint32_t nSerialSize=0;
	unsigned int serial_size;
	void *st;
	
	out.write(_T("#!AMR\012"), 6);
#ifdef USEAMR_FLOAT
	uint8_t serial[62];
	Log2(verbLevDebug1,"+");
	st = Encoder_Interface_init(m_pParameters->m_bParameter[paraBoolAllowDTX] ? 1 : 0);
	while (nSamplesLeft >= 160)
	{
		Log2(verbLevDebug1,"\b#");
		serial_size=Encoder_Interface_Encode(st,(enum Mode)nMode,(short *) pcIn, serial, 0);
		//fwrite(serial_data, sizeof (uint8), serial_size, file_encoded );
		out.write((char *)serial,serial_size);
		nSerialSize+=serial_size;
		Log2(verbLevDebug1,"\b*");
		nSamplesLeft-=160;
		pcIn+=160;
	};
	Log2(verbLevDebug1,"\n");
	Encoder_Interface_exit(st);
#else
	sid_syncState *sid_state = NULL; 
	int i;
	enum Mode used_mode; 
	enum TXFrameType tx_type; 
	#define MAX_PACKED_SIZE (MAX_SERIAL_SIZE / 8 + 2)
	#define SERIAL_FRAMESIZE (1+MAX_SERIAL_SIZE+5) 
	uint8_t packed_bits[MAX_PACKED_SIZE];
	int16_t packed_size;
	int16_t new_speech[L_FRAME];         /* Pointer to new speech data        */
	int16_t serial[SERIAL_FRAMESIZE];    /* Output bitstream buffer           */

	int16_t  reset_flag; 

	Speech_Encode_FrameState *speech_encoder_state = NULL; 
	/*-----------------------------------------------------------------------*
	* Initialisation of the coder.                                          *
	*-----------------------------------------------------------------------*/
	if (   Speech_Encode_Frame_init(&speech_encoder_state, m_pParameters->m_bParameter[paraBoolAllowDTX] ? 1 : 0, "encoder") || sid_sync_init (&sid_state))
	{
		Log2(verbLevErrors,"failed to init encoding for this frame\n");
		exit(-1);
	}

	/*-----------------------------------------------------------------------*
	* Process speech frame by frame                                         *
	*-----------------------------------------------------------------------*/
	frame = 0;
	//while (fread (new_speech, sizeof (int16), L_FRAME, file_speech) == L_FRAME)

	Log2(verbLevMessages,"compressing sample data...\n");
	Log2(verbLevDebug2,"progress: ");

	while (nSamplesLeft >= L_FRAME)
	{
		/* read new mode string from file if required */
		frame++;
		    
		/* zero flags and parameter bits */
		for (i = 0; i < SERIAL_FRAMESIZE; i++)
			serial[i] = 0;

		/* check for homing frame */
		reset_flag = encoder_homing_frame_test(pcIn);
		    
		/* encode speech */
		Speech_Encode_Frame(speech_encoder_state,(enum Mode) nMode, pcIn, &serial[1], &used_mode); 

		/* include frame type and mode information in serial bitstream */
		sid_sync (sid_state, used_mode, &tx_type);

		packed_size = PackBits(used_mode,(enum Mode)nMode, tx_type, &serial[1], packed_bits);

		/* write file storage format bitstream to output file */
		out.write((char *)packed_bits,packed_size);
		nSerialSize+=packed_size;
		Log2(verbLevDebug2,"*");

		nSamplesLeft-=160;
		pcIn+=160;

		/* perform homing if homing frame was detected at encoder input */
		if (reset_flag != 0)
		{
			Speech_Encode_Frame_reset(speech_encoder_state);
			sid_sync_reset(sid_state);
		}
	}
	Log2(verbLevDebug2,"\n%d frame(s) processed\n", frame);
		
	/*-----------------------------------------------------------------------*
	* Close down speech coder                                               *
	*-----------------------------------------------------------------------*/
	Speech_Encode_Frame_exit(&speech_encoder_state);
	sid_sync_exit (&sid_state);
#endif
	m_nCSChannels=m_pCSSource->m_nCSChannels;
	m_nCSSamplesPerSecond=m_pCSSource->m_nCSSamplesPerSecond;
	m_nCSBitsPerSample=0;
	m_nCSSize=nSerialSize;
	m_nFileSize=out.tellp();
}

/*\
 * <---------- RenderDestination ---------->
 * @m 
 * --> I N <-- @p
 * ostream &out - 
 * CMobileSampleContent *pSource - 
\*/
void CAWBFile::Write(ostream &out)
{
	//Log("bits per sample: %d, bitrate: %dbps, sample rate: %dHz, channels: %d\n",m_pCSSource->m_nCSBitsPerSample,m_pParameters->m_nParameter[paraNumBitrate],m_pCSSource->m_nCSSamplesPerSecond,m_pCSSource->m_nCSChannels);

	ASSERT(m_pParameters);
	ASSERT(m_pCSSource);
	if (m_pParameters->m_nParameter[paraNumBitrate] == 0)
	{
		m_pParameters->m_nParameter[paraNumBitrate]=23850;
		Log2(verbLevWarnings,"bitrate not set, using highest possible: %d\n",m_pParameters->m_nParameter[paraNumBitrate]);
	}
	if (m_pCSSource->m_nCSChannels != 1)
	{
		Log2(verbLevErrors,"channel count incompatible\n");
		throw new CFormatException(CFormatException::formaterrSource,"channel count incompatible");
	}
	if (m_pCSSource->m_nCSBitsPerSample != 16)
	{
		Log2(verbLevErrors,"sample width incompatible\n");
		throw new CFormatException(CFormatException::formaterrSource,"sample width incompatible");
	}
	if (m_pCSSource->m_nCSSamplesPerSecond != 16000)
	{
		Log2(verbLevErrors,"sample rate incompatible\n");
		throw new CFormatException(CFormatException::formaterrSource,"sample rate incompatible");
	}
	int nMode=nGetMode(subFormatWB,m_pParameters->m_nParameter[paraNumBitrate]);
	if (nMode < 0)
	{
		Log2(verbLevErrors,"bitrate incompatible\n");
		throw new CFormatException(CFormatException::formaterrSource,"bitrate incompatible");
	}
	signed short int *pcIn=(signed short int *)m_pCSSource->m_pcCSBuffer;
	uint32_t nSamplesLeft=m_pCSSource->m_nCSSize/2;
	int allow_dtx=m_pParameters->m_bParameter[paraBoolAllowDTX] ? 1 : 0;
	unsigned int serial_size;
	void *st;
	uint8_t serial[NB_SERIAL_MAX];
	uint32_t nSerialData=0;

	if ((st = E_IF_init()) != NULL)
	{
		Log2(verbLevDebug1,"compressing sample data...\n");
		Log2(verbLevDebug2,"progress: ");

		out.write(_T("#!AMR-WB\012"), 9);
		while (nSamplesLeft >= 320)
		{
			serial_size = E_IF_encode(st, nMode, (signed short int *)pcIn, (uint8_t *)serial, allow_dtx);

			Log2(verbLevDebug2,"*");
			
			out.write((char *)serial,serial_size);
			nSerialData+=serial_size;
			
			nSamplesLeft-=320;
			pcIn+=320;
		};
		Log2(verbLevDebug2,"\n");
	}
	m_nCSChannels=m_pCSSource->m_nCSChannels;
	m_nCSSamplesPerSecond=m_pCSSource->m_nCSSamplesPerSecond;
	m_nCSBitsPerSample=0;
	m_nCSSize=nSerialData;
	m_nFileSize=out.tellp();
}

/*\
 * <---------- read ---------->
 * @m 
 * --> I N <-- @p
 * std::istream &ar - 
\*/
void CAWBFile::Read(std::istream &ar)
{
	const uint8_t block_size[16]= {18, 24, 33, 37, 41, 47, 51, 59, 61, 6, 6, 0, 0, 0, 1, 1};
	CBufferCollector bc;
	int16_t synth[L_FRAME16k];              // Buffer for speech @ 16kHz  
	uint8_t serial[NB_SERIAL_MAX];
    int16_t mode;
    int32_t frame;
	void *st;

	ar.clear();
	ar.seekg(9,ios_base::beg);

	st = D_IF_init();

    frame = 0;

	bool bDone=false;
    while (!bDone)
    {
		try
		{
			ar.read((char *)serial,sizeof (char));
			mode = (int16_t)((serial[0] >> 3) & 0x0F);
			ar.read((char *)&serial[1], block_size[mode] - 1);
			frame++;

			Log2(verbLevDebug2,"Decoding AWB frame: %ld\n",frame);

			D_IF_decode( st, serial, synth, _good_frame);

			Log2(verbLevDebug3,"Frame decoded, storing package\n");

			bc.CreateCopyPacket((unsigned char *)synth,L_FRAME16k * 2);
		}
		catch(istream::failure const &e)
		{
			Log2(verbLevDebug1,"read error on bit-stream (%s)\n",e.what());
			bDone=true;
		}
	};
	D_IF_exit(st);
	
	m_nCSChannels=1;
	m_nCSSamplesPerSecond=16000;
	m_nCSSize=bc.nGetSize();
	m_nCSBitsPerSample=16;
	if ((m_pcCSBuffer=calloc(m_nCSSize,1)) != NULL)
		bc.nCopyLinear((uint8_t *)m_pcCSBuffer,m_nCSSize);
}
