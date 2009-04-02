#ifndef FIDPROPERTYINCLUDED
#define FIDPROPERTYINCLUDED
#include "../retroBase/MobileContent.h"

class CFIDProperty : public CMobileProperty
{
public:
	DllExport CFIDProperty(void);
	DllExport virtual ~CFIDProperty(void);

	enum fidpropIDs
	{
		prboolContainsIMelodyHead,
		prboolContainsTempo,
		prboolContainsName,
		prboolContainsApplication,
		prboolContainsWallpaper,
		prboolContainsBackground,
		prboolContainsRingtone,
		prboolContainsScreensaver,
		prboolContainsShutdown,
		prboolContainsBootup,
		prboolInterlaced,
		prboolTransparent,
		prboolProgressive,
		prnumStyle,		
		prnumAverageInterFrameDelay,
		prnumFrames,
		prnumWidth,
		prnumHeight,
		prnumBitsPerPixel,
		prnumColors,
		prnumResolutionX,
		prnumResolutionY,
		prnumEventCount,
		prnumSmsCount,
		prstrCompression
	};

protected:
	DllExport static void InitPropertyMapping_static(CMobileProperty *p);
	DllExport virtual void InitPropertyMapping(void);
};
#endif
