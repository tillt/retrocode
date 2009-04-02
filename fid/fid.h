#ifndef FIDincluded
#define FIDincluded
#define FID_MAJOR_VERSION      1      /* Major version number */
#define FID_MINOR_VERSION      60     /* Minor version number */
#define FID_TYPE_VERSION       0      /* 0 alpha, 1 beta, 2 release */
#define FID_PATCH_VERSION      0      /* Patch level */
//#define FID_BUILDCOUNT			BUILD_NUM
#define FID_ALPHA_VERSION     (FID_TYPE_VERSION==0)
#define FID_BETA_VERSION      (FID_TYPE_VERSION==1)
#define FID_RELEASE_VERSION   (FID_TYPE_VERSION==2)
#endif
