#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include "../include/Resource.h"
#include "../retroBase/Basics.h"
#include "../retroBase/MobileContent.h"
#include "../retroBase/FIDProperty.h"
#include "../retroBase/MIDIFile.h"
#include "../retroBase/MIDIFileWriter.h"
#include "MIDIFileDoc.h"
#include "MIDIProperty.h"

CMIDIProperty::CMIDIProperty(void)
{
}

CMIDIProperty::~CMIDIProperty(void)
{
}

void CMIDIProperty::InitFromContent(LPCSTR pszPath,unsigned int nSize,CMobileContent *pm)
{
	CMobileProperty::InitFromContent(pszPath,nSize,pm);
	CMIDIFileDoc *pMidi=(CMIDIFileDoc *)pm;
	if (pMidi)
	{
		char buffer[255];
		_itoa(pMidi->GetFormat(),buffer,10);
		SetProperty_long(prstrSubFormat,CMIDIFile::sGetFormatName(pMidi->GetFormat()).c_str(),"id",buffer);
		SetProperty(prboolContainsMIP,pMidi->ContainedMip());
		SetProperty(prboolContainsVibra,pMidi->ContainedVibra());
		SetProperty(prboolContainsExtraPerc,pMidi->ContainedExtraPerc());
		SetProperty(prboolDP2Vibra,pMidi->IsDP2Vibra());
		SetProperty(prnumSequencePolyphony,(uint32_t)pMidi->GetSequencePolyphony());
		SetProperty(prnumPlaylength,(uint32_t)pMidi->GetPlaytime());
		SetProperty(prstrName,pMidi->GetSongName().c_str());
		SetProperty(prstrCopyright,pMidi->GetCopyright().c_str());

		PatchObject *pPatchIn,*pPatchOut;
		if ((pPatchIn=pMidi->pGetFirstPatch()) != NULL)
		{
			do
			{
				pPatchOut=new PatchObject();
				memcpy(pPatchOut,pPatchIn,sizeof(PatchObject));
				strcpy(pPatchOut->szName,pMidi->GetInstrumentName(pPatchOut->nBank,pPatchOut->nMappedPatch,pPatchOut->nChannel).c_str());
				//pPatchOut->nSize=pMidi->GetInstrumentSize(pPatchOut->nBank,pPatchOut->nMappedPatch,pPatchOut->nChannel);
				m_listPatches.push_back(pPatchOut);
			}while ((pPatchIn=pMidi->pGetNextPatch()) != NULL);
		}

		CPatchSample *pSampleIn,*pSampleOut;
		if ((pSampleIn=pMidi->pGetFirstSample()) != NULL)
		{
			do
			{
				pSampleOut=new CPatchSample();
				memcpy(pSampleOut,pSampleIn,sizeof(CPatchSample));
				m_listSamples.push_back(pSampleOut);
			}while ((pSampleIn=pMidi->pGetNextSample()) != NULL);
		}
	}
}

void CMIDIProperty::writeXML(ostream &ar)
{
	unsigned int i,o;
	tstring strName;
	int nTotal=0;
	uint32_t nFileSize=0;
	int nChannel,nBank,nPatch,nMappedPatch;

	CMobileProperty::writeXML(ar);

	for (i=0;i < m_listPatches.size();i++)
		nTotal+=m_listPatches[i]->nSize;

	nTotal+=(unsigned int)m_listSamples.size() * 1267;

	ar << "\t<BankMemUsage>" << nTotal << "</BankMemUsage>\n";

	GetProperty(prnumFileSize,nFileSize);
	ar << "\t<TotalMemUsage>" << (nTotal+nFileSize) << "</TotalMemUsage>\n";

	for (i=0;i < m_listPatches.size();i++)
	{
		nChannel=m_listPatches[i]->nChannel+1;
		nPatch=m_listPatches[i]->nPatch;
		nBank=m_listPatches[i]->nBank;
		nMappedPatch=m_listPatches[i]->nMappedPatch;
		strName=m_listPatches[i]->szName;
		ar << "\t<PatchReference channel=\"" << nChannel << "\"";
		ar << " bank=\"" << nBank << "\"";
		ar << " id=\"" << nPatch << "\"";
		ar << " mid=\"" << nMappedPatch << "\">";
		ar << strName << "\n";
		for (o=0;m_listPatches[i]->anSamples[o] != 0;o++)
			ar << "\t\t<PatchSample id=\"" << m_listPatches[i]->anSamples[o] << "\"/>\n";
		ar << "\t</PatchReference>\n";
	}

	for (i=0;i < m_listSamples.size();i++)
	{
		int nSize=(m_listSamples[i]->nSize*2)+86;
		ar << "\t<Sample id=\"" << m_listSamples[i]->nIndex << "\"";
		ar << " size=\"" << nSize << "\" name=\"" << m_listSamples[i]->szName << "\"/>\n";
	}
}
