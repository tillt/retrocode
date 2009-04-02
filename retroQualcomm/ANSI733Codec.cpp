/*\
 * CANSI733Codec.cpp
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
/*\
 * Based and making heavy use of the Qualcomm and Lucent Reference Implementation 
 * for QCELP speech/sound compression: Qualcomm ANSI 733 C code 20040315-025
 *
 * This interface class is GPLed whereas the reference implementation
 * is not.
\*/
/**********************************************************************
Each of the companies; Qualcomm, and Lucent (hereinafter 
referred to individually as "Source" or collectively as "Sources") do 
hereby state:

To the extent to which the Source(s) may legally and freely do so, the 
Source(s), upon submission of a Contribution, grant(s) a free, 
irrevocable, non-exclusive, license to the Third Generation Partnership 
Project 2 (3GPP2) and its Organizational Partners: ARIB, CCSA, TIA, TTA, 
and TTC, under the Source's copyright or copyright license rights in the 
Contribution, to, in whole or in part, copy, make derivative works, 
perform, display and distribute the Contribution and derivative works 
thereof consistent with 3GPP2's and each Organizational Partner's 
policies and procedures, with the right to (i) sublicense the foregoing 
rights consistent with 3GPP2's and each Organizational Partner's  policies 
and procedures and (ii) copyright and sell, if applicable) in 3GPP2's name 
or each Organizational Partner's name any 3GPP2 or transposed Publication 
even though this Publication may contain the Contribution or a derivative 
work thereof.  The Contribution shall disclose any known limitations on 
the Source's rights to license as herein provided.

When a Contribution is submitted by the Source(s) to assist the 
formulating groups of 3GPP2 or any of its Organizational Partners, it 
is proposed to the Committee as a basis for discussion and is not to 
be construed as a binding proposal on the Source(s).  The Source(s) 
specifically reserve(s) the right to amend or modify the material 
contained in the Contribution. Nothing contained in the Contribution 
shall, except as herein expressly provided, be construed as conferring 
by implication, estoppel or otherwise, any license or right under (i) 
any existing or later issuing patent, whether or not the use of 
information in the document necessarily employs an invention of any 
existing or later issued patent, (ii) any copyright, (iii) any 
trademark, or (iv) any other intellectual property right.

With respect to the Software necessary for the practice of any or 
all Normative portions of the QCELP-13 Variable Rate Speech Codec as 
it exists on the date of submittal of this form, should the QCELP-13 be 
approved as a Specification or Report by 3GPP2, or as a transposed 
Standard by any of the 3GPP2's Organizational Partners, the Source(s) 
state(s) that a worldwide license to reproduce, use and distribute the 
Software, the license rights to which are held by the Source(s), will 
be made available to applicants under terms and conditions that are 
reasonable and non-discriminatory, which may include monetary compensation, 
and only to the extent necessary for the practice of any or all of the 
Normative portions of the QCELP-13 or the field of use of practice of the 
QCELP-13 Specification, Report, or Standard.  The statement contained above 
is irrevocable and shall be binding upon the Source(s).  In the event 
the rights of the Source(s) in and to copyright or copyright license 
rights subject to such commitment are assigned or transferred, the 
Source(s) shall notify the assignee or transferee of the existence of 
such commitments.
*******************************************************************/ 
#include "stdafx.h"
#ifndef USE_QUALCOMM_LIBRARY
#ifdef WIN32
#include <tchar.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <memory.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include "../retroBase/Basics.h"
#ifdef __cplusplus
extern "C"{
#endif
#include "qcelp/celp.h"
#include "qcelp/pack.h"
#include "qcelp/tty.h"
#include "qcelp/tty_dbg.h"
#include "qcelp/ops.h"  
#ifdef __cplusplus
}
#endif
#include "ANSI733Codec.h"


CANSI733Codec::CANSI733Codec(void)
{
	m_pEncoder_memory=NULL;
}

CANSI733Codec::~CANSI733Codec(void)
{
}

void CANSI733Codec::Deinit(void)
{
	if (m_pEncoder_memory == NULL)
		return;
	tty_deinit_settings(m_pTTY);
	free_encoder_and_decoder(m_pEncoder_memory, m_pDecoder_memory);
	delete m_pEncoder_memory;
	delete m_pDecoder_memory;
	delete m_pPacket;
	delete m_pControl;
	m_pEncoder_memory=NULL;
}

void CANSI733Codec::Init(bool bModeEncode,bool bVarRate)
{
	m_pEncoder_memory=new ENCODER_MEM;
	m_pDecoder_memory=new DECODER_MEM;
	m_pPacket=new PACKET;
	m_pControl=new CONTROL;
	ZeroMemory(m_pEncoder_memory, sizeof(ENCODER_MEM));
	ZeroMemory(m_pDecoder_memory, sizeof(DECODER_MEM));
	ZeroMemory(m_pPacket, sizeof(PACKET));
	ZeroMemory(m_pControl, sizeof(CONTROL));

	m_pControl->min_rate  = bVarRate ? EIGHTH : FULLRATE_VOICED;
	m_pControl->max_rate  = FULLRATE_VOICED;
	m_pControl->avg_rate  = 9.0;
	m_pControl->target_snr_thr  =10.0;
	m_pControl->pf_flag   =PF_ON;
	m_pControl->cb_out    =NO;
	m_pControl->pitch_out =NO;
	m_pControl->num_frames=UNLIMITED;
	m_pControl->print_packets=NO;
	m_pControl->output_encoder_speech=NO;
	m_pControl->form_res_out=NO;
	m_pControl->reduced_rate_flag=YES;
	m_pControl->unvoiced_off=NO;
	m_pControl->pitch_post   =YES;
	m_pControl->per_wght = (float)PERCEPT_WGHT_FACTOR;
	m_pControl->target_after_out=NO;
	m_pControl->fractional_pitch=YES;
	m_pControl->decode_only = bModeEncode ? NO : YES;
	m_pControl->encode_only = bModeEncode ? YES : NO;
	m_pControl->qcp_compatible=YES;
	m_pTTY=tty_init_settings();
	initialize_encoder_and_decoder(m_pEncoder_memory, m_pDecoder_memory, m_pControl);
}

unsigned long CANSI733Codec::nEncode(unsigned char *pDest,uint32_t *pnDestSize,short *pSource,unsigned long nSize)
{
	int retval;
	unsigned int i;
	unsigned long frame_num;
	unsigned long total_frames;
	unsigned long nSamples;

	unsigned long nProcessed=0;
	unsigned long nCount=0;
	unsigned long nLeft=0;
    short tmpbuf[FSIZE];

	float in_speech[FSIZE+LPCSIZE-FSIZE+LPCOFFSET];
    float out_speech[FSIZE];

	for(i = 0; i < LPCORDER; i++)
	{
		m_pPacket->lsp[i] = 0.0;
		m_pPacket->lpc[i] = 0;
	}
	m_pEncoder_memory->frame_num = 0;
	frame_num = 0;
	nSamples=nSize>>1;
	nLeft=nSamples;
	m_pControl->num_frames = total_frames = (nSamples + FSIZE-1) / FSIZE;
	*pnDestSize=0;

	for( i=0 ; i < LPCSIZE-FSIZE+LPCOFFSET ; i++ )
		in_speech[i] = 0;

	while( m_pControl->num_frames == UNLIMITED || frame_num < (uint32_t)m_pControl->num_frames )
	{
		nCount=min(FSIZE,nLeft);
		memcpy(tmpbuf,pSource,nCount*sizeof(short));
		pSource+=nCount;
		if(nCount < FSIZE )
		{
			if( nSamples < FSIZE )
			{
				//If at beginning of file, zero pad beginning of array
				for( i=0;i < FSIZE-nCount;i++ )
					tmpbuf[i] = 0;
			}
			else
			{
				//If at the end of the file, zero pad the end of array
				for(i=nCount;i < FSIZE;i++ )
					tmpbuf[i] = 0;
			}
		}
		for(i=0;(unsigned int)i < nCount;i++)
			in_speech[LPCSIZE-FSIZE+LPCOFFSET+i]=(float)(tmpbuf[FSIZE-nCount+i])/(float)4.0;
		nLeft-=nCount;
		if( nCount <= 0 )
			break;
		else if(nCount < FSIZE)
		{
			for (i=nCount;i < FSIZE;i++)
				in_speech[LPCSIZE-FSIZE+LPCOFFSET+i]=0;
		}
		retval=encoder(m_pTTY,in_speech, m_pPacket, m_pControl,m_pEncoder_memory, out_speech);
		if (retval != QCELPERR_OK)
		{
			Log2(verbLevErrors,"ANSI-733 encoding error: %02X\n",retval);
			return frame_num;
		}
		update_snr(ENCODER, in_speech, out_speech, &(m_pControl->snr));
		Log2(verbLevDebug3,"mode: %02X\n",m_pPacket->mode);
		memcpy(pDest,m_pPacket->data,NUMBYTES(m_pPacket->mode));
		pDest+=NUMBYTES(m_pPacket->mode);
		*pnDestSize+=NUMBYTES(m_pPacket->mode);
		for (i=0; i<LPCSIZE-FSIZE+LPCOFFSET; i++)
			in_speech[i]=in_speech[i+FSIZE];
		frame_num++;
		m_pEncoder_memory->frame_num = frame_num;
	};
	Log2(verbLevDebug3,"encoded: %d packets\n",frame_num);
	return frame_num;
}

unsigned long CANSI733Codec::nDecode(short *pDest,unsigned char *pSource,unsigned long nPackets,unsigned long nSize)
{
	int   i,retval;
	unsigned long frame_num;
	unsigned long nLeft;
	unsigned long nCount;
	short *pStart=pDest;

    float   out_speech[FSIZE];

	for(i = 0; i < LPCORDER; i++)
	{
		m_pPacket->lsp[i] = 0.0;
		m_pPacket->lpc[i] = 0;
	}

	m_pEncoder_memory->frame_num = 0;
	frame_num = 0;
	nLeft=nSize;
	
	Log2(verbLevDebug3,"decoding: %d packets\n",nPackets);
	while(nPackets)
	{
		m_pPacket->mode=*pSource;
		Log2(verbLevDebug3,"mode: %02X\n",m_pPacket->mode);
		if (m_pPacket->mode >= 0 && m_pPacket->mode <= 4)
		{
			nCount=NUMBYTES(m_pPacket->mode);
			if(nLeft < nCount)
			{
				Log2(verbLevErrors,"packet truncated\n");
				break;
			}
			memcpy(m_pPacket->data,pSource,nCount);
			pSource+=nCount;
			nLeft-=nCount;
			retval=decoder(m_pTTY,out_speech, m_pPacket, m_pControl, m_pDecoder_memory);
			if (retval != QCELPERR_OK)
			{
				Log2(verbLevErrors,"ANSI-733 decoding error: %02X\n",retval);
				return (unsigned long)(pDest-pStart);
			}
			float ftmp;
			for(i=0;i < FSIZE; i++)
			{
				ftmp=(float)out_speech[i] * (float)4.0;
				ftmp=(ftmp < (float)-32768.0)  ? (float)-32768.0 : ftmp;
				ftmp=(ftmp > (float)32767.0)  ?  (float)32767.0 : ftmp;
				*pDest = (short)ftmp;
				++pDest;	
			}
			frame_num++;
			m_pEncoder_memory->frame_num = frame_num;
		}
		else
		{
			//skipped illegal packet
			Log2(verbLevErrors,"packet mode illegal: %02X (nLeft: %d)\n",m_pPacket->mode,nLeft);
			break;
		}
		--nPackets;
	};
	return (unsigned long)(pDest-pStart);
}
#endif

