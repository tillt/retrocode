#ifndef AVIPROPERTYincluded
#define AVIPROPERTYincluded
class CAVIProperty : public CMobileProperty
{
public:
	CAVIProperty(void);
	~CAVIProperty(void);
	virtual void InitFromContent(LPCSTR szPath, unsigned int nSize,CMobileContent *pm);
};
#endif
