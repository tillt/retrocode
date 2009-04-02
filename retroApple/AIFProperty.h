#ifndef AIFPROPERTYincluded
#define AIFPROPERTYincluded
class CAIFProperty : public CMobileProperty
{
public:
	CAIFProperty(void);
	~CAIFProperty(void);

	virtual void InitFromContent(LPCTSTR szPath, uint32_t nSize,CMobileContent *pm);
};
#endif
