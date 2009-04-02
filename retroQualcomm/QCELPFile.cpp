/*\
 * QCELPFile.cpp
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
#include <iostream>
#include <string>
#include <map>
#ifndef WIN32
#include <netinet/in.h>
#endif
#include "../include/Resource.h"
#include "../retroBase/Basics.h"
#include "../retroBase/Integer.h"
#include "../retroBase/MyString.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/Endian.h"
#include "../retroBase/Packer.h"

#ifdef USE_QUALCOMM_LIBRARY
#ifndef WIN32
#include <dlfcn.h>
#else
#include <shlwapi.h>
#endif 
#define QSCLGUID
#include "qscl/qscl.h"
#else
#include "ANSI733Codec.h"
#endif
#include "QCELPFile.h"
#include "QCELPProperty.h"
#include "Version.h"

#ifdef USE_QUALCOMM_LIBRARY
#define DYNAMIC_QSCL 1
#ifdef WIN32
#define dlsym		GetProcAddress
#define dlerror		lastWinError
#define dlclose(a)	FreeLibrary((HMODULE)a)
#endif
#endif

DYNIMPPROPERTY(CQCELPFile,CQcelpProperty)

const unsigned char CQCELPFile::guidCodecPCM[16]={		0x19,0xdd,0xf9,0x18, 0xac,0xe0,0x43,0x05, 0x98,0xef,0x2c,0x6c, 0xa6,0xc8,0xb6,0x14	};
const unsigned char CQCELPFile::guidCodecULAW[16]={		0x95,0xb9,0x1c,0x42, 0x1f,0x14,0x42,0x40, 0x97,0xa1,0x04,0x5b, 0xdb,0x08,0x84,0xf8	};
const unsigned char CQCELPFile::guidCodecQ13K[16]={		0x41,0x6d,0x7f,0x5e, 0x15,0xb1,0xd0,0x11, 0xba,0x91,0x00,0x80, 0x5f,0xb4,0xb9,0x7e	};
const unsigned char CQCELPFile::guidCodecQ13KS[16]={	0x42,0x6d,0x7f,0x5e, 0x15,0xb1,0xd0,0x11, 0xba,0x91,0x00,0x80, 0x5f,0xb4,0xb9,0x7e	};

TCHAR CQCELPFile::m_szQcelpVersion[255]="";

/*\ 
 * <---------- CQCELPFile::CQCELPFile ----------> 
 * @m 
\*/ 
CQCELPFile::CQCELPFile(void)
{
	ZeroMemory(&m_Header,sizeof(m_Header));
	m_nMagicSize=4;
	m_pcMagic="RIFF";
	m_sFormatName="Qualcomm PureVoice (QCELP/QCP)";
	m_sDefaultExtension=_T("qcp");
	m_sFormatDescription.Load(IDS_FORMDESC_QCP);
	m_sFormatCredits=_T("The Qcelp codec is entirely based on: \"RFC 3625 - The QCP File Format and Media Types for Speech Data\", Copyright (c) 2003 by The Internet Society");
#ifdef USE_QUALCOMM_LIBRARY
	m_sFormatCredits+=_T("; \"Qualcomm Speech Codec Library\", Copyright (c) 2003 by QUALCOMM, Inc.");
#else
	m_sFormatCredits+=_T("; \"C-code for ANSI-733\", Copyright (c) 2002 by QUALCOMM, Inc.");
#endif
	m_encodingCaps.freqs[CSampleCompatibility::supports16Bit|CSampleCompatibility::supportsMono].addDigit(8000);
}


/*\ 
 * <---------- CQCELPFile::~CQCELPFile ----------> 
 * @m 
\*/ 
CQCELPFile::~CQCELPFile(void)
{
}

void *CQCELPFile::pInitLibrary(void)
{
#ifdef USE_QUALCOMM_LIBRARY
	IQsclCodec *codec;
#else
	CANSI733Codec *codec;
#endif
#ifdef USE_QUALCOMM_LIBRARY
#ifdef DYNAMIC_QSCL
#ifdef WIN32
	const char *szDll="qscl.dll";
	char szPath[_MAX_PATH];
	strcpy(szPath,szDll);
	m_qscl = LoadLibrary(szPath);
#else
    m_qscl = dlopen ("libqscl.so", RTLD_LAZY);
#endif

    if (!m_qscl)
    {	
#ifdef WIN32
		LPVOID lpMsgBuf;
		
		uint32_t dw = GetLastError(); 
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0, NULL );
		OutputDebugString(_T("SYSTEM ERROR\n"));
		OutputDebugString((LPTSTR)lpMsgBuf);
		OutputDebugString(_T("\n")); 
		Log2(verbLevErrors,"unable to load QSCL (%s)\n",(LPTSTR)lpMsgBuf);
		TRACEIT2("unable to load QSCL dynamic library\n");
		LocalFree(lpMsgBuf);
#else
        Log2(verbLevErrors,"unable to load QSCL: %s\n",dlerror());
        TRACEIT2("unable to load QSCL shared object\n");
#endif
		TRACEIT2("unable to load QSCL library\n");
		return NULL;
    }
    // Map address of class factory.
	#ifdef WIN32 
	void * (*ci) (const QsclGuid &classId,const QsclGuid &ifId) = (void * (*) (const QsclGuid &, const QsclGuid &))(dlsym ((HMODULE)m_qscl, "QsclCreateInstance"));
	#else
	void * (*ci) (const QsclGuid &classId,const QsclGuid &ifId) = (void * (*) (const QsclGuid &, const QsclGuid &))(dlsym (m_qscl, "QsclCreateInstance"));
	#endif
	if (!ci)
    {
		TRACEIT2("unable to access QSCL class factory\n");
		Log2(verbLevErrors,"unable to access QSCL class factory\n");
		dlclose(m_qscl);
        return false;
    }
    // Create converter object.
	codec = reinterpret_cast<IQsclCodec *>(ci(ID_QsclCodecQcelp, ID_IQsclCodec));
    if (!codec)
    {
		TRACEIT2("unable to get codec object\n");
		Log2(verbLevErrors,"unable to get codec object\n");
		dlclose(m_qscl);
        return NULL;
	}
#else
	codec = reinterpret_cast<IQsclCodec *>(QsclCreateInstance(ID_QsclCodecQcelp, ID_IQsclCodec));
#endif
#else
	codec = new CANSI733Codec();
#endif
	return codec;
}

#ifdef USE_QUALCOMM_LIBRARY
/*\ 
 * <---------- CQCELPFile::DeinitLibrary ----------> 
 * @m 
\*/ 
void CQCELPFile::DeinitLibrary(void)
{
	if (m_qscl)
	{
#ifdef WIN32
		FreeLibrary((HMODULE)m_qscl);
#else
		dlclose(m_qscl);
#endif
	}
	m_qscl=NULL;
}
#else
void CQCELPFile::DeinitLibrary(CANSI733Codec *codec)
{
	if (codec)
	{
		codec->Deinit();
		delete codec;
	}
}
#endif
/*\ 
 * <---------- CQCELPFile::read ----------> 
 * @m 
 * --> I N <-- @p
 * istream &ar - reference of input stream
\*/ 
void CQCELPFile::Read(istream &ar)
{
	uint32_t nDestSize;
	uint32_t nSourceSize;
	uint32_t nSizeInPackets=0;
	uint32_t nVariableRate=0;
	uint32_t nTag,nSize;
	uint32_t nOffset=nReadHeader(ar,&m_Header);
	m_pcCSBuffer=NULL;

	Unpacker up(ar, true);

	TRACEIT2("header processed, now reading actual sample data...\n");
	if (m_nFileSize)
	{
		TRACEIT2("nOffset(%d) == m_nFileSize(%d)?\n",nOffset,m_nFileSize-1);
		while (nOffset < (unsigned long)m_nFileSize-1)
		{
			up.read("l",&nTag);
			up.read("l",&nSize);
			ASSERT(nSize && nSize < (unsigned long)m_nFileSize);
			switch (TOFOURCC(nTag))
			{
				case MAKEFOURCC('v','r','a','t'):
					up.read("l",&nVariableRate);
					up.read("l",&nSizeInPackets);
					if (nVariableRate == 0)
					{
						Log2(verbLevDebug1,"sample has a fixed rate\n");
					}
					else
					{
						Log2(verbLevDebug1,"sample has a variable rate (%08X)\n",nVariableRate);
					}
					Log2(verbLevDebug1,"size in packets(%08X)\n",nSizeInPackets);
					if (nSize > 8)
						ar.seekg(nSize-8,ios_base::cur);
				break;
				case MAKEFOURCC('d','a','t','a'):
					m_nCSSize=nSize;
					m_nCSChannels=1;
					m_nCSBitsPerSample=m_Header.wBitsPerSample;
					m_nCSSamplesPerSecond=m_Header.wSamplingRate;
					if (!m_bParseOnly)
					{
						m_pcCSBuffer=(unsigned char *)calloc(nSize,1);
						if (m_pcCSBuffer)
							ar.read((char *)m_pcCSBuffer,nSize);
						else
							throw new CFormatException(CFormatException::formaterrUnknown,"couldnt reserve enough memory");
					}
					else
					{
						m_pcCSBuffer=NULL;
						ar.seekg(nSize,ios_base::cur);
					}
				break;
				default:
					ar.seekg(nSize,ios_base::cur);
			}
			nOffset+=nSize+8;
		};
		if (nSizeInPackets == 0)
		{
			Log2(verbLevErrors,"invalid packet size, correcting...\n");
			nSizeInPackets=m_nCSSize / 35;
			if (m_nCSSize % 35)
			{
				++nSizeInPackets;
				Log2(verbLevErrors,"trailed by incomplete block\n");
			}
		}

		signed short *pcDest;
		uint32_t nBlocks;

		Log2(verbLevDebug1,"blocks to unpack: %d\n",nSizeInPackets);
		Log2(verbLevDebug1,"num of rates %d\n",m_Header.vRate.numOfRates);
		Log2(verbLevDebug1,"bytes per packet %d,%d,%d,%d\n",m_Header.vRate.bytesPerPacket[0],m_Header.vRate.bytesPerPacket[1],m_Header.vRate.bytesPerPacket[2],m_Header.vRate.bytesPerPacket[3]);
		
		nSourceSize=m_nCSSize;
		m_nCSSize=nDestSize=(m_Header.wBlockSize*2)*nSizeInPackets;
		Log2(verbLevDebug1,"expecting decoding to result to: %d bytes\n",nDestSize);

		if (m_pcCSBuffer)
		{
			if (nDestSize)
			{
				pcDest=new signed short [nDestSize/2];
				if (pcDest)
				{
#ifdef USE_QUALCOMM_LIBRARY
					IQsclCodec *codec;
					if ((codec=(IQsclCodec *)pInitLibrary()) != NULL)
					{
						codec->configure(nVariableRate == 0 ? ID_RateFixedFull : ID_RateVariable,0);
						nBlocks=codec->convert(pcDest,m_pcCSBuffer,nSizeInPackets,false,(unsigned long *)&nDestSize);		
						DeinitLibrary();
						Log2(verbLevDebug1,"decoding resulted to: %d bytes\n",nBlocks*2);
						m_pcCSBuffer=pcDest;
						m_nCSSize=nBlocks*2;
					}
#else
					CANSI733Codec *codec;
					if ((codec=(CANSI733Codec *)pInitLibrary()) != NULL)
					{
						codec->Init(false,false);
						nBlocks=codec->nDecode(pcDest,(unsigned char *)m_pcCSBuffer,nSizeInPackets,nSourceSize);
						DeinitLibrary(codec);
						Log2(verbLevDebug1,"decoding resulted to: %d bytes\n",nBlocks*2);
						m_pcCSBuffer=pcDest;
						m_nCSSize=nBlocks*2;
					}
#endif
				}
				else
				{
					TRACEIT2("couldnt alloc memory for conversion destination\n");
					throw new CFormatException(CFormatException::formaterrUnknown,"couldnt alloc memory for conversion destination");
				}
			}
			else
			{
				TRACEIT2("internal error - destination sample size calced as zero\n");
				throw new CFormatException(CFormatException::formaterrUnknown,"destination sample size calced as zero");
			}
		} 
	}
	else
	{
		TRACEIT2("internal error - source file is empty\n");
		throw new CFormatException(CFormatException::formaterrUnknown,"source file is empty");
	}
}

/*\ 
 * <---------- CQCELPFile::Decode ----------> 
 * @m 
 * --> I N <-- @p
 * bool bVariableRate - indicates if source data has a variable bitrate
\*/ 
#ifdef USE_QUALCOMM_LIBRARY
void CQCELPFile::InitDecode(IQsclCodec *codec,bool bVariableRate)
{
	if (!codec->configure(!bVariableRate ? ID_RateFixedFull : ID_RateVariable,0))
	{
		TRACEIT2("couldnt configure codec\n");
		throw new CFormatException(CFormatException::formaterrUnknown,"couldnt configure codec");
	}
}
#else
//void CQCELPFile::InitDecode(CANSI733Codec *codec,bool bVariableRate)
#endif 

#ifdef USE_QUALCOMM_LIBRARY
uint32_t CQCELPFile::nDecode(IQsclCodec *codec,void *pSource,void *pDest,uint32_t nSourceBlocks,uint32_t nDestSize)
{
	uint32_t nBlocks;
	TRACEIT2("blocks to unpack: %d\n",nSourceBlocks);
	nBlocks=codec->convert(pDest,pSource,nSourceBlocks,false,(unsigned long *)&nDestSize);		
	TRACEIT2("got: %d\n",nBlocks);
	return nBlocks;
}
#endif 

#ifdef USE_QUALCOMM_LIBRARY
void CQCELPFile::InitEncode(IQsclCodec *codec)
{
	if (!codec->configure(ID_RateFixedFullWithHeader,0))
	{
		Log2(verbLevErrors,"failed to configure codec\n");
		throw new CFormatException(CFormatException::formaterrUnknown,"failed to configure codec");
	}
}
#else
#endif 

/*\
 * <---------- nEncode ---------->
 * @m 
 * --> I N <-- @p
 * bool bVariableRate - 
 * int nChannels - 
 * int nBitsPerSample - 
 * int nSamplesPerSecond - 
 * void *pSource - 
 * void *pDest - 
 * uint32_tint nSourceSize - 
 * <-- OUT --> @r
 * uint32_t- 
\*/
#ifdef USE_QUALCOMM_LIBRARY
uint32_t CQCELPFile::nEncode(IQsclCodec *codec,int nChannels,int nBitsPerSample,int nSamplesPerSecond,void *pSource,void *pDest,uint32_t nSourceSize)
{
	static bool bConfigured=false;
	if (nChannels != 1)
	{
		Log2(verbLevErrors,"channel count incompatible\n");
		throw new CFormatException(CFormatException::formaterrSource,"channel count incompatible");
	}
	if (nBitsPerSample != 16)
	{
		Log2(verbLevErrors,"sample width incompatible\n");
		throw new CFormatException(CFormatException::formaterrSource,"sample width incompatible");
	}
	if (nSamplesPerSecond != 8000)
	{
		Log2(verbLevErrors,"sample rate incompatible\n");
		throw new CFormatException(CFormatException::formaterrSource,"sample rate incompatible");
	}
	if (!bConfigured)
	{
		if (!codec->configure(ID_RateFixedFullWithHeader,0))
		{
			Log2(verbLevErrors,"failed to configure codec\n");
			throw new CFormatException(CFormatException::formaterrUnknown,"failed to configure codec");
		}
	}
	uint32_t nDestSize=nSourceSize;
	uint32_t nBlocks=nSourceSize/2;
	nBlocks=codec->convert(pDest,pSource,nBlocks,true,(unsigned long *)&nDestSize);

	Log2(verbLevDebug1,"%d blocks encoded\n",nBlocks);
	if (nBlocks == 0)
	{
		IQsclError *error = reinterpret_cast<IQsclError *>(codec->queryInterface (ID_IQsclError));
		if (error && error->wasLastError ())
		{
			string str=string("qscl error: ")+string(error->getLastError());
			error->release ();
			Log2(verbLevErrors,str.c_str());
			throw new CFormatException(CFormatException::formaterrUnknown,str.c_str());
		}
	}
	return nDestSize;
}
#endif


/*\ 
 * <---------- CQCELPFile::nGetPlaytime ----------> 
 * @m 
 * --> I N <-- @p
 * int nSize - size of sample data
 * <-- OUT --> @r
 * int - playtime in microseconds
 \*/ 
unsigned int CQCELPFile::nGetPlaytime(unsigned int nSize,unsigned int nBitPerSecond)
{
	unsigned int nRet=0;
	unsigned int nNom,nDiv;
	nNom=nSize*8;
	nDiv=nBitPerSecond;
	ASSERT(nDiv);
	if (nDiv)
		nRet=round(nNom,nDiv);
	return nRet;
}


/*\ 
 * <---------- CQCELPFile::nReadHeader ----------> 
 * @m 
 * --> I N <-- @p
 * istream &ar - reference to input stream
 * QCELPHEADER *pQcelp - pointer to header data
 * <-- OUT --> @r
 * uint32_t- number of bytes read
\*/ 
uint32_t CQCELPFile::nReadHeader(istream &ar, QCELPHEADER *pQcelp) 
{ 
	uint32_t nOffset=0;
	Unpacker up(ar, true);
	//Read the first 4 bytes, which will hold 'RIFF' 
	up.read("4b",pQcelp->RIFF); 
	nOffset+=sizeof(pQcelp->RIFF);
	//Now, read the size 
	up.read("l",&pQcelp->nSize); 
	nOffset+=sizeof(pQcelp->nSize);
	//Now, read 'QCELP' and 'fmt' in a single procedure 
	up.read("8b",pQcelp->QCELPfmt); 
	nOffset+=sizeof(pQcelp->QCELPfmt);
	//Now, read wFormatLength 
	up.read("l",&pQcelp->nFormatLength); 
	nOffset+=sizeof(pQcelp->nFormatLength);
	//Now, read the wFormatTag  
	up.read("s",&pQcelp->wFormatTag); 
	nOffset+=sizeof(pQcelp->wFormatTag);
	//Now, read the codec identifiers
	up.read("16b",&pQcelp->format);
	nOffset+=sizeof(pQcelp->format);
	if (!memcmp(&pQcelp->format,guidCodecQ13KS,16))
		m_nSubFormat=2;
	if (!memcmp(&pQcelp->format,guidCodecQ13K,16))
		m_nSubFormat=1;
	if (!memcmp(&pQcelp->format,guidCodecPCM,16))
		m_nSubFormat=0;
	if (!memcmp(&pQcelp->format,guidCodecULAW,16))
		m_nSubFormat=3;
	//Now, read the codec version
	up.read("s",&pQcelp->wCodecVersion); 
	nOffset+=sizeof(pQcelp->wCodecVersion);
	//QCELPcodecname
	up.read("80b",&pQcelp->QCELPcodecname); 
	nOffset+=sizeof(pQcelp->QCELPcodecname);	
	TRACEIT2("codec name %s\n",pQcelp->QCELPcodecname);
	//Read BPS
	up.read("s",&pQcelp->wAverageBps); 
	nOffset+=sizeof(pQcelp->wAverageBps);	
	TRACEIT2("wAverageBps: %d\n",pQcelp->wAverageBps);
	up.read("s",&pQcelp->wPacketSize); 
	nOffset+=sizeof(pQcelp->wPacketSize);	
	TRACEIT2("wPacketSize: %d\n",pQcelp->wPacketSize);
	up.read("s",&pQcelp->wBlockSize); 
	nOffset+=sizeof(pQcelp->wBlockSize);	
	TRACEIT2("wBlockSize: %d\n",pQcelp->wBlockSize);
	//Read Sample Rate
	up.read("s",&pQcelp->wSamplingRate); 
	nOffset+=sizeof(pQcelp->wSamplingRate);	
	TRACEIT2("samples per second: %d\n",pQcelp->wSamplingRate);
	//Read Bits per Sample
	up.read("s",&pQcelp->wBitsPerSample); 
	nOffset+=sizeof(pQcelp->wBitsPerSample);	
	TRACEIT2("bits per sample: %d\n",pQcelp->wBitsPerSample);
    // contains rate header format info
	up.read("l",&pQcelp->vRate.numOfRates);
	nOffset+=sizeof(pQcelp->vRate.numOfRates);
	ar.read((char *)&pQcelp->vRate.bytesPerPacket[0],sizeof(pQcelp->vRate.bytesPerPacket));
	nOffset+=sizeof(pQcelp->vRate.bytesPerPacket);
	ar.read((char*)&pQcelp->reserved,sizeof(pQcelp->reserved));
	nOffset+=sizeof(pQcelp->reserved)*5;
	TRACEIT2("offset %ld = tell %ld\n",nOffset,ar.tellg());
	return nOffset;
} 


/*\ 
 * <---------- CQCELPFile::bMagicHead ----------> 
 * @m 
 * --> I N <-- @p
 * std::istream &ar - reference to input stream
 * uint32_tnSize - filesize in byte
 * <-- OUT --> @r
 * bool - TRUE=ok
\*/ 
bool CQCELPFile::bMagicHead(std::istream &ar,uint32_t nSize)
{
	uint32_t i;
	unsigned char cCompare;

	if (m_nMagicSize == 0 || nSize < 13)
		return false;
	for (i=0;i < m_nMagicSize;i++)
	{
		ar.read((char *)&cCompare,1);
		if (cCompare != *(m_pcMagic+i))
			return false;
	}
	ar.seekg(8,ios_base::beg);
	const char *pcSubTag={"QLCM"};
	for (i=0;i < 4;i++)
	{
		ar.read((char *)&cCompare,1);
		if (cCompare != pcSubTag[i])
			return false;
	}
	return true;
}

/*\ 
 * <---------- CQCELPFile::sGetFormatName ----------> 
 * @m 
 * --> I N <-- @p
 * int nFormat - internal format identifier
 * <-- OUT --> @r
 * tstring - descriptive format name
 \*/ 
tstring CQCELPFile::sGetFormatName(int nFormat)
{
	map<int,tstring> mapFormatName;
	mapFormatName[0x0000]=tstring(_T("Pulse Code Modulation"));
	mapFormatName[0x0001]=tstring(_T("IS-733 Qualcomm PureVoice 13K"));
	mapFormatName[0x0002]=tstring(_T("IS-733 Qualcomm PureVoice 13K (SmartRate)"));
	mapFormatName[0x0003]=tstring(_T("G.711 uLaw"));
	return mapFormatName[nFormat];
}

/*\ 
 * <---------- CQCELPFile::RenderDestination ----------> 
 * @m 
 * --> I N <-- @p
 * ostream &out - reference to output stream
\*/ 
void CQCELPFile::Write(ostream &out)
{
	Packer pk(out, true);

	uint32_t nBlocks;
	uint32_t nDestBufferSize=m_pCSSource->m_nCSSize;
	uint32_t nDestSize=m_pCSSource->m_nCSSize;
 
	uint32_t nFileSize=0;
	uint32_t nVariableRate=0;
 
#ifdef USE_QUALCOMM_LIBRARY
	IQsclCodec *codec;
	if ((codec=(IQsclCodec *)pInitLibrary()) != NULL)
	{
#else
	CANSI733Codec *codec;
	if ((codec=(CANSI733Codec *)pInitLibrary()) != NULL)
	{
#endif
		void *pcDest=new char [nDestBufferSize];

#ifdef USE_QUALCOMM_LIBRARY
		//codec->configure(nVariableRate == 0 ? ID_RateFixedFull : ID_RateVariable,0);
		codec->configure(ID_RateFixedFullWithHeader,0);
		nDestSize=nEncode(	codec,
							m_pCSSource->m_nCSChannels,
							m_pCSSource->m_nCSBitsPerSample,
							m_pCSSource->m_nCSSamplesPerSecond,
							m_pCSSource->m_pcCSBuffer,
							pcDest,
							m_pCSSource->m_nCSSize	);
		DeinitLibrary();
		nBlocks=nDestSize/35;
		if (nDestSize%35)
			++nBlocks;
#else
		codec->Init(true,true);
		TRACEIT2("running RI encoder...\n");
		nBlocks=codec->nEncode((unsigned char *)pcDest,&nDestSize,(short *)m_pCSSource->m_pcCSBuffer,m_pCSSource->m_nCSSize);
		DeinitLibrary(codec);
#endif
		TRACEIT2("%d blocks encoded\n",nBlocks);
		const unsigned char data[170] = 
		{
			0x51, 0x4C, 0x43, 0x4D, 0x66, 0x6D, 0x74, 0x20, 0x96, 0x00, 0x00, 0x00, 0x01, 0x00, 0x41, 0x6D, 
			0x7F, 0x5E, 0x15, 0xB1, 0xD0, 0x11, 0xBA, 0x91, 0x00, 0x80, 0x5F, 0xB4, 0xB9, 0x7E, 0x02, 0x00, 
			0x51, 0x63, 0x65, 0x6C, 0x70, 0x20, 0x31, 0x33, 0x4B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
			0x3F, 0x38, 0x23, 0x00, 0xA0, 0x00, 0x40, 0x1F, 0x10, 0x00, 0x04, 0x00, 0x00, 0x00, 0x03, 0x01, 
			0x07, 0x02, 0x10, 0x03, 0x22, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
			0x00, 0x00, 0x76, 0x72, 0x61, 0x74, 0x08, 0x00, 0x00, 0x00
		};
		nFileSize=nDestSize+186;
		out.write("RIFF",4);
		pk.write("l",&nFileSize);
		out.write((char *)data,170);
		pk.write("l",&nVariableRate);
		pk.write("l",&nBlocks);
		out.write("data",4);
		pk.write("l",&nDestSize);
		out.write((const char *)pcDest,nDestSize);
		m_nCSSize=nDestSize;
		m_nFileSize=out.tellp();
		m_nCSSamplesPerSecond=m_pCSSource->m_nCSSamplesPerSecond;
		m_nCSChannels=m_pCSSource->m_nCSChannels;
		m_nCSBitsPerSample=0;
		delete [] pcDest;
	}
}

/*\ 
 * <---------- szGetQcelpVersion ----------> 
 * @m 
 * <-- OUT --> @r
 * LPCTSTR 
\*/ 
LPCTSTR CQCELPFile::szGetQcelpVersion(void)
{
#ifdef USE_QUALCOMM_LIBRARY
	IQsclCodec *pCodec;
	CQCELPFile qc;
	
	if ((pCodec=(IQsclCodec *)qc.pInitLibrary()) != NULL)
	{
		sprintf (m_szQcelpVersion,"%d",pCodec->getVersion());
		qc.DeinitLibrary();
		return m_szQcelpVersion;
	}
	else
		return "<not loaded>";
#else
	return CANSI733Codec::pszGetVersion();
#endif
}
