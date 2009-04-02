#ifndef MFMPROPERTYincluded
#define MFMPROPERTYincluded
class CMFMProperty: public CMobileProperty
{
public:
	CMFMProperty(void);
	~CMFMProperty(void);
	virtual void InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm);
};
#endif
