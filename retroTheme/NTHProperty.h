#ifndef NTHPROPERTYincluded
#define NTHPROPERTYincluded
class CNTHProperty : public CFIDProperty
{
public:
	CNTHProperty(void);
	~CNTHProperty(void);
	virtual void InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm);
};
#endif
