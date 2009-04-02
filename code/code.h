//#undef BUILD_NUM
//#include "code.ver"
#ifndef CODEincluded
#define CODEincluded
#define CODE_MAJOR_VERSION      1      /* Major version number */
#define CODE_MINOR_VERSION      60     /* Minor version number */
#define CODE_TYPE_VERSION       0      /* 0 alpha, 1 beta, 2 release */
#define CODE_PATCH_VERSION      2      /* Patch level */
//#define CODE_BUILDCOUNT			BUILD_NUM
#define CODE_ALPHA_VERSION     (CODE_TYPE_VERSION==0)
#define CODE_BETA_VERSION      (CODE_TYPE_VERSION==1)
#define CODE_RELEASE_VERSION   (CODE_TYPE_VERSION==2)

typedef struct {
		int nType;
		int nIdentifier;
		int nMask;
		void *pPara;
		TCHAR *pszShortOption;
		TCHAR *pszLongOption;
		TCHAR *pszDescription;	
		TCHAR *pszLongDescription;	
} cmdLinePara;

#endif
