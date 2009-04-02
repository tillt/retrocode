/*\
 * MobileContent.cpp
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
#include <memory.h>
#include <string.h>
#include <strstream>
#include "Basics.h"
#include "MobileContent.h"
#include "MyString.h"
#include "../include/Resource.h"
#include "Endian.h" 


CFormatException::CFormatException(int nExceptionCode,LPCTSTR pszInfo) : truntime_error(tstring(pszInfo)), m_nCode(nExceptionCode)
{
}


LPCSTR CFormatException::szGetErrorMessage()
{
#ifdef _AFX
	CString strWhat;
	CString strText;
#else
	CMyString strText;
	CMyString strWhat;
#endif
	if (m_nCode < formaterrLast)
		strText.LoadString(IDS_FORMATEXCEPTION_BASE+m_nCode);
	else
		strText.LoadString(IDS_FORMATEXCEPTION_UNKNOWN);
	ASSERT(!strText.empty());
	strWhat=what();
	m_sErrDescription=strText;
	if (!strWhat.empty())
		m_sErrDescription+=": "+strWhat;
	return m_sErrDescription.c_str();
}

CFormatException::~CFormatException() throw()
{
}

int CFormatException::nGetExceptionCode()
{
	return m_nCode;
}

CMobileSampleContent::CMobileSampleContent(int iFormatId) : CMobileContent(iFormatId),m_pParameters(NULL),m_pcCSBuffer(NULL),m_nCSSize(0),m_nCSChannels(0),m_nCSSamplesPerSecond(0),m_pCSSource(NULL)
{
	TRACEIT2("new sample...\n");
}

CMobileSampleContent::~CMobileSampleContent(void)
{
	if (m_pCSSource)
	{
		TRACEIT2("releasing source memory\n");
		m_pCSSource->Release();
	}
	TRACEIT2("destructing sample (m_pcCSBuffer)...\n");
	if (m_pcCSBuffer)
		Free(m_pcCSBuffer);
	m_pcCSBuffer=0;
	TRACEIT2("sample buffer free\n");
}

void CSampleCompatibility::determineFittestFormat(uint32_t *pnChannels,uint32_t *pnSampleRate)
{
	map<uint32_t,CDigitalValueRange>::iterator iter;
	uint32_t nHighest,nLowest;
	uint32_t iSourceMask=0;

	iSourceMask |= *pnChannels == 2 ? CSampleCompatibility::supportsStereo : CSampleCompatibility::supportsMono;
	iSourceMask |= CSampleCompatibility::supports16Bit;
	nHighest=0;
	nLowest=0xFFFFFFFF;
	for(iter=freqs.begin();freqs.end() != iter;iter++)
	{
		if ((iter->first & iSourceMask) == iSourceMask)
		{
			if (iter->second.m_bIsRange)
			{
				if (*pnSampleRate >= iter->second.m_values[0] && *pnSampleRate <= iter->second.m_values[1])
					return;
				else
				{
					nLowest=min(nLowest,(uint32_t)iter->second.m_values[0]);
					nHighest=max(nHighest,(uint32_t)iter->second.m_values[1]);
				}
			}
			else
			{
				for(uint32_t i=0;i < iter->second.m_values.size();i++)
				{
					if (iter->second.m_values[i] == *pnSampleRate)
						return;
					else
					{
						nLowest=min(nLowest,(uint32_t)iter->second.m_values[i]);
						nHighest=max(nHighest,(uint32_t)iter->second.m_values[i]);
					}
				}
			}
		}
	}
	if (nHighest > 0)
	{
		if ((uint32_t)*pnSampleRate < nLowest)
			*pnSampleRate=nLowest;
		else
			*pnSampleRate=nHighest;
	}
	else
	{
		if (*pnChannels > 1)
		{
			*pnChannels-=1;
			determineFittestFormat(pnChannels,pnSampleRate);
		}
		else
		{
			TRACEIT2("that is odd, couldnt find anything that fits\n");
		}
	}
}

bool CSampleCompatibility::bIsCompatible(uint32_t nBitsPerSample,uint32_t nChannels,uint32_t nSampleRate)
{
	map<uint32_t,CDigitalValueRange>::iterator iter;
	uint32_t iSourceMask=0;
	iSourceMask |= nChannels == 2 ? CSampleCompatibility::supportsStereo : CSampleCompatibility::supportsMono;
	iSourceMask |= nBitsPerSample == 16 ? CSampleCompatibility::supports16Bit : CSampleCompatibility::supports8Bit;
	TRACEIT2("checking sample compatiblity...\n");
	for(iter=freqs.begin();freqs.end() != iter;iter++)
	{
		if ((iter->first & iSourceMask) == iSourceMask)
		{
			TRACEIT2("bitwidth and channel count supported...\n");
			if (iter->second.m_bIsRange)
			{
				if (nSampleRate >= iter->second.m_values[0] && nSampleRate <= iter->second.m_values[1])
				{
					TRACEIT2("frequency in range - format compatible\n");
					return true;
				}
			}
			else
			{
				for(uint32_t i=0;i < iter->second.m_values.size();i++)
				{
					if (iter->second.m_values[i] == nSampleRate)
					{
						TRACEIT2("frequency matching - format compatible\n");
						return true;
					}
				}
			}
			//return true;
		}
	}
	TRACEIT2("input format incompatible\n");
	return false;
}

void CMobileSampleContent::CreateFile(ofstream &ar,const TCHAR *pszFileName)
{
	ar.open(pszFileName,ios_base::binary | ios_base::out);
}

void CMobileSampleContent::CloseFile(ofstream &ar)
{
	ar.close();
}

void CMobileSampleContent::Free(void *pBuffer)
{
	if (pBuffer != NULL)
	{
		TRACEIT2("free: %p\n",pBuffer);
		free(pBuffer);
	}
}

void *CMobileSampleContent::Alloc(uint32_t nSize)
{
	void *ptr=calloc(nSize,1);
	TRACEIT2("alloc: %p\n",ptr);
	return ptr;
}

void CMobileSampleContent::Reset(void *pNew,uint32_t nSize)
{
	if (m_pcCSBuffer)
		Free(m_pcCSBuffer);
	m_pcCSBuffer=pNew;
	m_nCSSize=nSize;
}

void CMobileSampleContent::AttachParameter(converting_parameters *parm)
{
	ASSERT(parm);
	m_pParameters=parm;
}

uint32_t CMobileSampleContent::nGetSamplePlaytime(void)
{
	uint32_t nRet=0;
	uint64_t nNom;
	uint32_t nDiv;
	nNom=((uint64_t)m_nCSSize*1000)*8;
	nDiv=(m_nCSSamplesPerSecond*m_nCSBitsPerSample)*m_nCSChannels;
	TRACEIT2("nNom %ld\n",nNom);
	TRACEIT2("nDiv %ld\n",nDiv);
	//ASSERT(nDiv);
	if (nDiv)
		nRet=round(nNom,nDiv);
	TRACEIT2("value=%ld\n",nRet);
	return nRet;
}

void CMobileSampleContent::Write(ostream &ar)
{
	Log2(verbLevErrors,"this CODEC is in fact just a decoder as it does not support encoding\n");
	TRACEIT2("this has to be overridden if needed\n");
	ASSERT(FALSE);
}
/*
void CMobileContent::Serialize(istream &ar)
{
	TRACEIT2("this has to be overridden\n");
	ASSERT(FALSE);
}
*/
void CMobileSampleContent::RenderID3V1(ostream &out,CMobileSampleContent *pSource)
{
	int i;
	out.write("TAG",3);
	//title
	i=min((uint32_t)pSource->m_strInfo[infoTitle].length(),30);
	out.write(pSource->m_strInfo[infoTitle].c_str(),i);
	if (i < 30)
		out.write("\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000",30-i);
	//artist
	i=min((uint32_t)pSource->m_strInfo[infoArtist].length(),30);
	out.write(pSource->m_strInfo[infoArtist].c_str(),i);
	if (i < 30)
		out.write("\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000",30-i);
	//album
	i=min((uint32_t)pSource->m_strInfo[infoAlbum].length(),30);
	out.write(pSource->m_strInfo[infoAlbum].c_str(),i);
	if (i < 30)
		out.write("\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000",30-i);
	//date created
	i=min((uint32_t)pSource->m_strInfo[infoYear].length(),4);
	out.write(pSource->m_strInfo[infoYear].c_str(),i);
	if (i < 4)
		out.write("\000\000\000\000",4-i);
	//comments
	i=min((uint32_t)pSource->m_strInfo[infoComments].length(),30);
	out.write(pSource->m_strInfo[infoComments].c_str(),i);
	if (i < 30)
		out.write("\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000",30-i);
	//genre (12 = other) 
	out.write("\014",1);
}

CMobileContent::CMobileContent(uint32_t iFormatId) : m_nFormatId(iFormatId), 
													m_nMagicSize(0),
													m_pcMagic(""), 
													m_sFormatName("unknown"),
													m_sFormatDescription("n/a"),
													m_sDefaultExtension("raw"),
													m_pListModules(NULL),
													m_bParseOnly(false)
{
}

CMobileContent::~CMobileContent(void)
{
	TRACEIT2("killing mobile content\n");
}

/*\
 * <---------- bMagicHead ---------->
 * @m generic magic byte/s identification
 * --> I N <-- @p
 * std::istream &ar - input stream
 * uint32_tnSize - input size
 * <-- OUT --> @r
 * bool - true=matched
\*/
bool CMobileContent::bMagicHead(std::istream &ar,uint32_t nSize)
{
	uint32_t i;
	unsigned char cCompare;

	m_nFileSize=nSize;

	Log2(verbLevDebug3,"comparing %d byte with: \"%s\" \n",m_nMagicSize,m_pcMagic);

	if (m_nMagicSize == 0 || nSize < m_nMagicSize)
		return false;
	for (i=0;i < m_nMagicSize;i++)
	{
		ar.read((char *)&cCompare,1);
		if (cCompare != (unsigned char)*(m_pcMagic+i))
			return false;
	}
	return true;
}

/*\
 * <---------- bLoad ---------->
 * @m read from file path
 * --> I N <-- @p
 * const char *pszPath - path to source file
 * <-- OUT --> @r
 * bool - true=ok
\*/
bool CMobileContent::bLoad(const char *pszPath)
{
	bool bRet=true;
	ifstream ar;
	try 
	{
		ar.open(pszPath,ios_base::in | ios_base::binary);
		ar.seekg(0L,ios_base::end);
		m_nFileSize=ar.tellg();
		ar.seekg(0L,ios_base::beg);
		Read(ar);
		ar.close();
	}
	catch(ifstream::failure const &e)
	{
		TRACEIT("CMobileContent::Load - file exception: %s\n",e.what());
		bRet=false;
	}
	return bRet;
}

/*\
 * <---------- bLoad ---------->
 * @m read from memory stream
 * --> I N <-- @p
 * char *pcBuffer - pointer to source data
 * unsigned int nSize - size of the data package
 * <-- OUT --> @r
 * bool - true=ok
\*/
bool CMobileContent::bLoad(char *pcBuffer,uint32_t nSize)
{
	bool bRet=true;
	istrstream ar(pcBuffer,nSize);
	m_nFileSize=nSize;
	try 
	{
		Read(ar);
	}
	catch(ifstream::failure const &e)
	{
		TRACEIT2("file exception: %s\n",e.what());
		bRet=false;
	}
	return bRet;
}

//CMobileProperty::CMobileProperty(void) : m_sFilePath(""), m_nFileSize(0), m_sFormatName("")
/*
bool CMobileSampleContent::bRenderWaveDisplay(CMemoryBitmap *pbm,MyCol *pColBackFrom,MyCol *pColBackTo,MyCol *pColForeSweep,MyCol *pColMax,unsigned int nGlossHeight,unsigned int nGlossFrame,double dGlossFrom,double dGlossTo)
{
	bool bRet=false;
	uint32_t nSample=0;
	void *pcCurrentSample=NULL;
	int y,yy;

	uint32_t nWidth=pbm->nGetClientWidth();
	uint32_t nHeight=pbm->nGetClientHeight();

	void *pcEnd=NULL;
	uint32_t nBitPerSample=m_nCSBitsPerSample == 8 ? 8 : 16;
	int32_t nSampleStepping;
	
	uint32_t nSampleCount;
	uint32_t nSampleWeight;
	signed nPeakNeg,nPeakPos;
	uint32_t nActive=0;

	TRACEIT2("sample loaded, size is %d byte...\n",m_nCSSize);

	//CMemoryBitmap bm(nWidth,nHeight,nFramePix,pColBackFrom,pColFore);

	if(pbm)
	{
		pbm->Erase();

		if (pColBackFrom && pColBackTo)
			pbm->FadeFill(pColBackFrom,pColBackTo);

		if (m_pcCSBuffer && nWidth && m_nCSChannels)
		{
			nSampleStepping=(nBitPerSample>>3)*m_nCSChannels;
			nSampleCount=m_nCSSize / ((nBitPerSample>>3)*m_nCSChannels);
			nSampleWeight=(nSampleCount*100)/nWidth;
			pcEnd=(unsigned char *)m_pcCSBuffer+m_nCSSize;
			nPeakPos=0;
			nPeakNeg=0;
			nActive=0;
			pcCurrentSample=(char *)m_pcCSBuffer+(nActive/100);
			pbm->SetPenCol(pColMax);
			for (unsigned int x=0;x < nWidth;x++)
			{
				if (pcCurrentSample > pcEnd)
					break;
				
				for (;nActive < nSampleWeight;nActive+=100)
				{			
					if (nBitPerSample == 16)
					{
						nSample=*(int16_t *)pcCurrentSample;
						if (m_nCSChannels > 1)
							nSample=(nSample + *(((int16_t *)pcCurrentSample)+1))/2;
					}
					else
					{
						nSample=*(unsigned char *)pcCurrentSample;
						if (m_nCSChannels > 1)
							nSample=(nSample+*(((unsigned char *)pcCurrentSample)+1))/2;
						nSample=(nSample<<8)-32768;
					}
					//TRACEIT2("%d\n",nSample);
					if (nSample < nPeakNeg)
						nPeakNeg=nSample;
					if (nSample > nPeakPos)
						nPeakPos=nSample;

					pcCurrentSample=(char *)pcCurrentSample+nSampleStepping;
				}
				y=nHeight/2;
				y+=(nPeakNeg*(int16_t)(nHeight/2)) / (int16_t)0x8000;

				yy=nHeight/2;
				yy+=(nPeakPos*(int16_t)(nHeight/2)) / (int16_t)0x8000;

				//pbm->SetPenCol(pColFore);
				pbm->LineVert(x,y,yy,pColForeSweep);

				pbm->Plot(x,y);
				pbm->Plot(x,yy);
				
				nPeakPos=0;
				nPeakNeg=0;
				nActive=nActive%nSampleWeight;
			}
			if (pcCurrentSample && pcEnd)
			{
				TRACEIT2("diff: %d\n",(char *)pcEnd-(char *)pcCurrentSample);
			}
		}
		else
		{
			TRACEIT2("no sample in memory!\n");
		}

		if (nGlossHeight)
		{
			pbm->Gloss(0,0,pbm->nGetWidth(),nGlossHeight,dGlossFrom,dGlossTo);
			pbm->Gloss(0,pbm->nGetHeight()/2+2*((pbm->nGetHeight()/2)-nGlossHeight),pbm->nGetWidth(),pbm->nGetHeight(),0.8,1.0);
		}
	}
	return bRet;
}
*/

/*\
 * <---------- CMobileSampleContent :: SetSourceSample ----------> 
 * @m assign a source for the converting process
 * --> I N <-- @p
 * CMobileSampleContent *pSource - source sample object pointer
\*/
void CMobileSampleContent::AttachSourceSample(CMobileSampleContent *pSource)
{
	m_pCSSource=pSource;
	m_pCSSource->AddRef();
}

/*
bool CMobileSampleContent::bRenderWaveDisplay2(	CMemoryBitmap *pbmNorm,CMemoryBitmap *pbmPlay,
												MyCol *pColBackFrom,MyCol *pColBackTo,
												MyCol *pColPlayFrom,MyCol *pColPlayTo,
												MyCol *pColNormSweep,MyCol *pColNormMax,
												MyCol *pColPlaySweep,MyCol *pColPlayMax,
												unsigned int nGlossHeight,unsigned int nGlossFrame,double dGlossFrom,double dGlossTo)
{
	bool bRet=false;
	signed int nSample=0;
	void *pcCurrentSample=NULL;
	int y,yy,y2,yy2;

	unsigned int nWidth=pbmNorm->nGetClientWidth();
	unsigned int nHeight=pbmNorm->nGetClientHeight();

	void *pcEnd=NULL;
	int nBitPerSample=m_nCSBitsPerSample == 8 ? 8 : 16;
	int nSampleStepping;
	
	uint32_tnSampleCount;
	uint32_tnSampleWeight;
	signed nPeakNeg,nPeakPos;
	uint32_tnActive=0;

	TRACEIT2("sample loaded, size is %d byte...\n",m_nCSSize);

	//CMemoryBitmap bm(nWidth,nHeight,nFramePix,pColBackFrom,pColFore);

	if(pbmNorm)
	{
		pbmNorm->Erase();
		pbmPlay->Erase();

		if (pColBackFrom && pColBackTo)
		{
			pbmNorm->FadeFill(pColBackFrom,pColBackTo);
			pbmPlay->FadeFill(pColPlayFrom,pColPlayTo);
		}

		if (m_pcCSBuffer && nWidth && m_nCSChannels)
		{
			nSampleStepping=(nBitPerSample>>3)*m_nCSChannels;
			nSampleCount=m_nCSSize / ((nBitPerSample>>3)*m_nCSChannels);
			nSampleWeight=(nSampleCount*100)/nWidth;
			pcEnd=(unsigned char *)m_pcCSBuffer+m_nCSSize;
			nPeakPos=0;
			nPeakNeg=0;
			nActive=0;
			pcCurrentSample=(char *)m_pcCSBuffer+(nActive/100);
			pbmNorm->SetPenCol(pColNormMax);
			pbmPlay->SetPenCol(pColPlayMax);
			for (unsigned int x=0;x < nWidth;x++)
			{
				if (pcCurrentSample > pcEnd)
					break;
				
				for (;nActive < nSampleWeight;nActive+=100)
				{			
					if (nBitPerSample == 16)
					{
						nSample=*(signed short int *)pcCurrentSample;
						if (m_nCSChannels > 1)
							nSample=(nSample + *(((signed short int *)pcCurrentSample)+1))/2;
					}
					else
					{
						nSample=*(unsigned char *)pcCurrentSample;
						if (m_nCSChannels > 1)
							nSample=(nSample+*(((unsigned char *)pcCurrentSample)+1))/2;
						nSample=(nSample<<8)-32768;
					}
					//TRACEIT2("%d\n",nSample);
					if (nSample < nPeakNeg)
						nPeakNeg=nSample;
					if (nSample > nPeakPos)
						nPeakPos=nSample;

					pcCurrentSample=(char *)pcCurrentSample+nSampleStepping;
				}
				y=nHeight/2;
				y+=(nPeakNeg*(signed int)(nHeight/2)) / (signed int)0x8000;
				if(y)
					y2=y-1;
				else
					y2=y;

				yy=nHeight/2;
				yy+=(nPeakPos*(signed int)(nHeight/2)) / (signed int)0x8000;
				yy2=min((signed int)nHeight,yy+1);

				//pbm->SetPenCol(pColFore);
				//if (x%2)
				{
					pbmNorm->LineVert(x,y,yy,pColNormSweep);
					pbmNorm->Plot(x,y);
					pbmNorm->Plot(x,yy);
					pbmPlay->LineVert(x,y2,yy2,pColPlaySweep);
					pbmPlay->Plot(x,y2);
					pbmPlay->Plot(x,yy2);
				}	
				nPeakPos=0;
				nPeakNeg=0;
				nActive=nActive%nSampleWeight;
			}
			if (pcCurrentSample && pcEnd)
			{
				TRACEIT2("diff: %d\n",(char *)pcEnd-(char *)pcCurrentSample);
			}
		}
		else
		{
			TRACEIT2("no sample in memory!\n");
		}

		if (nGlossHeight)
		{
			pbmNorm->Gloss(0,0,pbmNorm->nGetWidth(),nGlossHeight,dGlossFrom,dGlossTo);
			pbmNorm->Gloss(0,pbmNorm->nGetHeight()/2+2*((pbmNorm->nGetHeight()/2)-nGlossHeight),pbmNorm->nGetWidth(),pbmNorm->nGetHeight(),0.8,1.0);
			pbmPlay->Gloss(0,0,pbmNorm->nGetWidth(),nGlossHeight,dGlossFrom,dGlossTo);
			pbmPlay->Gloss(0,pbmNorm->nGetHeight()/2+2*((pbmNorm->nGetHeight()/2)-nGlossHeight),pbmNorm->nGetWidth(),pbmNorm->nGetHeight(),0.8,1.0);
		}
	}
	return bRet;
}
*/

/*\
 * <---------- fCalcRMS ---------->
 * @m calc peak and rms level
 * --> I N <-- @p
 * signed short int *ibuf - input data
 * uint32_tnSampleCount - number of samples
 * <-- OUT --> @r
 * double - RMS value
\*/
/*
double CMobileSampleContent::fCalcRMS(signed short int *ibuf,uint32_tnSampleCount)
{
	uint32_tnToDo;
	signed short int nIn;
	uint32_tint nThreshAbs;
	uint32_tint nAbsolute;
	uint32_tnCounted;
	double fRes;
	double fRet;
	double fSum=0.0;
	uint32_tint nPeak=0;
	uint32_tint nPeakOffset=0;

	nThreshAbs=ndBToAmp(m_fRMSThreshDb);
	TRACEIT2("threshhold %f, abs: %04Xh\n",m_fRMSThreshDb,nThreshAbs);
	
	if (nSampleCount)
	{
		nToDo=nSampleCount;
		nCounted=0;

		while (nToDo--)
		{
			nIn=(*ibuf++);
			if (nIn < 0)							//get absolute value of this sample
				nAbsolute=0-nIn;
			else
				nAbsolute=nIn;
			if (nAbsolute > nThreshAbs)
			{
				fSum+=(signed long int)nIn*(signed long int)nIn;
				++nCounted;
			}
			if (nAbsolute > m_nPeak)				//sample value higher than current max?
			{
				TRACEIT2("peak %04X (%1.2fdB) at %08d\n",nAbsolute,fAmpTodB(nAbsolute),nSampleCount-nToDo);
				nPeak=nAbsolute;					//remember this peak as the new max
				nPeakOffset=nSampleCount-nToDo;
			}
		};
		if (nCounted)
			fRes=sqrt(fSum/nSampleCount);
		else
			fRes=0.0;
	}
	fRet=fAmpTodB((const uint32_tint)fRes);
	if (_finite(fRet) == 0)
		fRet = -96.0;
	return fRet;
}

signed int CMobileSampleContent::ndBToAmp(const double dB)
{ 
	return (signed int)(0x8000 * pow(10.0,dB/20.0)); 
}

double CMobileSampleContent::fAmpTodB(const signed int nAmp)
{
	return 20.0*log10(fabs((double)nAmp / 0x8000));
}
*/

CReferenceObject::CReferenceObject(void) 
{ 
	m_nRefCount=0; 
}

CReferenceObject::~CReferenceObject(void) 
{ 
	if (m_nRefCount) 
	{ 
		TRACEIT2("object still referenced somewhere\n"); 
	} 
}

void CReferenceObject::AddRef(void) 
{
	TRACEIT2("adding a reference to our object %s\n",typeid(*this).name()); 
	++m_nRefCount;
}

void CReferenceObject::Release(void) 
{ 
	TRACEIT2("%s release called\n",typeid(*this).name());
	if (--m_nRefCount == 0) 
	{
		TRACEIT2("no more references active, safe to delete\n");
		Log2(verbLevDebug3,"no more references active, safe to delete\n");
		delete this;
	}
	else
	{
		TRACEIT2("still %d references active\n",m_nRefCount); 
	}
}
