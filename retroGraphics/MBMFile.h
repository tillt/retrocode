#ifndef MBMFILEincluded
#define MBMFILEincluded
class CMBMFile : public CMobileContent
{
public:
	DYNOBJECT(CMBMFile )
	DYNDEFPROPERTY

	CMBMFile(void);
	virtual ~CMBMFile(void);

	void Read(istream &ar);

	static tstring sGetFormatName (int nFormat);
	
	int nGetWidth();
	int nGetHeight();
	int nGetBitsPerPixel();
	int nGetColors();
	int nGetSubFormat();

protected:
		typedef struct 
	{
		unsigned long nKDirectFileStoreLayoutUidValue;
		unsigned long nKMultiBitmapFileImageUid;
		unsigned long nReservedUid;
		unsigned long nChecksum;
		unsigned long nTrailerOffset;
	} MBMFileHeader;
	typedef struct
	{
		unsigned long nImageSize;
		unsigned long nHeaderSize;
		unsigned long nWidth;
		unsigned long nHeight;
		unsigned long nTwipsWidth;
		unsigned long nTwipsHeight;
		unsigned long nBitsPerPixel;
		unsigned long nImageIsColor;
		unsigned long nPaletteSize;
		unsigned long nCompression;
	} MBMImageHeader;

	MBMFileHeader m_fileHeader;
	MBMImageHeader m_imgHeader;
	unsigned long int m_nFileCount;

};
#endif
