/*\
 * SISFile.cpp
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
#ifdef WIN32
#include <io.h>
#include <direct.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <map>
#include <stdlib.h>
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/FIDProperty.h"
#include "ThemeBaseContent.h"
#include "unzip/unzip.h"
#include "zlib.h"
#include "ZIPArchive.h"
#include "SISFile.h"
#include "SISProperty.h"
#include "zlib.h"

DYNIMPPROPERTY(CSISFile,CSISProperty)


CSISFile::CSISFile(void)
{
	m_sFormatName="EPOC Installer";
}

CSISFile::~CSISFile(void)
{
}

/*\
 * <---------- sGetNarrowString ---------->
 * @m 
 * --> I N <-- @p
 * unsigned char *pcWide - 
 * <-- OUT --> @r
 * tstring - 
\*/
tstring CSISFile::sGetNarrowString(unsigned char *pcWide)
{
	unsigned nLen=0;
	tstring sRet="";

	while (*pcWide)
	{
		if (++nLen > 1024)
		{
			Log2(verbLevErrors,"string exceeds size limit\n");
			break;
		}
		sRet+=*pcWide;
		pcWide+=2;
	};
	return sRet;
}

/*\
 * <---------- read ---------->
 * @m 
 * --> I N <-- @p
 * istream &ar - 
\*/
void CSISFile::Read(istream &ar)
{
	m_nSubFormat=0;
	m_nEPOCVersion=0;
	m_bUnicode=false;
	m_bContainsApplication=false;
	m_bContainsConfig=false;
	m_bContainsOption=false;
	m_bContainsPatch=false;
	m_bContainsSystem=false;
	m_bContainsUpgrade=false;
	m_iAutoFile=0;
	m_iBitmapFile=0;
	m_iDescFile=0;
	m_iRingtoneFile=0;
	m_iSkinFile=0;
	m_iSubSisFile=0;

	switch (m_nMainFormat)
	{
		case 1:		
			ParsePlainSIS(ar);		
		break;
		case 2:		
			ParseSignedSIS(ar);		
		break;
		default:
			throw new CFormatException(CFormatException::formaterrInvalid,"unknown format");
	}
}

void CSISFile::ParseSignedHeader(unsigned char *pc)
{
	unsigned char *pcHeadEnd;
	uint32_t dwTotal=0;
	uint32_t dwHeaderSize;
	unsigned char *pcOld;
	if (*(uint32_t *)pc == 0x0000000D)		//header
	{
		pc+=4;
		dwHeaderSize=*(uint32_t *)pc;
		pc+=4;
		pcHeadEnd=pc+dwHeaderSize;
		pcOld=pc;
		while (pc < pcHeadEnd)
		{
			dwTotal=0;
			
			switch(*(uint32_t  *)pc)
			{
				case 0x0E:		pc=pcParseComponentRecords(pc+8,(*(uint32_t  *)(pc+4))-8,pcOld,&dwTotal,1);			break;
				case 0x11:		pc=pcParseRequisiteRecords(pc+8,(*(uint32_t  *)(pc+4))-8,pcOld,&dwTotal,1);			break;
				case 0x1C:		pc=pcParseFileRecords(pc+8,(*(uint32_t  *)(pc+4))-8,pcOld,&dwTotal,1);				break;
				default:		pc=pcSkipSubRecords(pc,*(uint32_t  *)(pc+4),pcOld,pcOld,&dwTotal,1);
			}
			//pc=pcSkipSubRecords(pc,*(uint32_t  *)(pc+4),pcOld,pcOld,&dwTotal,1);
			TRACEIT2("----------total: %Xh (left: %d)-----------------\n",dwTotal,pcHeadEnd-pc);
		};
	}
}

tstring CSISFile::sParseSignedString(unsigned char *pc)
{
	tstring strRet="";
	uint32_t  dwSize=*(uint32_t  *)(pc+4);
	unsigned char *pcEnd=pc+8+dwSize;

	if (*(uint32_t  *)pc == 0x01)
	{
		pc+=8;
		while (pc < pcEnd)
		{
			strRet+=(char)*pc;
			pc+=2;
		};
	}
	else
	{
		TRACEIT2("expected 0x01 - failed to parse file\n");
	}
	return strRet;
}

unsigned char *CSISFile::pcSkipSubRecords(unsigned char *pc,uint32_t  dwSize,unsigned char *pcRoot,unsigned char *pcParent,uint32_t  *pdwTotalSize,int level)
{
	unsigned char *pcEnd;
	unsigned char *pcNext;
	unsigned char *pcOld=pc;
	uint32_t  dwRecordSize;
	uint32_t  dwPad;
	uint32_t  dwSubTotalSize;

	pcEnd=pc+dwSize;

	if (level > 10)
		return pcEnd;
	dwSubTotalSize=0;
	pcNext=pc;
	if (pcEnd-pc < 8)
	{
		TRACEIT2("offset: %08Xh: level: %02d - record tag: %08X, specified size: %Xh bytes\n",pc-pcRoot,level,*(uint32_t  *)pc,*(uint32_t  *)(pc+4));
		dwRecordSize=*(uint32_t  *)(pc+4);
		dwPad=0;
		if (dwRecordSize % 4)
			dwPad=(((dwRecordSize/4)+1)*4)-dwRecordSize;
		dwSubTotalSize+=dwRecordSize+8+dwPad;
		pcNext=pc+8+dwRecordSize+dwPad;
		pc+=8+dwRecordSize;
	}
	else
	{
		while (pcNext < pcEnd && pcEnd-pc >= 8)
		{
			pc=pcNext;
			TRACEIT2("offset: %08Xh: level: %02d - record tag: %08X, specified size: %Xh bytes\n",pc-pcRoot,level,*(uint32_t  *)pc,*(uint32_t  *)(pc+4));
			switch (*(uint32_t  *)pc)
			{
				case 0x00:
				case 0x70:
					pcNext=pc+4;
					pc+=4;
					dwSubTotalSize+=4;
				break;
				case 0x01:
				case 0x04:
				case 0x06:
				case 0x07:
				case 0x09:
				case 0x0B:			//
				case 0x25:			//certificate data
				case 0x28:			//??
					dwRecordSize=*(uint32_t  *)(pc+4);
					dwPad=0;
					if (dwRecordSize % 4)
						dwPad=(((dwRecordSize/4)+1)*4)-dwRecordSize;
					dwSubTotalSize+=dwRecordSize+8+dwPad;
					pcNext=pc+8+dwRecordSize+dwPad;
					pc+=8+dwRecordSize;
				break;
				default:
					unsigned char *pcOld=pc;
					dwRecordSize=*(uint32_t  *)(pc+4);
					if(dwRecordSize > 8)
					{
						if (level > 1)
							dwSubTotalSize+=8;
						pcNext=pc=pcSkipSubRecords(pc+8,dwRecordSize,pcRoot,pc+8,&dwSubTotalSize,level+1);
					}
					else
					{
						TRACEIT2("a usually subdevided tag suddenly doesnt have enough room for child elements\n");
						dwRecordSize=*(uint32_t  *)(pc+4);
						dwPad=0;
						if (dwRecordSize % 4)
							dwPad=(((dwRecordSize/4)+1)*4)-dwRecordSize;
						dwSubTotalSize+=dwRecordSize+8+dwPad;
						pcNext=pc+8+dwRecordSize+dwPad;
						pc+=8+dwRecordSize;
					}
			}
 		};
	}
	*pdwTotalSize+=dwSubTotalSize;
	return pcNext;
}

unsigned char *CSISFile::pcSkipRecord(unsigned char *pc)
{
	uint32_t  dwPadRecord=0;
	if (*(uint32_t  *)(pc+4) % 4)
		dwPadRecord=(((*(uint32_t  *)(pc+4)/4)+1)*4)-*(uint32_t  *)(pc+4);
	pc+=8+*(uint32_t  *)(pc+4)+dwPadRecord;
	return pc;
}

unsigned char *CSISFile::pcParseComponentRecords(unsigned char *pc,uint32_t  dwSize,unsigned char *pcRoot,uint32_t  *pdwTotalSize,int level)
{
	uint32_t  dwPad=0;
	unsigned char *pcOld=pc;
	if (dwSize % 4)
		dwPad=(((dwSize/4)+1)*4)-dwSize;
	TRACEIT2("record %Xh, size: %X h\n",*(uint32_t  *)pc,*(uint32_t  *)(pc+4));
	//skip first UID
	if (*(uint32_t  *)pc == 0x09)
		pc=pcSkipRecord(pc);
	else
	{
		TRACEIT2("unexpected record %02Xh instead of 09h - failed parsing\n",*(uint32_t  *)pc);
		throw new CFormatException(CFormatException::formaterrInvalid,"unexpected record");
	}
	TRACEIT2("record %Xh, size: %X h\n",*(uint32_t  *)pc,*(uint32_t  *)(pc+4));
	//skip vendor name
	if (*(uint32_t  *)pc == 0x01)
		pc=pcSkipRecord(pc);
	else
	{
		TRACEIT2("unexpected record %02Xh instead of 01h - failed parsing\n",*(uint32_t  *)pc);
		throw new CFormatException(CFormatException::formaterrInvalid,"unexpected record");
	}
	//make sure we are right
	if (*(uint32_t  *)pc == 0x02)
		pc+=8;
	else
	{
		TRACEIT2("unexpected record %02Xh instead of 02h - failed parsing\n",*(uint32_t  *)pc);
		throw new CFormatException(CFormatException::formaterrInvalid,"unexpected record");
	}
	//
	if (*(uint32_t  *)pc == 0x01)
	{
		m_strName=sParseSignedString(pc);
		TRACEIT2("title: %s\n",m_strName.c_str());
	}
	else
	{
		TRACEIT2("unexpected record %02Xh instead of 01h - failed parsing\n",*(uint32_t  *)pc);
		throw new CFormatException(CFormatException::formaterrInvalid,"unexpected record");
	}
	return pcOld+dwSize+dwPad+8;
}

unsigned char *CSISFile::pcParseRequisiteRecords(unsigned char *pc,uint32_t  dwSize,unsigned char *pcRoot,uint32_t  *pdwTotalSize,int level)
{
	int iReq=0;
	unsigned char *pcEnd;
	uint32_t  dwPad=0;
	unsigned char *pcOld=pc;
	if (dwSize % 4)
		dwPad=(((dwSize/4)+1)*4)-dwSize;

	pcEnd=pc+dwSize;

	if (level > 10)
		return pcEnd;

	while (pcEnd-pc >= 8)
	{
		TRACEIT2("offset: %08Xh: level: %02d - record tag: %08X, specified size: %Xh bytes\n",pc-pcRoot,level,*(uint32_t  *)pc,*(uint32_t  *)(pc+4));
		switch(*(uint32_t  *)pc)
		{
			case 0x02:
				if (*(uint32_t  *)(pc+4) > 4)
					pc+=8;
				else
				{
					pc=pcSkipRecord(pc);
					if (*(uint32_t  *)pc == 0x02)
						pc+=8;
				}
				if (*(uint32_t  *)pc == 0x12)
					pc+=8;
			break;
			case 0x70:
				pc+=4;
			break;
			default:
				TRACEIT2("unexpected record %Xh - failed parsing\n",*(uint32_t  *)pc);
				throw new CFormatException(CFormatException::formaterrInvalid,"unexpected record");
		}
		TRACEIT2("offset: %08Xh: level: %02d - record tag: %08X, specified size: %Xh bytes\n",pc-pcRoot,level,*(uint32_t  *)pc,*(uint32_t  *)(pc+4));
		if(*(uint32_t  *)pc == 0x09 && *(uint32_t  *)(pc+4) == 0x04)
		{
			pc+=8;
			TRACEIT2("requisite #%d: UID %08Xh\n",iReq+1,*(uint32_t  *)pc);
			m_listSignedRequisites.push_back(*(uint32_t  *)pc);
			++iReq;
			pc+=4;
		}
		else
		{
			TRACEIT2("unexpected record - failed parsing\n");
			throw new CFormatException(CFormatException::formaterrInvalid,"unexpected record");
		}
		if(*(uint32_t  *)pc == 0x05)
			pc=pcSkipRecord(pc);
		else
		{
			TRACEIT2("unexpected file record - failed parsing\n");
			throw new CFormatException(CFormatException::formaterrInvalid,"unexpected file record");
		}
		if(*(uint32_t  *)pc == 0x02)
			pc=pcSkipRecord(pc);
		else
		{
			TRACEIT2("unexpected file record - failed parsing\n");
			throw new CFormatException(CFormatException::formaterrInvalid,"unexpected file record");
		}
 	};
	return pcOld+dwSize+dwPad+8;
}

unsigned char *CSISFile::pcParseSignatureRecords(unsigned char *pc,uint32_t  dwSize,unsigned char *pcRoot,uint32_t  *pdwTotalSize,int level)
{
	uint32_t  dwPad=0;
	unsigned char *pcOld=pc;
	if (dwSize % 4)
		dwPad=(((dwSize/4)+1)*4)-dwSize;
	return pcOld+dwSize+dwPad+8;
}

unsigned char *CSISFile::pcParseFileRecords(unsigned char *pc,uint32_t  dwSize,unsigned char *pcRoot,uint32_t  *pdwTotalSize,int level)
{
	int iFile=0;
	unsigned char *pcEnd;
	uint32_t  dwExpectedTag=0x18;
	uint32_t  dwNextExpectedTag=0x00;
	uint32_t  dwRecordSize;
	uint32_t  dwPad;
	uint32_t  dwSubTotalSize;

	uint32_t  dwTotalFileRecordSize;

	pcEnd=pc+dwSize;

	if (level > 10)
		return pcEnd;
	dwSubTotalSize=0;

	if (*(uint32_t  *)pc == 0x02)
	{
		pc+=8;
		++level;
	}
	else
	{
		TRACEIT2("illegal file record - failed parsing\n");
	}

	while (pcEnd-pc >= 8)
	{
		TRACEIT2("offset: %08Xh: level: %02d - record tag: %08X, specified size: %Xh bytes\n",pc-pcRoot,level,*(uint32_t  *)pc,*(uint32_t  *)(pc+4));
		if (*(uint32_t  *)pc == dwExpectedTag)
		{
			dwExpectedTag=dwNextExpectedTag;
			++dwNextExpectedTag;
		}
		else
		{
			TRACEIT2("illegal file record - failed parsing\n");
			throw new CFormatException(CFormatException::formaterrInvalid);
		}
		pc+=4;
		dwTotalFileRecordSize=*(uint32_t  *)pc;
		pc+=4;
		if(*(uint32_t  *)pc == 0x01)
		{
			tstring strFile;
			dwRecordSize=*(uint32_t  *)(pc+4);
			dwPad=0;
			if (dwRecordSize % 4)
				dwPad=(((dwRecordSize/4)+1)*4)-dwRecordSize;
			strFile=sParseSignedString(pc);
			pc+=dwRecordSize+8+dwPad;
			TRACEIT2("file #%d: %s\n",iFile+1,strFile.c_str());
			EvalFileName(strFile.c_str(),iFile+1);
			iFile++;
		}
		else
		{
			TRACEIT2("illegal file record - failed parsing\n");
			throw new CFormatException(CFormatException::formaterrInvalid);
		}
		if(*(uint32_t  *)pc == 0x01 && *(uint32_t  *)(pc+4) == 0x00)
			pc+=8;
		else
		{
			TRACEIT2("illegal file record - failed parsing\n");
			throw new CFormatException(CFormatException::formaterrInvalid);
		}
		if(*(uint32_t  *)pc == 0x19)
			pc=pcSkipRecord(pc);
		else
		{
			TRACEIT2("illegal file record - failed parsing\n");
			throw new CFormatException(CFormatException::formaterrInvalid);
		}
		switch(*(uint32_t  *)pc)
		{
			case 0x01:
				pc+=4;
				if(*(uint32_t  *)pc != 0x00)
				{
					TRACEIT2("unexpected record size - failed parsing\n");
					throw new CFormatException(CFormatException::formaterrInvalid);
				}
				pc+=4;
				pc+=16;
			break;
			case 0x08:
				pc+=4;
				if(*(uint32_t  *)pc != 0x00)
				{
					TRACEIT2("unexpected record size - failed parsing\n");
					throw new CFormatException(CFormatException::formaterrInvalid);
				}
				pc+=4;
				pc+=20;
				if(*(uint32_t  *)pc == 0x02 && *(uint32_t  *)(pc+4) == 0x04)
					pc=pcSkipRecord(pc);
				else
				{
					TRACEIT2("unexpected record - failed parsing\n");
					throw new CFormatException(CFormatException::formaterrInvalid);
				}
				if(*(uint32_t  *)pc == 0x02 && *(uint32_t  *)(pc+4) == 0x04)
					pc=pcSkipRecord(pc);
				else
				{
					TRACEIT2("unexpected record - failed parsing\n");
					throw new CFormatException(CFormatException::formaterrInvalid);
				}
			break;
		}
 	};
	return pc;
}

void CSISFile::ParseSignedSIS(istream &ar)
{
	uint32_t  dwCompSize,dwDeSize;
	unsigned char *pcCompressed;
	unsigned char *pcHeader;
	tstring strText="";

	m_bUnicode=true;
	m_nEPOCVersion=6;

	ar.seekg(52,ios_base::beg);
	ar.read((char *)&dwCompSize,4);
	ar.seekg(4,ios_base::cur);
	ar.read((char *)&dwDeSize,4);
	
	pcCompressed=new unsigned char[dwCompSize];
	pcHeader=new unsigned char[dwDeSize];
	ar.seekg(68,ios_base::beg);
	ar.read((char *)pcCompressed,dwCompSize);
	int nDone=0;
	if ((nDone=nDecompress(pcCompressed,dwCompSize,pcHeader,dwDeSize)) > 40)
	{
		/*
		FILE *fp;
		if ((fp=fopen("header.bin","wb")) != NULL)
		{
			fwrite(pcHeader,1,dwDeSize,fp);
			fclose(fp);
		}
		*/
		ParseSignedHeader(pcHeader);
		m_nSubFormat=nCalcSignedFormatId();
	}
	else
	{
		TRACEIT2("invalid header compression\n");
		throw new CFormatException(CFormatException::formaterrInvalid);
	}
}

void CSISFile::ParsePlainSIS (istream &ar)
{
	SISHeaderEx head;
	int i;
	int iDesc=0;
	unsigned char *pcWideName=NULL;
	tstring strName;

	ar.read((char *)&head.head,sizeof(SISHeader));
	TRACEIT2("UID1: %08Xh\n",head.head.dwUID1);
	TRACEIT2("UID2: %08Xh\n",head.head.dwUID2);
	TRACEIT2("UID3: %08Xh\n",head.head.dwUID3);
	TRACEIT2("UID4: %08Xh\n",head.head.dwUID4);
	switch(head.head.dwUID2)
	{
		case 0x1000006D:	m_nEPOCVersion=3;	break;							//EPOC 3,4 or 5
		case 0x10003A12:	m_nEPOCVersion=6;	break;							//EPOC 6
		default:			Log2(verbLevErrors,"unknown UID2 (%08Xh)\n",head.head.dwUID2);
	}
	switch(head.head.dwInstVers)
	{
		case 68:	
		case 100:	m_nEPOCVersion=3;	break;									//EPOC 3,4 or 5
		case 200:	m_nEPOCVersion=6;	break;									//EPOC 6
		default:	Log2(verbLevErrors,"unknown installer version (%08Xh)\n",head.head.dwInstVers);
	}
	if (m_nEPOCVersion == 6)											//is it symbian?
		ar.read((char *)&head.dwSignPoint,32);							//yes->read extended header information
	TRACEIT2("Number of Languages: %d\n",head.head.nNumLang);
	TRACEIT2("Number of Files: %d\n",head.head.nNumFiles);
	TRACEIT2("Number of Requisites: %d\n",head.head.nNumReq);
	TRACEIT2("Options: %04Xh\n",head.head.nOptions);
	if (head.head.nOptions & 0x0001)
	{
		TRACEIT2("is unicode\n");
		m_bUnicode=true;
	}
	if (m_nEPOCVersion < 6 || (head.head.nOptions & 0x0008) == 0x0008)
	{
		TRACEIT2("is not compressed\n");
		m_bCompressed=false;
	}
	else
	{
		TRACEIT2("is compressed\n");
		m_bCompressed=true;
	}
	TRACEIT2("Type: %04Xh\n",head.head.nType);
	//package description
	switch (head.head.nType)
	{
		//0x0000 SA SISAPP Contains an application (the default) 
		case 0x0000:	m_bContainsApplication=true;		break;
		//0x0001 SY SISSYSTEM Contains a shared/system component such as a DLL or OPX (EPOC release 6) 
		case 0x0001:	m_bContainsSystem=true;				break;
		//0x0002 SO SISOPTION Contains an optional component, independently selectable by the user (EPOC release 6) 
		case 0x0002:	m_bContainsOption=true;				break;
		//0x0003 SC SISCONFIG Configures an existing application or service (EPOC release 6) 
		case 0x0003:	m_bContainsConfig=true;				break;
		//0x0004 SP SISPATCH Patches an existing component (EPOC release 6) 
		case 0x0004:	m_bContainsPatch=true;				break;
		//0x0005 SU SISUPGRADE Upgrades an existing component (EPOC release 6) 
		case 0x0005:	m_bContainsUpgrade=true;			break;
		default:
			Log2(verbLevErrors,"unknown Type (%Xh)\n",head.head.nType);
	}
	TRACEIT2("Variant: %08Xh\n",head.head.dwVariant);
	ar.seekg(head.head.dwLangPoint,ios_base::beg);
	for (i=0;i < head.head.nNumLang;i++)
	{
		unsigned short nLanguageId;
		ar.read((char *)&nLanguageId,2);
		TRACEIT2("Language #%d: %s\n",i+1,sGetLanguageName(nLanguageId).c_str());
		m_listLanguages.push_back(nLanguageId);
	}
	bParseComponent(ar,&head);
	bParseRequisites(ar,&head);
	bParseFiles(ar,&head);
	ProcessFiles(ar,&head);
	//read file that is used during installation for display
	if (m_iDescFile)
		m_strRemark=sExtractTextFile(ar,m_iDescFile);
	m_nSubFormat=nCalcNormalFormatId();
}


/*\
 * <---------- sExtractTextFile ---------->
 * @m 
 * --> I N <-- @p
 * istream &ar - stream reference
 * int iFile - index (+1)
 * <-- OUT --> @r
 * tstring - string content (narrow)
\*/
tstring CSISFile::sExtractTextFile(istream &ar,int iFile)
{
	tstring strContent;
	unsigned char *pcDescription;
	unsigned char *pcCompressed;
	SISFileHeaderEx *pFile;

	pFile=m_listFiles[iFile-1];
	TRACEIT2("extract file #%d - file point: %08X\n",iFile,pFile->head.dwFilePoint);
	ar.seekg(pFile->head.dwFilePoint,ios_base::beg);
	pcCompressed=new unsigned char[pFile->head.dwFileLen+1];
	ar.read((char *)pcCompressed,pFile->head.dwFileLen);
	if (m_bCompressed)
	{
		pcDescription=new unsigned char[pFile->dwOrigFileLen+1];
		nDecompress(pcCompressed,pFile->head.dwFileLen,pcDescription,pFile->dwOrigFileLen);
		*(pcDescription+pFile->dwOrigFileLen)=0;
	}
	else
	{
		pcDescription=pcCompressed;
		*(pcDescription+pFile->head.dwFileLen)=0;
	}
	if (m_bUnicode)
		strContent=sGetNarrowString(pcDescription);
	else
		strContent=(const char *)pcDescription;
	if (m_bCompressed)
		delete [] pcDescription;
	delete [] pcCompressed;				
	return strContent;
}

unsigned char *CSISFile::pcExtractFile(istream &ar,int iFile,unsigned long &nSize)
{
	tstring strContent;
	unsigned char *pcCompressed;
	SISFileHeaderEx *pFile;
	unsigned char *pcOut=NULL;

	ASSERT(iFile);
	pFile=m_listFiles[iFile-1];
	TRACEIT2("extract file #%d - file point: %08X\n",iFile,pFile->head.dwFilePoint);
	ar.seekg(pFile->head.dwFilePoint,ios_base::beg);
	pcCompressed=new unsigned char[pFile->head.dwFileLen+1];
	ar.read((char *)pcCompressed,pFile->head.dwFileLen);
	if (m_bCompressed)
	{
		pcOut=new unsigned char[pFile->dwOrigFileLen+1];
		nDecompress(pcCompressed,pFile->head.dwFileLen,pcOut,pFile->dwOrigFileLen);
		delete [] pcCompressed;				
		nSize=pFile->dwOrigFileLen;
	}
	else
	{
		pcOut=pcCompressed;
		nSize=pFile->head.dwFileLen;
	}
	return pcOut;
}


void CSISFile::EvalFileName(tstring strIn,int iFile)
{
	CMyString strName=strIn;

	if (!strName.nCompareRPartNoCase(".skn"))
		m_iSkinFile=iFile;
	if (!strName.nCompareRPartNoCase(".wav"))
		m_iRingtoneFile=iFile;
	if (!strName.nCompareRPartNoCase(".mid"))
		m_iRingtoneFile=iFile;
	if (!strName.nCompareRPartNoCase(".amr"))
		m_iRingtoneFile=iFile;
	if (!strName.nCompareRPartNoCase(".awb"))
		m_iRingtoneFile=iFile;
	if (!strName.nCompareRPartNoCase(".rmf"))
		m_iRingtoneFile=iFile;
	if (!strName.nCompareRPartNoCase(".mbm"))
		m_iBitmapFile=iFile;
	if (!strName.nCompareRPartNoCase(".sis"))
		m_iSubSisFile=iFile;
}

/*\
 * <---------- ProcessFiles ---------->
 * @m 
 * --> I N <-- @p
 * istream &ar - 
 * SISHeaderEx *pHead - 
\*/
void CSISFile::ProcessFiles(istream &ar,SISHeaderEx *pHead)
{
	int i;
	SISFileHeaderEx *pFile;
	unsigned char *pcWideName;
	unsigned char *pcData;
	unsigned long nSize;
	tstring strName;

	m_iDescFile=0;
	m_iRingtoneFile=0;
	m_iSubSisFile=0;
	m_iBitmapFile=0;
	m_iSkinFile=0;

	for (i=0;i < pHead->head.nNumFiles;i++)
	{
		unsigned char *pcFile=NULL;
		pFile=m_listFiles[i];
		TRACEIT2("File Entry #%d\n",i+1);
		TRACEIT2("file type: %08X\n",pFile->head.dwFileType);
		TRACEIT2("file details: %08X\n",pFile->head.dwFileDetails);
		TRACEIT2("srcfile name len: %d\n",pFile->head.dwSrcNamLen);
		TRACEIT2("srcfile name point: %08X\n",pFile->head.dwSrcNamPoint);
		TRACEIT2("dstfile name len: %d\n",pFile->head.dwDstNamLen);
		TRACEIT2("dstfile name point: %08X\n",pFile->head.dwDstNamPoint);
		TRACEIT2("file size: %d\n",pFile->head.dwFileLen);
		TRACEIT2("file point: %08X\n",pFile->head.dwFilePoint);
		if (m_nEPOCVersion >= 6)
		{
			TRACEIT2("orig file size: %d\n",pFile->dwOrigFileLen);
			TRACEIT2("mime len: %d\n",pFile->dwMimeLen);
			TRACEIT2("mime point: %08X\n",pFile->dwMimePoint);
		}
		pcWideName=new unsigned char[pFile->head.dwDstNamLen+1];
		*(pcWideName+pFile->head.dwDstNamLen)=0;
		ar.seekg(pFile->head.dwDstNamPoint,ios_base::beg);
		ar.read((char *)pcWideName,pFile->head.dwDstNamLen);
		if (m_bUnicode)
			strName=sGetNarrowString(pcWideName);
		else
			strName=(const char *)pcWideName;
		delete [] pcWideName;
		TRACEIT2("file name: %s\n",strName.c_str());
		strncpy(pFile->pcFileName,strName.c_str(),min(strName.length(),255));
		pFile->pcFileName[min(strName.length(),255)]=0;
		switch(pFile->head.dwFileType)
		{
			case 0x00:		//normal file
				if (strName.length() > 4)
					EvalFileName(strName.c_str(),i+1);

				pcData=pcExtractFile(ar,i+1,nSize);
				if (pcData)
				{
					poProcessFile((char *)pcData,nSize,strName.c_str());
					delete [] pcData;
				}
			break;
			case 0x01:		//display file while installing
				if (pFile->head.dwFileDetails == 0x0000 || pFile->head.dwFileDetails == 0x0002)
					m_iDescFile=i+1;
			break;
			case 0x02:		//SIS component file
				m_iSubSisFile=i+1;
			break;
			case 0x03:		//File to be run during installation and/or removal
				m_iAutoFile=i+1;
			break;
			case 0x04:		//File does not yet exist, but will be created when the application is run
			break;
			case 0x05:		//Open file
			break;
		}
	}
}

/*\
 * <---------- bParseComponent ---------->
 * @m 
 * --> I N <-- @p
 * istream &ar - 
 * SISHeaderEx *pHead - 
 * <-- OUT --> @r
 * bool - 
\*/
bool CSISFile::bParseComponent(istream &ar,SISHeaderEx *pHead)
{
	char sVersion[32];
	uint32_t  dwPoint;
	uint32_t  dwSize;
	unsigned char *pcWideName;

	ar.seekg(pHead->head.dwCompPoint,ios_base::beg);
	//for (i=0;i < head.head.nNumLang;i++)
	{
		ar.read((char *)&dwSize,4);
		ar.read((char *)&dwPoint,4);
	}
	TRACEIT2("component name len: %d\n",dwSize);
	TRACEIT2("component name point: %08X\n",dwPoint);
	ar.seekg(dwPoint,ios_base::beg);
	pcWideName=new unsigned char[dwSize+1];
	ar.read((char *)pcWideName,dwSize);
	*(pcWideName+dwSize)=0;
	if (m_bUnicode)
		m_strName=sGetNarrowString(pcWideName);
	else
		m_strName=(const char *)pcWideName;
	delete [] pcWideName;

	sprintf(sVersion,"%d.%d",pHead->head.nMajorVers,pHead->head.nMinorVers);
	if (pHead->head.nMajorVers || pHead->head.nMinorVers)
		m_strName+=tstring(" ")+sVersion;
	TRACEIT2("component name: %s\n",m_strName.c_str());
	return true;
}

/*\
 * <---------- bParseFiles ---------->
 * @m 
 * --> I N <-- @p
 * istream &ar - 
 * SISHeaderEx *pHead - 
 * <-- OUT --> @r
 * bool - 
\*/
bool CSISFile::bParseFiles(istream &ar,SISHeaderEx *pHead)
{
	int i;
	uint32_t  dwCount;
	uint32_t  dwSize;
	uint32_t  dwType;
	SISFileHeaderEx *pFile;
	bool bRet=false;

	ar.seekg(pHead->head.dwFilePoint,ios_base::beg);
	for (i=0;i < pHead->head.nNumFiles;i++)
	{
		unsigned char cSelectedOptions[16];
		//
		//...denn Gott fragte als erstes die Steine:
		//"Wollt ihr xxx sein?"
		//"Nein Herr, wir sind nicht hart genug"
		//
		ar.read((char *)&dwType,4);
		TRACEIT2("File Record #%d Type: %08X\n",i+1,dwType);
		switch(dwType)
		{
			case 0x00000000:	//Simple file line
			case 0x00000001:	//Multiple language files line
				pFile=new SISFileHeaderEx;
				ar.read((char *)&pFile->head,sizeof(SISFileHeader));
				if (m_nEPOCVersion >= 6)
					ar.read((char *)&pFile->dwOrigFileLen,12);
				m_listFiles.push_back(pFile);
			break;
			case 0x00000002:	//Options line
				ar.read((char *)&dwCount,4);
				for (unsigned int o=0;o < dwCount;o++)
				{
					ar.read((char *)&dwSize,4);
					TRACEIT2("skipping Option String #%d with %d bytes\n",o+1,dwSize);
					ar.seekg(dwSize,ios_base::cur);
				}
				ar.read((char *)cSelectedOptions,16);
			break;
			case 0x00000003:	//If line
			case 0x00000004:	//ElseIf line
				ar.read((char *)&dwSize,4);
				TRACEIT2("skipping If or ElseIf with %d bytes\n",dwSize);
				ar.seekg(dwSize,ios_base::cur);
			break;
			case 0x00000005:	//Else line
			case 0x00000006:	//EndIf line
				//do nothing
			break;
		}
	}
	return bRet;
}

/*\
 * <---------- bParseRequisites ---------->
 * @m 
 * --> I N <-- @p
 * istream &ar - 
 * SISHeaderEx *pHead - 
 * <-- OUT --> @r
 * bool - 
\*/
bool CSISFile::bParseRequisites(istream &ar,SISHeaderEx *pHead)
{
	TRACEIT2("req point: %08X\n",pHead->head.dwReqPoint);
	int i;
	tstring strName;
	SISRequisite *pReq;
	unsigned char *pcWideName;

	ar.seekg(pHead->head.dwReqPoint,ios_base::beg);
	for (i=0;i < pHead->head.nNumReq;i++)
	{
		pReq=new SISRequisite;
		ar.read((char *)pReq,20);
		TRACEIT2("req UID: %08X\n",pReq->dwUID);
		//TRACEIT2("product identifier for: %s\n",sGetRequiredProductName(m_nRequiresProductId).c_str());
		TRACEIT2("req version: %d.%d\n",pReq->nVersionMajor,pReq->nVersionMinor);
		TRACEIT2("req variant: %08X\n",pReq->dwVariant);
		TRACEIT2("req name len: %d\n",pReq->dwNameLen);
		TRACEIT2("req name point: %08X\n",pReq->dwNamePoint);
		m_listRequisites.push_back(pReq);
	}
	for (i=0;i < (int)m_listRequisites.size();i++)
	{
		pReq=m_listRequisites[i];
		ar.seekg(pReq->dwNamePoint,ios_base::beg);
		pcWideName=new unsigned char[pReq->dwNameLen+1];
		ar.read((char *)pcWideName,pReq->dwNameLen);
		*(pcWideName+pReq->dwNameLen)=0;
		if (m_bUnicode)
			strName=sGetNarrowString(pcWideName);
		else
			strName=(const char *)pcWideName;
		delete [] pcWideName;
		strncpy(pReq->pcDescription,strName.c_str(),255);
		pReq->pcDescription[255]=0;
		TRACEIT2("requisite #%d description: %s\n",i+1,strName.c_str());
	}
	

	return true;
}

/*\
 * <---------- nMapProductId ---------->
 * @m 
 * --> I N <-- @p
 * uint32_t  dwUID - 
 * <-- OUT --> @r
 * int - 
\*/
int CSISFile::nMapProductId(uint32_t  dwUID)
{
	map<int,int> mapUID;
	mapUID[0x101F6F88]=1;			//Series 60 Platform 0.9
	mapUID[0x101F795F]=2;			//Series 60 Platform 1.0
	mapUID[0x101F8201]=3;			//Series 60 Platform 1.1
	mapUID[0x101F8202]=4;			//Series 60 Platform 1.2 
	mapUID[0x101F7960]=5;			//Series 60 Platform, 2nd Edition 
	mapUID[0x101F9115]=6;			//Series 60 Platform, 2nd Edition, Feature Pack 1
	mapUID[0x10200BAB]=7;			//Series 60 Platform, 2nd Edition, Feature Pack 2
	mapUID[0x102032BD]=8;			//Series 60 Platform, 2nd Edition, Feature Pack 3
	mapUID[0x101F7961]=9;			//Series 60 Platform, 3rd Edition
	mapUID[0x102032BE]=10;			//Series 60 Platform, 3rd Edition, Feature Pack 1
	mapUID[0x101F7962]=11;			//Nokia 3650
	mapUID[0x101F7963]=12;			//Nokia 6600
	mapUID[0x1020216B]=13;			//Nokia 6620
	mapUID[0x101F7964]=14;			//Nokia 6630
	mapUID[0x10200f99]=15;			//Nokia 6680
	mapUID[0x101FD5DB]=16;			//Nokia 7610
	mapUID[0x101F6F87]=17;			//Nokia 7650
	mapUID[0x101F8A64]=18;			//Nokia N-Gage
	mapUID[0x101F9071]=19;			//Siemens SX1
	mapUID[0x10200F97]=20;			//Nokia 3230
	mapUID[0x101FB3F4]=21;			//Nokia 6260
	mapUID[0x101FD5DC]=22;			//Nokia 6670
	mapUID[0x102078D0]=23;			//Nokia 6681
	mapUID[0x102078CF]=24;			//Nokia 6682
	mapUID[0x10200F9A]=25;			//Nokia N70
	mapUID[0x10200F98]=26;			//Nokia N90
	mapUID[0x200005F8]=27;			//Nokia 3250
	mapUID[0x20001856]=28;			//Nokia E60
	mapUID[0x20001858]=29;			//Nokia E61
	mapUID[0x20001857]=30;			//Nokia E70
	mapUID[0x200005FF]=31;			//Nokia N71
	mapUID[0x200005FB]=32;			//Nokia N73
	mapUID[0x200005F9]=33;			//Nokia N80
	mapUID[0x200005FC]=34;			//Nokia N91
	mapUID[0x200005FA]=35;			//Nokia N92
	//Series 80 (Series80ProductID)
	mapUID[0x101F8ED2]=36;			//Series 80 v2.0
	mapUID[0x101F8ED1]=37;			//Nokia 9300
	mapUID[0x101F8DDB]=38;			//Nokia 9500
	//Series 90
	mapUID[0x101FBE05]=39;			//Nokia 7710
	//UIQ (UIQProductID)
	mapUID[0x101F617B]=40;			//UIQ v2.0 (UIQ20ProductID) 
	mapUID[0x101F61CE]=41;			//UIQ v2.1 (UIQ21ProductID) 
	mapUID[0x101F80BE]=42;			//SonyEricsson P80x 
	mapUID[0x101FBB35]=43;			//SonyEricsson P90x 
	mapUID[0x101F6300]=44;			//UIQ v3
	//0x101F8582 = series 60 skinning support 2.6
	//0x10207113 = series 60 skinning support 2.8 (scalable skins)

	return mapUID[dwUID];
}

/*\
 * <---------- sGetRequiredProductName ---------->
 * @m 
 * --> I N <-- @p
 * int nRequisite - 
 * <-- OUT --> @r
 * tstring - 
\*/
tstring CSISFile::sGetFormatName(int nRequisite)
{
	tstring strName[45]=
	{	
		"Undefined",
		"Series 60 Platform 0.9",
		"Series 60 Platform 1.0",
		"Series 60 Platform 1.1",
		"Series 60 Platform 1.2", 
		"Series 60 Platform, 2nd Edition",
		"Series 60 Platform, 2nd Edition, Feature Pack 1",
		"Series 60 Platform, 2nd Edition, Feature Pack 2",
		"Series 60 Platform, 2nd Edition, Feature Pack 3",
		"Series 60 Platform, 3rd Edition",
		"Series 60 Platform, 3rd Edition, Feature Pack 1",
		"Nokia 3650",
		"Nokia 6600",
		"Nokia 6620",
		"Nokia 6630",
		"Nokia 6680",
		"Nokia 7610",
		"Nokia 7650",
		"Nokia N-Gage",
		"Siemens SX1",
		"Nokia 3230",
		"Nokia 6260",
		"Nokia 6670",
		"Nokia 6681",
		"Nokia 6682",
		"Nokia N70",
		"Nokia N90",
		"Nokia 3250",
		"Nokia E60",
		"Nokia E61",
		"Nokia E70",
		"Nokia N71",
		"Nokia N73",
		"Nokia N80",
		"Nokia N91",
		"Nokia N92",
		"Series 80 v2.0",
		"Nokia 9300",
		"Nokia 9500",
		"Nokia 7710",
		"UIQ v2.0",
		"UIQ v2.1",
		"SonyEricsson P80x",
		"SonyEricsson P90x",
		"UIQ v3"
	};
	bool bSkin=false;
	tstring strRet="";
	int iSkinVersion;
	int iReqIndex;

	iReqIndex=nRequisite & 0x3F;
	bSkin=(nRequisite & 0xC0) > 0;
	iSkinVersion=(nRequisite & 0xC0)>>6;

	if (iReqIndex)
		strRet=strName[iReqIndex];
	if (bSkin)
	{
		const char *szVersion[3]=
		{
			"2.6",
			"2.8",
			"3.0"
		};
		if (!strRet.empty())
			strRet+=" - ";
		strRet+=string("Theme ");
		if (iSkinVersion > 1 && iSkinVersion < 5)
			strRet+=string("V") + szVersion[iSkinVersion-2];
		else
			strRet+=string("Unknown Version");
	}
	return strRet;
}

/*\
 * <---------- sGetLanguageName ---------->
 * @m 
 * --> I N <-- @p
 * unsigned short nLanguageId - 
 * <-- OUT --> @r
 * tstring - 
\*/
tstring CSISFile::sGetLanguageName(unsigned short nLanguageId)
{
	map<int,tstring> mapLang;

	mapLang[0x0000]=tstring(_T("Test"));
	mapLang[0x0001]=tstring(_T("UK English"));
	mapLang[0x0002]=tstring(_T("French"));
	mapLang[0x0003]=tstring(_T("German"));
	mapLang[0x0004]=tstring(_T("Spanish"));
	mapLang[0x0005]=tstring(_T("Italian"));
	mapLang[0x0006]=tstring(_T("Swedish"));
	mapLang[0x0007]=tstring(_T("Danish"));
	mapLang[0x0008]=tstring(_T("Norwegian"));
	mapLang[0x0009]=tstring(_T("Finnish"));
	mapLang[0x000A]=tstring(_T("American English"));
	mapLang[0x000B]=tstring(_T("Swiss French"));
	mapLang[0x000C]=tstring(_T("Swiss German"));
	mapLang[0x000D]=tstring(_T("Portuguese"));
	mapLang[0x000E]=tstring(_T("Turkish"));
	mapLang[0x000F]=tstring(_T("Icelandic"));
	mapLang[0x0010]=tstring(_T("Russian"));
	mapLang[0x0011]=tstring(_T("Hungarian"));
	mapLang[0x0012]=tstring(_T("Dutch"));
	mapLang[0x0013]=tstring(_T("Belgian Flemish"));
	mapLang[0x0014]=tstring(_T("Australian English"));
	mapLang[0x0015]=tstring(_T("Belgian French"));
	mapLang[0x0016]=tstring(_T("Austrian German"));
	mapLang[0x0017]=tstring(_T("New Zealand English"));
	mapLang[0x0018]=tstring(_T("International French"));
	mapLang[0x0019]=tstring(_T("Czech"));
	mapLang[0x001A]=tstring(_T("Slovak"));
	mapLang[0x001B]=tstring(_T("Polish"));
	mapLang[0x001C]=tstring(_T("Slovenian"));
	mapLang[0x001D]=tstring(_T("Taiwan Chinese"));
	mapLang[0x001E]=tstring(_T("Hong Kong Chinese"));
	mapLang[0x001F]=tstring(_T("PRC Chinese"));
	mapLang[0x0020]=tstring(_T("Japanese"));
	mapLang[0x0021]=tstring(_T("Thai"));
	mapLang[0x0022]=tstring(_T("Afrikaans"));
	mapLang[0x0023]=tstring(_T("Albanian"));
	mapLang[0x0024]=tstring(_T("Amharic"));
	mapLang[0x0025]=tstring(_T("Arabic"));
	mapLang[0x0026]=tstring(_T("Armenian"));
	mapLang[0x0027]=tstring(_T("Tagalog"));
	mapLang[0x0028]=tstring(_T("Belarussian")); 
	mapLang[0x0029]=tstring(_T("Bengali"));
	mapLang[0x002A]=tstring(_T("Bulgarian"));
	mapLang[0x002B]=tstring(_T("Burmese"));
	mapLang[0x002C]=tstring(_T("Catalan"));
	mapLang[0x002D]=tstring(_T("Croatian"));
	mapLang[0x002E]=tstring(_T("Canadian English"));
	mapLang[0x002F]=tstring(_T("International English"));
	mapLang[0x0030]=tstring(_T("South African English"));
	mapLang[0x0031]=tstring(_T("Estonian"));
	mapLang[0x0032]=tstring(_T("Farsi"));
	mapLang[0x0033]=tstring(_T("Canadian French"));
	mapLang[0x0034]=tstring(_T("Scots Gaelic"));
	mapLang[0x0035]=tstring(_T("Georgian"));
	mapLang[0x0036]=tstring(_T("Greek"));
	mapLang[0x0037]=tstring(_T("Cyprus Greek"));
	mapLang[0x0038]=tstring(_T("Gujarati"));
	mapLang[0x0039]=tstring(_T("Hebrew"));
	mapLang[0x003A]=tstring(_T("Hindi"));
	mapLang[0x003B]=tstring(_T("Indonesian"));
	mapLang[0x003C]=tstring(_T("Irish"));
	mapLang[0x003D]=tstring(_T("Swiss Italian"));
	mapLang[0x003E]=tstring(_T("Kannada"));
	mapLang[0x003F]=tstring(_T("Kazakh"));
	mapLang[0x0040]=tstring(_T("Khmer"));
	mapLang[0x0041]=tstring(_T("Korean"));
	mapLang[0x0042]=tstring(_T("Laothian"));
	mapLang[0x0043]=tstring(_T("Latvian"));
	mapLang[0x0044]=tstring(_T("Lithuanian"));
	mapLang[0x0045]=tstring(_T("Macedonian"));
	mapLang[0x0046]=tstring(_T("Malay"));
	mapLang[0x0047]=tstring(_T("Malayalam"));
	mapLang[0x0048]=tstring(_T("Marathi"));
	mapLang[0x0049]=tstring(_T("Moldavian"));
	mapLang[0x004A]=tstring(_T("Mongolian"));
	mapLang[0x004B]=tstring(_T("Norwegian-Nynorsk"));
	mapLang[0x004C]=tstring(_T("Brazilian Portuguese"));
	mapLang[0x004D]=tstring(_T("Punjabi"));
	mapLang[0x004E]=tstring(_T("Romanian"));
	mapLang[0x004F]=tstring(_T("Serbian"));
	mapLang[0x0050]=tstring(_T("Sinhalese"));
	mapLang[0x0051]=tstring(_T("Somali"));
	mapLang[0x0052]=tstring(_T("International Spanish"));
	mapLang[0x0053]=tstring(_T("Latin American Spanish"));
	mapLang[0x0054]=tstring(_T("Swahili"));
	mapLang[0x0055]=tstring(_T("Finland Swedish"));
	mapLang[0x0057]=tstring(_T("Tamil"));
	mapLang[0x0058]=tstring(_T("Telugu"));
	mapLang[0x0059]=tstring(_T("Tibetan"));
	mapLang[0x005A]=tstring(_T("Tigrinya"));
	mapLang[0x005B]=tstring(_T("Cyprus Turkish"));
	mapLang[0x005C]=tstring(_T("Turkmen"));
	mapLang[0x005D]=tstring(_T("Ukrainian"));
	mapLang[0x005E]=tstring(_T("Urdu"));
	mapLang[0x0060]=tstring(_T("Vietnamese"));
	mapLang[0x0061]=tstring(_T("Welsh"));
	mapLang[0x0062]=tstring(_T("Zulu"));
	
	return mapLang[nLanguageId];
}

/*\
 * <---------- nDecompress ---------->
 * @m 
 * --> I N <-- @p
 * unsigned char *pcSource - 
 * unsigned int nSrcSize - 
 * unsigned char *pcDest - 
 * unsigned int nDstSize - 
 * <-- OUT --> @r
 * bool - 
\*/
int CSISFile::nDecompress(unsigned char *pcSource,unsigned int nSrcSize,unsigned char *pcDest,unsigned int nDstSize)
{
	#define CHUNK 16384
	int ret=0;
	unsigned int nBufferSize=nDstSize;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

	if (nSrcSize && nDstSize && pcSource && pcDest)
	{
		unsigned int nByteLeft=nSrcSize;
		unsigned have;
		z_stream strm;
		strm.zalloc = Z_NULL;
		strm.zfree = Z_NULL;
		strm.opaque = Z_NULL;
		strm.avail_in = 0;
		strm.next_in = Z_NULL;
		if ((ret=inflateInit(&strm)) == Z_OK)
		{	
			//decompress until deflate stream ends or end of file
			do 
			{
				//strm.avail_in = fread(in, 1, CHUNK, source);
				strm.avail_in = min(CHUNK,nByteLeft);
				memcpy(in,pcSource,strm.avail_in);
				pcSource+=strm.avail_in;
				nByteLeft-=strm.avail_in;
				
				if (strm.avail_in == 0)
					break;
				strm.next_in = in;
				//run inflate() on input until output buffer not full
				do 
				{
					//just like in def(), the same output space is provided for each call of inflate()
					strm.avail_out = CHUNK;
					strm.next_out = out;

					ret = inflate(&strm, Z_NO_FLUSH);
					if (ret == Z_STREAM_ERROR)
					{
						Log2(verbLevErrors,"zLib stream error\n");
						return false;
					}
					switch (ret) 
					{
						case Z_NEED_DICT:
						case Z_DATA_ERROR:
						case Z_MEM_ERROR:
							(void)inflateEnd(&strm);
							return false;
					}
					have = CHUNK - strm.avail_out;
					if (have > nDstSize)
					{
						Log2(verbLevErrors,"file exceeds buffer by %d bytes!!!!\n",have-nDstSize);
						memcpy(pcDest,out,nDstSize);
					}
					else
						memcpy(pcDest,out,have);
					pcDest+=have;
					nDstSize-=have;
					if (nDstSize == 0)
					{
						ret=Z_STREAM_END;
						strm.avail_out=CHUNK;
					}
					//The inner do-loop ends when inflate() has no more output as indicated by not filling the output buffer, just as for deflate(). In this case, we cannot assert that strm.avail_in will be zero, since the deflate stream may end before the file does.
				} while (strm.avail_out == 0);
				//done when inflate() says it's done
			} while (ret != Z_STREAM_END);
			//clean up and return
			(void)inflateEnd(&strm);
		}
		else
			Log2(verbLevErrors,"failed to init ZLib\n");
	}
	return nBufferSize-nDstSize;
}

/*\
 * <---------- bMagicHead ---------->
 * @m 
 * --> I N <-- @p
 * std::istream &ar - 
 * unsigned long nSize - 
 * <-- OUT --> @r
 * bool - 
\*/
bool CSISFile::bMagicHead(std::istream &ar,uint32_t nSize)
{
	bool bRet=false;
	const unsigned char pcMagic[2][4]={	{0x19,0x04,0x00,0x10},
										{0x7a,0x1a,0x20,0x10}	};
	char *acCompare=new char[12];
	
	ar.read(acCompare,12);
	//normal SIS package?	
	if (!memcmp(acCompare+8,pcMagic[0],4))
	{	//yes->...
		bRet=true;	
		m_nMainFormat=1;
	}
	else if (!memcmp(acCompare,pcMagic[1],4))
	{
		bRet=true;	
		m_nMainFormat=2;
	}
	delete [] acCompare;
	return bRet;
}

/*\
 * <---------- nCalcFormatId ---------->
 * @m 
 * <-- OUT --> @r
 * int - 
\*/
int CSISFile::nCalcNormalFormatId(void)
{
	int i,nRet,nFormatId=0;
	m_iThemeVersion=0;
	for (i=0;i < (int)m_listRequisites.size();i++)
	{
		if ((nRet=nMapProductId(m_listRequisites[i]->dwUID)) > 0 && nRet > nFormatId)
			nFormatId=nRet;
		switch (m_listRequisites[i]->dwUID)
		{
			//series 60 skinning support 2.6
			case 0x101F8582:
				if (m_iThemeVersion < 1)
					m_iThemeVersion=1;
			break;
			//series 60 skinning support 2.8 (scalable skins)
			case 0x10207113:
				if (m_iThemeVersion < 2)
					m_iThemeVersion=2;
			break;
			//series 60 3rd edition, skinning support
			case 0xA00000EB:
				if (m_iThemeVersion < 3)
					m_iThemeVersion=3;
			break;
		}
	} 
	if (m_iSkinFile > 0)
		nFormatId|=(1+m_iThemeVersion)<<6;
	TRACEIT2("format id: %d\n",nFormatId);
	TRACEIT2("skin file index: %d\n",m_iSkinFile);
	return nFormatId;
}

int CSISFile::nCalcSignedFormatId(void)
{
	int i,nRet,nFormatId=0;
	m_iThemeVersion=0;
	for (i=0;i < (int)m_listSignedRequisites.size();i++)
	{
		if ((nRet=nMapProductId(m_listSignedRequisites[i])) > 0 && nRet > nFormatId)
			nFormatId=nRet;
		switch (m_listSignedRequisites[i])
		{
			//series 60 skinning support 2.6
			case 0x101F8582:
				if (m_iThemeVersion < 1)
					m_iThemeVersion=1;
			break;
			//series 60 skinning support 2.8 (scalable skins)
			case 0x10207113:
				if (m_iThemeVersion < 2)
					m_iThemeVersion=2;
			break;
			//series 60 3rd edition, skinning support
			case 0xA00000EB:
				if (m_iThemeVersion < 3)
					m_iThemeVersion=3;
			break;
		}
	} 
	if (m_iSkinFile > 0)
		nFormatId+=45*(1+m_iThemeVersion);
	TRACEIT2("format id: %d\n",nFormatId);
	TRACEIT2("skin file index: %d\n",m_iSkinFile);
	return nFormatId;
}
