#ifndef NOKIAPROPincluded
#define NOKIAPROPincluded
class CNokiaProperty : public CFIDProperty
{
public:
	CNokiaProperty(void);
	~CNokiaProperty(void);
	virtual void InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm);
};
#endif
