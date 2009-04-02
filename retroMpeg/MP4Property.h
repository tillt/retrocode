#ifndef MP4PROPERTYincluded
#define MP4PROPERTYincluded
class CMP4File;
class CMP4Property : public CMobileProperty
{
public:
	CMP4Property(void);
	~CMP4Property(void);
	virtual void InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm);

	tstring m_strPath;

	//int		m_nMode;
	//int		m_nObjectType;
	//tstring m_strInfo[CAMRFile::infoLast];
};

#endif
