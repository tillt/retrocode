#ifndef SISPROPERTYincluded
#define SISPROPERTYincluded
class CSISProperty : public CFIDProperty
{
public:
	CSISProperty(void);
	~CSISProperty(void);
	virtual void InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm);
};
#endif
