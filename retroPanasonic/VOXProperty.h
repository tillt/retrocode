#ifndef VOXPROPERTYincluded
#define VOXPROPERTYincluded
class CVOXProperty: public CMobileProperty
{
public:
	CVOXProperty(void);
	~CVOXProperty(void);
	virtual void InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm);
};
#endif
