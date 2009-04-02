#ifndef MP3PROPERTYincluded
#define MP3PROPERTYincluded
class CMP3File;
class CMP3Property : public CMobileProperty
{
public:
	CMP3Property(void);
	~CMP3Property(void);
	virtual void InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm);
	void PartialInitFromContent(CMP3File *pMpeg);
};
#endif
