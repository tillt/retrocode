/*\
 * MobileProperty.cpp
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
#include <string.h>
#include <strstream>
#include "Basics.h"
#include "Integer.h"
#include "MyString.h"
#include "MobileContent.h"
#include "../include/Resource.h"
#include "Endian.h"

CMobileProperty::CMobileProperty(void)
{
	InitPropertyMapping();
}

CMobileProperty::~CMobileProperty(void)
{
}

const TCHAR *CMobileProperty::pszGetFirstUsedProperty(void)
{
	m_iGetIndex=0;
	return pszGetNextUsedProperty();
}

const TCHAR *CMobileProperty::pszGetNextUsedProperty(void)
{
	if ((unsigned int)m_iGetIndex < m_astrPropSet.size())
		return m_astrPropSet[m_iGetIndex++].c_str();
	else
		return NULL;
}

/*\
 * <---------- InitFromContent ---------->
 * @m basic property object initialize
 * --> I N <-- @p
 * LPCTSTR szPath - source file path
 * unsigned int nSize - file size
 * CMobileContent *pm - content data object
\*/
void CMobileProperty::InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm)
{
	TCHAR idbuffer[256]="";
	TCHAR formatbuffer[256]="";
	ASSERT(szPath);
	if (pm)
	{
		Log2(verbLevDebug2,"source file path: %s\n",szPath);		
		if (szPath)
		{
			//SetProperty(prstrPath,pm->m_sFileName.c_str());
			SetProperty(prstrPath,szPath);
		}
		_itoa(pm->m_nFormatId,idbuffer,10);
		strcpy(formatbuffer,pm->m_sFormatName.c_str());
	}
	else
	{
		CMyString str;
		_itoa(0,idbuffer,10);
		str.LoadString(IDS_FORMATNAME_UNKNOWN);
		strcpy(formatbuffer,str.c_str());
	}
	SetProperty_long(prstrFormat,formatbuffer,"id",idbuffer);
	Log2(verbLevDebug2,"file size: %d\n",nSize);		
	SetProperty(prnumFileSize,nSize);
}

void CMobileProperty::writeXMLhead(ostream &ar)
{
	ar << "<ContentInfo>\n";
}

void CMobileProperty::writeXMLtail(ostream &ar)
{
	ar << "</ContentInfo>\n";
}

/*\
 * <---------- CMobileProperty :: writeXML ----------> 
 * @m render xml property snipped
 * --> I N <-- @p
 * ostream &ar - output stream
\*/
void CMobileProperty::writeXML(ostream &ar)
{
	for (unsigned int i=0;i < m_astrPropSet.size();i++)
		RenderXmlProperty(m_astrPropSet[i].c_str(),ar);
}

tstring CMobileProperty::sXmlTidy(tstring &strIn)
{
	return sXmlTidy(strIn.c_str());
}

/*\
 * <---------- sXmlTidy ---------->
 * @m translation function for XML-conformant strings
 * --> I N <-- @p
 * const TCHAR *pszIn - non-xml conformant string
 * <-- OUT --> @r
 * tstring - xml-conformant string
\*/
tstring CMobileProperty::sXmlTidy(const TCHAR *pszIn)
{
	int i;
	tstring sOut;

	map<TCHAR,tstring> mapChar;
	map<TCHAR,tstring>::const_iterator iter;
	for (char c=' ';c <= '~';c++)
		mapChar[c]=c;
	mapChar['&']=_T("&#38;");
	mapChar['€']=_T("&#8364;");
	mapChar['£']=_T("&#163;");
	mapChar['¿']=_T("&#191;");
	mapChar['¡']=_T("&#161;");
	mapChar['<']=_T("&#60;");
	mapChar['>']=_T("&#62;");
	mapChar['\"']=_T("&#34;");
	mapChar['à']=_T("&#224;");
	mapChar['À']=_T("&#192;");
	mapChar['á']=_T("&#225;");
	mapChar['Á']=_T("&#193;");
	mapChar['ä']=_T("&#228;");
	mapChar['Ä']=_T("&#196;");
	mapChar['ó']=_T("&#243;");
	mapChar['Ó']=_T("&#211;");
	mapChar['ö']=_T("&#246;");
	mapChar['Ö']=_T("&#214;");
	mapChar['©']=_T("&#169;");
	mapChar['è']=_T("&#232;");
	mapChar['È']=_T("&#200;");
	mapChar['é']=_T("&#233;");
	mapChar['É']=_T("&#201;");
	mapChar['ü']=_T("&#252;");
	mapChar['Ü']=_T("&#220;");
	mapChar['í']=_T("&#237;");
	mapChar['Í']=_T("&#205;");
	mapChar['ñ']=_T("&#241;");
	mapChar['Ñ']=_T("&#209;");

	for (i=0;i < (int)strlen(pszIn);i++)
	{
		if (pszIn[i] == '&' && pszIn[i+1] == '#')
			sOut+=pszIn[i];
		else
		{
			iter=mapChar.find((TCHAR)pszIn[i]);
			if (iter != mapChar.end())
				sOut+=iter->second;
			else
			{
				TRACEIT2("illegal character code 0x%02X",pszIn[i]);
			}
		}
	}
	return sOut;
}

void CMobileProperty::setProp(const TCHAR *pszId,const TCHAR *pszName,const TCHAR *pszValue)
{
	if (pszValue)
		m_mapPropAttribute[tstring(pszId)][tstring(pszName)]=pszValue;
	m_astrPropSet.push_back(tstring(pszId));
}

void CMobileProperty::setProperty(const TCHAR *pszId,const TCHAR *pszString,const TCHAR *pszName,const TCHAR *pszValue)
{
	m_mapPropString[tstring(pszId)]=pszString;
	setProp(pszId,pszName,pszValue);
}

void CMobileProperty::setProperty(const TCHAR *pszId,uint32_t nNumber,const TCHAR *pszName,const TCHAR *pszValue)
{
	m_mapPropNumber[tstring(pszId)]=nNumber;
	setProp(pszId,pszName,pszValue);
}

void CMobileProperty::setProperty(const TCHAR *pszId,bool bFlag,const TCHAR *pszName,const TCHAR *pszValue)
{
	m_mapPropBool[tstring(pszId)]=bFlag;
	setProp(pszId,pszName,pszValue);
}

tstring CMobileProperty::strGetInfoToPropertyStringIndex(uint32_t nInfoId)
{
	tstring im[CMobileSampleContent::infoLast];

	im[CMobileSampleContent::infoTitle]="prstrName";
	im[CMobileSampleContent::infoSubTitle]="prstrSubTitle";
	im[CMobileSampleContent::infoAlbum]="prstrAlbum";
	im[CMobileSampleContent::infoArtist]="prstrArtist";
	im[CMobileSampleContent::infoArchiveLocation]="prstrArchiveLocation";
	im[CMobileSampleContent::infoCommissioned]="prstrCommissioned";
	im[CMobileSampleContent::infoComments]="prstrComment";
	im[CMobileSampleContent::infoComposer]="prstrComposer";
	im[CMobileSampleContent::infoCopyright]="prstrCopyright";
	im[CMobileSampleContent::infoCropped]="prstrCropped";
	im[CMobileSampleContent::infoDate]="prstrDate";
	im[CMobileSampleContent::infoDimensions]="prstrDimensions";
	im[CMobileSampleContent::infoDotsPerInch]="prstrDotsPerInch";
	im[CMobileSampleContent::infoEngineer]="prstrEngineer";
	im[CMobileSampleContent::infoEncodedBy]="prstrEncodedBy";
	im[CMobileSampleContent::infoGenre]="prstrGenre";
	im[CMobileSampleContent::infoKeywords]="prstrKeywords";
	im[CMobileSampleContent::infoLightness]="prstrLightness";
	im[CMobileSampleContent::infoMedium]="prstrMedium";
	im[CMobileSampleContent::infoPalette]="prstrPalette";
	im[CMobileSampleContent::infoProduct]="prstrProduct";
	im[CMobileSampleContent::infoSubject]="prstrSubject";
	im[CMobileSampleContent::infoSoftware]="prstrSoftware";
	im[CMobileSampleContent::infoSharpness]="prstrSharpness";
	im[CMobileSampleContent::infoSource]="prstrSource";
	im[CMobileSampleContent::infoSourceForm]="prstrSourceForm";
	im[CMobileSampleContent::infoTechnician]="prstrTechnician";
	im[CMobileSampleContent::infoSoundScheme]="prstrSoundScheme";
	im[CMobileSampleContent::infoPublisher]="prstrPublisher";
	im[CMobileSampleContent::infoUseLicense]="prstrUseLicense";
	im[CMobileSampleContent::infoLicenseURL]="prstrLicenseURL";
	im[CMobileSampleContent::infoLicenseTerm]="prstrLicenseTerm";
	im[CMobileSampleContent::infoExpirationDate]="prstrExpirationDate";
	im[CMobileSampleContent::infoTempoDescription]="prstrTempoDescription";		
	im[CMobileSampleContent::infoOriginalSource]="prstrOriginalSource";
	im[CMobileSampleContent::infoComposerNote]="prstrComposerNote";
	im[CMobileSampleContent::infoWords]="prstrWords";
	im[CMobileSampleContent::infoWriter]="prstrWriter";
	im[CMobileSampleContent::infoArranged]="prstrArranged";
	im[CMobileSampleContent::infoCategory]="prstrCategory";
	im[CMobileSampleContent::infoSubCategory]="prstrSubCategory";
	im[CMobileSampleContent::infoCopyManaged]="prstrCopyManaged";
	im[CMobileSampleContent::infoManagementInfo]="prstrManagementInfo";
	im[CMobileSampleContent::infoCarrier]="prstrCarrier";
	im[CMobileSampleContent::infoVendor]="prstrVendor";	
	im[CMobileSampleContent::infoDateCreated]="prstrDateCreated";
	im[CMobileSampleContent::infoDateRevised]="prstrDateRevised";
	im[CMobileSampleContent::infoYear]="prstrYear";
	im[CMobileSampleContent::infoIndex]="prstrIndex";
	return im[nInfoId];
}

/*\
 * <---------- CMobileProperty :: RenderXmlProperty ----------> 
 * @m 
 * --> I N <-- @p
 * const char *pszId - 
 * ostream &ar       - 
 * <-- OUT --> @r
 * 
\*/
void CMobileProperty::RenderXmlProperty(const char *pszId,ostream &ar)
{
	bool bRender=false;
	map<string,uint32_t>::iterator iter;
	if ((iter=m_mapPropType.find(pszId)) != m_mapPropType.end())
	{
		switch (iter->second)
		{
			case typeNumber:
			case typeBool:
				bRender=true;
			break;
			case typeString:
				if (!m_mapPropString[pszId].empty())
					bRender=true;
			break;
		}
	}
	if (bRender)
	{
		map<string,string>::iterator iterAttr;

		ar << "\t<" << m_mapPropXmlName[pszId];
		for (iterAttr=m_mapPropAttribute[pszId].begin();iterAttr != m_mapPropAttribute[pszId].end();iterAttr++)
			ar << " " << iterAttr->first << "=\"" << iterAttr->second << "\"";
		ar << ">";
		switch (m_mapPropType[pszId])
		{
			case typeNumber:
				ar << m_mapPropNumber[pszId];
			break;
			case typeBool:
				ar << m_mapPropBool[pszId];
			break;
			case typeString:
				ar << sXmlTidy(m_mapPropString[pszId]);
			break;
		}
		ar << "</" << m_mapPropXmlName[pszId] << ">\n";
	}
}

/*\
 * <---------- CMobileProperty :: getProp ----------> 
 * @m 
 * --> I N <-- @p
 * const char *pszId - 
 * string &strName   - 
 * string &strValue  - 
 * <-- OUT --> @r
 * 
\*/
bool CMobileProperty::getProp(const char *pszId,string &strName,string &strValue)
{
	strstream inout;
	strValue="";
	switch (m_mapPropType[pszId])
	{
		case typeNumber:
			inout << m_mapPropNumber[pszId] << endl;
		break;
		case typeBool:
			inout << m_mapPropBool[pszId] << endl;
		break;
		case typeString:
			inout << sXmlTidy(m_mapPropString[pszId]) << endl;
		break;
	}
	strValue=inout.str();
	return true;
}

/*\
 * <---------- CMobileProperty :: getProperty ----------> 
 * @m 
 * --> I N <-- @p
 * const char *pszId - 
 * bool &bFlag       - 
 * <-- OUT --> @r
 * 
\*/
bool CMobileProperty::getProperty(const char *pszId,bool &bFlag)
{
	map<string,bool>::iterator iter;
	bFlag=false;
	if ((iter=m_mapPropBool.find(tstring(pszId))) != m_mapPropBool.end())
		bFlag=iter->second;
	return true;
}

/*\
 * <---------- CMobileProperty :: getProperty ----------> 
 * @m 
 * --> I N <-- @p
 * const char *pszId - 
 * bool &bFlag       - 
 * string &strName   - 
 * string &strValue  - 
 * <-- OUT --> @r
 * 
\*/
/*
bool CMobileProperty::getProperty(const char *pszId,bool &bFlag,string &strName,string &strValue)
{
	bool bRet=CMobileProperty::getProperty(pszId,bFlag);
	if (bRet)
		bRet=getProp(pszId,strName,strValue);
	return bRet;
}
*/

/*\
 * <---------- CMobileProperty :: getProperty ----------> 
 * @m 
 * --> I N <-- @p
 * const char *pszId - 
 * int &nValue       - 
 * <-- OUT --> @r
 * 
\*/
bool CMobileProperty::getProperty(const char *pszId,uint32_t &nValue)
{
	map<string,uint32_t>::iterator iter;	
	nValue=0;
	if ((iter=m_mapPropNumber.find(tstring(pszId))) != m_mapPropNumber.end())
		nValue=iter->second;
	return true;
}

/*\
 * <---------- CMobileProperty :: getProperty ----------> 
 * @m 
 * --> I N <-- @p
 * const char *pszId - 
 * int &nValue       - 
 * string &strName   - 
 * string &strValue  - 
 * <-- OUT --> @r
 * 
\*/
/*
bool CMobileProperty::getProperty(const char *pszId,uint32_t &nValue,string &strName,string &strValue)
{
	bool bRet=CMobileProperty::getProperty(pszId,nValue);
	if (bRet)
		bRet=getProp(pszId,strName,strValue);
	return bRet;
}
*/

/*\
 * <---------- CMobileProperty :: getProperty ----------> 
 * @m 
 * --> I N <-- @p
 * const char *pszId - 
 * tstring &str      - 
 * <-- OUT --> @r
 * 
\*/
bool CMobileProperty::getProperty(const char *pszId,tstring &str)
{
	map<string,string>::iterator iter;	
	str="";
	if ((iter=m_mapPropString.find(tstring(pszId))) != m_mapPropString.end())
		str=iter->second;
	return true;
}
/*\
 * <---------- CMobileProperty :: getProperty ----------> 
 * @m 
 * --> I N <-- @p
 * const char *pszId - 
 * string &str       - 
 * string &strName   - 
 * string &strValue  - 
 * <-- OUT --> @r
 * 
\*/
/*
bool CMobileProperty::getProperty(const char *pszId,string &str,string &strName,string &strValue)
{
	bool bRet=CMobileProperty::getProperty(pszId,str);
	if (bRet)
		bRet=getProp(pszId,strName,strValue);
	return bRet;
}
*/

bool CMobileProperty::getPropertyWithAttribute(const char *pszId,string &strProp,const char *pszAttr,string &strValue)
{
	bool bRet=CMobileProperty::getProperty(pszId,strProp);
	if (bRet)
	{
		if (!m_mapPropAttribute[pszId][pszAttr].empty())
		{
			map<string,string>::iterator iter;

			if ((iter=m_mapPropAttribute[pszId].find(pszAttr)) != m_mapPropAttribute[pszId].end())
				strValue=iter->second;
			else
				bRet=false;
		}
		else
			bRet=false;
	}
	return bRet;
}

/*\
 * <---------- InitPropertyMapping ---------->
 * @m link numeric property identifiers with XML property names and their types
\*/
void CMobileProperty::InitPropertyMapping(void)
{
	TRACEIT2("CMobileProperty::InitpropertyMapping\n");

	propertyid(prnumFileSize,typeNumber,"FileSize");

	propertyid(prnumSampleRate,typeNumber,"SamplesPerSecond");
	propertyid(prnumChannels,typeNumber,"Channels");

	propertyid(prnumBitsPerSample,typeNumber,"BitsPerSample");
	propertyid(prnumBitRate,typeNumber,"BitsPerSecond");
	propertyid(prnumSequencePolyphony,typeNumber,"SequencePolyphony");
	propertyid(prnumPlaylength,typeNumber,"PlayTime");
	propertyid(prnumSequenceFormat,typeNumber,"SequenceFormat");
	propertyid(prnumSampleSize,typeNumber,"SampleSize");
	propertyid(prnumSamplePlaytime,typeNumber,"MaxSamplePlaytime");
	propertyid(prnumMpegVersion,typeNumber,"MPEGVersion");
	propertyid(prnumMinBitRate,typeNumber,"MinBitsPerSecond");
	propertyid(prnumMaxBitRate,typeNumber,"MaxBitsPerSecond");
	propertyid(prnumLoopCount,typeNumber,"LoopCount");
	propertyid(prnumFrameRate,typeNumber,"FrameRate");

	propertyid(prnumFrameWidth,typeNumber,"FrameWidth");
	propertyid(prnumFrameHeight,typeNumber,"FrameHeight");
	propertyid(prnumFrameBitPerPixel,typeNumber,"FrameBitPerPixel");
	propertyid(prnumFrameBitRate,typeNumber,"FrameBitRate");
	
	propertyid(prboolContainsMIP,typeBool,"ContainsMIP");
	propertyid(prboolContainsVibra,typeBool,"UsesVibra");
	propertyid(prboolOriginalRecording,typeBool,"OriginalRecording");
	propertyid(prboolContainsExtraPerc,typeBool,"UsesExtraPerc");
	propertyid(prboolDP2Vibra,typeBool,"DP2Vibra");
	propertyid(prboolContainsSamples,typeBool,"UsesSamples");
	propertyid(prboolContainsPicture,typeBool,"UsesGraphix");
	propertyid(prboolContainsSynthesizer,typeBool,"UsesSynthesizer");
	propertyid(prboolStatusSave,typeBool,"StatusBitSave");
	propertyid(prboolStatusCopy,typeBool,"StatusBitCopy");
	propertyid(prboolStatusEdit,typeBool,"StatusBitEdit");
	propertyid(prboolContainsHumanVoice,typeBool,"UsesHumanVoice");
	propertyid(prboolVariableBitRate,typeBool,"VariableBitRate");
	propertyid(prboolContainsCRC,typeBool,"UsesCRC");
	propertyid(prboolJointStereo,typeBool,"JointStereo");
	propertyid(prboolCopyrighted,typeBool,"Copyrighted");
	propertyid(prboolPrivate,typeBool,"Private");
	propertyid(prboolLoop,typeBool,"ContainsLoop");

	propertyid(prstrPath,typeString,"FilePath");
	propertyid(prstrFormat,typeString,"Format");
	propertyid(prstrSubFormat,typeString,"SubFormat");
	propertyid(prstrEncoding,typeString,"EncodingScheme");
	propertyid(prstrName,typeString,"MetaTitle");
	propertyid(prstrCopyright,typeString,"MetaCopyright");
	propertyid(prstrArranged,typeString,"MetaArranger");
	propertyid(prstrComment,typeString,"MetaComment");
	propertyid(prstrSubTitle,typeString,"MetaSubTitle");
	propertyid(prstrAlbum,typeString,"MetaAlbum");
	propertyid(prstrArtist,typeString,"MetaArtist");
	propertyid(prstrArchiveLocation,typeString,"MetaArchiveLocation");
	propertyid(prstrCategory,typeString,"MetaCategory");
	propertyid(prstrCommissioned,typeString,"MetaCommissioned");
	propertyid(prstrComposer,typeString,"MetaComposer");
	
	propertyid(prstrCropped,typeString,"MetaCropped");
	propertyid(prstrDate,typeString,"MetaDate");
	propertyid(prstrDateCreated,typeString,"MetaDateCreated");
	propertyid(prstrDateRevised,typeString,"MetaDateRevised");
	propertyid(prstrDimensions,typeString,"MetaDimensions");
	propertyid(prstrDotsPerInch,typeString,"MetaDotsPerInch");
	propertyid(prstrEngineer,typeString,"MetaEngineer");
	propertyid(prstrEncodedBy,typeString,"MetaEncodedBy");
	propertyid(prstrExpirationDate,typeString,"MetaExpireDate");
	propertyid(prstrHeaderType,typeString,"HeaderType");
	propertyid(prstrGenre,typeString,"MetaGenre");
	propertyid(prstrKeywords,typeString,"MetaKeywords");
	propertyid(prstrLicenseURL,typeString,"MetaLicenseUrl");
	propertyid(prstrLicenseTerm,typeString,"MetaLicenseTerm");
	propertyid(prstrLightness,typeString,"MetaLightness");
	propertyid(prstrMedium,typeString,"MetaMedium");
	propertyid(prstrPalette,typeString,"MetaPalette");
	propertyid(prstrProduct,typeString,"MetaProduct");
	propertyid(prstrPublisher,typeString,"MetaPublisher");
	propertyid(prstrSubject,typeString,"MetaSubject");
	propertyid(prstrSoftware,typeString,"MetaSoftware");
	propertyid(prstrSharpness,typeString,"MetaSharpness");
	propertyid(prstrSource,typeString,"MetaSource");
	propertyid(prstrSourceForm,typeString,"MetaSourceForm");
	propertyid(prstrSubCategory,typeString,"MetaSubCategory");
	propertyid(prstrSoundScheme,typeString,"MetaSoundScheme");
	propertyid(prstrTechnician,typeString,"MetaTechnician");
	propertyid(prstrUseLicense,typeString,"MetaLicenseUse");
	propertyid(prstrTempoDescription,typeString,"MetaTempoDescription");
	propertyid(prstrOriginalSource,typeString,"MetaOriginalSource");
	propertyid(prstrComposerNote,typeString,"MetaComposerNote");
	propertyid(prstrWords,typeString,"MetaWords");
	propertyid(prstrWriter,typeString,"MetaWriter");
	propertyid(prstrCopyManaged,typeString,"MetaManagedBy");
	propertyid(prstrManagementInfo,typeString,"MetaManagementInfo");
	propertyid(prstrCarrier,typeString,"MetaCarrier");
	propertyid(prstrVendor,typeString,"MetaVendor");		
	propertyid(prstrYear,typeString,"MetaYear");
	propertyid(prstrIndex,typeString,"MetaIndexNumber");
}

#ifdef _DEBUG
void CMobileProperty::dump(void)
{
	for (uint32_t i=0;i < m_astrPropSet.size();i++)
	{
		TRACEIT2("prop: %s\n",m_astrPropSet[i].c_str());
		dump_value(m_astrPropSet[i].c_str());
	}
}

void CMobileProperty::dump_value(const char *pszId)
{
	string strName, strValue;
	map<string,uint32_t>::iterator iter;
	if ((iter=m_mapPropType.find(pszId)) != m_mapPropType.end())
	{
		getProp(pszId,strName,strValue);
		TRACEIT2("val: \"%s\"\n",strValue.c_str());
	}
	else
	{
		TRACEIT2("\"%s\" is empty\n",pszId);
	}
}
#endif
