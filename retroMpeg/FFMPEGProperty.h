#ifndef FFMPEGPROPERTYincluded
#define FFMPEGPROPERTYincluded
#include "../retroBase/MobileContent.h"

class CFFMPEGFile;

class CFFMPEGProperty : public CMobileProperty
{
public:
	CFFMPEGProperty (void);
	~CFFMPEGProperty (void);
	virtual void InitFromContent(LPCTSTR szPath, unsigned int nSize,CMobileContent *pm);
};
#endif
