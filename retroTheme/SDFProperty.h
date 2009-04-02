#ifndef SDFSAMSUNGPROPERTYincluded
#define SDFSAMSUNGPROPERTYincluded
class CSDFProperty : public CFIDProperty
{
public:
	CSDFProperty(void);
	~CSDFProperty(void);
	virtual void InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm);
};
#endif
