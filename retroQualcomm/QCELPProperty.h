#ifndef QCELPPROPERTYincluded
#define QCELPPROPERTYincluded
class CQcelpProperty: public CMobileProperty
{
public:
	CQcelpProperty(void);
	~CQcelpProperty(void);
	virtual void InitFromContent(LPCSTR szPath, unsigned int nSize,CMobileContent *pm);
};
#endif
