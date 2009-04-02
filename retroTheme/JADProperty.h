#ifndef JADPROPERTYincluded
#define JADPROPERTYincluded
class CJADProperty : public CFIDProperty
{
public:
	CJADProperty(void);
	~CJADProperty(void);
	virtual void InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm);
};
#endif
