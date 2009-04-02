INCLUDES = -I../retroBase

noinst_HEADERS =    \
RMFBasics.h  RMFEncoder.h  RMFFile.h  RMFInstrument.h  RMFProperty.h  RMFSequence.h  RMFSound.h  Version.h  stdafx.h

retroBeatnik_sources = \
RMFBasics.cpp  RMFFile.cpp  RMFInstrument.cpp  RMFProperty.cpp  RMFSequence.cpp  RMFSound.cpp retroBeatnik.cpp

install-libLTLIBRARIES:
	$(mkinstalldirs) @RETROCODEC_INSTALL_PATH@
	$(LIBTOOL) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) .libs/libretrobeatnik.@RETROLIBEXT@ @RETROCODEC_INSTALL_PATH@/retroBeatnik.@RETROLIBEXT@

lib_LTLIBRARIES = libretrobeatnik.la

libretrobeatnik_la_SOURCES = $(retroBeatnik_sources)
libretrobeatnik_la_LIBADD = @RETROLIBS@
