#ifndef GIFPROPERTYincluded
#define GIFPROPERTYincluded
class CGIFProperty : public CFIDProperty
{
public:
	CGIFProperty(void);
	~CGIFProperty(void);
	virtual void InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm);
};
#endif
