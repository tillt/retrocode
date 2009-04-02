#ifndef SAGEMROPincluded
#define SAGEMROPincluded
class CSagemProperty : public CFIDProperty
{
public:
	CSagemProperty(void);
	~CSagemProperty(void);
	virtual void InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm);
};
#endif
