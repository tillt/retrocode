#ifndef WMAFILEincluded
#define WMAFILEincluded
class CWMAFile : public CFFMPEGFile
{
public:
	DYNOBJECT(CWMAFile)
	DYNDEFPROPERTY

	CWMAFile(void);
	virtual ~CWMAFile(void);

protected:
};
#endif
