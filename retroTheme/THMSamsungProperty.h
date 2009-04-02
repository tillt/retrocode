#ifndef THMSAMSUNGPROPERTYincluded
#define THMSAMSUNGPROPERTYincluded
class CTHMSamsungProperty : public CFIDProperty
{
public:
	CTHMSamsungProperty(void);
	~CTHMSamsungProperty(void);
	virtual void InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm);
};
#endif
