#ifndef SMAFPROPERTYincluded
#define SMAFPROPERTYincluded
class CSMAFProperty : public CMobileProperty
{
public:
	CSMAFProperty(void);
	~CSMAFProperty(void);
	virtual void InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm);
};
#endif
