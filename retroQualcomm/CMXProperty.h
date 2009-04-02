#ifndef CMXPROPERTYincluded
#define CMXPROPERTYincluded
class CCMXProperty : public CMobileProperty
{
public:
	CCMXProperty(void);
	~CCMXProperty(void);
	virtual void InitFromContent(LPCSTR szPath, unsigned int nSize,CMobileContent *pm);
};
#endif
