#ifndef MIDIPROPERTYincluded
#define MIDIPROPERTYincluded

class CMIDIFileDoc;
class CMIDIProperty : public CMobileProperty
{
public:
	CMIDIProperty(void);
	~CMIDIProperty(void);
	virtual void InitFromContent(LPCSTR szPath, unsigned int nSize,CMobileContent *pm);
	virtual void writeXML(ostream &ar);
	CPatchArray m_listPatches;
	CSampleArray m_listSamples;
};
#endif
