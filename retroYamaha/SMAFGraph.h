#ifndef SMAFGRAPHincluded
#define SMAFGRAPHincluded
class CSMAFSample;
class CSMAFEvent;
class CSMAFImage;

class CSMAFGraph : public CSMAFDecoder
{
public:
	class ZeroEvent
	{
	public:
		ZeroEvent(int nSize,const TCHAR *szDesc,void *pPara=NULL) : nEventSize(nSize),szDescription(szDesc),pParameter(pPara){};
		//ZeroEvent(int nSize,const TCHAR *szDesc,unsigned char cPara) : nEventSize(nSize),szDescription(szDesc),cParameter(cPara),pParameter(NULL){};
		~ZeroEvent(){};

		int nEventSize;
		const TCHAR *szDescription;
		
		void *pParameter;
		unsigned char *cParameter;
	};


	CSMAFGraph(int nChannelOffset=0);
	~CSMAFGraph();

	//CSMAFEvent *pGetEvent(int iEvent){return m_Events[iEvent];};

	void Decode(unsigned char **pcBuffer,unsigned int nSize);
	void DecodeGraphChunk(unsigned char **pcBuffer,unsigned char *pcLimit,tstring &sIdentifier,unsigned int *pnSize,unsigned char *pcChunkAttribute);
	void ExportImages(const char *pszPathPrefix);

	enum GIMDChunkIDs{		gimdGig	};

	enum GraphChunkIDs {	gcidGtsu,
							gcidGsq,
							gcidGimd,
							gcidMtsu,
							gcidMtsp,
							gcidMSTR,
							gcidMtsq	};
protected:
	int nDecodeVariableQuantity (unsigned char **pcBuffer,unsigned char *pcLimit,int *n);
	void DecodeGIMD(unsigned char **pcBuffer,unsigned int nSize);
	
	vector<CSMAFImage *> m_Images;
	//vector<CSMAFEvent *> m_Events;

	int m_nFormat;
	int m_nResolution;
	int m_nChannelOffset;
};

class CSMAFImage
{
public:
	CSMAFImage(const char *pcSrc,uint32_t nSize);
	~CSMAFImage();

	void Decode(unsigned char **pcBuffer,unsigned int nSize);
	void Write(ostream &out);

protected:
	void *m_pcData;
	unsigned int m_nSize;
};

#endif
