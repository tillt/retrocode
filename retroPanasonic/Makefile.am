INCLUDES = -I../retroBase

noinst_HEADERS =    \
MFMFile.h  MFMProperty.h  MFMSample.h  VOXFile.h  VOXProperty.h  Version.h  stdafx.h

retroPanasonic_sources = \
MFMFile.cpp  MFMProperty.cpp  MFMSample.cpp  VOXFile.cpp  VOXProperty.cpp retroPanasonic.cpp

install-libLTLIBRARIES:
	$(mkinstalldirs) @RETROCODEC_INSTALL_PATH@
	$(LIBTOOL) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) .libs/libretropanasonic.@RETROLIBEXT@ @RETROCODEC_INSTALL_PATH@/retroPanasonic.@RETROLIBEXT@

lib_LTLIBRARIES = libretropanasonic.la

libretropanasonic_la_SOURCES = $(retroPanasonic_sources)
libretropanasonic_la_LIBADD = @RETROLIBS@
