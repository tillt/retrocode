//#undef BUILD_NUM
//#include "retroBase.ver"
//#define RETROBASE_BUILDCOUNT		BUILD_NUM
#define RETROBASE_MAJOR_VERSION      1      /* Major version number */
#define RETROBASE_MINOR_VERSION      9      /* Minor version number */
#define RETROBASE_TYPE_VERSION       0      /* 0 alpha, 1 beta, 2 release */
#define RETROBASE_PATCH_VERSION      0      /* Patch level */
#define RETROBASE_ALPHA_VERSION     (RETROBASE_TYPE_VERSION==0)
#define RETROBASE_BETA_VERSION      (RETROBASE_TYPE_VERSION==1)
#define RETROBASE_RELEASE_VERSION   (RETROBASE_TYPE_VERSION==2)
