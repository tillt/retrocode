#ifndef SWFPROPERTYincluded
#define SWFPROPERTYincluded
class CSWFFile;
class CSWFProperty : public CMP3Property
{
public:
	CSWFProperty(void);
	~CSWFProperty(void);

	virtual void InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm);
};
#endif
