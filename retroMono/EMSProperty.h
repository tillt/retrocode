#ifndef EMSPROPincluded
#define EMSPROPincluded
class CEMSProperty : public CFIDProperty
{
public:
	CEMSProperty(void);
	~CEMSProperty(void);

	virtual void InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm);

	const static int emssubformatUnclassified=0;
	const static int emssubformatGeneric=1;
	const static int emssubformatEricsson=2;
	const static int emssubformatSiemens=3;
};
#endif
