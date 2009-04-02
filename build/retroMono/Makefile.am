INCLUDES = -I../retroBase

noinst_HEADERS =    \
EMSFile.h EMSProperty.h  \
MotorolaFile.h MotorolaProperty.h  \
MSEQFile.h MSEQProperty.h  \
NokiaFile.h NokiaProperty.h  \
SagemFile.h SagemProperty.h  \
MonoFile.h \
resource.h Version.h stdafx.h

retroMono_sources = \
EMSFile.cpp EMSProperty.cpp  \
MotorolaFile.cpp MotorolaProperty.cpp  \
MSEQFile.cpp MSEQProperty.cpp  \
NokiaFile.cpp NokiaProperty.cpp  \
SagemFile.cpp SagemProperty.cpp  \
MonoFile.cpp \
retroMono.cpp

install-libLTLIBRARIES:
	$(mkinstalldirs) @RETROVALIDATOR_INSTALL_PATH@
	$(LIBTOOL) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) .libs/libretromono.@RETROLIBEXT@ @RETROVALIDATOR_INSTALL_PATH@/retroMono.@RETROLIBEXT@

lib_LTLIBRARIES = libretromono.la

libretromono_la_SOURCES = $(retroMono_sources)
libretromono_la_LIBADD = @RETROLIBS@
