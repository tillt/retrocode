#ifndef THMPROPERTYincluded
#define THMPROPERTYincluded
class CTHMProperty : public CFIDProperty
{
public:
	CTHMProperty(void);
	~CTHMProperty(void);
	virtual void InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm);
};
#endif
