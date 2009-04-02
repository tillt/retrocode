#ifndef RAWPROPERTYincluded
#define RAWPROPERTYincluded
class CRAWProperty : public CMobileProperty
{
public:
	CRAWProperty(void);
	~CRAWProperty(void);
	virtual void InitFromContent(LPCSTR szPath, unsigned int nSize,CMobileContent *pm);
};
#endif
