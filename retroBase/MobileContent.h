#ifndef MOBILECONTENTincluded
#define MOBILECONTENTincluded
#ifdef WIN32
#ifndef RETROBASE_EXPORTS
#undef DllExport
#define DllExport 
#else
#undef DllExport
#define DllExport   __declspec( dllexport )
#endif
#else
#undef DllExport
#define DllExport
#endif

DllExport uint32_t nMakeID4(const char *cID4);
DllExport char *pcSplitID4(uint32_t nTag,char *pcDest);

#define DYNDELETE(func,a) void func(void) {	this->Release();	};
#define DYNCREATE(func,a) static void *func(void) {	a *pRet = new a();	pRet->AddRef();	return pRet;	};

#define DYNDEFINE(func)	  void *func(void);
#define DYNIMPLEMENT(a,func,b) void *a::func(void) { b *pRet = new b();	pRet->AddRef();	return pRet;	};

#define DYNOBJECT(a) DYNCREATE(ObjectCreate,a) DYNDELETE(ObjectDelete,a)
#define DYNDEFPROPERTY DYNDEFINE(PropertyCreate)
#define DYNIMPPROPERTY(a,b) DYNIMPLEMENT(a,PropertyCreate,b)

//#define DELETEMYOBJECT(a)	{	a->ObjectDelete();	if (a->bSafeToDelete())	delete a; };

#define LISTFORMATINIT	vector<CFormatEntry *> listFormat; vector<CFormatEntry *>::iterator iter;
#define LISTFORMAT(a,b) listFormat.push_back(new CFormatEntry((void *)a::ObjectCreate,b));
#define LISTFORMATFREE 	while (listFormat.size()) { delete listFormat[0]; listFormat.erase(listFormat.begin()); };

#define LIBFORMATINIT	map<int,void *(*)()> listFormat; map<int, void *(*)()>::iterator iter;
#define LIBFORMAT(b,a)	listFormat[b]=(void *(*)())a::ObjectCreate;
#define LIBFORMATFREE 	listFormat.erase();

enum cmdlineparatype {	cmdParaBool,
						cmdParaSwitch,
						cmdParaString,							
						cmdParaFloat,
						cmdParaNumber,
						cmdParaNone};

enum switchidx {		paraSwitchVersion,
						paraSwitchHelp,
						paraSwitchManual,
						paraSwitchNormalize,
						paraSwitchAutoCrop,
						paraSwitchNoMeta,
						paraSwitchReverseOrder,
						paraSwitchGoldWave,
						paraSwitchFact,
						paraSwitchAutoAdapt,
						paraSwitchLast	};

#define					formatParaMaskAny		0
#define					formatParaMaskUpdate	-1

enum boolidx {			paraBoolSaveEnabled,
						paraBoolEditEnabled,
						paraBoolTransferEnabled,
						paraBoolAllowDTX,
						paraBoolAllowID3,
						paraBoolStreamSamples,
						paraBoolProTag,
						paraBoolAllowCRC,
						paraBoolAllowVBR,
						paraBoolAllowJointStereo,
						paraBoolCopyrighted,
						paraBoolLast	};

enum floatidx {			paraFloatVolGain,
						paraFloatVolLimitGain,
						paraFloatFrameRate,
						paraFloatVolRmsNorm,
						paraFloatLast	};

enum numberidx {		paraNumVolume,
						paraNumTempo,
						paraNumChannels,
						paraNumBitrate,
						paraNumLowpassFreq,
						paraNumHighpassFreq,
						paraNumBandpassFreq,
						paraNumBandpassWidth,
						paraNumSamplerate,
						paraNumPlaytime,
						paraNumFadetime,
						paraNumLoopcount,
						paraNumCompression,
						paraNumAdts,
						paraNumAac,
						paraNumEncode,
						paraNumVerbosity,
						paraNumOffset,
						paraNumSmpOffset,
						paraNumSmpPlaytime,
						paraNumSmpFadeIn,
						paraNumSmpFadeOut,
						paraNumSmpLooptime,
						paraNumFrameWidth,
						paraNumFrameHeight,
						paraNumBackgroundRGB,
						paraNumDevice,
						paraNumBlockSize,
						paraNumQuality,
						paraNumLast	};

enum stringidx {		paraStrTitle,
						paraStrSubTitle,
						paraStrArtist,
						paraStrComposer,
						paraStrNote,
						paraStrWriter,
						paraStrCategory,
						paraStrSubcategory,
						paraStrCopyright,
						paraStrVendor,
						paraStrPublisher,
						paraStrArranger,
						paraStrEncoder,
						paraStrManagement,
						paraStrManagedBy,
						paraStrCarrier,
						paraStrLicenseUse,
						paraStrLicenseTerm,
						paraStrLicenseUrl,
						paraStrLicenseExp,
						paraStrDateCreated,
						paraStrDateRevised,
						paraStrSource,
						paraStrTempo,
						paraStrIndex,
						paraStrSoftware,
						paraStrImageExportPath,
						paraStrPathCodecs,
						paraStrRingbackXml,
						paraStrRingbackCountry,
						paraStrLast	};

typedef struct
{
	string		m_strParameter[paraStrLast];
	int32_t		m_nParameter[paraNumLast];
	bool		m_bSwitch[paraSwitchLast];
	bool		m_bParameter[paraBoolLast];
	float		m_fParameter[paraFloatLast];
}converting_parameters;

class CDigitalValueRange
{
public:
	CDigitalValueRange() { m_bIsRange=false; };
	~CDigitalValueRange() {};

	void addDigit(uint32_t nValue) { m_values.push_back(nValue); };
	void setRange(uint32_t nMin,uint32_t nMax) { m_bIsRange=true; addDigit(nMin); addDigit(nMax); };

	bool m_bIsRange;
	vector<uint32_t> m_values;
};

class CSampleCompatibility
{
public:
	DllExport CSampleCompatibility() {};
	DllExport ~CSampleCompatibility() {};

	DllExport bool bIsCompatible(uint32_t nBitPerSample,uint32_t nChannels,uint32_t nSampleRate);
	DllExport void determineFittestFormat(uint32_t *pnChannels,uint32_t *pnSampleRate);

	enum {
		supports8Bit=1,
		supports16Bit=2,
		supportsMono=4,
		supportsStereo=8
	};
	map<uint32_t,CDigitalValueRange> freqs;
};


class CFormatEntry
{
public:
	CFormatEntry(void *p,uint32_t n) : pCreator(p),nFormat(n) {};
	~CFormatEntry() {};

	void *pCreator;
	uint32_t nFormat;
};

typedef vector<CFormatEntry *> ListOfFormats;

class CFormatException : public truntime_error
{
public:
	DllExport CFormatException(int nExceptionCode,LPCSTR pszInfo="");
	DllExport virtual ~CFormatException() throw();

	DllExport LPCSTR szGetErrorMessage();
	DllExport LPCSTR szGetExtErrorMessage();
	DllExport int nGetExceptionCode();

	enum FormatErrorCodes	{	formaterrOk,
								formaterrUnknownFormat,
								formaterrUnknown,
								formaterrUnknownChunk,
								formaterrTruncated,
								formaterrInvalid,
								formaterrInvalidChunkSize,
								formaterrChecksum,
								formaterrUnknownSubChunk,
								formaterrUnknownTrackFormat,
								formaterrUnknownSequenceFormat,
								formaterrIncompatibleSampleFormat,
								formaterrSource,
								formaterrParameters,
								formaterrLast	};

protected:
	int m_nCode;
	string m_sErrDescription;
	
};

class CReferenceObject
{
public:
	CReferenceObject(void);
	virtual ~CReferenceObject(void);

	DllExport void AddRef(void);
	DllExport void Release(void);
	
	DllExport bool bSafeToDelete(void) {	return m_nRefCount == 0; };

protected:
	int m_nRefCount;
};


class CMobileContent : public CReferenceObject
{
public:
	static const unsigned char formatUnknown=0;
	static const unsigned char formatMIDI=1;
	static const unsigned char formatSMAF=2;
	static const unsigned char formatRMF=3;
	static const unsigned char formatAMR=4;
	static const unsigned char formatMFM=5;
	static const unsigned char formatMP3=6;
	static const unsigned char formatWAV=7;
	static const unsigned char formatMFI=8;
	static const unsigned char formatCMX=9;
	static const unsigned char formatQCELP=10;
	static const unsigned char formatNokia=11;
	static const unsigned char formatMotorola=12;
	static const unsigned char formatSagem=13;
	static const unsigned char formatEMS=14;
	static const unsigned char formatSEO=15;
	static const unsigned char formatAIF=16;
	static const unsigned char formatMSEQ=17;
	static const unsigned char formatXMF=18;
	static const unsigned char formatAAC=19;
	static const unsigned char formatMP4=20;
	static const unsigned char format3GP=21;
	static const unsigned char format3G2=22;
	static const unsigned char formatGIF=23;
	static const unsigned char formatPNG=24;
	static const unsigned char formatJPEG=25;
	static const unsigned char formatBMP=26;
	static const unsigned char formatWMA=27;
	static const unsigned char formatRA=28;
	static const unsigned char formatOGG=29;
	static const unsigned char formatNTH=30;
	static const unsigned char formatTHM=31;
	static const unsigned char formatUTZ=32;
	static const unsigned char formatSIS=33;
	static const unsigned char formatJAR=34;
	static const unsigned char formatJAD=35;
	static const unsigned char formatSWF=36;
	static const unsigned char formatVOX=37;
	static const unsigned char formatULaw=38;
	static const unsigned char formatALaw=39;
	static const unsigned char formatAVI=40;
	static const unsigned char formatTHMS=41;
	static const unsigned char formatMTF=42;
	static const unsigned char formatMBM=43;
	static const unsigned char formatSDF=44;
	static const unsigned char formatAWB=45;
	static const unsigned char formatBMXMF=46;
	static const unsigned char formatRAW=47;
	static const unsigned char formatITA=48;

	DllExport CMobileContent(uint32_t iFormatId=0);
	DllExport virtual ~CMobileContent(void);

	/// <summary>test this summary</summary>
	DllExport void ParseOnly(bool bFlag=true) {m_bParseOnly=bFlag;};
	/// <summary>test this antoher summary</summary>
	DllExport void SetFormatId(int iFormatId) {m_nFormatId=iFormatId;};
	/// sets the filename
	DllExport void SetFileName(const char *pcFileName) {m_sFileName=pcFileName;};
	DllExport virtual bool bMagicHead(std::istream &ar,uint32_t nSize);
	
	DllExport bool bLoad(const char *pszPath);
	DllExport bool bLoad(char *pcBuffer,unsigned int nSize);

	DllExport virtual void Read(std::istream &ar)=0;

	DllExport virtual void *PropertyCreate(void)=0;

	DllExport virtual void ObjectDelete(void)=0;

	DllExport virtual void SetLastError(const char *pcErrorMessage) { m_sLastError=pcErrorMessage; };
	DllExport virtual const char *pcGetLastError(void) {return m_sLastError.c_str();};

	DllExport void AttachModules(void *pModules) { m_pListModules=pModules; };

	uint32_t		m_nFormatId;
	uint32_t		m_nMagicSize;
	char			*m_pcMagic;
	string			m_sFormatName;
	string			m_sFileName;
	CMyString		m_sFormatDescription;
	CMyString		m_sFormatCredits;
	CMyString		m_sDefaultExtension;
	vector<CMyString>	m_listAltExtensions;
	CMyString		m_sLastError;
	uint32_t		m_nFileSize;

	void			*m_pListModules;

protected:
	bool m_bParseOnly;
};


class CMemoryBitmap;

class CMobileSampleContent : public CMobileContent
{
public:
	DllExport CMobileSampleContent(int iFormatId=0);
	DllExport virtual ~CMobileSampleContent(void);

	//creates a new ringtone based on the sample data given by the mobilesamplecontent object pointer pSource and writes it to the output stream ar
	DllExport virtual void AttachSourceSample(CMobileSampleContent *pSource);
	//creates a new ringtone and writes it to the output stream ar
	DllExport virtual void Write(ostream &ar);

	//hand over the de/encoding parameters
	DllExport virtual void AttachParameter(converting_parameters *parm);
	//
	DllExport void Reset(void *pNew,uint32_t nSize);

	DllExport static void *Alloc(uint32_t nSize);
	DllExport static void Free(void *pBuffer);

	DllExport virtual void CreateFile(ofstream &ar,const char *pszFileName);
	DllExport virtual void CloseFile(ofstream &ar);
	
	DllExport uint32_t nGetSampleCount(void) {return 1;};
	DllExport void SelectSample(uint32_t nIndex) {};

	DllExport uint32_t nGetSamplePlaytime(void);
	DllExport uint32_t nGetChannels(void){return m_nCSChannels;};
	DllExport uint32_t nGetBitsPerSample(void){return m_nCSBitsPerSample;};
	DllExport uint32_t nGetSamplesPerSecond(void){return m_nCSSamplesPerSecond;};

	//DllExport bool bRenderWaveDisplay(CMemoryBitmap *pbm,MyCol *pColBackFrom,MyCol *pColBackTo,MyCol *pColFore,MyCol *pColMax,unsigned int nGlossHeight,unsigned int nGlossFrame,double dGlossFrom,double dGlossTo);
	//DllExport bool bRenderWaveDisplay2(CMemoryBitmap *pbmNorm,CMemoryBitmap *pbmPlay,MyCol *pColBackFrom,MyCol *pColBackTo,MyCol *pColPlayFrom,MyCol *pColPlayTo,MyCol *pColNormSweep,MyCol *pColNormMax,MyCol *pColPlaySweep,MyCol *pColPlayMax,unsigned int nGlossHeight,unsigned int nGlossFrame,double dGlossFrom,double dGlossTo);
	
	converting_parameters *m_pParameters;	//

	CMobileSampleContent *m_pCSSource;		//input sample pointer
	void *m_pcCSBuffer;						//sample buffer for conversion and export
	uint32_t m_nCSSize;						//sample size
	uint32_t m_nCSChannels;					//
	uint32_t m_nCSSamplesPerSecond;			//
	uint32_t m_nCSBitsPerSample;			//

	CSampleCompatibility m_encodingCaps;	//
	typedef pair<int,int> StringResourceIdentifier;
	vector<StringResourceIdentifier> m_encodingPara;
#define addPara(a,b) push_back(StringResourceIdentifier(a,b))

	enum InfoIndex {infoTitle,
					infoSubTitle,
					infoAlbum,
					infoArtist,
					infoArchiveLocation,
					infoCommissioned,
					infoComments,
					infoComposer,
					infoCopyright,
					infoCropped,
					infoDate,
					infoDimensions,
					infoDotsPerInch,
					infoEngineer,
					infoEncodedBy,
					infoGenre,
					infoKeywords,
					infoLightness,
					infoMedium,
					infoPalette,
					infoProduct,
					infoSubject,
					infoSoftware,
					infoSharpness,
					infoSource,
					infoSourceForm,
					infoTechnician,
					infoSoundScheme,
					infoPublisher,
					infoUseLicense,
					infoLicenseURL,
					infoLicenseTerm,
					infoExpirationDate,
					infoTempoDescription,						
					infoOriginalSource,
					infoComposerNote,
					infoWords,
					infoWriter,
					infoArranged,
					infoCategory,
					infoSubCategory,
					infoCopyManaged,
					infoManagementInfo,
					infoCarrier,
					infoVendor,						
					infoDateCreated,
					infoDateRevised,
					infoYear,
					infoIndex,
					infoLast	};


	string m_strInfo[49];
	
protected:
	DllExport void RenderID3V1(ostream &out,CMobileSampleContent *pSource);
};

class CMobileProperty : public CReferenceObject
{
public:
	DllExport CMobileProperty(void);
	DllExport virtual ~CMobileProperty(void);

	DllExport virtual void InitFromContent(LPCSTR szPath,	uint32_t nSize,CMobileContent *pm);
	DllExport virtual void writeXMLhead (ostream &ar);
	DllExport virtual void writeXML (ostream &ar);
	DllExport virtual void writeXMLtail (ostream &ar);
#ifdef _DEBUG
	DllExport virtual void dump(void);
	DllExport virtual void dump_value(const char *pszName);
#endif
	DllExport string sXmlTidy(const char *pszIn);
	DllExport string sXmlTidy(tstring &strIn);

	DllExport void RenderXmlProperty(const char *pszId,ostream &ar);

	DllExport const char *pszGetFirstUsedProperty(void);
	DllExport const char *pszGetNextUsedProperty(void);

	DllExport void setProperty(const char *pszId,const char *pszString,const char *pszName=NULL,const char *pszValue=NULL);
	DllExport void setProperty(const char *pszId,uint32_t nNumber,const char *pszName=NULL,const char *pszValue=NULL);
	DllExport void setProperty(const char *pszId,bool bFlag,const char *pszName=NULL,const char *pszValue=NULL);

	DllExport bool getProperty(const char *pszId,bool &bProp);
	//DllExport bool getProperty(const char *pszId,bool &bProp,string &strName,string &strValue);
	DllExport bool getProperty(const char *pszId,uint32_t &nProp);
	//DllExport bool getProperty(const char *pszId,uint32_t &nProp,string &strName,string &strValue);
	DllExport bool getProperty(const char *pszId,string &strProp);
	DllExport bool getPropertyWithAttribute(const char *pszId,string &strProp,const char *pszAttr,string &strValue);
	
	DllExport int nGetPropertyType(const char *pszId) { return m_mapPropType[pszId];};

	#define propertyid_static(obj,sym,typ,xml)	obj->m_mapPropType[#sym]=typ; obj->m_mapPropXmlName[#sym]=xml
	#define propertyid(sym,typ,xml)	m_mapPropType[#sym]=typ; m_mapPropXmlName[#sym]=xml
	#define SetProperty_long(sym,a,b,c)	setProperty(#sym,a,b,c)
	#define SetProperty(sym,a)	setProperty(#sym,a)
	#define GetProperty_long(sym,a,b,c)	getProperty(#sym,a,b,c)
	#define GetProperty(sym,a)	getProperty(#sym,a)
	#define mapit(sym,b) m_mapPropXmlName[#sym]=b
	#define id2str(sym) #sym

	enum proTypes
	{
		typeString,
		typeNumber,
		typeBool
	};
	
	enum propIDs
	{
		prnumFileSize,
		prnumSampleRate,
		prnumChannels,
		prnumBitsPerSample,
		prnumBitRate,
		prnumSequencePolyphony,
		prnumPlaylength,
		prnumSequenceFormat,
		prnumSampleSize,
		prnumSamplePlaytime,
		prnumMpegVersion,
		prnumMinBitRate,
		prnumMaxBitRate,
		prnumLoopCount,
		prnumFrameBitPerPixel,
		prnumFrameRate,
		prnumFrameBitRate,
		prnumFrameWidth,
		prnumFrameHeight,

		prboolContainsMIP,
		prboolContainsVibra,
		prboolContainsExtraPerc,
		prboolDP2Vibra,
		prboolContainsSamples,
		prboolContainsSynthesizer,
		prboolContainsPicture,
		prboolStatusSave,
		prboolStatusCopy,
		prboolStatusEdit,
		prboolContainsHumanVoice,
		prboolVariableBitRate,
		prboolContainsCRC,
		prboolJointStereo,
		prboolCopyrighted,
		prboolOriginalRecording,
		prboolPrivate,
		prboolLoop,
	
		prstrFormat,
		prstrSubFormat,
		prstrEncoding,
		prstrName,
		prstrCopyright,
		prstrArranged,
		prstrComment,
		prstrSubTitle,
		prstrAlbum,
		prstrArtist,
		prstrArchiveLocation,
		prstrCommissioned,
		prstrComposer,
		prstrCropped,
		prstrDate,
		prstrDimensions,
		prstrDotsPerInch,
		prstrEngineer,
		prstrEncodedBy,
		prstrGenre,
		prstrKeywords,
		prstrLightness,
		prstrMedium,
		prstrPalette,
		prstrProduct,
		prstrSubject,
		prstrSoftware,
		prstrSharpness,
		prstrSource,
		prstrSourceForm,
		prstrTechnician,
		prstrSoundScheme,
		prstrPublisher,
		prstrUseLicense,
		prstrLicenseURL,
		prstrLicenseTerm,
		prstrExpirationDate,
		prstrTempoDescription,
		prstrOriginalSource,
		prstrComposerNote,
		prstrWords,
		prstrWriter,
		prstrCategory,
		prstrSubCategory,
		prstrCopyManaged,
		prstrManagementInfo,
		prstrCarrier,
		prstrVendor,
		prstrDateCreated,
		prstrDateRevised,
		prstrYear,
		prstrIndex,
		prstrHeaderType,
		prstrPath
	};
protected:
	DllExport static void InitPropertyMapping_static(CMobileProperty *p);
	DllExport virtual void InitPropertyMapping(void);
	
	DllExport void setProp(const char *pszId,const char *pszName,const char *pszValue);
	DllExport string strGetInfoToPropertyStringIndex(uint32_t nInfoId);

	DllExport bool getProp(const char *pszId,string &strName,string &strValue);

	map<string,uint32_t> m_mapPropNumber;
	map<string,string> m_mapPropString;
	map<string,bool> m_mapPropBool;
	vector<string> m_astrPropSet;

	typedef map<string,string> AttributePair;
	map<string,AttributePair> m_mapPropAttribute;

	map<string,uint32_t> m_mapPropType;
	map<string,string> m_mapPropXmlName;

	uint32_t m_iGetIndex;
};

#endif
