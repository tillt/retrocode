INCLUDES = -I../retroBase

noinst_HEADERS =    \
ANSI733Codec.h CMXFile.h  CMXProperty.h  CMXSample.h  QCELPFile.h  QCELPProperty.h  Version.h  stdafx.h

retroPanasonic_sources = \
ANSI733Codec.cpp CMXFile.cpp  CMXProperty.cpp  CMXSample.cpp  QCELPFile.cpp  QCELPProperty.cpp  retroQualcomm.cpp

DESTDIR=@RETROCODEC_INSTALL_PATH@

install-libLTLIBRARIES:
	$(mkinstalldirs) @RETROCODEC_INSTALL_PATH@
	$(LIBTOOL) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) .libs/libretroqualcomm.@RETROLIBEXT@ @RETROCODEC_INSTALL_PATH@/retroQualcomm.@RETROLIBEXT@

lib_LTLIBRARIES = libretroqualcomm.la

AM_CXXFLAGS = @RETROQUALCOMM_CPPFLAGS@ @CXXFLAGS@ 
libretroqualcomm_la_SOURCES = $(retroQualcomm_sources)
libretroqualcomm_la_LIBADD = @RETROLIBS@
