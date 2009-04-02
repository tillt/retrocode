#ifndef XMFFILEincluded
#define XMFFILEincluded
#include "../retroBase/MobileContent.h"

class CXMFFile : public CMobileContent
{
public:
	DYNOBJECT(CXMFFile)
	DYNDEFPROPERTY

	CXMFFile(void);
	~CXMFFile(void);
	void Read(istream &ar);
protected:
};
#endif
