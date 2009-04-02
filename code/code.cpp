/*\
 * code.cpp
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
/*\
 * Notes for Developers
 * 
 * Basic function overview of the RetroCode commandline application:
 * -----------------------------------------------------------------
 * RetroCode identifies the source stream's format using all available 
 * codecs, reads and decodes it entirely, keeping all important assets 
 * in memory. The codecs are derived from CMobileSampleContent. 
 *
 * Sample data internally is to be 16bit. When the input stream data 
 * is lower than that, the first step needed after decoding this sample 
 * is scale it towards 16bit.
 *
 * During the filtering process (filter, normalize, rate convert,...) 
 * all resulting data is maintained in that source-sample object.
 * Finally the source object is handed over to an instance of the
 * destination format class which then encodes the data and writes 
 * it to the output stream.
 *
 * Important CMobileSampleContent (codec) methods:
 * -----------------------------------------------------------------
 * -> Read - reads, parses and decodes the file
 *
 * -> SetSource - attaches the source sample object to the file that is to 
 * be writting
 *
 * -> Write - encodes and writes the file
 * For more members of CMobileContent and CMobileSampleContent, 
 * check MobileContent.h
 *
 * Developing new codecs:
 * -----------------------------------------------------------------
 * For a good and relative clean example of a codec, check out
 * retroApple containing the AIFF parser - well that technically is not 
 * a codec, but we wont be that scientific in here.
 *
 * (Arguable) Design flaws:
 * -----------------------------------------------------------------
 * - Can only handle one stream per sample (through the conversion chain). 
 * That really may become a major drawback in the future. Though
 * making it multi-stream capable
 * - Codecs and file-formats are wildly mixed. 
 * That could relatively easy be changed in the future.
 * - Does not stream samples but handles everything in 
 * memory. 
 * Assuming its used at sample rates of around 44kHz an entire CD
 * of uncompressed audio fits into the memory of a modern workstation 
 * (multiple times).
 * - Why another sample en/decoding tool, most formats are handled 
 * by established open-source-tools like ffmpeg and sox already.
 * The RetroCode SDK also contains RetroFID which actually was the 
 * motivator in the first place for creating all this. Next reason
 * was the fact that most of those established tools didnt handle meta-
 * data that well.
 *
 * Project afterlife:
 * -----------------------------------------------------------------
 * By today still no other tool handles such broad selection of mobile
 * content formats. The mobile technology however is moving towards
 * "desktop-formats". A step mostly driven with a lot of power by the 
 * "newcomers" of the device manufacturers (e.g. Apple, Microsoft). 
 * Also a development that is very much simplyfying the entire mobile 
 * content market a lot for all involved parties up to the comsumer. 
 *
\*/
#include "stdafx.h"
#ifdef WIN32
#include <windows.h>
#include <crtdbg.h>
#else
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>
#endif
#include <time.h>
#include <stdio.h>
#include <iostream>
#include <fstream>

#include "Config.h"
#include "../include/Resource.h"
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/CommonAccess.h"
#include "../retroBase/Modules.h"
#include "../retroBase/Endian.h"
#include "../retroBase/ResourceVersion.h"
#include "code.h"
#include "CLIParameters.h"
#include "Process.h"

int m_iFiles=0;
tstring m_strFile[2];

int nBackMap[cmdParaNone][255];

static const char_t szAppName[]=_T("RetroCode(tm)");
static const char_t szAppCopyright[]=_T("Copyright (C) 2004-2005, Retro Ringtones LLC, Copyright (C) 2006-2009, MMSGURU");
static const char_t szParameterFormat[]=_T("%-4s\t%-10s\t%-10s\t%s\n");

/*\
 * <---------- sRenderValidInputSampleFormats ---------->
 * @m creates a readable string from the sample compatibility data found inside the mobile sample content object
\*/

/// <summary>creates a readable string from the sample compatibility data found inside the mobile sample content object</summary>
/// <param name="pm">pointer to CMobileSampleContent object</param>
/// <returns>readable string showing the allowed sample format</returns>
tstring sRenderValidInputSampleFormats(CMobileSampleContent *pm)
{
	tstring strOut="";
	if (!((CMobileSampleContent *)pm)->m_encodingCaps.freqs.empty())
	{
		for (	map<uint32_t,CDigitalValueRange>::iterator iterRanges = ((CMobileSampleContent *)pm)->m_encodingCaps.freqs.begin();
				iterRanges != ((CMobileSampleContent *)pm)->m_encodingCaps.freqs.end();
				iterRanges++)
		{
			tstring strBits="";
			if ( (iterRanges->first & CSampleCompatibility::supports8Bit) == CSampleCompatibility::supports8Bit )
				strBits+="8Bit";
			if ( (iterRanges->first & CSampleCompatibility::supports16Bit) == CSampleCompatibility::supports16Bit )
			{
				if (!strBits.empty())
					strBits+=" or ";
				strBits+="16Bit";
			}
			tstring strChans="";
			if ( (iterRanges->first & CSampleCompatibility::supportsMono) == CSampleCompatibility::supportsMono )
				strChans+="Mono";
			if ( (iterRanges->first & CSampleCompatibility::supportsStereo) == CSampleCompatibility::supportsStereo )
			{
				if (!strChans.empty())
					strChans+=" or ";
				strChans+="Stereo";
			}
			strBits+=", "+strChans+", ";
			if (iterRanges->second.m_bIsRange)
			{
				char szRange[40];
				sprintf(szRange,"%dHz to %dHz",iterRanges->second.m_values[0],iterRanges->second.m_values[1]);
				strBits+=szRange;
			}
			else
			{
				char szValue[16];
				for (unsigned int o=0;o < iterRanges->second.m_values.size();o++)
				{
					if (o > 0)
						strBits+=" or ";
					sprintf(szValue,"%dHz",iterRanges->second.m_values[o]);
					strBits+=szValue;
				}						
			}
			strOut+=strBits+"\n";
		}
	}
	return strOut;
}

/*\
 * <---------- *pInstanceByExtension ----------> 
 * @m instantiate a sample object typed by the file-extension given
 * --> I N <-- @p
 * const TCHAR *pszDestName  - file-name used for the destination sample file object (including the extension)
 * VectorModules listModules - list of formats usable
 * <-- OUT --> @r
 * CMobileSampleContent * - pointer to freshly instantiated sample instance or NULL if failed
\*/
CMobileSampleContent *pInstanceByExtension(const TCHAR *pszDestName, VectorModules listModules)
{
	CMobileSampleContent *pDest=NULL;
	CMyString strExtension;

	strExtension=CCommonAccess::sGetFileExtension(pszDestName);
	Log2(verbLevDebug2,"searching for codec able to create %s - files\n",strExtension.c_str());
	if (!strExtension.empty())
	{
		CMobileSampleContent *pm=NULL;
		strExtension.ToLower();
		for (int i=0;i < (int)listModules.size();i++)
		{
			Log2(verbLevDebug2,"testing module from %s\n",listModules[i]->pcGetPath());
			int *nFormats=reinterpret_cast<int *>(listModules[i]->pGetQueryLibrary()(RETROLIBQ_FORMATS));
			while(*nFormats != 0)
			{
				pm=reinterpret_cast<CMobileSampleContent *>(listModules[i]->pGetCreateCodecInstance()(*nFormats));
				Log2(verbLevDebug2,"compare with default extension for \"%s\": %s\n",pm->m_sFormatName.c_str(),pm->m_sDefaultExtension.c_str());
				if (!strExtension.compare(pm->m_sDefaultExtension))
				{
					Log2(verbLevDebug2,"destination format chosen \"%s\"\n",pm->m_sFormatName.c_str());
					return pm;
				}
				for (int o=0;o < pm->m_listAltExtensions.size();o++)
				{
					Log2(verbLevDebug2,"compare with alternative extension %s\n",pm->m_listAltExtensions[o].c_str());
					if(!strExtension.compare(pm->m_listAltExtensions[o]))
					{
						Log2(verbLevDebug2,"destination format chosen by alt extension \"%s\"\n",pm->m_sFormatName.c_str());
						return pm;
					}
				}
				TRACEIT2("deleting object\n");
				pm->Release();
				pm=NULL;
				++nFormats;
				TRACEIT2("next one...\n");
			};
		}
	}
	Log2(verbLevErrors,IDS_ERR_DSTFORMATUNKNOWN);
	return NULL;
}

/*\ 
 * <---------- *pCreateFromSample ----------> 
 * @m create content stream from source data
 * --> I N <-- @p
 * CMobileSampleContent *pSource - pointer to source format class
 * const TCHAR *pszDestName - filename for the destination
 * converting_parameters *parm - parameter list
 * <-- OUT --> @r
 * CMobileSampleContent - pointer to instantiated destination content class
\*/ 
bool bConvertSample(CMobileSampleContent *pDest,const char *pszDestName,CMobileSampleContent *pSource,converting_parameters *parm)
{
	bool bRet=true;
	ofstream ar;
	tstring out;

	Log2(verbLevDebug2,"destination format chosen \"%s\"\n",pDest->m_sFormatName.c_str());
	pDest->AttachParameter(&code);
	if (!pDest->m_encodingCaps.bIsCompatible(pSource->m_nCSBitsPerSample,pSource->m_nCSChannels,pSource->m_nCSSamplesPerSecond))
	{
		Log2(verbLevErrors,"this sample format is not usable for the chosen destination codec or file format\n");
		out=tstring("Instead use auto adaption (-aa/--autoadapt) or convert to: ") + sRenderValidInputSampleFormats((CMobileSampleContent *)pDest);
		Log2(verbLevWarnings,out.c_str());
		bRet=false;
	}
	else
	{
		ar.exceptions(ios_base::badbit|ios_base::eofbit|ios_base::failbit);
		try
		{
			Log2(verbLevDebug2,IDS_PRG_CREATINGFILE,pszDestName);
			//open file
			pDest->CreateFile(ar,pszDestName);
			pDest->AttachSourceSample(pSource);
			//render content
			pDest->Write(ar);
			//close file
			pDest->CloseFile(ar);
		}
		catch (CFormatException *fe)
		{
			Log2(verbLevErrors,IDS_ERR_FORMATEXCEPTION,fe->szGetErrorMessage());
			pDest->SetLastError(fe->szGetErrorMessage());
			delete fe;
			bRet=false;
		}
		catch (istream::failure const &e)
		{
			Log2(verbLevErrors,IDS_ERR_FILEEXCEPTION,e.what());
			pDest->SetLastError(e.what());
			bRet=false;
		}
	}
	return bRet;
}

/*\
 * <---------- CopyParameters ----------> 
 * @m 
 * --> I N <-- @p
 * converting_parameters *pDest - 
 * converting_parameters *pSrc  - 
\*/
void CopyParameters(converting_parameters *pDest,converting_parameters *pSrc)
{
	int i;
	for (i=0;i < paraStrLast;i++)
		pDest->m_strParameter[i]=pSrc->m_strParameter[i];
	for (i=0;i < paraNumLast;i++)
		pDest->m_nParameter[i]=pSrc->m_nParameter[i];
	for (i=0;i < paraSwitchLast;i++)
		pDest->m_bSwitch[i]=pSrc->m_bSwitch[i];
	for (i=0;i < paraBoolLast;i++)
		pDest->m_bParameter[i]=pSrc->m_bParameter[i];
	for (i=0;i < paraFloatLast;i++)
		pDest->m_fParameter[i]=pSrc->m_fParameter[i];
}

/*\ 
 * <---------- nProcessFile ----------> 
 * @m run all needed steps for a complete conversion 
 * --> I N <-- @p
 * converting_parameters *parm - parameter list
 * ListOfFormats - format list
 * <-- OUT --> @r
 * int - -1=error
\*/ 
int nProcessFile(const char *pszFile,converting_parameters *parm,VectorModules &listModules)
{
	int i,nRet=0;
	CMobileSampleContent *pSource,*pDest;

	//filename to title conversion
	if (parm->m_strParameter[paraStrTitle] == _T("<FILENAME>"))
	{
		int nAt;
		tstring strTitle=pszFile;
#ifdef WIN32
		if ((nAt=(int)strTitle.rfind("\\")) != -1)
#else
		if ((nAt=(int)strTitle.rfind("/")) != -1)
#endif
			strTitle=strTitle.substr(nAt+1);
		if ((nAt=(int)strTitle.rfind(".")) != -1)
			strTitle=strTitle.substr(0,nAt);
		parm->m_strParameter[paraStrTitle]=strTitle;
	}
	Log2(verbLevMessages,IDS_PRG_DEFTITLE,parm->m_strParameter[paraStrTitle].c_str());

	//
	//read source file
	// 
	Log2(verbLevMessages,IDS_PRG_CONVERTING,m_strFile[0].c_str());
	if ((pSource=(CMobileSampleContent *)CCommonAccess::pReadAnyFormat(m_strFile[0].c_str(),&listModules,parm)) == NULL)
	{
		Log2(verbLevErrors,"failed to read source file\n");
		nRet=-1;
	}
	else
	{
		CMyString str;
#ifdef _DEBUG
		CMobileProperty *pp;
		Log2(verbLevMessages,IDS_PRG_DECODED,pSource->m_sFormatName.c_str());
		pp=(CMobileProperty *)pSource->PropertyCreate();
		pp->InitFromContent(m_strFile[0].c_str(),pSource->m_nFileSize,pSource);
		pp->getProperty("prstrSubFormat",str);
		pp->writeXML(cout);
		pp->Release();
#endif

		Log2(verbLevMessages,"sample size: %d\n",pSource->m_nCSSize);

		if (pSource->m_nCSBitsPerSample)
		{
			Log2(verbLevMessages,IDS_PRG_BITSPERSAMPLE,pSource->m_nCSBitsPerSample);
		}
		Log2(verbLevMessages,IDS_PRG_SAMPLEATTRIBS,pSource->m_nCSSamplesPerSecond,pSource->m_nCSChannels,pSource->m_nFileSize,pSource->m_nCSSize);

		int anParaInfoMap[][2]=
		{
			{	paraStrTitle,		CMobileSampleContent::infoTitle				},
			{	paraStrSubTitle,	CMobileSampleContent::infoSubTitle			},
			{	paraStrArtist,		CMobileSampleContent::infoArtist			},
			{	paraStrComposer,	CMobileSampleContent::infoComposer			},
			{	paraStrNote,		CMobileSampleContent::infoComments			},
			{	paraStrWriter,		CMobileSampleContent::infoWriter			},
			{	paraStrCategory,	CMobileSampleContent::infoCategory			},
			{	paraStrSubcategory,	CMobileSampleContent::infoSubCategory		},
			{	paraStrCopyright,	CMobileSampleContent::infoCopyright			},
			{	paraStrVendor,		CMobileSampleContent::infoVendor			},
			{	paraStrPublisher,	CMobileSampleContent::infoPublisher			},
			{	paraStrArranger,	CMobileSampleContent::infoArranged			},
			{	paraStrEncoder,		CMobileSampleContent::infoEncodedBy			},
			{	paraStrManagement,	CMobileSampleContent::infoManagementInfo	},
			{	paraStrManagedBy,	CMobileSampleContent::infoCopyManaged		},
			{	paraStrCarrier,		CMobileSampleContent::infoCarrier			},
			{	paraStrLicenseUse,	CMobileSampleContent::infoUseLicense		},
			{	paraStrLicenseTerm,	CMobileSampleContent::infoLicenseTerm		},
			{	paraStrLicenseUrl,	CMobileSampleContent::infoLicenseURL		},
			{	paraStrLicenseExp,	CMobileSampleContent::infoExpirationDate	},
			{	paraStrDateCreated,	CMobileSampleContent::infoDateCreated		},
			{	paraStrDateRevised,	CMobileSampleContent::infoDateRevised		},
			{	paraStrSource,		CMobileSampleContent::infoSource			},
			{	paraStrTempo,		CMobileSampleContent::infoTempoDescription	},
			{	paraStrIndex,		CMobileSampleContent::infoIndex				},
			{	paraStrSoftware,	CMobileSampleContent::infoSoftware			},
			//put new meta-bindings here
			{	paraStrLast,		CMobileSampleContent::infoLast				}
		};

		//
		//overwrite source metadata with commandline attributes
		//
		Log2(verbLevMessages,"metadata:\n");
		
		str="";
		for (i=0;anParaInfoMap[i][1] != CMobileSampleContent::infoLast;i++)
		{
			if (!parm->m_strParameter[anParaInfoMap[i][0]].empty())									//any value given on the commandline for this attribute?
				pSource->m_strInfo[anParaInfoMap[i][1]]=parm->m_strParameter[anParaInfoMap[i][0]];	//yes->overwrite the source data (if existing) with that!
			if (!pSource->m_strInfo[anParaInfoMap[i][1]].empty())									//
			{
				//Log2(verbLevMessages,"%s: %s\n",cmdPara[nBackMap[cmdParaString][anParaInfoMap[i][0]]].pszLongOption,pSource->m_strInfo[anParaInfoMap[i][1]].c_str());
				str+=cmdPara[nBackMap[cmdParaString][anParaInfoMap[i][0]]].pszLongOption;
				str+=" ";
				str+=pSource->m_strInfo[anParaInfoMap[i][1]];
				str+="\n";
			}
		}
		if (!str.empty())
			str+="\n";
		Log2(verbLevMessages,str.c_str());

		//nasty Goldwave bug emulation enabled?
		if (parm->m_bSwitch[paraSwitchGoldWave])
		{	//yes->...
			parm->m_strParameter[paraStrTitle]="";
			for (i=0;i < CMobileSampleContent::infoLast;i++)
				pSource->m_strInfo[i]="";
			pSource->m_strInfo[CMobileSampleContent::infoSoftware]="GoldWave (C) Chris S. Craig, http://www.goldwave.com";
		}

		//if (pSource->m_nCSSize == 0)
		if (pSource->m_nCSSize == 0 || pSource->m_nCSBitsPerSample == 0)
		{
			Log2(verbLevErrors,"no usable sample sound found\n");
			nRet = -1;
		}
		else
		{
			//
			//instantiate destination sample object
			//
			if ((pDest=pInstanceByExtension(m_strFile[1].c_str(),listModules)) == NULL)
			{
				Log2(verbLevErrors,"could not create destination format instance\n");
				nRet = -1;
			}
			else
			{
				//
				//auto adapt
				//
				if(parm->m_nParameter[paraSwitchAutoAdapt] > 0 && (parm->m_nParameter[paraNumChannels] == 0 && parm->m_nParameter[paraNumSamplerate] == 0))
					AutoAdapt(pDest,pSource,parm);
				//
				//16bit conversion where needed
				//
				if (pSource->m_nCSBitsPerSample == 8)
					ConvertTo16Bit(pSource);
				//
				//crop
				//
				if (parm->m_nParameter[paraNumSmpOffset] != 0 || parm->m_nParameter[paraNumSmpPlaytime] != 0 || parm->m_bSwitch[paraSwitchAutoCrop])
					Crop(pSource,parm);
				//
				//normalize
				//
				if (parm->m_bSwitch[paraSwitchNormalize])
					Normalize(pSource);
				//
				//channel conversion
				//
				if (parm->m_nParameter[paraNumChannels] != 0)
					ConvertChannels(pSource,parm);
				//
				//gain control filter functions
				//
				if (parm->m_fParameter[paraFloatVolLimitGain] != 0.0 || parm->m_fParameter[paraFloatVolGain] != 0.0)
					GainControl(pSource,parm);
				//
				//butterworth filter functions
				//
				if (parm->m_nParameter[paraNumLowpassFreq] != 0 || parm->m_nParameter[paraNumHighpassFreq] != 0 || parm->m_nParameter[paraNumBandpassFreq] != 0)
					ButterworthFilter(pSource,parm);
				//
				//sample rate converter
				//
				if (parm->m_nParameter[paraNumSamplerate] != 0)
					ConvertSampleRate(pSource,parm);
				//
				//normalize
				//
				if (parm->m_bSwitch[paraSwitchNormalize])
					Normalize(pSource);
				//
				//looping
				//
				if (parm->m_nParameter[paraNumSmpLooptime] != 0)
					Loop(pSource,parm);
				//
				//root mean square level normalizer
				//
				if (parm->m_fParameter[paraFloatVolRmsNorm] != 0.0)
					RmsNormalize(pSource,parm);
				//
				//ringback synth mixer
				//
				if (!parm->m_strParameter[paraStrRingbackCountry].empty())
					RingbackMixer(pSource,parm);
				//
				//fade in
				//
				if (parm->m_nParameter[paraNumSmpFadeIn] != 0)
					FadeIn(pSource,parm);
				//
				//fade out
				//
				if (parm->m_nParameter[paraNumSmpFadeOut] != 0)
					FadeOut(pSource,parm);
				//
				//create destination file
				//
				Log2(verbLevMessages,"creating destination file %s...\n",m_strFile[1].c_str());
				if (!bConvertSample(	pDest,
										m_strFile[1].c_str(),
										pSource,
										parm))
				{
					TRACEIT2("failed to create destination object\n");
					nRet=-2;
				}
				else
				{
					Log2(verbLevMessages,IDS_PRG_ENCODED,pDest->m_sFormatName.c_str());
					if (pDest->m_nCSBitsPerSample)
						Log2(verbLevMessages,IDS_PRG_BITSPERSAMPLE,pDest->m_nCSBitsPerSample);
					Log2(verbLevMessages,IDS_PRG_SAMPLEATTRIBS,pDest->m_nCSSamplesPerSecond,pSource->m_nCSChannels,pDest->m_nFileSize,pDest->m_nCSSize);

					TRACEIT2("processing successfull\n");
					nRet=1;
				}
				pDest->Release();
			}
		}
		pSource->Release();
	}
	return nRet;
}

/*\ 
 * <---------- RenderParameterValue ----------> 
 * @m display a paramater value
 * --> I N <-- @p
 * cmdLinePara *pSource - pointer to commandline parameter structure
 * tstring &dest - reference of destination string object
 \*/ 
void RenderParameterValue(cmdLinePara *pSource,tstring &dest)
{
	dest.clear();
	switch(pSource->nType)
	{
		case cmdParaBool:
		{
			bool *pParameter=(bool *)pSource->pPara;
			if (*pParameter)
				dest=_T("true");
			else
				dest=_T("false");
		}
		break;
		case cmdParaNumber:
		{
			char szBuffer[255];
			uint32_t*pParameter=(uint32_t*)pSource->pPara;
			sprintf(szBuffer,"%d",*pParameter);
			dest=szBuffer;
		}
		break;
		case cmdParaFloat:
		{
			char szBuffer[255];
			float *pParameter=(float *)pSource->pPara;
			sprintf(szBuffer,"%f",*pParameter);
			dest=szBuffer;
		}
		break;
		case cmdParaString:
		{
			tstring *pParameter=(tstring *)pSource->pPara;
			dest=*pParameter;
		}
		break;
	}
}

/*\ 
 * <---------- nFillPara ----------> 
 * @m copy parameter value from the commandline into its storage
 * --> I N <-- @p
 * cmdLinePara *pDest - pointer to the identified commandline parameter
 * char *pSource=NULL - pointer to commandline at the parameter
 * <-- OUT --> @r
 * int  - 
 \*/ 
int nFillPara(cmdLinePara *pDest,char *pSource=NULL)
{
	int nUsed=0;
	ASSERT(pDest);
	if (pDest)
	{
		switch(pDest->nType)
		{
			case cmdParaSwitch:
			{
				bool *pParameter=(bool *)pDest->pPara;
				ASSERT(pParameter);
				*pParameter=true;
				TRACEIT2("identified switch \"%s\"\n",pDest->pszLongOption);
				nUsed=1;
			}
			break;
			case cmdParaBool:
			{
				bool *pParameter=(bool *)pDest->pPara;
				ASSERT(pParameter);
				nUsed=1;
				if (pSource)
				{
					if (*pSource == '1' || !_stricmp(pSource,"true"))
						*pParameter=true;
					else
						*pParameter=false;
					TRACEIT2("identified boolean parameter \"%s\" = %d\n",pDest->pszLongOption,*pParameter);
					++nUsed;
				}
				else
				{
					TRACEIT2("missing parameter for boolean\n");
				}
			}
			break;
			case cmdParaString:
			{
				tstring *pParameter=(tstring *)pDest->pPara;
				nUsed=1;
				if (pSource)
				{
					ASSERT(pParameter);
					*pParameter=pSource;
					TRACEIT2("identified string parameter \"%s\" = \"%s\"\n",pDest->pszLongOption,pParameter->c_str());
					++nUsed;
				}
				else
				{
					TRACEIT2("missing parameter for string\n");
				}
			}
			break;
			case cmdParaNumber:
			{
				int *pParameter=(int *)pDest->pPara;
				nUsed=1;
				if (pSource)
				{
					ASSERT(pParameter);
					sscanf(pSource,"%d",pParameter);
					TRACEIT2("identified numeric parameter \"%s\" = %d\n",pDest->pszLongOption,*pParameter);
					++nUsed;
				}
				else
				{
					TRACEIT2("missing parameter for number\n");
				}
			}
			break;
			case cmdParaFloat:
			{
				float *pParameter=(float *)pDest->pPara;
				nUsed=1;
				if (pSource)
				{
					ASSERT(pParameter);
					sscanf(pSource,"%f",pParameter);
					++nUsed;
					TRACEIT2("identified float parameter \"%s\" = %f\n",pDest->pszLongOption,*pParameter);
				}
				else
				{
					TRACEIT2("missing parameter for float\n");
				}
			}
			break;
		}
	}
	return nUsed;
}

/*\ 
 * <---------- ParseCommandLine ----------> 
 * @m parse the application commandline arguments
 * --> I N <-- @p
 * int argc - number of supplied arguments (+1)
 * char **argv - pointer to array of arguments
\*/ 
void ParseCommandLine(int argc,char **argv,converting_parameters *pCode)
{
	int i=1,o;
	while(i < argc)
	{
		if (*argv[i] == '-')
		{
			int nIdentified=0;
			for (o=0;cmdPara[o].pszDescription != NULL;o++)
			{
				if (cmdPara[o].nType != cmdParaNone)
				{
					if ((*(argv[i]+1) == '-' && !_stricmp(argv[i]+2,cmdPara[o].pszLongOption)) || !_stricmp(argv[i]+1,cmdPara[o].pszShortOption))
					{
						if (i+1 < argc)
							nIdentified=nFillPara(&cmdPara[o],argv[i+1]);
						else
							nIdentified=nFillPara(&cmdPara[o]);
					}
					if(nIdentified)
						break;
				}
			}
			if (nIdentified == 0)
			{
				Log2(verbLevErrors,IDS_ERR_INVALIDOPT,argv[i]);
				pCode->m_bSwitch[paraSwitchHelp]=true;
				++i;
			}
			else
				i+=nIdentified;
		}
		else
		{
			m_strFile[m_iFiles]=argv[i++];
			if (m_iFiles < 2)
				++m_iFiles;
		}
	}
}

/*\ 
 * <---------- bApplicationValid ----------> 
 * @m silly attempt to reduce spreading of copies
 * <-- OUT --> @r
 * bool - true=ok
 \*/ 
bool bApplicationValid(void)
{
	return true;
}

/*\
 * <---------- BackmapParameters ---------->
 * @m create a map from a parameter identifier to its index in the list
\*/
void BackmapParameters(void)
{
	int i;
	for (i=0;cmdPara[i].pszDescription != NULL;i++)
	{
		switch(cmdPara[i].nType)
		{
			case cmdParaBool:
			case cmdParaFloat:
			case cmdParaNumber:
			case cmdParaString:
			case cmdParaSwitch:
				nBackMap[cmdPara[i].nType][cmdPara[i].nIdentifier]=i;
			break;
		}
	}
}

/*\
 * <---------- szGetRetroCodeVersion ----------> 
 * @m get the application release version
 * <-- OUT --> @r
 * const char * - pointer to version string
\*/
const char_t *szGetRetroCodeVersion(void)
{
#if CODE_ALPHA_VERSION
	static const char_t str[] = XSTR(CODE_MAJOR_VERSION) _T(".") XSTR(CODE_MINOR_VERSION) _T(" (alpha ") XSTR(CODE_PATCH_VERSION) _T(", ") __DATE__ _T(" ") __TIME__ _T(")");
#elif CODE_BETA_VERSION
 	static const char_t str[] = XSTR(CODE_MAJOR_VERSION) _T(".") XSTR(CODE_MINOR_VERSION) _T(" (beta ") XSTR(CODE_PATCH_VERSION) _T(", ") __DATE__ _T(")");
#elif CODE_RELEASE_VERSION && (CODE_PATCH_VERSION > 0)
	static const char_t str[] = XSTR(CODE_MAJOR_VERSION) _T(".") XSTR(CODE_MINOR_VERSION) _T(".") XSTR(CODE_PATCH_VERSION);
#else
	static const char_t str[] = XSTR(CODE_MAJOR_VERSION) _T(".") XSTR(CODE_MINOR_VERSION);
#endif
    return str;
}

/*\
 * <---------- strAppOutputHeader ----------> 
 * @m get the application console ouput header
 * <-- OUT --> @r
 * tstring - displayable text
\*/
tstring strAppOutputHeader(void)
{
	tstring str="";
	str+=_T("------------------------------------------------------------------------------\n");
	str+=tstring(szAppName) + tstring(_T(" "));
	str+=tstring(szGetRetroCodeVersion()) + tstring(_T(" "));
#ifdef WIN32
	str+=tstring(_T("Win32"));
#else
#ifdef DARWIN
	str+=tstring(_T("Darwin"));
#else
	str+=tstring(_T("generic unix"));
#endif
#endif
	str+=tstring(_T("\n"));
	str+=tstring(szAppCopyright) + tstring(_T("\n"));
	str+=_T("$Id: code.cpp,v 1.23 2009/03/26 20:54:14 lobotomat Exp $\n");
	str+=_T("------------------------------------------------------------------------------\n");
	str+=_T("This program is free software: you can redistribute it and/or modify\n");
	str+=_T("it under the terms of the GNU General Public License as published by\n");
	str+=_T("the Free Software Foundation, either version 3 of the License, or\n");
	str+=_T("(at your option) any later version.\n");
	str+=_T("\n");
	str+=_T("This program is distributed in the hope that it will be useful,\n");
	str+=_T("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
	str+=_T("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
	str+=_T("GNU General Public License for more details.\n");
	str+=_T("\n");
	str+=_T("You should have received a copy of the GNU General Public License\n");
	str+=_T("along with this program.  If not, see <http://www.gnu.org/licenses/>.\n");
	str+=_T("\n");
	return str;
}

tstring strAppDescriptionHeader(void)
{
	tstring str="";
	str+=_T("------------------------------------------------------------------------------\n");
	str+=_T("DESCRIPTION:\n");
	str+=_T("RetroCode(tm) is a universal mobile content codec. RetroCode(tm) is able to read and write most common sample based ringtone formats.\n");
	str+=_T("\n");
	return str;
}

tstring strAppSyntaxHeader(void)
{
	tstring str="";
	str+=_T("------------------------------------------------------------------------------\n");
	str+=_T("SYNTAX:\n");
	str+=_T("code [-SHORT] [--LONG] SOURCE DESTINATION\n");
	return str;
}

/*\
* <---------- setupDefaults ----------> 
 * @m default value setup
 * --> I N <-- @p
 * converting_parameters *code - 
\*/
void SetupDefaults(converting_parameters *code)
{
	memset(code->m_bSwitch,0,paraBoolLast*sizeof(bool));			//
	memset(code->m_nParameter,0,paraNumLast*sizeof(int));			//
	memset(code->m_bParameter,0,paraSwitchLast*sizeof(bool));		//
	code->m_bParameter[paraBoolEditEnabled]=false;					//
	code->m_bParameter[paraBoolSaveEnabled]=true;					//
	code->m_bParameter[paraBoolTransferEnabled]=false;				//
	code->m_bParameter[paraBoolCopyrighted]=true;					//
	code->m_bParameter[paraBoolAllowJointStereo]=true;				//
	code->m_strParameter[paraStrTitle]="<FILENAME>";				//
	code->m_nParameter[paraNumVolume]=100;							//
	code->m_nParameter[paraNumTempo]=100;							//
	code->m_nParameter[paraNumBitrate]=0;							//
	code->m_nParameter[paraNumBandpassWidth]=15000;					//
	code->m_fParameter[paraFloatVolGain]=0.0;						//
	code->m_fParameter[paraFloatVolLimitGain]=0.0;					//
	code->m_nParameter[paraNumEncode]=4;							//MPEG4
	code->m_nParameter[paraNumAac]=1;								//LC
	code->m_nParameter[paraNumAdts]=1;								//ADTS
	code->m_nParameter[paraNumLoopcount]=15;						//
	code->m_nParameter[paraNumVerbosity]=verbLevMessages;			//
	code->m_nParameter[paraNumOffset]=0;							//
	code->m_nParameter[paraNumPlaytime]=26000;						//
	code->m_nParameter[paraNumFadetime]=4000;						//
	code->m_nParameter[paraNumFrameWidth]=1;						//
	code->m_nParameter[paraNumFrameHeight]=1;						//
	code->m_nParameter[paraNumBackgroundRGB]=0x000000;				//
	code->m_nParameter[paraNumQuality]=0;							//
	code->m_strParameter[paraStrPathCodecs]=RETROCODEC_LIB_PATH;	//
	code->m_strParameter[paraStrRingbackXml]="ringback_config.xml";	//
	code->m_strParameter[paraStrImageExportPath]="";				//
}

void SetupDebugging(void)
{
#ifdef WIN32
	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE );
	_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE );
	_CrtSetReportFile( _CRT_ERROR, _CRTDBG_FILE_STDOUT );
	_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE );
	_CrtSetReportFile( _CRT_ASSERT, _CRTDBG_FILE_STDOUT );

	int tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    tmpDbgFlag |= _CRTDBG_DELAY_FREE_MEM_DF;
    tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
    _CrtSetDbgFlag(tmpDbgFlag);
#endif
}

/*\
* <---------- main ----------> 
 * @m application standard main function
 * --> I N <-- @p
 * int argc     - number of arguments
 * char **argv - list of arguments
 * <-- OUT --> @r
 * int - 1=ok
\*/
#ifdef WIN32
int _tmain(int argc, _TCHAR **argv)
#else
int main(int argc, char **argv)
#endif
{
	Endian endian;
	VectorModules listModules;
	int nOk=0;
	int nError=0;
	int i;
	int nRet=0;

	CMyString::InitStringtable();

	SetupDebugging();

	//setup default values
	SetupDefaults(&code);
	//create a map from a parameter identifier to its index in the list
	BackmapParameters();
	//parse commandline, get parameters
	if (argc >= 2)
		ParseCommandLine(argc,argv,&code);
	else
		code.m_bSwitch[paraSwitchHelp]=true;
	if (m_iFiles < 2 && !code.m_bSwitch[paraSwitchManual])
		code.m_bSwitch[paraSwitchHelp]=true;
	//setup verbosity
	SetVerbosity(code.m_nParameter[paraNumVerbosity]);
	//locate the libraries (codecs)
	LocateLibraries(code.m_strParameter[paraStrPathCodecs].c_str(),&listModules);
	//initialize all found libraries
	InitLibraries(&listModules);
	
	//initialize the endianess
	endian.init();
	Log2(verbLevDebug1,"Running on a %s machine\n", endian.bIsBigEndian() ? "big-endian" : "little-endian");

	//display program version
	if(code.m_bSwitch[paraSwitchVersion])
	{
		Log2(verbLevMessages,strAppOutputHeader().c_str());
		Log2(verbLevMessages,"------------------------------------------------------------------------------\n");
		Log2(verbLevMessages,"$Revision: 1.23 $, $Date: 2009/03/26 20:54:14 $\n");
		Log2(verbLevMessages,"------------------------------------------------------------------------------\n");
		Log2(verbLevMessages,"--Components--\n");
		Log2(verbLevMessages,"retroBase %s\n",szGetRetroBaseVersion());
		for(unsigned int i=0;i < listModules.size();i++)
			Log2(verbLevMessages,"%s %s\n",listModules.at(i)->pcGetName(),reinterpret_cast<LPCTSTR>(listModules[i]->pGetQueryLibrary()(RETROLIBQ_VERSION)));
		nRet=-1;
	}
	else if(code.m_bSwitch[paraSwitchHelp])
	{
		int i;
		tstring strShort,strLong,strDefault;
		Log2(verbLevMessages,strAppOutputHeader().c_str());
		Log2(verbLevMessages,strAppDescriptionHeader().c_str());
		Log2(verbLevMessages,strAppSyntaxHeader().c_str());
		Log2(verbLevMessages,"------------------------------------------------------------------------------\n");
		Log2(verbLevMessages,"SHORT\tLONG\t\t(DEFAULT)\tDESCRIPTION\n");
		for (i=0;cmdPara[i].pszDescription != NULL;i++)
		{
			if (cmdPara[i].nType != cmdParaNone)
			{
				RenderParameterValue(&cmdPara[i],strDefault);
				Log2(verbLevMessages,szParameterFormat,cmdPara[i].pszShortOption,cmdPara[i].pszLongOption,strDefault.c_str(),cmdPara[i].pszDescription);
			}
			else
				Log2(verbLevMessages,"                                        --- %s ---\n",cmdPara[i].pszDescription);
		}
		Log2(verbLevMessages,"------------------------------------------------------------------------------\n");
		Log2(verbLevMessages,"EXAMPLE:\n");
		Log2(verbLevMessages,"code test.wav test.mmf -tt \"Eat my shorts\" -at \"B.Simpson\" --save true\n");
		Log2(verbLevMessages,"------------------------------------------------------------------------------\n");
		Log2(verbLevMessages,"Use -m/--manual for a full parameter overview\n");
		Log2(verbLevMessages,"------------------------------------------------------------------------------\n");
		nRet=-1;
	}
	else if (code.m_bSwitch[paraSwitchManual])
	{
		int o;
		tstring strShort,strLong,strDefault,strFormatCredits;
		Log2(verbLevMessages,strAppOutputHeader().c_str());
		Log2(verbLevMessages,strAppDescriptionHeader().c_str());
		Log2(verbLevMessages,strAppSyntaxHeader().c_str());
		Log2(verbLevMessages,"------------------------------------------------------------------------------\n");
		Log2(verbLevMessages,"GENERAL PARAMETERS: --LONG (-SHORT)\n");

		for (o=0;cmdPara[o].pszDescription != NULL;o++)
		{
			if (cmdPara[o].nMask == formatParaMaskAny)
			{
				if (cmdPara[o].pszLongDescription)
				{
					string strDefault;
					RenderParameterValue(&cmdPara[o],strDefault);
					Log2(verbLevMessages,"\n--%s (-%s)\tdefault: %s\n",cmdPara[o].pszLongOption,cmdPara[o].pszShortOption,strDefault.c_str());
					Log2(verbLevMessages,"%s\n",cmdPara[o].pszLongDescription);
				}
			}
		}
		Log2(verbLevMessages,"\n------------------------------------------------------------------------------\n");
		Log2(verbLevMessages,"FORMAT SPECIFIC PARAMETERS: --LONG (-SHORT)\n");

		CMobileContent *pm=NULL;
		for (i=0;i < (int)listModules.size();i++)
		{
			int *nFormats=reinterpret_cast<int *>(listModules[i]->pGetQueryLibrary()(RETROLIBQ_FORMATS));
			while(*nFormats != 0)
			{
				pm=reinterpret_cast<CMobileContent *>(listModules[i]->pGetCreateCodecInstance()(*nFormats));
				if (i)
					Log2(verbLevMessages,"\n");

				Log2(verbLevMessages,"\n%s\n- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n",pm->m_sFormatName.c_str());
				Log2(verbLevMessages,"%s\n",pm->m_sFormatDescription.c_str());
				if (!pm->m_sFormatCredits.empty())
				{
					if (!strFormatCredits.empty())
						strFormatCredits+="\n\n";
					strFormatCredits+=pm->m_sFormatCredits;
				}
				Log2(verbLevMessages,"--------\n");
				Log2(verbLevMessages,"Valid input sample format/s:\n");
				Log2(verbLevMessages,sRenderValidInputSampleFormats( (CMobileSampleContent * )pm ).c_str());
				Log2(verbLevMessages,"--------");

				for (o=0;cmdPara[o].pszDescription != NULL;o++)
				{
					if ((cmdPara[o].nMask & formatParaMaskUpdate) == formatParaMaskUpdate)
					{
						if (cmdPara[o].pszLongDescription)
						{
							for (unsigned int u=0;u < ((CMobileSampleContent *)pm)->m_encodingPara.size();u++)
							{
								if (cmdPara[o].nType == ((CMobileSampleContent *)pm)->m_encodingPara[u].first &&
									cmdPara[o].nIdentifier == ((CMobileSampleContent *)pm)->m_encodingPara[u].second)
								{
									string strDefault;
									RenderParameterValue(&cmdPara[o],strDefault);
									Log2(verbLevMessages,"\n--%s (-%s)",cmdPara[o].pszLongOption,cmdPara[o].pszShortOption);
									if (!strDefault.empty())
										Log2(verbLevMessages,"\tdefault: %s",strDefault.c_str());
									Log2(verbLevMessages,"\n");
									Log2(verbLevMessages,"%s\n",cmdPara[o].pszLongDescription);
								}
							}
						}
					}
				}
				pm->Release();
				pm=NULL;
				++nFormats;
			};
		}

		const char *pszDocExamples[]=
		{
			_T("code test_input.wav test_ouput.mmf -tt \"Eat my shorts\" -at \"B.Simpson\" --save true"),
			_T("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -"),
			_T("This will convert the WAVE file \"test_input.wav\" into a SMAF file named \"test_output.mmf\", encoding the specified title (-tt) and artist (-at) as metadata and setting the saveable bit (--save) to true."),
			_T(""),
			_T("code hq_input.wav test.aac -br 64000 -c 1 -s 32000 -lpf 16000"),
			_T("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -"),
			_T("This will read and decode the WAVE file \"hq_input.wav\", reduce the sample to mono (-c) and the samplerate (-s) into 32kHz. Before reducing the samplerate, a lowpassfilter (-lpf) is applied at 16kHz. The resulting sample is then encoded using the AAC Low Complexity (LC) profile."),
			_T(""),
			_T("code foobar.wav test.mp4 -br 64000 -dt 0 -mp 4"),
			_T("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -"),
			_T("This will convert the WAVE file \"foobar.wav\" and encode it into the MP4-file \"test.mp4\" using the AAC Low Complexity [LC] profile, storing the sample as MPEG4 (-mp) raw AAC (-dt) packets."),
			_T(""),
			_T("code in.rmf out.pmd"),
			_T("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -"),
			_T("This will read and decode the RMF file \"in.rmf\" which is then converted into the CMF file \"out.pmd\"\"."),
			_T(""),
			_T("code in.mmf out.amr -br 12200"),
			_T("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -"),
			_T("This will read and decode the SMAF file \"in.mmf\" which is then converted with a resulting bitrate (-br) of 12200 into the AMR-NB file \"out.amr\"\"."),

			NULL
		};
		const char *pszDocCredits[]=
		{
			_T("Be aware that some of the used codecs are based on fragmential specifications, released by their original IP holders."),
			strFormatCredits.c_str(),
			_T("The ID3 parser is entirely based on: \"id3lib\", Copyright (c) 1999, 2000 by Scott Thomas Haug, 2002 by Thijmen Klok."),
			_T("The Butterworth filter is entirely based on: \"Sound Processing Kit - A C++ Class Library for Audio Signal Processing\", Copyright (c) 1995-1998 Kai Lassfolk."),
			_T("RetroCode(tm) and all its components were originally assembled in 2005 by Till Toenshoff for inhouse usage at Retro Ringtones LLC."),
			NULL
		};
		Log2(verbLevMessages,"\n------------------------------------------------------------------------------\n");
		Log2(verbLevMessages,"EXAMPLES:\n");
		for (i=0;pszDocExamples[i] != NULL;i++)
			Log2(verbLevMessages,"%s\n",pszDocExamples[i]);
		Log2(verbLevMessages,"\n------------------------------------------------------------------------------\n");
		Log2(verbLevMessages,"CREDITS:\n");
		for (i=0;pszDocCredits[i] != NULL;i++)
		{
			if (i != 0)
				Log2(verbLevMessages,"\n\n");
			Log2(verbLevMessages,"%s",pszDocCredits[i]);
		}
		Log2(verbLevMessages,"\n------------------------------------------------------------------------------\n");
		nRet=-1;
	}
	//check protection
	if(bApplicationValid() && nRet == 0)
	{
		tstring strExt,strPath;
		tstring strSource=m_strFile[0];
		tstring strDestination=m_strFile[1];

		if (strSource.find("*.") != string::npos)
		{
			if (strDestination.find("*.") == string::npos)
				Log2(verbLevMessages,"Wildcard given for source but not given for the destination file/s.\n");
			else
			{
				char spath[_MAX_PATH],dpath[_MAX_PATH];
				char bdrive[_MAX_DRIVE],sdrive[_MAX_DRIVE],ddrive[_MAX_DRIVE];
				char bdir[_MAX_DIR],sdir[_MAX_DIR],ddir[_MAX_DIR];
				char bfname[_MAX_FNAME],sfname[_MAX_FNAME],dfname[_MAX_FNAME];
				char bext[_MAX_EXT],sext[_MAX_EXT],dext[_MAX_EXT];

			#ifdef WIN32
				WIN32_FIND_DATA FindFileData;
				HANDLE hFind;
			#else
				DIR *FindFileData;
				struct dirent *hFind;
			#endif	
				converting_parameters cpFile;

				_splitpath(strDestination.c_str(),ddrive,ddir,dfname,dext);
				_splitpath(strSource.c_str(),sdrive,sdir,sfname,sext);

			#ifdef WIN32
				hFind = FindFirstFile(strSource.c_str(),&FindFileData);
				if (hFind == INVALID_HANDLE_VALUE) 
			#else
				FindFileData = opendir (sdir);
				if (FindFileData == NULL )
				{
					Log2(verbLevMessages,"Unable to enter \"%s\"\n",sdir);
					return 0;
				}
				else
					hFind = readdir(FindFileData);
				if (hFind == NULL)
			#endif
				{
					Log2(verbLevMessages,"No matching files found for \"%s\"\n",strSource.c_str());
				}
				else 
				{
					do
					{
	#ifdef WIN32
						_splitpath(FindFileData.cFileName,bdrive,bdir,bfname,bext);
	#else
						_splitpath(hFind->d_name,bdrive,bdir,bfname,bext);
	#endif
						_makepath(spath,sdrive,sdir,bfname,bext);
						m_strFile[0]=spath;
						Log2(verbLevMessages,"The file found is \"%s\"\n", m_strFile[0].c_str());
						_makepath(dpath,ddrive,ddir,bfname,dext);
						m_strFile[1]=dpath;

						CopyParameters(&cpFile,&code);
						nRet=nProcessFile(m_strFile[1].c_str(),&cpFile,listModules);
						Log2(verbLevMessages,"RetroCode Processing Status: ");
						if (nRet == 1)
						{
							Log2(verbLevMessages,IDS_PRG_OK);
							++nOk;
						}
						else
						{
							Log2(verbLevMessages,IDS_PRG_ERROR);
							++nError;
						}
						Log2(verbLevMessages,"\n");
			#ifdef WIN32
					}while(FindNextFile(hFind, &FindFileData) != 0);
			#else
					}while((hFind = readdir(FindFileData)) != NULL);
			#endif
			#ifdef WIN32
					FindClose(hFind);
			#else
					closedir(FindFileData);
			#endif
				}
			}
		}
		else
		{
			nRet=nProcessFile(m_strFile[1].c_str(),&code,listModules);
		}
 	}
	if (nRet >= 0)
	{
		Log2(verbLevDebug2,"returning: %d\n",nRet);
		Log2(verbLevMessages,"RetroCode Processing Status: ");
		if (nRet == 0)
		{
			Log2(verbLevMessages,IDS_PRG_ERROR);
			++nError;
		}
		if (nRet == 1)
		{
			Log2(verbLevMessages,IDS_PRG_OK);
			++nOk;
		}
	}
	if (nOk || nError)
	{
		Log2(verbLevMessages,"\n/////////////////////////////////\n");
		Log2(verbLevMessages,IDS_PRG_COUNTRESULT,nOk,nError);
	}
	DeinitLibraries(&listModules);

#ifdef WIN32

#endif
	return nRet;
}
