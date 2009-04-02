#ifndef AMRPROPERTYincluded
#define AMRPROPERTYincluded
class CAMRFile;
class CAMRProperty : public CMobileProperty
{
public:
	CAMRProperty(void);
	~CAMRProperty(void);
	virtual void InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm);
	tstring m_strPath;
	int		m_nMode;
};
#endif
