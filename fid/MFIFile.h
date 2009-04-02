#ifndef MFIFILEincluded
#define MFIFILEincluded
#include "../retroBase/MobileContent.h"

class CMFIFile : public CMobileContent
{
public:
	DYNOBJECT(CMFIFile)
	DYNDEFPROPERTY

	CMFIFile(void);
	virtual ~CMFIFile(void);
	
	virtual void Read(istream &ar);

protected:
};
#endif
