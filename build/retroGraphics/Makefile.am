INCLUDES = -I../retroBase

noinst_HEADERS =    \
BMPFile.h BMPProperty.h  \
GIFFile.h GIFProperty.h  \
JPGFile.h JPGProperty.h  \
MBMFile.h MBMProperty.h \
PNGFile.h PNGProperty.h PngMemoryBitmap.h \
stdafx.h Version.h 

retroGraphics_sources = \
BMPFile.cpp BMPProperty.cpp  \
GIFFile.cpp GIFProperty.cpp  \
JPGFile.cpp JPGProperty.cpp  \
MBMFile.cpp MBMProperty.cpp \
PNGFile.cpp PNGProperty.cpp PngMemoryBitmap.cpp \
retroGraphics.cpp

install-libLTLIBRARIES:
	$(mkinstalldirs) @RETROVALIDATOR_INSTALL_PATH@
	$(LIBTOOL) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) .libs/libretrographics.@RETROLIBEXT@ @RETROVALIDATOR_INSTALL_PATH@/retroGraphics.@RETROLIBEXT@

lib_LTLIBRARIES = libretrographics.la

libretrographics_la_SOURCES = $(retroGraphics_sources)
libretrographics_la_LIBADD = @RETROLIBS@ -lpng -lgif -ljpeg
