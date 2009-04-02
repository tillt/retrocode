#ifndef JARPROPERTYincluded
#define JARPROPERTYincluded
class CJARProperty : public CFIDProperty
{
public:
	CJARProperty(void);
	~CJARProperty(void);
	virtual void InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm);
};
#endif
