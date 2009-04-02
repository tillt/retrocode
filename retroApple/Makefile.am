INCLUDES = -I../retroBase

noinst_HEADERS =    \
AIFFile.h           \
AIFProperty.h       \
Version.h           \
stdafx.h            

retroApple_sources = \
AIFFile.cpp \
AIFProperty.cpp \
retroApple.cpp

install-libLTLIBRARIES:
	$(mkinstalldirs) @RETROCODEC_INSTALL_PATH@
	$(LIBTOOL) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) .libs/libretroapple.@RETROLIBEXT@ @RETROCODEC_INSTALL_PATH@/retroApple.@RETROLIBEXT@

lib_LTLIBRARIES = libretroapple.la

libretroapple_la_SOURCES = $(retroApple_sources)
libretroapple_la_LIBADD = @RETROLIBS@
