#ifndef MTFPROPERTYincluded
#define MTFPROPERTYincluded
class CMTFProperty : public CFIDProperty
{
public:
	CMTFProperty(void);
	~CMTFProperty(void);
	virtual void InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm);
};
#endif
