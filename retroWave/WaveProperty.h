#ifndef WAVEPROPERTYincluded
#define WAVEPROPERTYincluded
class CWaveProperty : public CMobileProperty
{
public:
	CWaveProperty(void);
	~CWaveProperty(void);
	virtual void InitFromContent(LPCSTR szPath, unsigned int nSize,CMobileContent *pm);
};
#endif
