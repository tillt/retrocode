#ifndef UTZPROPERTYincluded
#define UTZPROPERTYincluded
class CUTZProperty : public CFIDProperty
{
public:
	CUTZProperty(void);
	~CUTZProperty(void);
	virtual void InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm);
};
#endif
