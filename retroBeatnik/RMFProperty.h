#ifndef RMFPROPERTYincluded
#define RMFPROPERTYincluded
class CRMFProperty : public CMobileProperty
{
public:
	CRMFProperty(void);
	~CRMFProperty(void);
	virtual void InitFromContent(LPCSTR szPath, unsigned int nSize,CMobileContent *pm);
};
#endif
