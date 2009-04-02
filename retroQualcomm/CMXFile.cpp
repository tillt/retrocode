/*\
 * CMXFile.cpp
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
#include <fstream>
#ifndef WIN32
#include <netinet/in.h>
#else
#include <winsock2.h>
#endif
#include "../include/Resource.h"
#include "../retroBase/Basics.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MyString.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/Endian.h"
#include "../retroBase/Packer.h"
#include "../retroBase/Adpcm.h"
#include "QCELPFile.h"
#include "CMXSample.h"
#include "CMXFile.h"
#include "CMXProperty.h"
#include "Version.h"


DYNIMPPROPERTY(CCMXFile,CCMXProperty)

 CCMXFile::CCMXFile(void)
{
	m_nMagicSize=4;
	m_pcMagic="cmid";
	m_sFormatName="Qualcomm Compact Multimedia Format (CMF/PMD)";
	//ZeroMemory(&m_Header,sizeof(m_Header));
	m_sDefaultExtension=_T("pmd");
	m_bContainsPicture=false;
	m_bContainsSample=false;
	m_nTempo=125;
	m_nTimebase=48;
	m_pCurrentSample=NULL;
	m_sFormatDescription.Load(IDS_FORMDESC_CMX);
	m_sFormatCredits=_T("Retro's CMF codec is based on: \"Internet-Draft: draft-atarius-cmf-00.txt\", Copyright (c) 2004 by The Internet Society; \"3GPP2 C.S0050-0 Version 1.0 - 3GPP2 File Formats for Multimedia Services\", Copyright (c) 2003 by 3GGP2.");
	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono].addDigit(8000);
	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono].addDigit(16000);
	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono].addDigit(32000);
	m_encodingPara.addPara(cmdParaNumber,paraNumCompression);
	m_encodingPara.addPara(cmdParaNumber,paraNumVolume);
	m_encodingPara.addPara(cmdParaNumber,paraNumLoopcount);
	m_encodingPara.addPara(cmdParaString,paraStrImageExportPath);
	m_encodingPara.addPara(cmdParaString,paraStrTitle);
	m_encodingPara.addPara(cmdParaString,paraStrCopyright);
	m_encodingPara.addPara(cmdParaString,paraStrPublisher);
	m_encodingPara.addPara(cmdParaString,paraStrDateCreated);
}

CCMXFile::~CCMXFile(void)
{
	int i;
	for (i=0;i < (int)m_Samples.size();i++)
	{
		ASSERT (m_Samples[i]);
		if (m_Samples[i] != NULL)
			delete m_Samples[i];
	}
	if (m_Samples.size())
		m_Samples.erase(m_Samples.begin(), m_Samples.end());
}

/*\
 * <---------- CCMXFile :: Read ----------> 
 * @m 
 * --> I N <-- @p
 * istream &ar - input stream reference
\*/
void CCMXFile::Read(istream &ar)
{
	Log2(verbLevDebug1,"reading header\n");
	try
	{
		ar.seekg(0,ios_base::beg);
		ReadHeader(ar,&m_Header);
	
		//read subchunks
		if (m_nFileSize >= 13 && m_Header.size+8 >= 13)
		{
			unsigned int nSize=min((unsigned long)m_nFileSize,(unsigned long)m_Header.size+8)-13;
			while (nSize)
				nSize-=ReadSubchunk(ar,&m_Header);
		}
		else
		{
			Log2(verbLevErrors,"invalid header size\n");
			throw new CFormatException(CFormatException::formaterrInvalid,"invalid header size");
		}
	}
	catch (istream::failure const &e)
	{
		Log2(verbLevErrors,"File Access Exception\n\t%s\n",e.what());
	}
	TRACEIT2("done\n");
}

/*\ 
 * <---------- CCMXFile::ReadSubchunk ----------> 
 * @m read a 
 * --> I N <-- @p
 * istream &ar - input stream reference
 * CMXHEADER *pCmx - pointer to header
 * <-- OUT --> @r
 * int - number of bytes read
\*/ 
int CCMXFile::ReadSubchunk(istream &ar, CMXHEADER *pCmx)
{
	int nRet=0;

	char sChunk[5],sAttribute[256];
	int nAttribute;
	uint32_t dwSize;
	uint16_t wSize,nRead;
	map<tstring,int> :: const_iterator iterChunk;
	map<tstring,int> mapChunkId;

	Unpacker up(ar, false);

	wSize=0;
	dwSize=0;

	ar.read(sChunk,4);
	sChunk[4]=0;
	nRet+=4;

	mapChunkId[tstring(_T("vers"))]=cidVERS;
	mapChunkId[tstring(_T("code"))]=cidCODE;
	mapChunkId[tstring(_T("titl"))]=cidTITL;
	mapChunkId[tstring(_T("date"))]=cidDATE;
	mapChunkId[tstring(_T("sorc"))]=cidSORC;
	mapChunkId[tstring(_T("copy"))]=cidCOPY;
	mapChunkId[tstring(_T("exsn"))]=cidEXSN;
	mapChunkId[tstring(_T("exsa"))]=cidEXSA;
	mapChunkId[tstring(_T("exsb"))]=cidEXSB;
	mapChunkId[tstring(_T("exsc"))]=cidEXSC;
	mapChunkId[tstring(_T("exst"))]=cidEXST;
	mapChunkId[tstring(_T("pcpi"))]=cidPCPI;
	mapChunkId[tstring(_T("cnts"))]=cidCNTS;
	mapChunkId[tstring(_T("prot"))]=cidPROT;
	mapChunkId[tstring(_T("poly"))]=cidPOLY;
	mapChunkId[tstring(_T("wave"))]=cidWAVE;
	mapChunkId[tstring(_T("tool"))]=cidTOOL;
	mapChunkId[tstring(_T("note"))]=cidNOTE;
	mapChunkId[tstring(_T("trac"))]=cidTRAC;

	Log2(verbLevDebug1,"decoded chunk: %s\n",sChunk);
	if ((iterChunk=mapChunkId.find(sChunk)) == mapChunkId.end())
	{
		tstring str;
		str=tstring(_T("unknown chunk: \""))+tstring(sChunk)+tstring(_T("\""));
		Log2(verbLevErrors,"unknown chunk\n");
		throw new CFormatException(CFormatException::formaterrInvalid,str.c_str());
	}
	if (iterChunk->second == cidTRAC)
	{
		//read data size
		up.read("l",&dwSize);
		wSize=0;
		Log2(verbLevDebug2,"long chunk size: %d\n",dwSize);
		nRet+=4;
		nRet+=dwSize;
	}
	else
	{
		//read data size
		up.read("s",&wSize);
		dwSize=wSize;
		Log2(verbLevDebug2,"short chunk size: %d\n",wSize);
		nRet+=2;
		nRet+=wSize;
	}
	if (dwSize)
	{
		switch(iterChunk->second)
		{
			case cidVERS:
				ar.read(sAttribute,4);
				sAttribute[4]=0;
				sscanf(sAttribute,"%04X",&nAttribute);
				pCmx->wVersion=nAttribute;
				Log2(verbLevDebug1,"format version: %04X\n",pCmx->wVersion);
				TRACEIT2("format version: %04X\n",pCmx->wVersion);
				dwSize-=4;
			break;
			case cidSORC:
				ar.read((char*)&(pCmx->cSourceInfo),1);
				dwSize-=1;
				Log2(verbLevDebug1,"source info: %02X\n",pCmx->cSourceInfo);
				TRACEIT2("source info: %02X\n",pCmx->cSourceInfo);
			break;
			case cidTITL:
				nRead=255;
				if (dwSize < 255)
					nRead=(unsigned short)dwSize;
				ar.read((char*)&pCmx->sTitle,nRead);
				pCmx->sTitle[nRead]=0;		 		
				dwSize-=nRead;
				Log2(verbLevDebug1,"title: %s\n",pCmx->sTitle);
				TRACEIT2("title: %s\n",pCmx->sTitle);
				m_strInfo[infoTitle]=tstring((TCHAR *)pCmx->sTitle);
			break;
			case cidCOPY:
				nRead=255;
				if (dwSize < 255)
					nRead=(unsigned short)dwSize;
				ar.read((char*)&pCmx->sCopyright,nRead);
				pCmx->sCopyright[nRead]=0;
				m_strInfo[infoCopyright]=tstring((TCHAR *)pCmx->sCopyright);
				dwSize-=nRead;
				Log2(verbLevDebug1,"copyright: %s\n",pCmx->sCopyright);
				TRACEIT2("copyright: %s\n",pCmx->sCopyright);
			break;
			case cidPROT:
				nRead=255;
				if (dwSize < 255)
					nRead=(unsigned short)dwSize;
				ar.read((char*)&pCmx->sProvider,nRead);
				pCmx->sProvider[nRead]=0;
				m_strInfo[infoPublisher]=tstring((TCHAR *)pCmx->sProvider);
				dwSize-=nRead;
				Log2(verbLevDebug1,"provider/author/publisher: %s\n",pCmx->sProvider);
				TRACEIT2("provider/author/publisher: %s\n",pCmx->sProvider);
			break;
			case cidDATE:
				ar.read((char*)&pCmx->sDate,8);
				pCmx->sDate[8]=0;
				dwSize-=8;
				Log2(verbLevDebug1,"date: %s\n",pCmx->sDate);
				m_strInfo[infoDateCreated]=tstring((TCHAR *)pCmx->sDate);
			break;
			case cidCODE:
				ar.read((char*)&pCmx->cCharset,1);
				dwSize-=1;
				Log2(verbLevDebug1,"charset: %02X\n",pCmx->cCharset);
			break;
			case cidTOOL:
				ar.read(sAttribute,dwSize);
				sAttribute[dwSize]=0;
				dwSize=0;
				Log2(verbLevDebug1,"tool: %s\n",sAttribute);
			break;
			case cidCNTS:
				if (!bParseCNTS(ar,dwSize,&dwSize))
				{
					Log2(verbLevErrors,"failed parsing CNTS chunk\n");
					throw new CFormatException(CFormatException::formaterrInvalid,"failed parsing CNTS chunk");
				}
			break;
			case cidTRAC:
				Log2(verbLevDebug1,"parse track...\n");
				if (!bParseTRAC(ar,dwSize,&dwSize))
				{
					Log2(verbLevErrors,"failed parsing TRAC chunk\n");
					throw new CFormatException(CFormatException::formaterrInvalid,"failed parsing TRAC chunk");
				}
			break;				
		}
		//skip unwanted data
		if (dwSize)
		{
			Log2(verbLevDebug2,"skipping %d bytes\n",dwSize);
			ar.seekg((int)ar.tellg()+dwSize);
		}
	}
	return nRet;
}

/*\
 * <---------- CCMXFile :: bParseCNTS ----------> 
 * @m parse content tag (CNTS)
 * --> I N <-- @p
 * istream &ar                        - input stream reference
 * uint32_tint wSize            - size of the CNTS tag
 * uint32_tint *pwSizeRemaining - bytes left in input stream
 * <-- OUT --> @r
 * bool - true=ok, false=failed
\*/
bool CCMXFile::bParseCNTS(istream &ar, uint32_t wSize,uint32_t *pwSizeRemaining)
{
	unsigned short int nLen;
	char *pBuffer;
	bool bRet=false;
	*pwSizeRemaining=wSize;

	if (wSize)
	{
		pBuffer=new char [wSize];
		if (pBuffer)
		{
			char *pChunk;
			ar.read(pBuffer,wSize);
			nLen=(unsigned short int)wSize;
			pChunk=pBuffer;
			while(nLen)
			{
				switch(TOFOURCC(*(uint32_t *)pChunk))
				{
					case MAKEFOURCC('W','A','V','E'):
						Log2(verbLevDebug1,"contains sample data\n");
						bRet=true;
						m_bContainsSample=true;
					break;
					case MAKEFOURCC('P','I','C','T'):
						Log2(verbLevDebug1,"contains picture data\n");
						bRet=true;
						m_bContainsPicture=true;
					break;
					default:
						Log2(verbLevWarnings,"unknown cnts subchunk\n");
						return false;
				}
				pChunk+=4;
				nLen-=4;
				if (nLen)
				{
					if(*(pChunk) == ';')
					{
						++pChunk;
						--nLen;
					}
				}
			};
			*pwSizeRemaining=0;
		}
		delete [] pBuffer;
	}
	Log2(verbLevDebug3,"returning %d\n",bRet);
	return bRet;
}


/*\
 * <---------- CCMXFile :: bParseTRAC ----------> 
 * @m parse a track (TRAC)
 * --> I N <-- @p
 * istream &ar                         - reference of input stream
 * uint32_tint dwSize            - size of the TRAC tag
 * uint32_tint *pdwSizeRemaining - n
 * <-- OUT --> @r
 * bool - true=ok, false=failed
\*/
bool CCMXFile::bParseTRAC(istream &ar, uint32_t dwSize,uint32_t *pdwSizeRemaining)
{
	uint32_t nLen;
	unsigned char *pBuffer;
	bool bRet=false;
	uint32_t nAt=0;
	uint32_t nDelta=0;

	Unpacker up(ar, false);

	*pdwSizeRemaining=dwSize;

	Log2(verbLevDebug1,"parsing track data...\n");
	if (dwSize)
	{
		pBuffer=new unsigned char [dwSize];
		if (pBuffer)
		{
			unsigned char nChannelId;
			unsigned char nChannelIndex;
			int nSampleFormat;
			unsigned char *p;
			uint32_t nExtSize;
			int nPictureId;
			int nPictureFormatId;
			uint32_t nNextPacket;

			ar.read((char *)pBuffer,dwSize);
			nLen=dwSize;
			p=pBuffer;
			hexdump("track data: ",(unsigned char *)p,72);
			while(nLen)
			{
				nDelta=*(p++);
				--nLen;
				nAt+=nDelta;
				Log2(verbLevDebug2,"at %d (delta %d)\n",nAt,nDelta);
				
				switch(*p)
				{
					case 0xFF:		//extendet event
						++p;
						--nLen;
						if (IsChannelFinepitch(*p))			//extA ?
						{
							Log2(verbLevDebug2,"extA\n");
							int nChannel=(*p&0xE0)>>5;
							m_nFinePitch[nChannel]=(uint32_t)(*p&0x1F)<<11;
							++p;
							m_nFinePitch[nChannel]|=(uint32_t)*p;
							++p;
							nLen-=2;
							Log2(verbLevDebug1,"channel %d finepitch %d at %d\n",nChannel,m_nFinePitch[nChannel],nAt);
						}
						else if (IsTimebaseTempo(*p))		//extB ?
						{
							static const int nTimebase[16]={6,12,24,48,96,192,384,0,15,30,60,120,240,480,960,0};
							Log2(verbLevDebug2,"extB\n");
							m_nTimebase=nTimebase[*p & 0x0F];
							++p;
							m_nTempo=*(p++);
							Log2(verbLevDebug1,"set new timebase to %dms and tempo to %d at %d\n",m_nTimebase,m_nTempo,nAt);
							nLen-=2;
						}
						else
						{
							int nJumpId;
							int nJumpCount;
							switch(*p)
							{
								case eCuepoint:					//cuepoint
									++p;
									switch (*(p++))
									{
										case 0x00:					//cue start
											m_nCueStartAt=nAt;
											Log2(verbLevDebug1,"set cue start to %d\n",nAt);
										break;
										case 0x01:					//cue end
											m_nCueEndAt=nAt;
											Log2(verbLevDebug1,"set cue end to %d\n",nAt);
										break;
										default:
											Log2(verbLevWarnings,"unknown cuepoint event at %d\n",nAt);
									}
									nLen-=2;
								break;
								case eJump:			//jump
									++p;
									Log2(verbLevDebug1,"jump at %d\n",nAt);
									switch (*p & 0xC0)
									{
										case 0x00:	//dest
											Log2(verbLevDebug1,"dest event identified\n");
										break;
										case 0x40:	//jump
											Log2(verbLevDebug1,"jump event identified\n");
										break;
										default:
											Log2(verbLevWarnings,"unknown jump event at %d\n",nAt);
									}
									nJumpId=(*p & 0x30)>>4;
									nJumpCount=(*p & 0x0F);
									Log2(verbLevDebug1,"jump id %d, count %d\n",nJumpId,nJumpCount);
									++p;
									nLen-=2;
								break;
								case eNOP:			//NOP
									++p;
									Log2(verbLevDebug1,"NOP at %d\n",nAt);
									if (*p != 0x00)
									{
										Log2(verbLevWarnings,"unexpected NOP extension %02X at %d\n",*p,nAt);
									}
									++p;
									nLen-=2;
								break;
								case eEOT:			//end of track
									++p;
									Log2(verbLevDebug1,"end of track at %d\n",nAt);
									if (*p != 0x00)
									{
										Log2(verbLevWarnings,"unexpected end-of-track extension %02X at %d\n",*p,nAt);
									}
									++p;
									nLen-=2;
									Log2(verbLevDebug2,"offset %Xh (left %d)\n",p-pBuffer,nLen);
								break;	
								case eMasterVol:			//master volume
									Log2(verbLevDebug1,"master volume at %d\n",nAt);
									p+=2;
									nLen-=2;
								break;
								case eMasterTune:			//master tune
									Log2(verbLevDebug1,"master tune at %d\n",nAt);
									p+=2;
									nLen-=2;
								break;
								case ePartConfig:			//part configuration
									Log2(verbLevDebug1,"part configuration at %d\n",nAt);
									p+=2;
									nLen-=2;
								break;
								case ePause:			//pause
									Log2(verbLevDebug1,"pause at %d\n",nAt);
									p+=2;
									nLen-=2;
								break;
								case eStop:				//stop
									Log2(verbLevDebug1,"stop at %d\n",nAt);
									p+=2;
									nLen-=2;
								break;
								case eReset:			//reset
									Log2(verbLevDebug1,"reset at %d\n",nAt);
									p+=2;
									nLen-=2;
								break;
								case eProgramChg:		//program-change
									Log2(verbLevDebug1,"program change at %d\n",nAt);
									p+=2;
									nLen-=2;
								break;
								case eBankChg:			//bank change
									Log2(verbLevDebug1,"bank change at %d\n",nAt);
									p+=2;
									nLen-=2;
								break;
								case eVolume:			//volume
									Log2(verbLevDebug1,"volume at %d\n",nAt);
									p+=2;
									nLen-=2;
								break;
								case ePanPot:			//panpot
									Log2(verbLevDebug1,"panpot at %d\n",nAt);
									p+=2;
									nLen-=2;
								break;
								case ePitchBend:			//pitchbend
									Log2(verbLevDebug1,"pitchbend at %d\n",nAt);
									p+=2;
									nLen-=2;
								break;
								case eChannelAssign:			//channel assign
									Log2(verbLevDebug1,"channel assign at %d\n",nAt);
									p+=2;
									nLen-=2;
								break;
								case ePitchRange:			//pitchbend range
									Log2(verbLevDebug1,"pitchbend range at %d\n",nAt);
									p+=2;
									nLen-=2;
								break;
								case eWaveChnVol:			//wave-channel-volume
									Log2(verbLevDebug1,"wave channel volume at %d\n",nAt);
									p+=2;
									nLen-=2;
								break;
								case eWaveChnPan:			//wave-channel-panpot
									Log2(verbLevDebug1,"pan pot at %d\n",nAt);
									p+=2;
									nLen-=2;
								break;
								case eTextCtrl:			//text-control
									/*
									; %x00 : Text Enable
									; %x01 : Text Disable
									; %x02 : Clear text
									; %x03 : reserved
									; %x04 : Increase cursor position by 1 byte
									; %x05 : Increase cursor position by 2 bytes
									*/
									Log2(verbLevDebug1,"text control at %d\n",nAt);
									p+=2;
									nLen-=2;
								break;
								case ePictCtrl:			//picture-control
									Log2(verbLevDebug1,"picture control at %d\n",nAt);
									++p;
									switch(*p)
									{
										case 0x00:			//picture enable
											Log2(verbLevDebug1,"picture enabled\n");
										break;
										case 0x01:			//picture disable
											Log2(verbLevDebug1,"picture disabled\n");
										break;
										case 0x02:			//picture clear
											Log2(verbLevDebug1,"picture cleared\n");
										break;
										default:
											Log2(verbLevWarnings,"unknown picture control %02X at %d\n",*p,nAt);
									}
									++p;
									nLen-=2;
								break;
								case eVibraCtrl:			//vibra-control
									Log2(verbLevDebug1,"vibra control at %d\n",nAt);
									p+=2;
									nLen-=2;
								break;
								case eLEDCtrl:			//LED-control
									Log2(verbLevDebug1,"led control at %d\n",nAt);
									p+=2;
									nLen-=2;
								break;
								case eWaveDataLen:			//wave data length
									++p;
									nExtSize=ntohs(*(unsigned short int*)p);
									Log2(verbLevDebug1,"wave data length %d at %d\n",nExtSize,nAt);
									p+=2;
									nChannelId=*p & 0x3F;
									nChannelIndex=(*p & 0xC0)>>6;
									Log2(verbLevDebug1,"channel id: %d\n",nChannelId);
									Log2(verbLevDebug1,"channel index: %d\n",nChannelIndex);
									++p;
									switch(*p & 0xC0)
									{
										case 0x00:										//sample store
											Log2(verbLevDebug1,"sample store mode at %d\n",nAt);
										break;
										case 0x40:										//sample set
											Log2(verbLevDebug1,"sample set mode at %d\n",nAt);
										break;
										case 0x80:										//sample recycle
											Log2(verbLevDebug1,"sample recycle mode at %d\n",nAt);
										break;
										default:
											Log2(verbLevWarnings,"unknown sample mode %02X at %d\n",*p,nAt);
									}
									nSampleFormat=*p & 0x3F;
									switch(nSampleFormat)
									{
										case 0x04:					//wave qcp
											Log2(verbLevDebug1,"sample format QCP at %d\n",nAt);
										break;
										case 0x05:					//wave ima
											Log2(verbLevDebug1,"sample format IMA at %d\n",nAt);
										break;
										default:
											Log2(verbLevWarnings,"unknown sample format %02X at %d\n",*p&0x3F,nAt);
											nSampleFormat=CCMXPacket::fmtUnknown;
									}
									++p;
									nNextPacket=htonl(*p);
									Log2(verbLevDebug2,"next sample packet offset %d at %d\n",nNextPacket,nAt);
									p+=4;
									switch(*p)
									{
										case 0x00:				//prev-flag-dis
											m_pCurrentSample=new CCMXSample(nSampleFormat,0);
											m_Samples.push_back(m_pCurrentSample);
											Log2(verbLevDebug1,"new sample created at %d\n",nAt);
										case 0x01:				//prev-flag-en
											ASSERT(m_pCurrentSample);
											if (m_pCurrentSample != NULL)
											{
												Log2(verbLevDebug2,"adding %d bytes to sample %d at %d\n",nExtSize-7,m_Samples.size(),nAt);
												m_pCurrentSample->AddPacket(p+1,nExtSize-7);
											}
										break;
										default:
											Log2(verbLevWarnings,"invalid sample prev flag %02X at %d\n",*p,nAt);
									}
									++p;
									//sample data following...
									//ExportSample(p,nExtSize-6);
									p+=nExtSize-7;
									nLen-=nExtSize+3;
								break;
								case eTextDataLen:			//text data length
									nExtSize=ntohs(*(unsigned short int*)(p+1));
									Log2(verbLevDebug1,"text data length %d at %d\n",nExtSize,nAt);
									p+=2;
									nLen-=2;
								break;
								case ePictDataLen:			//pict data length
									++p;
									nExtSize=ntohs(*(unsigned short int*)p);
									Log2(verbLevDebug1,"picture data length %d at %d\n",nExtSize,nAt);
									p+=2;
									nPictureId=*p;
									Log2(verbLevDebug1,"picture id: %d\n",*p);
									++p;
									switch(*p & 0xC0)
									{
										case 0x00:										//picture store
											Log2(verbLevDebug1,"picture store mode at %d\n",nAt);
										break;
										case 0x40:										//picture set
											Log2(verbLevDebug1,"picture set mode at %d\n",nAt);
										break;
										case 0x80:										//picture recycle
											Log2(verbLevDebug1,"picture recycle mode at %d\n",nAt);
										break;
										default:
											Log2(verbLevErrors,"unknown picture mode %02X at %d\n",*p,nAt);
									}
									nPictureFormatId=*p & 0x3F;
									switch(nPictureFormatId)
									{
										case pfBmp:				//picture format BMP
											Log2(verbLevDebug1,"picture format BMP at %d\n",nAt);
										break;
										case pfJpeg:			//picture format JPEG
											Log2(verbLevDebug1,"picture format JPEG at %d\n",nAt);
										break;
										case pfPng:				//picture format PNG
											Log2(verbLevDebug1,"picture format PNG at %d\n",nAt);
										break;
										default:
											Log2(verbLevWarnings,"unknown picture format %02X at %d\n",nPictureFormatId,nAt);
									}
									++p;
									if (*p != 0x00)
									{
										Log2(verbLevErrors,"unknown picture draw mode %02X at %d\n",*p,nAt);
									}
									++p;
									if (*p <= 100)			//X offset in percent
									{
										Log2(verbLevDebug1,"x offset %d% at %d\n",*p,nAt);
									}
									else if (*p == 101)		//left
									{
										Log2(verbLevDebug1,"x offset set to left\n",*p,nAt);
									}
									else if (*p == 102)		//center
									{
										Log2(verbLevDebug1,"x offset set to center\n",*p,nAt);
									}
									else if (*p == 103)		//right
									{
										Log2(verbLevDebug1,"x offset set to right\n",*p,nAt);
									}
									++p;
									if (*p <= 100)			//Y offset in percent
									{
										Log2(verbLevDebug1,"y offset %d% at %d\n",*p,nAt);
									}
									else if (*p == 101)		//left
									{
										Log2(verbLevDebug1,"y offset set to left\n",*p,nAt);
									}
									else if (*p == 102)		//center
									{
										Log2(verbLevDebug1,"y offset set to center\n",*p,nAt);
									}
									else if (*p == 103)		//right
									{
										Log2(verbLevDebug1,"y offset set to right\n",*p,nAt);
									}
									++p;
									//picture data following....
									if (m_pParameters && !m_pParameters->m_strParameter[paraStrImageExportPath].empty())
										ExportPicture(m_pParameters->m_strParameter[paraStrImageExportPath].c_str(),nPictureId,nPictureFormatId,p,nExtSize-5);
									p+=nExtSize-5;
									nLen-=nExtSize+3;
								break;
								case eAnimDataLen:			//anim data length
									Log2(verbLevDebug1,"animation data length %d at %d\n",nExtSize,nAt);
									nLen-=2;
									p+=2;								
								break;
								default:
									Log2(verbLevDebug1,"unknown D-event %02X at %d\n",*p,nAt);
							}
						}
					break;
				}
			};
			delete [] pBuffer;
			bRet=true;
			m_nPlaytime=nAt*10;
		}
	}
	if (bRet == true && m_Samples.size())
	{
		uint32_t nDecodedSize;
		signed short int *pnDecoded=NULL;
		//Log2(verbLevMessages,"exporting entire sample...\n");
		//m_Samples[0]->ExportRaw();
		TRACEIT2("decoding sample...\n");
		Log2(verbLevDebug1,"decoding sample...\n");
		m_Samples[0]->Decode(m_bParseOnly ? NULL : &pnDecoded,&nDecodedSize);
		TRACEIT2("decoding done\n");
		m_nCSBitsPerSample=16;
		m_nCSChannels=1;
		m_nCSSamplesPerSecond=m_Samples[0]->nGetSampleRate();
		m_nCSSize=nDecodedSize;
		if (!m_bParseOnly)
			m_pcCSBuffer=(void *)pnDecoded;
	}
	*pdwSizeRemaining=nLen;
	TRACEIT2("done\n");
	return bRet;
}

/*\
 * <---------- CCMXFile :: ExportPicture ----------> 
 * @m 
 * --> I N <-- @p
 * const char *pszPathPrefix  - 
 * int nPictureId             - 
 * unsigned char *p           - 
 * uint32_tint nExtSize - 
\*/
void CCMXFile::ExportPicture(const char *pszPathPrefix,int nPictureId,int nPictureFormatId,unsigned char *p,uint32_t nExtSize)
{
	char szPath[_MAX_PATH]="";
	const char *szFormatExtensions[3]=
	{
		"bmp",
		"jpg",
		"png"
	};
	char szExt[4]={"raw"};
	if (nPictureFormatId >= pfBmp && nPictureFormatId <= pfPng)
		strcpy(szExt,szFormatExtensions[nPictureFormatId-pfBmp]);
	sprintf(szPath,"%s_%03d.%s",pszPathPrefix,nPictureId,szExt);
	ofstream out(szPath,ios_base::out | ios_base::binary);
	out.write((const char *)p,nExtSize);
	out.close();
}

/*\
 * <---------- ExportSample ---------->
 * @m 
 * --> I N <-- @p
 * unsigned char *p - 
 * uint32_tint nExtSize - 
\*/
/*
void CCMXFile::ExportSample(unsigned char *p,uint32_tint nExtSize)
{
	FILE *fp;
	char cName[255]="sample.raw";

	Log2(verbLevMessages,"exporting sample data as \"%s\"\n",cName);

	if ((fp=fopen(cName,"wb")) != NULL)
	{
		fwrite(p,nExtSize,1,fp);
		fclose(fp);
	}
}
*/

/*\
 * <---------- CCMXFile :: ReadHeader ----------> 
 * @m 
 * --> I N <-- @p
 * istream &ar     - input stream reference
 * CMXHEADER *pCmx - 
\*/
void CCMXFile::ReadHeader(istream &ar, CMXHEADER *pCmx)
{ 
	Unpacker up(ar, false);
	//Read the first 4 bytes, which will hold 'cmid' = 	63 6D 69 64 
	ar.read((char *)pCmx->cmid,4); 
	if (memcmp(pCmx->cmid,"cmid",4))
	{
		tstring str;
		str=tstring(_T("unnknown header chunk"));
		Log2(verbLevErrors,"unknown header chunk\n");
		throw new CFormatException(CFormatException::formaterrInvalid,str.c_str());
	}
	//Now, read the size (a.cmx = 00 00 90 1E)
	up.read("l",&pCmx->size);
	//Now, read wHeaderLength  (00 7C)
	up.read("s",(char*)&pCmx->wHeaderLength); 
	//Now, read cMode (02)
	ar.read((char*)&pCmx->cMode,sizeof(pCmx->cMode));
	Log2(verbLevDebug1,"mode: %02X\n",pCmx->cMode);
	if (pCmx->cMode == 0x02 || pCmx->cMode == 0x00)
	{
		//read instruments mask (0A)
		ar.read((char*)&pCmx->cInstrumentsMask,sizeof(pCmx->cInstrumentsMask)); 
		Log2(verbLevDebug1,"instrument mask: %02X\n",pCmx->cInstrumentsMask);
	}
	else
	{
	}
	//read track count (01)
	ar.read((char*)&pCmx->cTracks,sizeof(pCmx->cTracks));
	Log2(verbLevDebug1,"track count: %02X\n",pCmx->cTracks);
} 

/*\
 * <---------- sGetFormatName ---------->
 * @m 
 * --> I N <-- @p
 * int nFormat - 
 * <-- OUT --> @r
 * tstring - 
\*/
tstring CCMXFile::sGetFormatName(int nFormat)
{
	CMyString strText;
	strText.Load(IDS_FORMATNAME_CMX11+(nFormat-1));
	return strText;
}

/*\
 * <---------- nGetFormat ---------->
 * @m 
 * <-- OUT --> @r
 * int - 
\*/
int CCMXFile::nGetFormat(void)
{
	int nRet=1;
	switch(m_Header.wVersion)
	{
		case 0x0201:	nRet=1;		break;		//2.1
		case 0x0300:	nRet=2;		break;		//2.2
		case 0x0400:	nRet=3;		break;		//3.0
		case 0x0410:	nRet=4;		break;		//
		case 0x0500:	nRet=5;		break;		//
	}
	return nRet;
}

/*\ 
 * <---------- CCMXFile::RenderDestination ----------> 
 * @m render a CMF file from our source data
 * --> I N <-- @p
 * ostream &out - reference to output stream
\*/ 
void CCMXFile::Write(ostream &out)
{
	uint32_t nFileSize=0;
	unsigned char cByteValue;
	tstring sChunk;
	unsigned short int nChunkSize;
	unsigned short int nHeaderSize=0x5E;
	uint32_t dwTrackSize;
	int nLoopcount=m_pParameters->m_nParameter[paraNumLoopcount];
	unsigned int nVolume=(0x3F*m_pParameters->m_nParameter[paraNumVolume])/100;
	unsigned int nFormat=m_pParameters->m_nParameter[paraNumCompression] == 1 ? 5 : 4;
	
	Packer pk(out, false);

	tstring sTitle,sCopyright,sPublisher;
	//m_pParameters->m_nParameter[paraNumPlaytime]

	dwTrackSize=nRenderTrack(nFormat,m_pCSSource->m_nCSSamplesPerSecond,nLoopcount,nVolume,m_pCSSource->m_nCSSize,(char *)m_pCSSource->m_pcCSBuffer);

	sTitle=m_pCSSource->m_strInfo[infoTitle];
	if (!sTitle.empty())
		nHeaderSize+=6+(unsigned short int)sTitle.length();

	sCopyright=m_pCSSource->m_strInfo[infoCopyright];
	if (!sCopyright.empty())
		nHeaderSize+=6+(unsigned short int)sCopyright.length();

	sPublisher=m_pCSSource->m_strInfo[infoPublisher];
	if(!sPublisher.empty())	
		nHeaderSize+=6+(unsigned short int)sPublisher.length();
	
	nFileSize=dwTrackSize+nHeaderSize+10;

	out.write("cmid",4);					//cmid
	pk.write("l",&nFileSize);				//filesize
	pk.write("s",&nHeaderSize);				//headersize
	cByteValue=0x02;
	pk.write("b",&cByteValue);				//song
	cByteValue=cmiMaleVocal;
	pk.write("b",&cByteValue);				//instrument mask
	cByteValue=1;
	pk.write("b",&cByteValue);				//number of tracks
	out.write("vers",4);					//vers
	nChunkSize=4;
	pk.write("s",&nChunkSize);				//
	if(nFormat == 4)
	{
		//2.2
		out.write("0300",4);
	}
	else
	{
		//3.0
		out.write("0400",4);
	}

	if (!sTitle.empty())
	{
		out.write("titl",4);				//titl
		nChunkSize=(unsigned short int)sTitle.length();
		pk.write("s",&nChunkSize);	
		out.write(sTitle.c_str(),(uint32_t)sTitle.length());
	}
	out.write("sorc",4);					//sorc
	nChunkSize=1;
	pk.write("s",&nChunkSize);	
	out.write("\000",1);

	if (!sCopyright.empty())
	{
		out.write("copy",4);				//copy
		nChunkSize=(unsigned short int)sCopyright.length();
		pk.write("s",&nChunkSize);	
		out.write(sCopyright.c_str(),(unsigned short int)sCopyright.length());
	}

	out.write("date",4);					//date
	nChunkSize=8;
	pk.write("s",&nChunkSize);	
	out.write("20050505",8);
	
	out.write("code",4);					//code
	nChunkSize=1;
	pk.write("s",&nChunkSize);	
	out.write("\000",1);

	if (!sPublisher.empty())
	{
		out.write("prot",4);				//prot
		nChunkSize=(unsigned short int)sPublisher.length();
		pk.write("s",&nChunkSize);	
		out.write(sPublisher.c_str(),(uint32_t)sPublisher.length());
	}

	out.write("wave",4);					//wave
	nChunkSize=1;
	pk.write("s",&nChunkSize);	
	out.write("\001",1);

	out.write("tool",4);					//tool
	nChunkSize=7;
	pk.write("s",&nChunkSize);
	out.write("3.2.185",7);

	out.write("note",4);					//note
	nChunkSize=2;
	pk.write("s",&nChunkSize);	
	out.write("\000\001",2);

	out.write("exst",4);					//exst
	nChunkSize=2;
	pk.write("s",&nChunkSize);	
	out.write("\000\000",2);

	out.write("pcpi",4);					//pcpi
	nChunkSize=1;
	pk.write("s",&nChunkSize);	
	out.write("\000",1);

	out.write("cnts",4);					//cnts
	nChunkSize=4;
	pk.write("s",&nChunkSize);	
	out.write("WAVE",4);

	out.write("trac",4);					//trac
	if (dwTrackSize > 0)
	{
		//dwChunkSize=htonl(dwTrackSize);
		char *pcTrack=new char [dwTrackSize];
		nRenderTrack(nFormat,m_pCSSource->m_nCSSamplesPerSecond,nLoopcount,nVolume,m_pCSSource->m_nCSSize,(char *)m_pCSSource->m_pcCSBuffer,pcTrack);
		pk.write("l",&dwTrackSize);	
		out.write(pcTrack,dwTrackSize);
		delete [] pcTrack;
	}
	m_nCSChannels=m_pCSSource->m_nCSChannels;
	m_nCSBitsPerSample=0;
	m_nCSSamplesPerSecond=m_pCSSource->m_nCSSamplesPerSecond;
	m_nCSSize=dwTrackSize;
	m_nFileSize=out.tellp();
}

/*\
 * <---------- *CCMXFile :: pAddExtEvent ----------> 
 * @m add an extended event to the ouput buffer
 * --> I N <-- @p
 * unsigned char *pOut      - pointer to output buffer
 * unsigned char nDelta     - timestamp delta
 * unsigned char nCommand   - event command id
 * unsigned char nParameter - event parameter
 * <-- OUT --> @r
 * pointer to output buffer after the added event
\*/
unsigned char *CCMXFile::pAddExtEvent(unsigned char *pOut,unsigned char nDelta,unsigned char nCommand,unsigned char nParameter)
{
	ASSERT(pOut);
	*(pOut++)=nDelta;		//delta
	*(pOut++)=0xFF;			//ext
	*(pOut++)=nCommand;		//jump destination
	*(pOut++)=nParameter;	//jump id
	return pOut;
}

/*\
 * <---------- nRenderTrack ---------->
 * @m render a CMF track using a sample and some attributes
 * --> I N <-- @p
 * int nFormat - 
 * int nSampleRate - sample rate in Hz
 * int nLoopcount - number of repetitions (0-15, 15=infinite)
 * int nVolume - 
 * uint32_tint nSourceSize - sample size in bytes
 * char *pcSource - pointer to source data
 * char *pcDest - pointer to destination buffer (NULL = simulate)
 * <-- OUT --> @r
 * uint32_t- size of the track in bytes
\*/
uint32_t CCMXFile::nRenderTrack(uint32_t nFormat,uint32_t nSampleRate,uint32_t nLoopcount,uint32_t nVolume,uint32_t nSourceSize,char *pcSource,char *pcDest)
{
	uint32_t i;
	uint32_t nSize=0;
	uint32_t nLastDelta;
	uint32_t nPacketCount=0;

	if (pcSource)
	{
		if (!m_Samples.size())
		{
			m_pCurrentSample=new CCMXSample(nFormat,nSampleRate);
			m_Samples.push_back(m_pCurrentSample);
			Log2(verbLevDebug2,"new sample created\n");
			Log2(verbLevDebug2,"Sample Format: %d\n",nFormat);
			m_pCurrentSample->Encode((short *)pcSource,nSourceSize);
		}

		nPacketCount=m_pCurrentSample->nGetPacketCount();
		nSize=12;
		if (nLoopcount > 1)
			nSize+=8;
		if (nVolume)
			nSize+=4;
		for (i=0;i < nPacketCount;i++)
			nSize+=12+m_Samples[0]->pGetPacket(i)->nGetSize();
		if (pcDest)
		{
			unsigned char *p=(unsigned char *)pcDest;

			//are we looping?
			if (nLoopcount > 1)
			{	//yes->embed a cuepoint (jump-point in CMX terminology)
				p=pAddExtEvent(p,0x01,eJump,0x00);	
			}
			//tempo/timebase defaults
			p=pAddExtEvent(p,0x00,0xC3,0x7D);
			//little rest for the engine
			p=pAddExtEvent(p,0x00,eNOP,0x00);
			nLastDelta=1;
			//any volume specified?
			if (nVolume)
			{	//yes->embed volume information
				p=pAddExtEvent(p,(unsigned char)nLastDelta,eWaveChnVol,nVolume);
				nLastDelta=0;
			}
			uint32_t nDecode,nDoneDelta=0,nDoneSize=0;
			for (i=0;i < nPacketCount;i++)
			{
				Log2(verbLevDebug3,"packet delta %d\n",nLastDelta);
				*(p++)=(unsigned char)nLastDelta;		//delta
				*(p++)=0xFF;			//ext
				*(p++)=eWaveDataLen;			//wave size
				unsigned short int nEncodedSize=(unsigned short int)m_Samples[0]->pGetPacket(i)->nGetSize();
				nEncodedSize=htons(nEncodedSize+7);
				memcpy(p,&nEncodedSize,2);
				p+=2;
				*(p++)=0x00;			//channel id, channel index
				*(p++)=0x40|nFormat;	//sample mode, format
				uint32_t nNextPacket=htonl(m_Samples[0]->pGetPacket(i)->nGetSize()+5+7);
				memcpy(p,&nNextPacket,4);
				p+=4;
				if (i == 0)
					*(p++)=0x00;			//new sample
				else
					*(p++)=0x01;			//continued sample

				nDecode=m_Samples[0]->pGetPacket(i)->nGetSize();
				ASSERT(nDecode > 2);
				nDoneSize+=nDecode-2;
				//sample data following....
				memcpy(p,m_Samples[0]->pGetPacket(i)->pGetData(),nDecode);
				p+=nDecode;
		
				nLastDelta=m_Samples[0]->nGetFragmentDelta(nDoneSize,nDoneDelta);
				nDoneDelta+=nLastDelta;
			}
			Log2(verbLevDebug2,"nop delta %d\n",nLastDelta);
			if (nLoopcount > 1)
			{
				p=pAddExtEvent(p,(unsigned char)nLastDelta,eJump,0x40 | (nLoopcount & 0x0F));	//jump id, repeat count, jump
				nLastDelta=0;
			}
			p=pAddExtEvent(p,(unsigned char)nLastDelta,eEOT,0x00);	//EOT
			Log2(verbLevDebug2,"used %d bytes for the track memory (assumed %d)\n",p-(unsigned char *)pcDest,nSize);
		}
	}
	return nSize;
}
