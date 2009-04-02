#ifndef PNGPROPERTYincluded
#define PNGPROPERTYincluded
class CPNGProperty : public CFIDProperty
{
public:
	CPNGProperty(void);
	~CPNGProperty(void);
	virtual void InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm);
};
#endif
