#ifndef SISFILEincluded
#define SISFILEincluded
#include "ThemeBaseContent.h"

class CSISFile : public CThemeBaseContent
{
public:
	DYNOBJECT(CSISFile)
	DYNDEFPROPERTY

	CSISFile(void);
	virtual ~CSISFile(void);

	void Read(istream &ar);

	int nGetSubFormat(void) {return m_nSubFormat;};

	tstring sGetTitle(void) {return m_strName;};
	tstring sGetRequiredProductName(int nProductId);
	tstring sGetRemark(void) {return m_strRemark;};
	tstring sGetFormatName(int nRequisite);

	bool bContainsScreensaver(void) {return m_iSkinFile > 0 && (m_bContainsApplication || m_iSubSisFile > 0);};
	bool bContainsBackground(void) {return m_iBitmapFile > 0;};
	bool bContainsRingtone(void) {return m_iRingtoneFile > 0;};
	bool bContainsApplication(void) {return m_bContainsApplication;};
	bool bContainsSkin(void) {return m_iSkinFile > 0;};
	
	bool bMagicHead(std::istream &ar,uint32_t nSize);

protected:
	typedef struct 
	{
		uint32_t dwUID1;
		uint32_t dwUID2;
		uint32_t dwUID3;
		uint32_t dwUID4;

		uint16_t nChecksum;
		uint16_t nNumLang;
		uint16_t nNumFiles;
		uint16_t nNumReq;
		uint16_t nInstLang;
		uint16_t nInstFiles;
		uint16_t nInstDrive;
		uint16_t nNumCap;

		uint32_t dwInstVers;
		uint16_t nOptions;
		uint16_t nType;
		uint16_t nMajorVers;
		uint16_t nMinorVers;
		uint32_t dwVariant;
		uint32_t dwLangPoint;
		uint32_t dwFilePoint;
		uint32_t dwReqPoint;
		uint32_t dwCertPoint;
		uint32_t dwCompPoint;
	}SISHeader;

	typedef struct 
	{
		SISHeader head;
		uint32_t dwSignPoint;
		uint32_t dwCapPoint;
		uint32_t dwInstSpace;
		uint32_t dwMaxInstSpc;
		unsigned char cReserved[16];
	}SISHeaderEx;

	typedef struct 
	{
		uint32_t dwFileType;
		uint32_t dwFileDetails;
		uint32_t dwSrcNamLen;
		uint32_t dwSrcNamPoint;
		uint32_t dwDstNamLen;
		uint32_t dwDstNamPoint;
		uint32_t dwFileLen;
		uint32_t dwFilePoint;
	}SISFileHeader;

	typedef struct 
	{
		SISFileHeader	head;
		uint32_t dwOrigFileLen;
		uint32_t dwMimeLen;
		uint32_t dwMimePoint;
		char			pcFileName[256];
	}SISFileHeaderEx;

	typedef struct 
	{
		uint32_t dwUID;
		uint16_t nVersionMajor;
		uint16_t nVersionMinor;
		uint32_t dwVariant;
		uint32_t dwNameLen;
		uint32_t dwNamePoint;
		char		pcDescription[256];
	}SISRequisite;

	vector<SISRequisite *> m_listRequisites;
	vector<SISFileHeaderEx *> m_listFiles;
	vector<int> m_listLanguages;
	vector<uint32_t> m_listSignedRequisites;

	int m_nMainFormat;
	int m_nEPOCVersion;
	int m_nRequiresProductId;
	int m_nSubFormat;

	bool m_bUnicode;
	bool m_bCompressed;

	bool m_bContainsApplication;
	bool m_bContainsSystem;
	bool m_bContainsOption;
	bool m_bContainsConfig;
	bool m_bContainsPatch;
	bool m_bContainsUpgrade;

	tstring sGetNarrowString(unsigned char *pcWide);
	tstring sGetLanguageName(unsigned short nLanguageId);
	int nDecompress(unsigned char *pcSource,unsigned int nSrcSize,unsigned char *pcDest,unsigned int nDstSize);
	int nMapProductId(uint32_t dwUID);

	unsigned char *pcParseFileRecords(unsigned char *pc,uint32_t dwSize,unsigned char *pcRoot,uint32_t *pdwTotalSize,int level);
	unsigned char *pcParseComponentRecords(unsigned char *pc,uint32_t dwSize,unsigned char *pcRoot,uint32_t *pdwTotalSize,int level);
	unsigned char *pcParseRequisiteRecords(unsigned char *pc,uint32_t dwSize,unsigned char *pcRoot,uint32_t *pdwTotalSize,int level);
	unsigned char *pcParseSignatureRecords(unsigned char *pc,uint32_t dwSize,unsigned char *pcRoot,uint32_t *pdwTotalSize,int level);
	void ParseSignedSIS(istream &ar);

	unsigned char *pcSkipRecord(unsigned char *pc);

	void ParseSignedHeader(unsigned char *pc);
	tstring sParseSignedString(unsigned char *pc);
	tstring sParseSignedStringRecord(unsigned char **pc);
	void ParsePlainSIS (istream &ar);
	void EvalFileName(tstring strName,int iFile);
	unsigned char *pcSkipSubRecords(unsigned char *pc,uint32_t dwSize,unsigned char *pcRoot,unsigned char *pcParent,uint32_t *dwTotalSize,int level);
	//unsigned char *pcSkipRecord(unsigned char *pc);
	void ParseSignedRequisitesSub(unsigned char *pc,uint32_t dwListSize);
	void ParseSignedRequisitesMain(unsigned char *pc,uint32_t dwListSize);
	bool bParseFiles(istream &ar,SISHeaderEx *pHead);
	bool bParseComponent(istream &ar,SISHeaderEx *pHead);
	bool bParseRequisites(istream &ar,SISHeaderEx *pHead);
	void ProcessFiles(istream &ar,SISHeaderEx *pHead);
	tstring sExtractTextFile(istream &ar,int iFile);
	unsigned char *pcExtractFile(istream &ar,int iFile,unsigned long &nSize);
	int nCalcNormalFormatId(void);
	int nCalcSignedFormatId(void);

	int m_iDescFile;
	int m_iSkinFile;
	int m_iSubSisFile;
	int m_iRingtoneFile;
	int m_iBitmapFile;
	int m_iAutoFile;
	uint32_t m_dwSignedPlatformRequisite;
	int m_iThemeVersion;
	
	tstring m_strName;
	tstring m_strRemark;
	tstring m_strDeviceName;
	tstring m_strWallpaper;
	tstring m_strRingtone;
	tstring m_strScreensaver;
	tstring m_strBackground;
	tstring m_strVersion;
};
#endif
