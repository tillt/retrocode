#ifndef ThemeBaseContentincluded
#define ThemeBaseContentincluded
class CThemeBaseContent : public CMobileContent
{
public:
	enum 
	{
		resDesktop = 0,
		resBackground = 1,
		resScreensaver,
		resRingtone,
		resBootup,
		resShutdown,
		resLast
	};

	CThemeBaseContent(void);
	~CThemeBaseContent(void);

	bool bContainsScreensaver(void) {return !m_strStdResource[resBackground].empty();};
	bool bContainsWallpaper(void) {return !m_strStdResource[resDesktop].empty();};
	bool bContainsRingtone(void) {return !m_strStdResource[resRingtone].empty();};
	bool bContainsBackground(void) {return !m_strStdResource[resBackground].empty();};
	bool bContainsShutdown(void) {return !m_strStdResource[resShutdown].empty();};
	bool bContainsBootup(void) {return !m_strStdResource[resBootup].empty();};

	unsigned int nGetWidth(void) {return m_nWidth;};
	unsigned int nGetHeight(void) {return m_nHeight;};
	unsigned int nGetColors(void) {return m_nColors;};

protected:
	CMobileProperty *poParseFromMemory(char *pcIn,unsigned long nFileSize,const char *pcPath);
	CMobileProperty *poProcessFile(char *pcIn,unsigned long nFileSize,const char *pcPath);

	void ExportRaw(const char *pcpath,char *pcOut,unsigned long nSize);

	unsigned long int m_nWidth;
	unsigned long int m_nHeight;
	unsigned long int m_nColors;
	tstring m_strStdResource[resLast];
};
#endif
