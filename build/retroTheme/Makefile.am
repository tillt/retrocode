INCLUDES = -I../retroBase

noinst_HEADERS =    \
JADFile.h JADProperty.h  \
JARFile.h JARProperty.h  \
MTFFile.h MTFProperty.h  \
NTHFile.h NTHProperty.h  \
SDFFile.h SDFProperty.h  \
SISFile.h SISProperty.h  \
THMFile.h THMProperty.h  \
UTZFile.h UTZProperty.h  \
THMSamsungFile.h THMSamsungProperty.h  \
TARArchive.h \
ZIPArchive.h \
ThemeBaseContent.h \
resource.h Version.h stdafx.h

retroTheme_sources = \
JADFile.cpp JADProperty.cpp  \
JARFile.cpp JARProperty.cpp  \
MTFFile.cpp MTFProperty.cpp  \
NTHFile.cpp NTHProperty.cpp  \
SDFFile.cpp SDFProperty.cpp  \
SISFile.cpp SISProperty.cpp  \
THMFile.cpp THMProperty.cpp  \
THMSamsungFile.cpp THMSamsungProperty.cpp  \
UTZFile.cpp UTZProperty.cpp  \
TARArchive.cpp \
ZIPArchive.cpp \
ThemeBaseContent.cpp \
retroTheme.cpp

install-libLTLIBRARIES:
	$(mkinstalldirs) @RETROVALIDATOR_INSTALL_PATH@
	$(LIBTOOL) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) .libs/libretrotheme.@RETROLIBEXT@ @RETROVALIDATOR_INSTALL_PATH@/retroTheme.@RETROLIBEXT@

lib_LTLIBRARIES = libretrotheme.la

libretrotheme_la_SOURCES = $(retroTheme_sources)
libretrotheme_la_LIBADD = @RETROLIBS@ -lminizip
