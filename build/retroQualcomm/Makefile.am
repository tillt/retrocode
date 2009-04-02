INCLUDES = -I../retroBase

noinst_HEADERS =    \
CMXFile.h  CMXProperty.h  CMXSample.h  QCELPFile.h  QCELPProperty.h  Version.h  stdafx.h

retroQualcomm_sources = \
CMXFile.cpp  CMXProperty.cpp  CMXSample.cpp  QCELPFile.cpp  QCELPProperty.cpp  retroQualcomm.cpp

DESTDIR=@RETROCODEC_INSTALL_PATH@

install-libLTLIBRARIES:
	$(mkinstalldirs) @RETROCODEC_INSTALL_PATH@
	$(LIBTOOL) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) .libs/libretroqualcomm.@RETROLIBEXT@ @RETROCODEC_INSTALL_PATH@/retroQualcomm.@RETROLIBEXT@

lib_LTLIBRARIES = libretroqualcomm.la

libretroqualcomm_la_SOURCES = $(retroQualcomm_sources)
libretroqualcomm_la_LIBADD = @RETROLIBS@