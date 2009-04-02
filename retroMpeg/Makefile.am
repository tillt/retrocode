INCLUDES = -I../retroBase

noinst_HEADERS =    \
AACFile.h      AMRProperty.h  FFMPEGProperty.h  MP4File.h      OGGFile.h  SWFProperty.h  stdafx.h \
AACProperty.h  MP3File.h         MP4Property.h  RAFile.h   Version.h \
AMRFile.h      FFMPEGFile.h   MP3Property.h     Mp4Export.h    SWFFile.h  WMAFile.h

retroMpeg_sources = \
retroMpeg.cpp \
AACFile.cpp AACProperty.cpp \
AMRFile.cpp AMRProperty.cpp \
FFMPEGFile.cpp FFMPEGProperty.cpp \
OGGFile.cpp \
RAFile.cpp \
WMAFile.cpp \
MP3File.cpp MP3Property.cpp \
MP4File.cpp MP4Property.cpp \
SWFFile.cpp SWFProperty.cpp 

install-libLTLIBRARIES:
	$(mkinstalldirs) @RETROCODEC_INSTALL_PATH@
	$(LIBTOOL) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) .libs/libretrompeg.@RETROLIBEXT@ @RETROCODEC_INSTALL_PATH@/retroMpeg.@RETROLIBEXT@

lib_LTLIBRARIES = libretrompeg.la
libretrompeg_la_SOURCES = $(retroMpeg_sources)
libretrompeg_la_CXXFLAGS = -DUSEAMR_FLOAT -DID3LIB_LINKOPTION=3
libretrompeg_la_LIBADD = @RETROLIBS@ -lavutil -lavcodec -lavformat -lmp3lame -lamrnb -lmad -lid3 -lfaac -lfaad -lmp4ff -lamrnb -lamrwb -lismacryp -lmp4v2
