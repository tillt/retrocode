INCLUDES = -I../retroBase

noinst_HEADERS =    \
SMAFAudio.h      SMAFDecoder.h  SMAFFile.h   SMAFProperty.h  SMAFTrack.h  stdafx.h \
SMAFEvent.h    SMAFGraph.h  SMAFSample.h    Version.h

retroYamaha_sources = \
SMAFAudio.cpp      SMAFDecoder.cpp  SMAFFile.cpp   SMAFProperty.cpp  SMAFTrack.cpp  \
SMAFEvent.cpp    SMAFGraph.cpp  SMAFSample.cpp  retroYamaha.cpp

install-libLTLIBRARIES:
	$(mkinstalldirs) @RETROCODEC_INSTALL_PATH@
	$(LIBTOOL) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) .libs/libretroyamaha.@RETROLIBEXT@ @RETROCODEC_INSTALL_PATH@/retroYamaha.@RETROLIBEXT@

lib_LTLIBRARIES = libretroyamaha.la
libretroyamaha_la_LIBADD = @RETROLIBS@
libretroyamaha_la_SOURCES = $(retroYamaha_sources)
