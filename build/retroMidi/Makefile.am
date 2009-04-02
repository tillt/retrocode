INCLUDES = -I../retroBase

noinst_HEADERS =    \
MIDIFileDoc.h MIDIProperty.h  \
resource.h Version.h stdafx.h

retroMidi_sources = \
MIDIFileDoc.cpp MIDIProperty.cpp  \
retroMidi.cpp

install-libLTLIBRARIES:
	$(mkinstalldirs) @RETROVALIDATOR_INSTALL_PATH@
	$(LIBTOOL) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) .libs/libretromidi.@RETROLIBEXT@ @RETROVALIDATOR_INSTALL_PATH@/retroMidi.@RETROLIBEXT@

lib_LTLIBRARIES = libretromidi.la

libretromidi_la_SOURCES = $(retroMidi_sources)
libretromidi_la_LIBADD = @RETROLIBS@
