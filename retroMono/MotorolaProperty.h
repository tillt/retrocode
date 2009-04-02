#ifndef MOTOROPincluded
#define MOTOROPincluded
class CMotorolaProperty : public CFIDProperty
{
public:
	CMotorolaProperty(void);
	~CMotorolaProperty(void);
	virtual void InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm);
};
#endif
