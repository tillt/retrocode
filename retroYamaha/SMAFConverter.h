#ifndef SMAFCONVERTER_Defined
#define SMAFCONVERTER_Defined
#ifndef _AFX
//	#include "Priv.h"
#endif
#include <iostream>

using namespace std;

class CMIDIFileDoc;
class CSMAFFile;

class CSMAFConverter
{
public:
	CSMAFConverter();
	~CSMAFConverter();

	void Convert(CMIDIFileDoc *pMIDI,CSMAFFile *pSMAF);
	void Convert(CSMAFFile *pSMAF,CMIDIFileDoc *pMIDI);

protected:
	CMIDIFileDoc *m_pMidi;
	CSMAFFile *m_pSmaf;
};
#endif
