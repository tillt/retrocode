#ifndef MFMFILEincluded
#define MFMFILEincluded
class CMFMSample;
class CMFMFile : public CMobileSampleContent
{
public:
	DYNOBJECT(CMFMFile)
	DYNDEFPROPERTY

	CMFMFile(void);
	virtual ~CMFMFile(void);

	virtual void Read(istream &ar);
	virtual void Write(ostream &out);

protected:
	enum ChunkIDs {	cidTITL,
					cidTRAC,		//
					cidSUPT,		//
					cidCOPY,		//
					cidNOTE,		//
					cidSORC,		//
					cidWAVE	};

	typedef struct TAGMFMHeader
	{
		char		mfmp[4];                //'m','f','m','p'                 
		uint32_t	size;                   //size of MFM file from here on 
		uint16_t	wVersion;				//version
		uint16_t	wSubTagCount;			//count of tags to follow
		uint16_t	wHeaderSize;			// 

		char		sTitle[256];
	}mfmHeader;

	int ReadSubchunk(istream &ar,mfmHeader *pHeader);
	bool bParseTRAC(istream &ar, uint32_t dwSize,uint32_t *pdwSizeRemaining);
	bool bParseWAVE(istream &ar, uint32_t dwSize,uint32_t *pdwSizeRemaining);

	CMFMSample *m_pCurrentSample;
	vector<CMFMSample *> m_Samples;
};
#endif
