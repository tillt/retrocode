#ifndef MBMPROPERTYincluded
#define MBMPROPERTYincluded
class CMBMProperty : public CFIDProperty
{
public:
	CMBMProperty(void);
	virtual ~CMBMProperty(void);
	virtual void InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm);
};
#endif
