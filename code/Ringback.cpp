/*\
 * Ringback.cpp
 * Copyright (C) 2004-2009, MMSGURU - written by Till Toenshoff
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
#include <math.h>
#include "../retroBase/Basics.h"
#include "../retroBase/Integer.h"
#include "Filter.h"
#include "Ringback.h"

CRingback::CRingback()
{
}

CRingback::~CRingback()
{
}

/*\
 * <---------- Init ---------->
 * @m initialize
 * --> I N <-- @p
 * int nSampleRate - sample rate
 * int nBitsPerSample - bits per sample
 * int nChannels - channels
 * int iSet - index of the ITU signal set
\*/
void CRingback::Init(int nSampleRate, int nBitsPerSample, int nChannels,const char *pszCountryConfigFile,const char *pszCountryId)
{
	m_nSampleRate=nSampleRate;
	m_nBitsPerSample=nBitsPerSample;
	m_nChannels=nChannels;

	bool bRet=true;
	unsigned int nFileSize;
	ifstream ar;

	ar.exceptions(ios_base::badbit|ios_base::eofbit|ios_base::failbit);
	try 
	{
		ar.open(pszCountryConfigFile,ios_base::in | ios_base::binary);
		ar.seekg(0L,ios_base::end);
		nFileSize=ar.tellg();
		ar.seekg(0L,ios_base::beg);
	}
	catch(ifstream::failure const &e)
	{
		TRACEIT2("file exception: %s\n",e.what());
		Log2(verbLevErrors,"could not open ITU-signal configuration \"%s\"\n",pszCountryConfigFile);
		return;
	}
	char *pcDest=NULL;
	if (nFileSize)
	{
		pcDest=new char[nFileSize+1];
		try 
		{
			ar.read(pcDest,nFileSize);
		}
		catch(ifstream::failure const &e)
		{
			TRACEIT2("file exception: %s\n",e.what());
			delete [] pcDest;
			Log2(verbLevErrors,"failed while reading ITU-signal configuration \"%s\"\n",pszCountryConfigFile);
			return;
		}
		pcDest[nFileSize]=0;
		SelectSignal((const char *)pcDest,pszCountryId);
		delete [] pcDest;
	}
}

bool CRingback::ParseSignal(const char *pcSignal)
{
	float fVal;
	CMyString strXml;
	CMyString strList;
	CMyString strElement;
	CMyString strContent;
	CMyString strAttribute;

	//TRACEIT2("XML: %s\n",pcSignal);
	strXml=pcSignal;

	set.dSineAmplification_dB=0.0;
	set.nComponentCount=0;
	set.nPhaseCount=0;
	set.nLoopStart=0;
	set.bMixTones=true;
	set.dFrequency[0]=set.dFrequency[1]=0;
	memset(set.nActivePhaseDuration,0,8*sizeof(int));
	memset(set.nPausePhaseDuration,0,8*sizeof(int));

	strElement=strXml.sGetXmlElementContent("sinegain");
	sscanf(strElement.c_str(),"%f",&fVal);
	set.dSineAmplification_dB=fVal;
	TRACEIT2("dSineAmplification_dB = %f\n",set.dSineAmplification_dB);

	strElement=strXml.sGetXmlElementContent("sinecount");
	sscanf(strElement.c_str(),"%d",&set.nComponentCount);
	TRACEIT2("nComponentCount = %d\n",set.nComponentCount);

	strElement=strXml.sGetXmlElementContent("phasecount");
	sscanf(strElement.c_str(),"%d",&set.nPhaseCount);
	TRACEIT2("nPhaseCount = %d\n",set.nPhaseCount);

	strElement=strXml.sGetXmlElementContent("indexoffset");
	sscanf(strElement.c_str(),"%d",&set.nLoopStart);
	TRACEIT2("nLoopStart = %d\n",set.nLoopStart);

	strElement=strXml.sGetXmlElement("combination");
	strAttribute=strElement.sGetXmlElementAttribute("mode");
	set.bMixTones=(strAttribute != "modulate");
	TRACEIT2("bMixTones = %d\n",set.bMixTones);

	strList=strXml.sGetXmlElement("tones");
	strContent=strXml.sGetCurrentXmlContent();
	//TRACEIT2("tones = %s\n",strContent.c_str());
	strElement=strContent.sGetFirstXmlElement("frequency");
	strAttribute=strContent.sGetCurrentXmlContent();
	int i=0;
	while (!strElement.empty() && i < 2)
	{
		sscanf(strAttribute.c_str(),"%f",&fVal);
		set.dFrequency[i]=fVal;
		TRACEIT2("dFrequency[%d] = %f\n",i,set.dFrequency[i]);
		strElement=strContent.sGetNextXmlElement("frequency");
		strAttribute=strContent.sGetCurrentXmlContent();
		++i;
	};
	set.nComponentCount=i;

	strList=strXml.sGetXmlElement("activephases");
	strContent=strXml.sGetCurrentXmlContent();
	//TRACEIT2("activaphases = %s\n",strContent.c_str());
	strElement=strContent.sGetFirstXmlElement("duration");
	strAttribute=strContent.sGetCurrentXmlContent();
	i=0;
	while (!strElement.empty() && i < 8)
	{		
		sscanf(strAttribute.c_str(),"%d",&set.nActivePhaseDuration[i]);
		TRACEIT2("nActivePhaseDuration[%d] = %d\n",i,set.nActivePhaseDuration[i]);
		strElement=strContent.sGetNextXmlElement("duration");
		strAttribute=strContent.sGetCurrentXmlContent();
		++i;
	};
	set.nPhaseCount=i;

	strList=strXml.sGetXmlElement("pausephases");
	strContent=strXml.sGetCurrentXmlContent();
	//TRACEIT2("pausephases = %s\n",strContent.c_str());
	strElement=strContent.sGetFirstXmlElement("duration");
	strAttribute=strContent.sGetCurrentXmlContent();
	i=0;
	while (!strElement.empty() && i < 8)
	{
		sscanf(strAttribute.c_str(),"%d",&set.nPausePhaseDuration[i]);
		TRACEIT2("nPausePhaseDuration[%d] = %d\n",i,set.nPausePhaseDuration[i]);
		strElement=strContent.sGetNextXmlElement("duration");
		strAttribute=strContent.sGetCurrentXmlContent();
		++i;
	};
	set.nPhaseCount=max(i,set.nPhaseCount);	
	return true;
}

bool CRingback::SelectSignal(const char *pcDest,const char *pcCountryId)
{
	typedef struct tagELEMENTTABLE
	{
		const char *szElementName[2];
		tstring *pstrAttributeDestination;
	}ElementTable;

	CMyString strXml;
	CMyString strElement;
	CMyString strContent;
	CMyString strAttribute;

	strXml=pcDest;

	strElement=strXml.sGetFirstXmlElement("itusignal");
	strContent=strXml.sGetCurrentXmlContent();
	while (!strElement.empty() && !strContent.empty())
	{
		strAttribute=strElement.sGetXmlElementAttribute("country");
		if (strAttribute == CMyString(pcCountryId))
			return ParseSignal(strContent.c_str());
		strElement=strXml.sGetNextXmlElement("itusignal");
		strContent=strXml.sGetCurrentXmlContent();
	};
	Log2(verbLevErrors,"ITU-signal definition \"%s\" not found\n",pcCountryId);
	return false;
}

/*\
 * <---------- Process ---------->
 * @m 
 * --> I N <-- @p
 * signed short int *ibuf - 
 * signed short int *obuf - 
 * uint32_tnSampleCount - 
\*/
void CRingback::Process(signed short int *ibuf,signed short int *obuf, uint32_t nSampleCount)
{
	const double pi = 3.1415926535;
	double genMixedLeft,genMixedRight;
	double fAngle[2] = {0.0,0.0};
	signed short int inLeft,inRight;
	double inSine[2];
	uint32_t nActiveCount,nPauseCount;
	uint32_t nActiveSamples[4],nPauseSamples[4];
	int nPhase;
	double fMixMul;
	bool bActive;
	int i;

	if(set.bMixTones)
		fMixMul=((ndBToAmp(set.dSineAmplification_dB) * set.nComponentCount) + 32767.0) / 32767.0;
	else
		fMixMul=(ndBToAmp(set.dSineAmplification_dB) + 32767.0) / 32767.0;
	for (i=0;i < set.nPhaseCount;i++)
	{
		nActiveSamples[i]=(unsigned long)(((uint64_t)m_nSampleRate*set.nActivePhaseDuration[i])/1000);
		nPauseSamples[i]=(unsigned long)(((uint64_t)m_nSampleRate*set.nPausePhaseDuration[i])/1000);
	}

	nPhase=0;
	bActive=true;
	nActiveCount=nActiveSamples[nPhase];
	nSampleCount>>=(m_nChannels-1);
	while (nSampleCount--)
	{
		inLeft = *ibuf;
		if (m_nChannels == 2)
			inRight = *(ibuf+1);
		if (bActive)
		{
			genMixedLeft=0;
			genMixedRight=0;
			for (i=0;i < set.nComponentCount;i++)
			{
				inSine[i] = sin(fAngle[i]);
				if (set.bMixTones || i == 0)
				{
					genMixedLeft += inSine[i];
					if (m_nChannels == 2)
						genMixedRight += inSine[i];
				}
				else
				{
					genMixedLeft *= inSine[i];
					if (m_nChannels == 2)
						genMixedRight *= inSine[i];
				}
			}
			genMixedLeft = ndBToAmp(set.dSineAmplification_dB) * genMixedLeft;

			if (m_nChannels == 2)
				genMixedRight = ndBToAmp(set.dSineAmplification_dB) * genMixedRight;

			genMixedLeft=(signed int)((genMixedLeft+(double)inLeft)/fMixMul);
			if (m_nChannels == 2)
				genMixedRight=(signed int)((genMixedRight+(double)inRight)/fMixMul);

			for (i=0;i < set.nComponentCount;i++)
			{
				fAngle[i]+=2 * pi * set.dFrequency[i] / m_nSampleRate;
				if(fAngle[i] > 2*pi)
					fAngle[i]-=2*pi;
			}
			if (!nActiveCount--)
			{
				bActive=false;
				nPauseCount=nPauseSamples[nPhase];
			}
		}
		else
		{
			genMixedLeft=(signed int)(inLeft/fMixMul);
			genMixedRight=(signed int)(inRight/fMixMul);
			if (!nPauseCount--)
			{
				for (i=0;i < set.nComponentCount;i++)
					fAngle[i]=0.0;
				if (nPhase < set.nPhaseCount-1)
					++nPhase;
				else
					nPhase=set.nLoopStart;
				bActive=true;
				nActiveCount=nActiveSamples[nPhase];
			}
		}
		if (m_nChannels == 2)
		{
			*obuf = (signed int)genMixedLeft;
			*(obuf+1) = (signed int)genMixedRight;
		}
		else
			*obuf = (signed int)genMixedLeft;
		
		obuf+=m_nChannels;
		ibuf+=m_nChannels;
	};
}
