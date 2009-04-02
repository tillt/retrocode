#ifndef MSEQPROPERTYincluded
#define MSEQPROPERTYincluded
class CMSEQProperty : public CFIDProperty
{
public:
	CMSEQProperty(void);
	~CMSEQProperty(void);
	virtual void InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm);
};
#endif
