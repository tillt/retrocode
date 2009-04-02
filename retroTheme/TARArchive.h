#ifndef TARFILEincluded
#define TARFILEincluded
class CTARArchive
{
public:
	CTARArchive(void);
	~CTARArchive(void);

	void ExtractOneFile(istream &ar,const char *szFilename,char **ppcDest,unsigned int *pnSize,char *pcOwner=NULL,bool bIsText=true);
private:
};
#endif
