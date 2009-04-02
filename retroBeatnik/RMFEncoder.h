/*\
 * $Log: RMFEncoder.h,v $
 * Revision 1.1.1.1  2007/11/07 10:30:16  lobotomat
 * no message
 *
 * Revision 1.1  2006/12/19 06:18:51  cvsuser
 * no message
 *
 * Revision 1.1.1.1  2006/04/17 20:31:08  Lobotomat
 * no message
 *
 * Revision 1.1  2005/03/31 21:47:42  lobotomat
 * outsourced SMAF and RMF conversion to shared libraries
 *
 * Revision 1.1  2005/03/17 14:38:47  lobotomat
 * initial check-in
 *
 *
\*/
#ifndef RMFENCODEincluded
#define RMFENCODEincluded
#include "RMFFile.h"

class CRMFEncode : public CRMFFile
{
public:
	CRMFEncode(void);
	~CRMFEncode(void);
protected:
};
#endif
