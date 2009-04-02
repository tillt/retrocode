/*\
 * SMAFFile.cpp
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
#include <iostream>
#include <fstream>
#ifdef WIN32
#include <strstream>
#include <Winsock2.h>
#else
#include <netinet/in.h>
#include <sstream>
#endif
#include "../include/Resource.h"
#include "../retroBase/Basics.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MyString.h"
#include "../retroBase/MobileContent.h"
#include "SMAFDecoder.h"
#include "SMAFTrack.h"
#include "SMAFGraph.h"
#include "SMAFSample.h"
#include "SMAFAudio.h"
#include "SMAFFile.h"
#include "SMAFProperty.h"

DYNIMPPROPERTY(CSMAFFile,CSMAFProperty)

const unsigned short int CSMAFFile::m_wCRCTable[256] = 
{
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
	0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
	0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
	0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
	0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
	0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
	0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
	0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
	0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
	0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
	0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
	0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
	0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
	0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
	0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
	0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
	0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
	0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
	0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
	0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
	0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
	0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
	0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
	0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
	0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
	0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
	0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

// CSMAFFile
CSMAFFile::CSMAFFile()
{
	m_strLastError=_T("ok");
	m_pcSMAF=NULL;
	m_pcStatusByte=NULL;
	m_pAudio=NULL;
	m_pGraph=NULL;
	m_nSequenceFormat=0;
	m_nEncoding=0;
	m_nFormat=smaffmtUnknown;

	m_bContainsHumanVoice=false;
	m_bContainsSamples=false;
	m_bContainsCustomInstruments=false;
	m_bContainsGraphix=false;
	m_bContainsSynthesizer=false;

	m_nPlaytime=0;
	m_nMagicSize=4;
	m_pcMagic="MMMD";
	m_sFormatName=_T("Yamaha Synthetic Music Application Format (SMAF/MMF)");
	m_sDefaultExtension=_T("mmf");
	m_nSamplePlaytime=0;
	m_bSampleFromTracks=false;
	m_sFormatDescription.Load(IDS_FORMDESC_SMAF);
	m_sFormatCredits=_T("Retro's SMAF codec is based on: \"Specification: Synthetic music Mobile Application Format\" Ver.3.06, Copyright (c) 1999-2002 by YAMAHA CORPORATION."),

	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono|CSampleCompatibility::supportsStereo].setRange(4000,44100);

	m_encodingPara.addPara(cmdParaBool,paraBoolStreamSamples);
	m_encodingPara.addPara(cmdParaBool,paraBoolSaveEnabled);
	m_encodingPara.addPara(cmdParaBool,paraBoolEditEnabled);
	m_encodingPara.addPara(cmdParaBool,paraBoolTransferEnabled);
	m_encodingPara.addPara(cmdParaBool,paraBoolProTag);
	m_encodingPara.addPara(cmdParaNumber,paraNumDevice);
	m_encodingPara.addPara(cmdParaString,paraStrImageExportPath);
	m_encodingPara.addPara(cmdParaString,paraStrArtist);
	m_encodingPara.addPara(cmdParaString,paraStrNote);
	m_encodingPara.addPara(cmdParaString,paraStrWriter);
	m_encodingPara.addPara(cmdParaString,paraStrCategory);
	m_encodingPara.addPara(cmdParaString,paraStrSubcategory);
	m_encodingPara.addPara(cmdParaString,paraStrCopyright);
	m_encodingPara.addPara(cmdParaString,paraStrVendor);
	m_encodingPara.addPara(cmdParaString,paraStrArranger);
	m_encodingPara.addPara(cmdParaString,paraStrManagement);
	m_encodingPara.addPara(cmdParaString,paraStrManagedBy);
	m_encodingPara.addPara(cmdParaString,paraStrCarrier);
	m_encodingPara.addPara(cmdParaString,paraStrDateCreated);
	m_encodingPara.addPara(cmdParaString,paraStrDateRevised);
}

CSMAFFile::~CSMAFFile()
{
	unsigned int i;
	TRACEIT2("erasing tracks...\n");
	for (i=0;i < (unsigned int)m_Tracks.size();i++)
	{
		ASSERT (m_Tracks[i]);
		if (m_Tracks[i] != NULL)
			delete m_Tracks[i];
	}
	if (m_Tracks.size())
		m_Tracks.erase(m_Tracks.begin(), m_Tracks.end());
	TRACEIT2("erasing smaf memory...\n");
	if (m_pcSMAF)
		delete [] m_pcSMAF;
	TRACEIT2("erasing audio memory...\n");
	if (m_pAudio)
		delete m_pAudio;
	if (m_bSampleFromTracks)
		m_pcCSBuffer=NULL;
	if (m_pGraph)
		delete m_pGraph;
}

/*\
 * <---------- CSMAFFile :: DecodeOPDASubChunk ----------> 
 * @m 
 * --> I N <-- @p
 * unsigned char **pcBuffer         - 
 * unsigned char *pcLimit           - 
 * tstring &sIdentifier             - 
 * unsigned int *pnSize             - 
 * unsigned char **pcChunkAttribute - 
\*/
void CSMAFFile::DecodeOPDASubChunk(unsigned char **pcBuffer,unsigned char *pcLimit,tstring &sIdentifier,unsigned int *pnSize,unsigned char **pcChunkAttribute)
{
	int i;
	unsigned char *pcLast;
	sIdentifier.clear();
	for (i=0;i < 2;i++)
	{
		sIdentifier+=**pcBuffer;
		*pcBuffer+=1;
	}
	//*pnSize=ntohs(*(unsigned short int *)*pcBuffer);
	unsigned short int wSizeCode;
	memcpy(&wSizeCode,*pcBuffer,2);
	*pnSize=htons(wSizeCode);
	*pcBuffer+=2;
	pcLast=*pcBuffer+*pnSize;
	if (pcLast > pcLimit)
	{
		Log2(verbLevErrors,"size:%d\n",*pnSize);
		throw new CFormatException(CFormatException::formaterrInvalidChunkSize,"OPDA data exceeding tag size");
	}
	*pcChunkAttribute=*pcBuffer;
	*pcBuffer=pcLast;
}

/*\
 * <---------- CSMAFFile :: DecodeCNTISubChunk ----------> 
 * @m 
 * --> I N <-- @p
 * unsigned char **pcBuffer         - 
 * unsigned char *pcLimit           - 
 * tstring &sIdentifier             - 
 * unsigned int *pnSize             - 
 * unsigned char **pcChunkAttribute - 
\*/
void CSMAFFile::DecodeCNTISubChunk(unsigned char **pcBuffer,unsigned char *pcLimit,tstring &sIdentifier,unsigned int *pnSize,unsigned char **pcChunkAttribute)
{
	int i;
	bool bEscaped=false;

	TRACEIT2("decoding CNTI subchunk\n");

	sIdentifier.clear();
	for (i=0;i < 2 && **pcBuffer >= ' ';i++)
	{
		sIdentifier+=**pcBuffer;
		*pcBuffer+=1;
	}
	if (**pcBuffer != ':')
	{
		Log2(verbLevErrors,"invalid separator\n");
		throw new CFormatException(CFormatException::formaterrInvalid,"invalid separator");
	}
	//skip ':'
	*pcBuffer+=1;
	*pcChunkAttribute=*pcBuffer;
	//while (**pcBuffer != ',')
	while (TRUE)
	{
		if (**pcBuffer == ',' && !bEscaped)
			break;
		if (*pcBuffer > pcLimit)
			throw new CFormatException(CFormatException::formaterrTruncated);
		if (**pcBuffer == '\\')
			bEscaped=true;
		else
			bEscaped=false;
		*pcBuffer+=1;
	};
	*pnSize=(unsigned int)(*pcBuffer-*pcChunkAttribute);
	//skip ','
	*pcBuffer+=1;
	TRACEIT2("CNTI subchunk decoded\n");
}

/*\
 * <---------- CSMAFFile :: EncodeStatusByte ----------> 
 * @m 
 * --> I N <-- @p
 * unsigned char cStatus - 
 * <-- OUT --> @r
 * 
\*/
unsigned char CSMAFFile::EncodeStatusByte(unsigned char cStatus)
{
	cStatus&=0xf8;	//mask out copy-, save- and edit-bit
	if (m_bStatusCopy)
		cStatus|=0x01;
	if (m_bStatusSave)
		cStatus|=0x02;
	if (m_bStatusEdit)
		cStatus|=0x04;
	return cStatus;
}

/*\
 * <---------- CSMAFFile :: DecodeStatusByte ----------> 
 * @m 
 * --> I N <-- @p
 * unsigned char cStatus - 
\*/
void CSMAFFile::DecodeStatusByte(unsigned char cStatus)
{
	m_bStatusCopy=(cStatus&0x01) == 0x01;
	m_bStatusSave=(cStatus&0x02) == 0x02;
	m_bStatusEdit=(cStatus&0x04) == 0x04;
}

/*\
 * <---------- CSMAFFile :: DecodeOPDA ----------> 
 * @m 
 * --> I N <-- @p
 * unsigned char **pcBuffer - 
 * unsigned int nSize       - 
\*/
void CSMAFFile::DecodeOPDA(unsigned char **pcBuffer,unsigned int nSize)
{
	map<tstring,int> :: const_iterator iterChunk;
	map<tstring,int> mapChunkId;

	mapChunkId[tstring(_T("M2"))]=scidM2;
	mapChunkId[tstring(_T("L2"))]=scidL2;
	mapChunkId[tstring(_T("ST"))]=scidST;
	mapChunkId[tstring(_T("A0"))]=scidA0;
	mapChunkId[tstring(_T("A1"))]=scidA1;
	mapChunkId[tstring(_T("A2"))]=scidA2;
	mapChunkId[tstring(_T("WW"))]=scidWW;
	mapChunkId[tstring(_T("SW"))]=scidSW;
	mapChunkId[tstring(_T("ES"))]=scidES;
	mapChunkId[tstring(_T("RF"))]=scidRF;
	mapChunkId[tstring(_T("HV"))]=scidHV;
	mapChunkId[tstring(_T("VN"))]=scidVN;
	mapChunkId[tstring(_T("GR"))]=scidGR;
	mapChunkId[tstring(_T("MI"))]=scidMI;
	mapChunkId[tstring(_T("CD"))]=scidCD;
	mapChunkId[tstring(_T("UD"))]=scidUD;
	mapChunkId[tstring(_T("CA"))]=scidCA;
	mapChunkId[tstring(_T("CN"))]=scidCN;
	mapChunkId[tstring(_T("CR"))]=scidCR;
	mapChunkId[tstring(_T("AW"))]=scidAW;
	mapChunkId[tstring(_T("AN"))]=scidAN;
	mapChunkId[tstring(_T("AS"))]=scidAS;
	mapChunkId[tstring(_T("LC"))]=scidLC;
	mapChunkId[tstring(_T("MD"))]=scidMD;

	if (m_nFormat < smaffmtMA3)
		m_nFormat=smaffmtMA3;
	while (nSize)
	{
		unsigned int nChunkSize;
		unsigned char cAttribute;
		tstring sChunk;
		//unsigned char *pSMAF=*pcBuffer;
		unsigned char *pLim=*pcBuffer+nSize;

		DecodeChunk(pcBuffer,pLim,sChunk,&nChunkSize,&cAttribute);
		nSize-=8;
		if (sChunk == _T("Pro"))
		{
			if (m_nFormat < smaffmtMA5)
				m_nFormat=smaffmtMA5;
			*pcBuffer+=nChunkSize;
			nSize-=nChunkSize;
		}
		else if (sChunk == _T("Dch"))
		{
			//DecodeOPDA(&pSMAF,&nSize);
			while (nChunkSize)
			{
				tstring sChunk;
				unsigned int nSubChunkSize=0;
				unsigned char *pcAttribute;
				
				DecodeOPDASubChunk(pcBuffer,pLim,sChunk,&nSubChunkSize,&pcAttribute);
				Log2(verbLevDebug2,"decoded subchunk: %s - size: %d\n",sChunk.c_str(),nSubChunkSize);
				if ((iterChunk=mapChunkId.find(sChunk)) == mapChunkId.end())
				{
					Log2(verbLevErrors,IDS_ERR_UNKSUBCHNK);
					throw new CFormatException(CFormatException::formaterrUnknownSubChunk,"unknown subchunk");
				}
				TCHAR szMsg[256];
				Log2(verbLevDebug2,"sub chunk: %s\n",sChunk.c_str());			
	#ifdef _UNICODE
				char szBuffer[256];
				int nCP,len=min(nSubChunkSize,254);
				memcpy (szBuffer,pcAttribute,len);
				szBuffer[len]=0;
				ZeroMemory(szMsg,255*sizeof(TCHAR));
				switch(m_nEncoding)
				{
					case smafencUTF8:		nCP=CP_UTF8;	break;
					default:				nCP=CP_ACP;
				}
				MultiByteToWideChar(nCP,0,(const char *)szBuffer,min(nSubChunkSize,255),szMsg,255); 
	#else
				memcpy (szMsg,pcAttribute,min(nSubChunkSize,255));
				szMsg[min(nSubChunkSize,255)]=0;
	#endif
				Log2(verbLevDebug2,"paramter: %s\n",szMsg);
				switch(iterChunk->second)
				{
					case scidCR:	SetInfoText(infoCopyright,szMsg);		break;
					case scidGR:	SetInfoText(infoCopyManaged,szMsg);		break;
					case scidMI:	SetInfoText(infoManagementInfo,szMsg);	break;
					case scidCD:	SetInfoText(infoDateCreated,szMsg);		break;
					case scidUD:	SetInfoText(infoDateRevised,szMsg);		break;
					case scidCN:	SetInfoText(infoCarrier,szMsg);			break;
					case scidSW:	SetInfoText(infoComposer,szMsg);		break;
					case scidCA:	SetInfoText(infoCategory,szMsg);		break;
					case scidWW:	SetInfoText(infoWords,szMsg);			break;
					case scidAW:	SetInfoText(infoArranged,szMsg);		break;
					case scidAN:	SetInfoText(infoArtist,szMsg);			break;
					case scidVN:	SetInfoText(infoVendor,szMsg);			break;
					case scidST:	SetInfoText(infoTitle,szMsg);			break;
					case scidA0:	DecodeAX((const TCHAR *)szMsg);			break;
					//case scidAS:	DecodeAS((const TCHAR *)szMsg);			break;
					//case scidMD:	DecodeMD((const TCHAR *)szMsg);			break;
					//case scidRF:	DecodeRF((const TCHAR *)szMsg);			break;
				}
				Log2(verbLevDebug2,"size left: %d\n",nSize);
				nSize-=nSubChunkSize+4;
				nChunkSize-=nSubChunkSize+4;
			};
		}
		else
		{
			Log2(verbLevDebug2,"expected Dch\n");
			throw new CFormatException(CFormatException::formaterrInvalid,"expected Dch");
		}
	};
}

/*\
 * <---------- DecodeAX ---------->
 * @m identify encoding software from A0-tag
 * --> I N <-- @p
 * const TCHAR *szMsg - A0 tag data
\*/
void CSMAFFile::DecodeAX(const TCHAR *szMsg)
{
	TCHAR szSoft[256];
	if(strlen(szMsg) > 2)
	{
		if (!strncmp(szMsg,"YW",2))
			sprintf (szSoft,"Yamaha WSC-MA2 %c.%c",szMsg[2],szMsg[3]);
		else if (!strncmp(szMsg,"Ya",2))
			sprintf (szSoft,"Yamaha ATS MA2 %c.%c",szMsg[2],szMsg[3]);
		else if (!strncmp(szMsg,"Y7",2))
			sprintf (szSoft,"Yamaha SSC MA2 %c.%c",szMsg[2],szMsg[3]);
		else if (!strncmp(szMsg,"Y9",2))
			sprintf (szSoft,"Yamaha SSC MA3 %c.%c",szMsg[2],szMsg[3]);
		else if (!strncmp(szMsg,"Yv",2))
			sprintf (szSoft,"Yamaha ATS MA5 %c.%c",szMsg[2],szMsg[3]);
		else if (!strncmp(szMsg,"Yj",2))
			sprintf (szSoft,"Yamaha ATS MA7 %c.%c",szMsg[2],szMsg[3]);
		else if (!strncmp(szMsg,"YK",2))
			sprintf (szSoft,"Yamaha WSD %c.%c",szMsg[2],szMsg[3]);
		else if (!strncmp(szMsg,"YA",2))
			sprintf (szSoft,"Yamaha WSC-MA2 1.0a");
		else
		{
			strncpy(szSoft,szMsg,strlen(szMsg)-2);
			szSoft[strlen(szMsg)-2]=0;
		}
		SetInfoText(infoSoftware,szSoft);
	}
}

/*\
 * <---------- DecodeCNTI ---------->
 * @m parse the content information chunk
 * --> I N <-- @p
 * unsigned char **pcBuffer - pointer to input buffer pointer
 * unsigned int nSize - number of bytes in chunk
\*/
void CSMAFFile::DecodeCNTI(unsigned char **pcBuffer,unsigned int nSize)
{
	map<tstring,int> :: const_iterator iterChunk;
	map<tstring,int> mapChunkId;

	mapChunkId[tstring(_T("M2"))]=scidM2;
	mapChunkId[tstring(_T("L2"))]=scidL2;
	mapChunkId[tstring(_T("ST"))]=scidST;
	mapChunkId[tstring(_T("A0"))]=scidA0;
	mapChunkId[tstring(_T("A2"))]=scidA2;
	mapChunkId[tstring(_T("WW"))]=scidWW;
	mapChunkId[tstring(_T("SW"))]=scidSW;
	mapChunkId[tstring(_T("ES"))]=scidES;
	mapChunkId[tstring(_T("RF"))]=scidRF;
	mapChunkId[tstring(_T("VN"))]=scidVN;
	mapChunkId[tstring(_T("GR"))]=scidGR;
	mapChunkId[tstring(_T("MI"))]=scidMI;
	mapChunkId[tstring(_T("CD"))]=scidCD;
	mapChunkId[tstring(_T("UD"))]=scidUD;
	mapChunkId[tstring(_T("CA"))]=scidCA;
	mapChunkId[tstring(_T("CN"))]=scidCN;
	mapChunkId[tstring(_T("CR"))]=scidCR;
	mapChunkId[tstring(_T("AW"))]=scidAW;
	mapChunkId[tstring(_T("AN"))]=scidAN;

	if (m_nFormat < smaffmtMA2)
		m_nFormat=smaffmtMA2;

	if (nSize < 5)
		throw new CFormatException(CFormatException::formaterrInvalidChunkSize,"CNTI chunk size invalid");

	//unknown
	*pcBuffer+=1;

	//content type
	Log2(verbLevDebug2,"content type: %02X\n",(unsigned char)**pcBuffer);
	*pcBuffer+=1;
	
	//character encoding scheme
	m_nEncoding=(unsigned char)**pcBuffer;						
	*pcBuffer+=1;

	//status byte
	m_pcStatusByte=(unsigned char *)*pcBuffer;
	TRACEIT2("status byte decoding...\n");
    DecodeStatusByte(*m_pcStatusByte);
	*pcBuffer+=1;

	//skip 0x00
	*pcBuffer+=1;

	nSize-=5;

	TRACEIT2("entering loop... partition size: %d\n",nSize);
	while (nSize)
	{
		TCHAR szMsg[256];
		tstring sChunk;
		unsigned int nSubChunkSize=0;
		unsigned char *pcAttribute;
		TRACEIT2("size left: %d\n",nSize);
		Log2(verbLevDebug2,"decoding subchunk: %c%c\n",(char)**pcBuffer,(char)*(*pcBuffer+1));
		DecodeCNTISubChunk(pcBuffer,*pcBuffer+nSize,sChunk,&nSubChunkSize,&pcAttribute);
		Log2(verbLevDebug2,"decoded subchunk: %s - size: %d\n",sChunk.c_str(),nSubChunkSize);
		if ((iterChunk=mapChunkId.find(sChunk)) == mapChunkId.end())
		{
			Log2(verbLevErrors,IDS_ERR_UNKSUBCHNK);
			throw new CFormatException(CFormatException::formaterrUnknownSubChunk,"unknown subchunk in CNTI");
		}
		Log2(verbLevDebug2,"sub chunk: %s\n",sChunk.c_str());			
#ifdef _UNICODE
		char szBuffer[256];
		int nCP,len=min(nSubChunkSize,255);
		memcpy (szBuffer,pcAttribute,len);
		szBuffer[len]=0;
		ZeroMemory(szMsg,255*sizeof(TCHAR));
		switch(m_nEncoding)
		{
			case smafencUTF8:		nCP=CP_UTF8;	break;
			default:				nCP=CP_ACP;
		}
		MultiByteToWideChar(nCP,0,(const char *)szBuffer,min(nSubChunkSize,255),szMsg,255); 
#else
		memcpy (szMsg,pcAttribute,min(nSubChunkSize,255));
		unsigned int i,o=0;
		for (i=0;i < min(nSubChunkSize,255);i++)
		{
			if (*(pcAttribute+i) != '\\')
				szMsg[o++]=*(pcAttribute+i);
		}
		szMsg[o]=0;
#endif		
		switch(iterChunk->second)
		{
			case scidCR:	SetInfoText(infoCopyright,szMsg);		break;
			case scidGR:	SetInfoText(infoCopyManaged,szMsg);		break;
			case scidMI:	SetInfoText(infoManagementInfo,szMsg);	break;
			case scidCD:	SetInfoText(infoDateCreated,szMsg);		break;
			case scidUD:	SetInfoText(infoDateRevised,szMsg);		break;
			case scidCN:	SetInfoText(infoCarrier,szMsg);			break;
			case scidSW:	SetInfoText(infoComposer,szMsg);		break;
			case scidCA:	SetInfoText(infoCategory,szMsg);		break;
			case scidWW:	SetInfoText(infoWords,szMsg);			break;
			case scidAW:	SetInfoText(infoArranged,szMsg);		break;
			case scidAN:	SetInfoText(infoArtist,szMsg);			break;
			case scidVN:	SetInfoText(infoVendor,szMsg);			break;
			case scidST:	SetInfoText(infoTitle,szMsg);			break;
			case scidA0:	DecodeAX(szMsg);						break;
		}
		nSize-=nSubChunkSize+4;
	};
}

/*\
 * <---------- CSMAFFile :: Decode ----------> 
 * @m decode the SMAF object in memory
\*/
void CSMAFFile::Decode(void)
{
	bool bDone=false;

	int nChannelOffset=0;
	unsigned char *pLim;
	unsigned char *pSMAF=m_pcSMAF;

	map<tstring,int> :: const_iterator iterChunk;
	map<tstring,int> mapChunkId;

	mapChunkId[tstring(_T("MMMD"))]=cidMMMD;
	mapChunkId[tstring(_T("CNTI"))]=cidCNTI;
	mapChunkId[tstring(_T("OPDA"))]=cidOPDA;
	mapChunkId[tstring(_T("MTR"))]=cidMTR;
	mapChunkId[tstring(_T("MSTR"))]=cidMSTR;
	mapChunkId[tstring(_T("Mtsu"))]=cidMtsu;
	mapChunkId[tstring(_T("Mtsq"))]=cidMtsq;
	mapChunkId[tstring(_T("Mspl"))]=cidMspl;
	mapChunkId[tstring(_T("ATR"))]=cidATR;
	mapChunkId[tstring(_T("Dch"))]=cidDch;
	mapChunkId[tstring(_T("Pro"))]=cidPro;
	mapChunkId[tstring(_T("Atsq"))]=cidAtsq;
	mapChunkId[tstring(_T("GTR"))]=cidGTR;

	Log2(verbLevDebug1,"decoding file...\n");

	m_nFormat=smaffmtUnknown;
	m_nHighestLocatedDeviceId=0;

	if (m_nSize < 4)
		throw new CFormatException(CFormatException::formaterrTruncated);
	m_nCheckSumOffset=m_nSize-2;

	if ((m_pcSMAF[m_nCheckSumOffset] == 0x1D && m_pcSMAF[m_nCheckSumOffset+1] == 0x0F) &&
		(m_pcSMAF[m_nCheckSumOffset-2] != 0x00 && m_pcSMAF[m_nCheckSumOffset-1] != 0x00))
	{
		m_nCheckSumOffset-=2;
	}
	pLim=m_pcSMAF+m_nSize;	
	int nToDo=m_nSize;


	//MMMD
	while (nToDo)
	{
		unsigned int nSize;
		unsigned char cAttribute;
		tstring sChunk;

		TRACEIT2("bytes left to parse: %d\n",nToDo);

		try
		{			
			DecodeChunk(&pSMAF,pLim,sChunk,&nSize,&cAttribute);
		}
		catch(CFormatException *se)
		{
			if (se->nGetExceptionCode() != CFormatException::formaterrInvalidChunkSize)
				throw;
			else
				bDone=true;
		}
		Log2(verbLevDebug2,"decoded chunk: %s - size: %d\n",sChunk.c_str(),nSize);
		if ((iterChunk=mapChunkId.find(sChunk)) == mapChunkId.end())
		{
			Log2(verbLevErrors,IDS_ERR_UNKCHNK);
			throw new CFormatException(CFormatException::formaterrUnknownChunk);
		}
		//TCHAR szText[256];
		CSMAFTrack *pTrack;
		switch(iterChunk->second)
		{
			case cidMMMD:						//MMMD
				if (m_nFormat < smaffmtMA1)		//
					m_nFormat=smaffmtMA1;		//that makes this officially an MA1 already
				nSize=0;
			break;
			case cidCNTI:						//CNTI
				DecodeCNTI(&pSMAF,nSize);
			break;
			case cidGTR:						//GTR
				m_pGraph=new CSMAFGraph(nChannelOffset);
				ASSERT(m_pGraph);
				if (m_pGraph)
				{
					m_bContainsGraphix=true;
					m_pGraph->Decode(&pSMAF,nSize);
					if (m_pParameters && !m_pParameters->m_strParameter[paraStrImageExportPath].empty())
					{
						TRACEIT2("img export path %s\n",m_pParameters->m_strParameter[paraStrImageExportPath].c_str());
						m_pGraph->ExportImages(m_pParameters->m_strParameter[paraStrImageExportPath].c_str());
					}
				}
			break;
			case cidOPDA:						//OPDA
				DecodeOPDA(&pSMAF,nSize);
			break;
			case cidATR:						//ATR
				ASSERT(m_pAudio == NULL);
				m_pAudio=new CSMAFAudio();
				m_bContainsSamples=true;
				ASSERT(m_pAudio);
				if (m_pAudio)
				{
					m_pAudio->Decode(&pSMAF,nSize);
					if (m_pAudio->nGetMaxPlaytime() > m_nSamplePlaytime)
						m_nSamplePlaytime=m_pAudio->nGetMaxPlaytime();
				}
			break;
			case cidMTR:						//MTR
				pTrack=new CSMAFTrack(nChannelOffset);
				ASSERT(pTrack);
				if (pTrack)
				{
					pTrack->Decode(&pSMAF,nSize);
					//m_bContainsSequence=true;
					m_Tracks.push_back(pTrack);
					nChannelOffset+=pTrack->nGetFormat() == CSMAFTrack::formatHandyphone ? 4 : 8;
				}
			break;
			default:
				pSMAF+=nSize;
		}
		nToDo-=nSize+8;

		if (pSMAF >= m_pcSMAF+m_nCheckSumOffset)
			break;
	};
	if (!bVerifyCheckSum())
	{
		Log2(verbLevWarnings,"checksum does not compute - this file will fail to play on some devices\n");
	}

	unsigned int i;
	unsigned int nTime=0,nMaxPlaytime=0;
	for (i=0;i < m_Tracks.size();i++)
	{
		m_Tracks[i]->CheckBankChanges();
		if (m_Tracks[i]->nGetSampleCount())
		{
			m_bContainsSamples=true;
			if (m_nFormat < smaffmtMA3)
				m_nFormat=smaffmtMA3;

			if (m_Tracks[i]->nGetMaxSamplePlaytime() > m_nSamplePlaytime)
				m_nSamplePlaytime=m_Tracks[i]->nGetMaxSamplePlaytime();
		}
		m_nHighestLocatedDeviceId=max(m_nHighestLocatedDeviceId,m_Tracks[i]->nGetHighestLocatedDeviceId());
		if (m_Tracks[i]->bUsesHumanVoice())
			m_bContainsHumanVoice=true;
		if (m_Tracks[i]->bUsesSynthesizer())
			m_bContainsSynthesizer=true;
		if (m_Tracks[i]->nGetEventCount())
		{
			nTime=m_Tracks[i]->nGetPlaytime();
			if (nTime > nMaxPlaytime)
				nMaxPlaytime=nTime;
		}
	}
	if (m_pAudio)
	{
		nTime=m_pAudio->nGetPlaytime();
		if (nTime > nMaxPlaytime)
			nMaxPlaytime=nTime;
	}
	m_nPlaytime=nMaxPlaytime;
	if (m_nHighestLocatedDeviceId <= MIDI_SYSXPID_MA7)
	{
		if (m_nFormat == smaffmtMA3 && m_nHighestLocatedDeviceId > MIDI_SYSXPID_MA3)
		{
			Log2(verbLevWarnings,"device identifier found within the device specific sequence data breaks the MA3 compatibility of the sample data\n");
			m_nFormat = smaffmtMA3 + (m_nHighestLocatedDeviceId-MIDI_SYSXPID_MA3);
		}
		if (m_nFormat == smaffmtMA5 && m_nHighestLocatedDeviceId > MIDI_SYSXPID_MA5)
		{
			Log2(verbLevWarnings,"device identifier found within the device specific sequence data breaks the MA5 compatibility of the sample data\n");
			m_nFormat = smaffmtMA5 + (m_nHighestLocatedDeviceId-MIDI_SYSXPID_MA5);
		}
	}
	else
		Log2(verbLevWarnings,"device identifier found within the device specific sequence data might break the SMAF compatibility entirely\n");
	Log2(verbLevDebug1,"SMAF format id: %d, min device id: %d\n",m_nFormat,m_nHighestLocatedDeviceId);
}

/*\
 * <---------- CSMAFFile :: sGetErrorMsg ----------> 
 * @m 
 * <-- OUT --> @r
 * 
\*/
tstring CSMAFFile::sGetErrorMsg()
{
	return m_strLastError;
}

/*\
 * <---------- CSMAFFile :: sGetEncodingName ----------> 
 * @m 
 * --> I N <-- @p
 * int nEncoding - 
 * <-- OUT --> @r
 * 
\*/
tstring CSMAFFile::sGetEncodingName(int nEncoding)
{
	int nResource=IDS_ENCODING_UNKNOWN;
	CMyString strText;
	switch (nEncoding)
	{
		case smafencShiftJis:
		case smafencLatin1:
		case smafencEucKr:
		case smafencHzGb:
		case smafencBig5:
		case smafencKoi8:
		case smafencTcVn:
			nResource=nEncoding+IDS_ENCODING_BASE;
		break;
		case smafencUTF8:
			nResource=IDS_ENCODING_UTF8;
		break;
	}
	strText.Load(nResource);
	return strText;
}

/*\
 * <---------- CSMAFFile :: sGetFormatName ----------> 
 * @m 
 * --> I N <-- @p
 * int nFormat - 
 * <-- OUT --> @r
 * 
\*/
tstring CSMAFFile::sGetFormatName(int nFormat)
{
	CMyString strText;
	strText.Load(IDS_FORMATNAME_MA1+(nFormat-1));
	return strText;
}

/*\
 * <---------- CSMAFFile :: bVerifyCheckSum ----------> 
 * @m check if the SMAF file object in memory has a valid checksum
 * <-- OUT --> @r
 * bool - true=valid
\*/
bool CSMAFFile::bVerifyCheckSum(void)
{	
	unsigned short int nCrc;
	memcpy(&nCrc,m_pcSMAF+m_nCheckSumOffset,2);
	return nCalcCheckSum(m_pcSMAF,m_nCheckSumOffset) == htons(nCrc);
}

/*\
 * <---------- CSMAFFile :: nCalcCheckSum ----------> 
 * @m generate a SMAF checksum
 * --> I N <-- @p
 * unsigned char *buf - pointer to input data
 * unsigned nbytes    - number of bytes in input
 * <-- OUT --> @r
 * unsigned short int - checksum value
\*/
unsigned short int CSMAFFile::nCalcCheckSum(unsigned char *buf, unsigned nbytes)
{
    unsigned char *p, *lim;
	unsigned short int nCRC=0xFFFF;
	unsigned short int iLookup;

    p = (unsigned char *)buf;
    lim = p + nbytes;
    while (p < lim)
    {
		iLookup=( (nCRC>>8) & 0xFF) ^ (unsigned short int)*(p++);
		nCRC=m_wCRCTable[iLookup] ^ ((nCRC & 0xFF) << 8);
	};
    return nCRC ^ 0xFFFF;
}

/*\
 * <---------- CSMAFFile :: Read ----------> 
 * @m read, parse and decode a file from the input stream
 * --> I N <-- @p
 * std::istream &ar - input stream object reference
\*/
void CSMAFFile::Read(std::istream &ar)
{
	bool bRet=false;
	int i;
	if (m_pcSMAF)
		delete [] m_pcSMAF;
	ar.seekg(0,ios_base::end);
	m_nSize=(int)ar.tellg();
	ar.seekg(0,ios_base::beg);
	if (m_nSize)
	{
		m_pcSMAF=new unsigned char[m_nSize];
		if (m_pcSMAF)
		{
			ar.read((char *)m_pcSMAF,m_nSize);
			Decode();
			bRet=true;
		}
	}
	CSMAFSample *pLargestAt=NULL;
	uint32_t nLargestSize=0;
	for (i=0;i < (int)m_Tracks.size();i++)
	{
		if (m_Tracks[i]->nGetSampleCount())
		{
			uint32_t o;
			for (o=0;o < m_Tracks[i]->nGetSampleCount();o++)
			{
				CSMAFSample *pSample=m_Tracks[i]->pGetSample(o);
				if (pSample && nLargestSize < pSample->nGetRawSize())
				{
					nLargestSize=pSample->nGetRawSize();
					pLargestAt=pSample;
				}
			}
		}
	}
	if (pLargestAt != NULL)
	{
		m_nCSBitsPerSample=16;
		m_nCSChannels=pLargestAt->nGetChannels();
		m_nCSSamplesPerSecond=pLargestAt->nGetSamplesPerSecond();
		m_nCSSize=pLargestAt->nGetRawSize();
		m_pcCSBuffer=pLargestAt->pcGetRawSample();
		m_bSampleFromTracks=true;
	}
	if (m_pAudio != NULL && m_pAudio->nGetSampleCount())
	{
		m_pAudio->nGetMaxPlaytime(&i);
		CSMAFSample *pSample=m_pAudio->pGetSample(i);
		if (pSample && pSample->pcGetRawSample())
		{
			if (pSample->nGetRawSize() > nLargestSize)
			{
				nLargestSize=pSample->nGetRawSize();
				m_nCSBitsPerSample=16;
				m_nCSChannels=pSample->nGetChannels();
				m_nCSSamplesPerSecond=pSample->nGetSamplesPerSecond();
				m_nCSSize=pSample->nGetRawSize();
				m_pcCSBuffer=pSample->pcGetRawSample();
			}
		}
		else
		{
			TRACEIT2("sample exists but appears to be empty!\n");
		}
	}
	if (m_nCSChannels > 1 && m_nFormat < smaffmtMA5)
	{
		m_nFormat=smaffmtMA5;
		if (m_nCSSamplesPerSecond > 12000 && m_nFormat < smaffmtMA7)
			m_nFormat=smaffmtMA7;
	}
	if (m_nCSChannels == 1)
	{
		if (m_nCSSamplesPerSecond > 8000)
		{
			if (m_nFormat < smaffmtMA3)
				m_nFormat=smaffmtMA3;
			if (m_nCSSamplesPerSecond > 16000)
			{
				if (m_nFormat < smaffmtMA5)
					m_nFormat=smaffmtMA5;
				if (m_nCSSamplesPerSecond > 24000)
				{
					if (m_nFormat < smaffmtMA7)
						m_nFormat=smaffmtMA7;
				}
			}
		}
	}
}

/*\
 * <---------- CSMAFFile :: Write ----------> 
 * @m render and write a file from the previously set source sample
 * --> I N <-- @p
 * ostream &out - output stream object reference
\*/
void CSMAFFile::Write(ostream &out)
{
	int nFormat=smaffmtMA2;
	
	Log2(verbLevDebug2,"bits per sample: %d, sample rate: %dHz, channels: %d\n",m_pCSSource->m_nCSBitsPerSample,m_pCSSource->m_nCSSamplesPerSecond,m_pCSSource->m_nCSChannels);
	
	if (m_pCSSource->m_nCSChannels > 1)
	{
		nFormat=smaffmtMA5;
		if (m_pCSSource->m_nCSSamplesPerSecond > 12000)
			nFormat=smaffmtMA7;
	}
	else
	{
		if ((m_pCSSource->m_nCSSamplesPerSecond != 4000 && m_pCSSource->m_nCSSamplesPerSecond != 8000) || m_pParameters->m_bParameter[paraBoolStreamSamples])
		{
			nFormat=smaffmtMA3;
			if (m_pCSSource->m_nCSSamplesPerSecond > 16000)
			{
				nFormat=smaffmtMA5;
				if (m_pCSSource->m_nCSSamplesPerSecond > 24000)
					nFormat=smaffmtMA7;
			}
		}
	}

	if (nFormat == smaffmtMA2)
		RenderDestinationStatic(out,m_pCSSource);
	else
		RenderDestinationStreaming(out,m_pCSSource,nFormat);
}

/*\
 * <---------- CSMAFFile :: nRenderCNTI ----------> 
 * @m render content information chunk
 * --> I N <-- @p
 * CMobileSampleContent *pSource - source sample object
 * unsigned char *pDest          - output buffer
 * <-- OUT --> @r
 * int - size of the freshly rendered chunk
\*/
int CSMAFFile::nRenderCNTI(CMobileSampleContent *pSource,unsigned char *pDest)
{
	typedef struct 
	{
		int nStringIndex;
		const char *pszTag;
		int nInfoIndex;
	}CNTIMap;

	CNTIMap mmap[]=
	{
		{	paraStrTitle,		"ST",	infoTitle	},
		{	paraStrArtist,		"AN",   infoArtist	},
		{	paraStrComposer,	"SW",	infoComposer},
		{	paraStrCopyright,	"CR",	infoCopyright},
		{	paraStrManagedBy,	"GR",	infoCopyManaged},
		{	paraStrManagement,	"MI",	infoManagementInfo},
		{	paraStrDateCreated,	"CD",	infoDateCreated},
		{	paraStrDateRevised,	"UD",	infoDateRevised},
		{	paraStrCarrier,		"CN",	infoCarrier},
		{	paraStrCategory,	"CA",	infoCategory},
		{	paraStrWriter,		"WW",	infoWords},
		{	paraStrArranger,	"AW",	infoArranged},
		{	paraStrVendor,		"VN",	infoVendor},
		{	0,					NULL	}
	};
	int i,nUsed=0;
	tstring sParameter;
	for (i=0;mmap[i].pszTag != NULL;i++)
	{
		sParameter=pSource->m_strInfo[mmap[i].nInfoIndex];
		if (!sParameter.empty())
		{
			if (pDest)
			{
				memcpy(pDest,mmap[i].pszTag,2);
				pDest+=2;
				*(pDest++)=':';
				memcpy(pDest,sParameter.c_str(),(int)sParameter.length());
				pDest+=(int)sParameter.length();
				*(pDest++)=',';
			}
			nUsed+=(int)sParameter.length()+4;
		}
		else
		{
			Log2(verbLevWarnings,"this format does not support encoding of metadata type %d %d\n",i,mmap[i].nInfoIndex);
		}
	}
	return nUsed;
}

/*\
 * <---------- CSMAFFile :: nRenderOPDA ----------> 
 * @m 
 * --> I N <-- @p
 * CMobileSampleContent *pSource - 
 * unsigned char *pDest          - 
 * <-- OUT --> @r
 * 
\*/
int CSMAFFile::nRenderOPDA(CMobileSampleContent *pSource,unsigned char *pDest)
{
	unsigned short int nSMAFValue;
	uint32_t dwSMAFValue;

	int nDataSize=33;
	const unsigned char pcOPDAData[53]={'D','c','h',0xFF,0x00,0x00,0x00,0x19,
										'E','S',0x00,0x01,0xF8,
										//'A','0',0x00,0x06,'Y','3','3','1','0','0',
										'A','0',0x00,0x06,'Y','K','1','1','0','0',
										//'A','2',0x00,0x06,'Y','3','3','1','0','0',
										'A','2',0x00,0x06,'Y','K','1','1','0','0',
	//AO starts here
										'P','r','o',0x05, 0x00,0x00,0x00,0x0C,
										0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0xFF,0xFF,0xFF,0xFF};

	typedef struct 
	{
		int nStringIndex;
		const char *pszTag;
		int nInfoIndex;
	}CNTIMap;

	CNTIMap mmap[]=
	{
		{	paraStrTitle,		"ST",	infoTitle	},
		{	paraStrArtist,		"AN",   infoArtist	},
		{	paraStrComposer,	"SW",	infoComposer},
		{	paraStrCopyright,	"CR",	infoCopyright},
		{	paraStrManagedBy,	"GR",	infoCopyManaged},
		{	paraStrManagement,	"MI",	infoManagementInfo},
		{	paraStrDateCreated,	"CD",	infoDateCreated},
		{	paraStrDateRevised,	"UD",	infoDateRevised},
		{	paraStrCarrier,		"CN",	infoCarrier},
		{	paraStrCategory,	"CA",	infoCategory},
		{	paraStrWriter,		"WW",	infoWords},
		{	paraStrArranger,	"AW",	infoArranged},
		{	paraStrVendor,		"VN",	infoVendor},
		{	0,					NULL	}
	};
	int i,nUsed=0,nMeta=0;
	tstring sParameter;
	
	if (m_pParameters->m_bParameter[paraBoolProTag])
		nDataSize+=20;

	//find out how much metadata we will encode
	for (i=0;mmap[i].pszTag != NULL;i++)
	{
		sParameter=pSource->m_strInfo[mmap[i].nInfoIndex];
		if (!sParameter.empty())
			nMeta+=(int)sParameter.length()+4;
	}
	if (!m_pParameters->m_bSwitch[paraSwitchNoMeta])
	{
		//any metadata given?
		if (nMeta)
		{	//yes->encode a Dch tag with it
			dwSMAFValue=htonl(nMeta);
			if (pDest)
			{
				memcpy(pDest,"Dch\001",4);
				pDest+=4;
				memcpy(pDest,&dwSMAFValue,4);
				pDest+=4;
			}
			nUsed+=8;
		}
		//encode metadata
		for (i=0;mmap[i].pszTag != NULL;i++)
		{
			sParameter=pSource->m_strInfo[mmap[i].nInfoIndex];
			if (!sParameter.empty())
			{
				if (pDest)
				{
					memcpy(pDest,mmap[i].pszTag,2);
					pDest+=2;
					nSMAFValue=htons((short int)sParameter.length());
					memcpy(pDest,&nSMAFValue,sizeof(unsigned short));
					pDest+=sizeof(unsigned short);
					memcpy(pDest,sParameter.c_str(),(int)sParameter.length());
					pDest+=(int)sParameter.length();
				}
				nUsed+=(int)sParameter.length()+4;
			}
		}
	}
	if (pDest)
	{
		memcpy(pDest,pcOPDAData,nDataSize);
		pDest+=nDataSize;
		//did we add the Pro5-Tag?
		if (m_pParameters->m_bParameter[paraBoolProTag])
		{	//yes->render playtime
			int nPlaytime=pSource->nGetSamplePlaytime()/4;
			if (pSource->nGetSamplePlaytime()/4)
				++nPlaytime;
			pDest-=4;
			dwSMAFValue=htonl(nPlaytime);
			memcpy(pDest,&dwSMAFValue,sizeof(unsigned long));
		}
	}
	nUsed+=nDataSize;
	return nUsed;
}

/*\
 * <---------- CSMAFFile :: RenderDestinationStatic ----------> 
 * @m 
 * --> I N <-- @p
 * ostream &out                  - 
 * CMobileSampleContent *pSource - 
\*/
void CSMAFFile::RenderDestinationStatic(ostream &out,CMobileSampleContent *pSource)
{
	bool bRet=false;
	int nOffset=0;
	unsigned char *pSMAF;
	unsigned char *pDest;

	unsigned char *pA2Start;
	uint32_t nA2Len;
	unsigned char *pA0Start;
	uint32_t nA0Len;

	unsigned char *pA0Entry;
	unsigned char *pA2Entry;

	Log2(verbLevDebug1,"using non-streaming pcm format\n");
	if (pSource->m_nCSChannels > 1)
	{
		Log2(verbLevErrors,"channel count incompatible\n");
		throw new CFormatException(CFormatException::formaterrIncompatibleSampleFormat,"channel count incompatible");
	}
	if (pSource->m_nCSBitsPerSample != 16)
	{
		Log2(verbLevErrors,"sample width incompatible\n");
		throw new CFormatException(CFormatException::formaterrIncompatibleSampleFormat,"sample width incompatible");
	}

	if (pSource->m_nCSSamplesPerSecond != 8000 && 
		pSource->m_nCSSamplesPerSecond != 4000 && 
		pSource->m_nCSSamplesPerSecond != 11025 &&
		pSource->m_nCSSamplesPerSecond != 22050 &&
		pSource->m_nCSSamplesPerSecond != 44100)
	{
		Log2(verbLevErrors,"sample rate incompatible\n");
		throw new CFormatException(CFormatException::formaterrIncompatibleSampleFormat,"sample rate incompatible");
	}

	m_bStatusCopy=m_pParameters->m_bParameter[paraBoolTransferEnabled];
	m_bStatusSave=m_pParameters->m_bParameter[paraBoolSaveEnabled];
	m_bStatusEdit=m_pParameters->m_bParameter[paraBoolEditEnabled];

	int nATRSizeOffset;
	uint32_t nCNTISize;
	uint32_t nSMAFSize;
	uint32_t nSmafValue;
	uint32_t nATRSize;
	uint32_t nADPCMSize;
	uint32_t nATSQSize;
	unsigned short nSmafShortValue;
	int nPlaytimeSize1,nPlaytimeSize2;
	int nMicroSecs=((uint64_t)500*pSource->m_nCSSize)/pSource->m_nCSSamplesPerSecond;
	int nPlaytime=((((nMicroSecs/4)+1)*4)+2)/4;
	char cPlaytime1[256],cPlaytime2[256];
	const unsigned char pcCNTIData[5][38]=
	{
		{	0x00, 'M', '2', ':', 0x00, ','},							//0x00 "M2:" 0x00,

		{	'A','0',':','Y','W','1','2','0','0',',',		//"A0:",TOOL-CODE, CHECKSUM CONTENT 1 ","
			'A','2',':','Y','W','1','2','0','0',',',		//"A2:",TOOL-CODE, CHECKSUM CONTENT 2 ","
/*
HERE IS WHERE A2 CHECKSUM CALC ENDS
HERE IS WHERE A0 CHECKSUM CALC STARTS 
*/

			'A', 'T', 'R',0x00	},										//"ATR" 0x00

			//default checksum 1: YA1100
			//default checksum 2: YA1100

		{	0x02, 0x02, 0x41, 0x73, 0x70, 0x49, 0x00, 0x00, 0x00, 0x10,		//
			0x73, 0x74, 0x3A, 0x00, 0x00, 0x00, 0x00, 0x2C, 0x73, 0x70,		//
			0x3A, 0x00, 0x00, 0x00, 0x0C, 0x2C, 0x41, 0x74, 0x73, 0x71 	},	//
		
		{	0x01, 0x00, 0x37, 0x7F, 0x01, 0x01	},							//
		
		{	0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x41, 0x77, 0x61, 0x01}		//

	};

	//calc smaf size
	nCNTISize=nRenderCNTI(pSource)+30;
	nADPCMSize=pSource->m_nCSSize/4;
	if (pSource->m_nCSSize%4)
		++nADPCMSize;
	nPlaytimeSize1=CSMAFTrack::nEncodeVariableQuantity(nPlaytime,(unsigned char *)cPlaytime1);
	nPlaytimeSize2=CSMAFTrack::nEncodeVariableQuantity(nPlaytime+1,(unsigned char *)cPlaytime2);

	nATSQSize=nPlaytimeSize1+nPlaytimeSize2+12;
	TRACEIT2("estimated sample size = %d\n",nADPCMSize);
	nATRSizeOffset=16+nCNTISize+10+24;
	nATRSize=nADPCMSize+nATSQSize+46;

	nSMAFSize=nCNTISize+18+nATRSize;
	Log2(verbLevDebug3,"estimated smaf size = %d\n",nSMAFSize+8);

	pSMAF=new unsigned char [nSMAFSize+8];
	pDest=pSMAF;

	memcpy(pDest,"MMMD",4);
	pDest+=4;
	nSmafValue=htonl(nSMAFSize);
	memcpy(pDest,&nSmafValue,4);
	pDest+=4;

	//CHECKSUM A2 STARTS HERE
	pA2Start=pDest;

	memcpy(pDest,"CNTI",4);
	pDest+=4;
	nSmafValue=htonl(nCNTISize);
	memcpy(pDest,&nSmafValue,4);
	pDest+=4;
	*(pDest++)=0x00;								//
	*(pDest++)=0x01;								//
	*(pDest++)=0x23;								//UTF8  0x01!
	*(pDest++)=EncodeStatusByte(0x00);				//Status Byte
	memcpy (pDest,(const char *)pcCNTIData[0],6);	//M2-prefix
	pDest+=6;										//
	pDest+=nRenderCNTI(pSource,pDest);				//CNTI / Metadata
	memcpy (pDest,(const char *)pcCNTIData[1],24);	//
	pA0Entry=pDest+7;
	pA2Entry=pDest+17;
	pDest+=24;
	pA0Start=pDest-4;
	nA2Len=(unsigned long)(pA0Start-pA2Start);

//	printf ("A2 len= 0x%02X\n",nA2Len);

	Log2(verbLevDebug1,"rendering SMAF sequence...\n");

	nSmafValue=htonl(nATRSize);
	memcpy(pDest,&nSmafValue,4);					//ATR size
	pDest+=4;
	*(pDest++)=0x00;								//
	*(pDest++)=0x00;																		//
	*(pDest++)=0x10|CSMAFAudio::cEncodeSampleRate(pSource->m_nCSSamplesPerSecond);			//
	*(pDest++)=0x00;																		//
	memcpy(pDest,(const char *)pcCNTIData[2],30);	//
	pDest+=30;
	nSmafValue=htonl(nATSQSize);
	memcpy(pDest,&nSmafValue,4);					//ATSQ size
	pDest+=4;
	memcpy(pDest,(const char *)pcCNTIData[3],6);	//ATSQ data
	pDest+=6;
	memcpy (pDest,cPlaytime1,nPlaytimeSize1);
	pDest+=nPlaytimeSize1;
	memcpy (pDest,cPlaytime2,nPlaytimeSize2);
	pDest+=nPlaytimeSize1;
	memcpy(pDest,(const char *)pcCNTIData[4],10);	//ATSQ trailer + AWA head
	pDest+=10;
	nSmafValue=htonl(nADPCMSize);
	memcpy(pDest,&nSmafValue,4);					//AWA size
	pDest+=4;

	Log2(verbLevDebug1,"compressing sample data...\n");

	CSMAFSample sample(pSource->m_nCSSamplesPerSecond, pSource->m_nCSChannels);
	sample.nEncode((short int *)pSource->m_pcCSBuffer,pSource->m_nCSSize,(char *)pDest);
	pDest+=nADPCMSize;

	nSmafShortValue=nRenderAXCheckSum((unsigned char *)pA2Start,nA2Len);
//	printf("A2 CRC: %04X\n",nSmafShortValue);
	nSmafShortValue=htons(nSmafShortValue);
	memcpy(pA2Entry,&nSmafShortValue,2);

	nA0Len=(unsigned long)(pDest-pA0Start);
	nSmafShortValue=nRenderAXCheckSum((unsigned char *)pA0Start,nA0Len);

//	printf("A0 CRC: %04X\n",nSmafShortValue);
	nSmafShortValue=htons(nSmafShortValue);
	memcpy(pA0Entry,&nSmafShortValue,2);

	nSmafShortValue=nCalcCheckSum((unsigned char *)pSMAF,(unsigned long)(pDest-pSMAF));
	nSmafShortValue=htons(nSmafShortValue);
	memcpy(pDest,&nSmafShortValue,2);					//CRC
	pDest+=2;

	out.write((const char *)pSMAF,(unsigned long)(pDest-pSMAF));

	m_nCSBitsPerSample=4;
	m_nCSSamplesPerSecond=pSource->m_nCSSamplesPerSecond;
	m_nCSChannels=pSource->m_nCSChannels;
	m_nCSSize=nADPCMSize;
	m_nFileSize=out.tellp();
}

/*\
 * <---------- CSMAFFile :: RenderDestinationStreaming ----------> 
 * @m render streaming sample SMAF file to the ouput stream
 * --> I N <-- @p
 * ostream &out                  - reference to the output stream object
 * CMobileSampleContent *pSource - pointer to the source sample object
 * int nFormat                   - chosen destination format
\*/
void CSMAFFile::RenderDestinationStreaming(ostream &out,CMobileSampleContent *pSource,int nFormat)
{
	bool bRet=false;
	int nOffset=0;
	unsigned char cFormatValue;
	unsigned char *pSMAF;
	unsigned char *pDest;
	uint32_t nA0Len,nA2Len;
	unsigned char *pA0Start,*pA0Entry;
	unsigned char *pA2Start,*pA2Entry;

	Log2(verbLevDebug1,"using streaming pcm format %d\n",nFormat);

	if (pSource->m_nCSChannels > 2)
	{
		Log2(verbLevErrors,"channel count incompatible\n");
		throw new CFormatException(CFormatException::formaterrIncompatibleSampleFormat,"channel count incompatible");
	}
	if (pSource->m_nCSBitsPerSample != 16)
	{
		Log2(verbLevErrors,"sample width incompatible\n");
		throw new CFormatException(CFormatException::formaterrIncompatibleSampleFormat,"sample width incompatible");
	}
	if (pSource->m_nCSSamplesPerSecond < 4000 || pSource->m_nCSSamplesPerSecond > 44100)
	{
		Log2(verbLevErrors,"sample rate incompatible\n");
		throw new CFormatException(CFormatException::formaterrIncompatibleSampleFormat,"sample rate incompatible");
	}

	m_bStatusCopy=m_pParameters->m_bParameter[paraBoolTransferEnabled];
	m_bStatusSave=m_pParameters->m_bParameter[paraBoolSaveEnabled];
	m_bStatusEdit=m_pParameters->m_bParameter[paraBoolEditEnabled];

	uint32_t nCNTISize,nOPDASize,nTrackSize,nMTSUSize,nMTSQSize;
	uint32_t nSMAFSize;
	uint32_t nSysexSize;
	uint32_t nSmafValue;
	uint32_t nADPCMSize;
	unsigned short nSmafShortValue;
	int nPlaytimeSize1,nPlaytimeSize2;
	int nMicroSecs=((uint64_t)500*pSource->m_nCSSize)/(pSource->m_nCSSamplesPerSecond*pSource->m_nCSChannels);
	int nPlaytime=((((nMicroSecs/4)+1)*4)+2)/4;
	nPlaytime+=127;
	char cPlaytime1[256],cPlaytime2[256];
	unsigned char nDeviceId=0x06;
	const unsigned char pcMTRData[16]={	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	//const unsigned char pcMTRData[16]={	0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40};

	const unsigned char pcMTSQData[15]={0x00,0xB0,0x00,0x7D,											//bank change high (to 32000)
										0x00,0xB0,0x20,0x00,											//bank change low
										0x00,0xC0,0x00,													//program change (to 000)
										0x00,0x90,0x00,0x7F};											//note on with velocity (note 0, velocity 127)


	const unsigned char pcsysxPanning[4]={				0x0B, 0xFF, 0x00, 0xFF	};


	switch(nFormat)
	{
		case smaffmtMA5:
			nDeviceId=MIDI_SYSXPID_MA5;
			m_pParameters->m_bParameter[paraBoolProTag]=true;
		break;
		case smaffmtMA7:
			nDeviceId=MIDI_SYSXPID_MA7;
			m_pParameters->m_bParameter[paraBoolProTag]=true;
		break;
		default:
			nDeviceId=MIDI_SYSXPID_MA3;
	}
	//calc smaf size
	nCNTISize=5;
	nOPDASize=nRenderOPDA(pSource);
	nTrackSize=16;

	nMTSQSize=15;
	nSysexSize=0;
	nADPCMSize=pSource->m_nCSSize/4;
	if (pSource->m_nCSSize%4)
		++nADPCMSize;
	nMTSUSize=0;

	if (nDeviceId == MIDI_SYSXPID_MA3)
		nMTSUSize+=CSMAFTrack::nRenderSetupResetSysEx(NULL,nDeviceId);
	nMTSUSize+=CSMAFTrack::nRenderSetupChnReserveSysEx(	NULL,	nDeviceId, 1);
	nSysexSize+=CSMAFTrack::nRenderMainVolumeSysEx(	NULL,nDeviceId,0x7F)+1;
	//if (nDeviceId > MIDI_SYSXPID_MA3)
	//	nSysexSize+=CSMAFTrack::nRenderSetupPanningSysEx((NULL,nDeviceId,0x40)+1;

	nPlaytimeSize1=CSMAFTrack::nEncodeVariableQuantity(nPlaytime+1,(unsigned char *)cPlaytime1);
	nPlaytimeSize2=CSMAFTrack::nEncodeVariableQuantity(nPlaytime+1,(unsigned char *)cPlaytime2);
	TRACEIT2("estimated sample size = %d\n",nADPCMSize);

	nSMAFSize=nCNTISize+nOPDASize+nTrackSize+(nMTSQSize+nSysexSize+3+nPlaytimeSize1+nPlaytimeSize1)+(nADPCMSize+nMTSUSize+nSysexSize+nMTSQSize)+40;
	Log2(verbLevDebug3,"estimated smaf size = %d\n",nSMAFSize+8);

	pSMAF=new unsigned char [nSMAFSize+8];
	pDest=pSMAF;

	memcpy(pDest,"MMMD",4);
	pDest+=4;
	nSmafValue=htonl(nSMAFSize);
	memcpy(pDest,&nSmafValue,4);
	pDest+=4;
	
	pA2Start=pDest;

	memcpy(pDest,"CNTI",4);
	pDest+=4;
	nSmafValue=htonl(nCNTISize);
	memcpy(pDest,&nSmafValue,4);
	pDest+=4;
	*(pDest++)=0x00;									//unknown
	*(pDest++)=nFormat >= smaffmtMA5 ? 0x34 : 0x32;		//content type
	*(pDest++)=0x01;									//UTF8
	*(pDest++)=EncodeStatusByte(0xF8);					//Status Byte
	*(pDest++)=0x00;									//
	
	memcpy(pDest,"OPDA",4);
	pDest+=4;

	nSmafValue=htonl(nOPDASize);
	memcpy(pDest,&nSmafValue,4);
	pDest+=4;
	pDest+=nRenderOPDA(pSource,pDest);

	int nShifter=0;
	if (m_pParameters->m_bParameter[paraBoolProTag])
		nShifter+=20;
	pA0Entry=pDest-(12+nShifter);
	pA2Entry=pDest-(2+nShifter);

	nA2Len=(unsigned long)(pDest-pA2Start);
	pA0Start=pDest;

	memcpy(pDest,"MTR",3);
	pDest+=3;
	*(pDest++) = nFormat >= smaffmtMA5 ?  0x06 : 0x05;	
	nSmafValue=htonl(	nTrackSize+
						nMTSUSize+
						nMTSQSize+
						nSysexSize+
						3+
						nPlaytimeSize1+
						nPlaytimeSize1+
						nADPCMSize+
						39);
	memcpy(pDest,&nSmafValue,4);
	pDest+=4;
	*(pDest++)=CSMAFTrack::formatMobileUncompressed;		//format type
	*(pDest++)=0x00;										//sequence type
	*(pDest++)=0x02;										//timebase duration
	*(pDest++)=0x02;										//timebase gate
	memcpy (pDest,(const char *)pcMTRData,nTrackSize);
	pDest+=nTrackSize;
	
	memcpy(pDest,"Mtsu",4);
	pDest+=4;
	nSmafValue=htonl(nMTSUSize);
	memcpy(pDest,&nSmafValue,4);
	pDest+=4;
	if (nDeviceId == MIDI_SYSXPID_MA3)
		pDest+=CSMAFTrack::nRenderSetupResetSysEx((char *)pDest,nDeviceId);
	pDest+=CSMAFTrack::nRenderSetupChnReserveSysEx((char *)pDest,nDeviceId,pSource->m_nCSChannels);
	//if (nDeviceId > MIDI_SYSXPID_MA3)
	//	pDest+=CSMAFTrack::nRenderSetupPanningSysEx((char *)pDest,nDeviceId,0x40);
	
	memcpy(pDest,"Mtsq",4);												//start of sequence data block
	pDest+=4;															//
	nSmafValue=htonl(nMTSQSize+nSysexSize+3+nPlaytimeSize1+nPlaytimeSize1);	//calc size of sequence data block
	memcpy(pDest,&nSmafValue,4);										//patch sequence data size
	pDest+=4;				
	
	*(pDest++)=0x00;																				//
	pDest+=CSMAFTrack::nRenderMainVolumeSysEx((char *)pDest,nDeviceId,0x7F);						//
	*(pDest-2)=0x7F;																				//full volume
	memcpy (pDest,(const char *)pcMTSQData,nMTSQSize);					//copy default sequence data
	pDest+=nMTSQSize;													//
	memcpy (pDest,(const char *)cPlaytime1,nPlaytimeSize1);				//append playtime of the sample
	pDest+=nPlaytimeSize1;												//
	memcpy (pDest,(const char *)cPlaytime1,nPlaytimeSize1);				//append delta until next sequence event is processed
	pDest+=nPlaytimeSize1;												//
	*(pDest++)=0xFF;													//terminate sequence
	*(pDest++)=0x2F;													//
	*(pDest++)=0x00;													//

	memcpy(pDest,"Mtsp",4);
	pDest+=4;
	nSmafValue=htonl(nADPCMSize+11);									//
	memcpy(pDest,&nSmafValue,4);
	pDest+=4;

	memcpy(pDest,"Mwa\001",4);
	pDest+=4;
	nSmafValue=htonl(nADPCMSize+3);
	memcpy(pDest,&nSmafValue,4);
	pDest+=4;

	cFormatValue=0x20;													//apdcm, 4bit/sample
	if (pSource->m_nCSChannels == 2)									//
		cFormatValue|=0x80;												//
	*(pDest++)=cFormatValue;											//
	nSmafShortValue=htons(pSource->m_nCSSamplesPerSecond);				//
	memcpy(pDest,&nSmafShortValue,sizeof(unsigned short int));			//
	pDest+=sizeof(unsigned short int);									//

	CSMAFSample sample(pSource->m_nCSSamplesPerSecond, pSource->m_nCSChannels);
	int nEncodedSize=sample.nEncode((short int *)pSource->m_pcCSBuffer,pSource->m_nCSSize,(char *)pDest);
	ASSERT((unsigned int)nEncodedSize <= nADPCMSize);
	if ((unsigned int)nEncodedSize < nADPCMSize)
	{
		TRACEIT2("padding with %d bytes of nothingness\n",nADPCMSize-nEncodedSize);
	}
	pDest+=nADPCMSize;
	
	nSmafShortValue=nRenderAXCheckSum((unsigned char *)pA2Start,nA2Len);
	TRACEIT2("A2 CRC: %04X\n",nSmafShortValue);
	nSmafShortValue=htons(nSmafShortValue);
	memcpy(pA2Entry,&nSmafShortValue,2);

	nA0Len=(unsigned long)(pDest-pA0Start);
	nSmafShortValue=nRenderAXCheckSum((unsigned char *)pA0Start,nA0Len);
	TRACEIT2("A0 CRC: %04X\n",nSmafShortValue);
	nSmafShortValue=htons(nSmafShortValue);
	memcpy(pA0Entry,&nSmafShortValue,2);

	nSmafShortValue=nCalcCheckSum((unsigned char *)pSMAF,(unsigned long)(pDest-pSMAF));
	nSmafShortValue=htons(nSmafShortValue);
	memcpy(pDest,&nSmafShortValue,2);				//CRC
	pDest+=2;

	out.write((const char *)pSMAF,(unsigned long)(pDest-pSMAF));
	m_nCSBitsPerSample=4;
	m_nCSSamplesPerSecond=pSource->m_nCSSamplesPerSecond;
	m_nCSChannels=pSource->m_nCSChannels;
	m_nCSSize=nADPCMSize;
	m_nFileSize=out.tellp();
}

/*\
 * <---------- CSMAFFile :: Encode ----------> 
 * @m patch a smaf file's status byte and recalc the trailing crc
 * NOTE: doesnt seem to make sense as it lacks of the updates of the other 
 * two checksums within a SMAF - it is dirty, but it actually works as the two
 * other checksums are hardly checked by anything - actually, no free software
 * does, no known handset does. Rumors say that the Yamaha silver-box software
 * would actually verify those.
\*/
void CSMAFFile::Encode()
{
	unsigned char cByte;
    unsigned short int nCrc;
	
	if (m_nSize <= 0x13)
		throw new CFormatException(CFormatException::formaterrInvalid);

	//patch status byte
    cByte=m_pcSMAF[0x13]&0xF8;
	if (m_bStatusCopy)
		cByte|=0x01;
	if (m_bStatusSave)
		cByte|=0x02;
	if (m_bStatusEdit)
		cByte|=0x04;
	m_pcSMAF[0x13]=cByte;

	//patch crc
	nCrc=nCalcCheckSum(m_pcSMAF,m_nCheckSumOffset);
	m_pcSMAF[m_nCheckSumOffset]=(nCrc>>8)&0xFF;
	m_pcSMAF[m_nCheckSumOffset+1]=nCrc&0xFF;
}

/*\
 * <---------- CSMAFFile :: nRenderAXCheckSum ----------> 
 * @m 
 * --> I N <-- @p
 * unsigned char *pcSource - 
 * uint32_tnLen      - 
 * <-- OUT --> @r
 * unsigned short int -
\*/
unsigned short int CSMAFFile::nRenderAXCheckSum(unsigned char *pcSource, uint32_t nLen)
{
	unsigned short int nRet;
	unsigned char cHigh,cLow;
	//voodoo magic
	nRet=nCalcCheckSum(pcSource,nLen) ^ 0x4D32;
	cHigh=(nRet >> 8) & 0x0F;
	cLow=nRet & 0x0F;
	if (cLow < 0x0A)
		cLow+=0x30;
	else
		cLow+=0x37;
	if (cHigh < 0x0A)
		cHigh+=0x30;
	else
		cHigh+=0x37;
	nRet=((unsigned short int)cHigh<<8) | (unsigned short int)cLow;
	return nRet;
}
