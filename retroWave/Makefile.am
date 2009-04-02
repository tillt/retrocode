INCLUDES = -I../retroBase

noinst_HEADERS =    \
AVIFile.h  AVIProperty.h  Version.h  WaveFile.h  WaveProperty.h  stdafx.h

retroWave_sources = \
AVIFile.cpp  AVIProperty.cpp  IMACodec.cpp  MSCodec.cpp  WaveFile.cpp  WaveProperty.cpp retroWave.cpp

install-libLTLIBRARIES:
	$(mkinstalldirs) @RETROCODEC_INSTALL_PATH@
	$(LIBTOOL) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) .libs/libretrowave.@RETROLIBEXT@ @RETROCODEC_INSTALL_PATH@/retroWave.@RETROLIBEXT@

lib_LTLIBRARIES = libretrowave.la

libretrowave_la_SOURCES = $(retroWave_sources)
libretrowave_la_LIBADD = @RETROLIBS@