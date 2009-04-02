#ifndef BMPPROPERTYincluded
#define BMPPROPERTYincluded
class CBMPProperty : public CFIDProperty
{
public:
	CBMPProperty(void);
	~CBMPProperty(void);
	virtual void InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm);
};
#endif
