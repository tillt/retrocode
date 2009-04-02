#ifndef RAFILEincluded
#define RAFILEincluded
class CRAFile : public CFFMPEGFile
{
public:
	DYNOBJECT(CRAFile)
	DYNDEFPROPERTY

	CRAFile(void);
	virtual ~CRAFile(void);

	bool bMagicHead(std::istream &ar,uint32_t nSize);
protected:

	virtual int nLocateBestAudioStream(AVFormatContext *pFormatContext);
};
#endif
