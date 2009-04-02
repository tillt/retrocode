#ifndef JPGPROPERTYincluded
#define JPGPROPERTYincluded
class CJPGProperty : public CFIDProperty
{
public:
	CJPGProperty(void);
	virtual ~CJPGProperty(void);
	virtual void InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm);
};
#endif
