#include "stdafx.h"
#include "Integer.h"
#include "MyString.h"
#include "MobileContent.h"
#include "FIDProperty.h"

CFIDProperty::CFIDProperty(void)
{
	CFIDProperty::InitPropertyMapping();
}

CFIDProperty::~CFIDProperty(void)
{
}

void CFIDProperty::InitPropertyMapping_static(CMobileProperty *p)
{
	/*
	TRACEIT2("CFIDProperty::InitpropertyMapping");
	CMobileProperty::InitPropertyMapping(p);
	propertyid_static(p,prnumWidth,typeNumber,"Width");
	propertyid_static(p,prnumHeight,typeNumber,"Height");
	propertyid_static(p,prnumBitsPerPixel,typeNumber,"BitsPerPixel");
	propertyid_static(p,prnumColors,typeNumber,"Colors");
	propertyid_static(p,prnumResolutionX,typeNumber,"ResolutionX");
	propertyid_static(p,prnumResolutionY,typeNumber,"ResolutionY");
	propertyid_static(p,prnumFrames,typeNumber,"Frames");
	propertyid_static(p,prnumAverageInterFrameDelay,typeNumber,"AvgIFrameDelay");
	propertyid_static(p,prnumLoopDelay,typeNumber,"LoopDelay");
	propertyid_static(p,prnumSmsCount,typeNumber,"SmsCount");
	propertyid_static(p,prnumEventCount,typeNumber,"EventCount");

	propertyid_static(p,prboolContainsWallpaper,typeBool,"ContainsWallpaper");
	propertyid_static(p,prboolContainsBackground,typeBool,"ContainsBackground");
	propertyid_static(p,prboolContainsScreensaver,typeBool,"ContainsScreensaver");
	propertyid_static(p,prboolContainsRingtone,typeBool,"ContainsRingtone");
	propertyid_static(p,prboolContainsApplication,typeBool,"ContainsApplication");
	propertyid_static(p,prboolContainsShutdown,typeBool,"ContainsShutdownImage");
	propertyid_static(p,prboolContainsBootup,typeBool,"ContainsBootupImage");

	propertyid_static(p,prboolContainsIMelodyHead,typeBool,"ContainsIMelodyHead");
	propertyid_static(p,prboolContainsTempo,typeBool,"ContainsTempo");
	propertyid_static(p,prboolContainsName,typeBool,"ContainsName");
	propertyid_static(p,prboolInterlaced,typeBool,"Interlaced");
	propertyid_static(p,prboolTransparent,typeBool,"Transparent");
	propertyid_static(p,prboolProgressive,typeBool,"Progressive");
	propertyid_static(p,prstrEncoding,typeString,"Encoding");	
	propertyid_static(p,prstrCompression,typeString,"Compression");
	*/
}

void CFIDProperty::InitPropertyMapping(void)
{
	TRACEIT2("CFIDProperty::InitpropertyMapping\n");
	//CMobileProperty::InitPropertyMapping();
	propertyid(prnumWidth,typeNumber,"Width");
	propertyid(prnumHeight,typeNumber,"Height");
	propertyid(prnumBitsPerPixel,typeNumber,"BitsPerPixel");
	propertyid(prnumColors,typeNumber,"Colors");
	propertyid(prnumResolutionX,typeNumber,"ResolutionX");
	propertyid(prnumResolutionY,typeNumber,"ResolutionY");
	propertyid(prnumFrames,typeNumber,"Frames");
	propertyid(prnumAverageInterFrameDelay,typeNumber,"AvgIFrameDelay");
	propertyid(prnumLoopDelay,typeNumber,"LoopDelay");
	propertyid(prnumSmsCount,typeNumber,"SmsCount");
	propertyid(prnumEventCount,typeNumber,"EventCount");

	propertyid(prboolContainsWallpaper,typeBool,"ContainsWallpaper");
	propertyid(prboolContainsBackground,typeBool,"ContainsBackground");
	propertyid(prboolContainsScreensaver,typeBool,"ContainsScreensaver");
	propertyid(prboolContainsRingtone,typeBool,"ContainsRingtone");
	propertyid(prboolContainsApplication,typeBool,"ContainsApplication");
	propertyid(prboolContainsShutdown,typeBool,"ContainsShutdownImage");
	propertyid(prboolContainsBootup,typeBool,"ContainsBootupImage");

	propertyid(prboolContainsIMelodyHead,typeBool,"ContainsIMelodyHead");
	propertyid(prboolContainsTempo,typeBool,"ContainsTempo");
	propertyid(prboolContainsName,typeBool,"ContainsName");
	propertyid(prboolInterlaced,typeBool,"Interlaced");
	propertyid(prboolTransparent,typeBool,"Transparent");
	propertyid(prboolProgressive,typeBool,"Progressive");
	propertyid(prstrEncoding,typeString,"Encoding");	
	propertyid(prstrCompression,typeString,"Compression");
}
