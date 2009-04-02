INCLUDES = -I../retroBase

noinst_HEADERS =    \
G711.h              \
Version.h           \
stdafx.h            

retroG711_sources = \
G711.cpp \
retroG711.cpp \
G711Property.cpp 

install-libLTLIBRARIES:
	$(mkinstalldirs) @RETROCODEC_INSTALL_PATH@
	$(LIBTOOL) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) .libs/libretrog711.@RETROLIBEXT@ @RETROCODEC_INSTALL_PATH@/retroG711.@RETROLIBEXT@

lib_LTLIBRARIES = libretrog711.la

libretrog711_la_SOURCES = $(retroG711_sources)
libretrog711_la_LIBADD = @RETROLIBS@
