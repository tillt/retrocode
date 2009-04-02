INCLUDES = -I../code -I../retroBase

noinst_HEADERS =    \
ButterWorth.h Process.h Config.h  EightToSixteenBit.h  Fade.h    Loop.h      		\
 Normalizer.h RateConverter.h  Ringback.h  stdafx.h ChannelConverter.h 	\
 Crop.h ExtNormalizer.h Filter.h  MyLanguage.h  PeakLimiter.h  resource.h  code.h

code_sources = \
ButterWorth.cpp       Process.cpp EightToSixteenBit.cpp  Filter.cpp  Normalizer.cpp     Ringback.cpp \
ChannelConverter.cpp  ExtNormalizer.cpp      PeakLimiter.cpp    code.cpp \
Crop.cpp              Fade.cpp               Loop.cpp    RateConverter.cpp 

bin_PROGRAMS = code

AM_CXXFLAGS = @CXXFLAGS@ -DRETROCODEC_LIB_PATH=\"@RETROCODEC_INSTALL_PATH@\"
code_SOURCES = $(code_sources)
code_LDADD = @RETROLIBS@
code_LDFLAGS = @LDFLAGS@ @RETROLDFLAGS@
