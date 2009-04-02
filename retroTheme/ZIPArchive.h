#ifndef ZIPFILEincluded
#define ZIPFILEincluded
LPCTSTR szGetZlibVersion(void);
class CZIPArchive
{
public:
	CZIPArchive(void);
	~CZIPArchive(void);

	bool bOpen(istream &ar);
	void Close(void);
	
	bool bFindFile(const char *filename);
	int nExtractCurrentFile(const char* password,char **ppcDest,unsigned int *pnSize,char *szComment=NULL,int nCommentBufferSize=0,bool bIsText=true);
	tstring sGetGlobalComment(void);

private:
	void InitFileXS(zlib_filefunc_def *pzlib_filefunc_def);

	unzFile m_uf;
};
#endif
