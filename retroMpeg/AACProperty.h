#ifndef AACPROPERTYincluded
#define AACPROPERTYincluded
class CAACFile;
class CAACProperty : public CMobileProperty
{
public:
	CAACProperty(void);
	~CAACProperty(void);
	virtual void InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm);
	tstring m_strPath;
};
#endif
