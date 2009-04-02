INCLUDES = -I../retroBase

noinst_HEADERS =    \
Adpcm.h             \
Basics.h            \
BitStream.h         \
CommonAccess.h      \
CompressedIO.h      \
MemoryBitmap.h      \
Midispec.h          \
MobileContent.h     \
MyLanguage.h        \
MyString.h          \
Modules.h 			\
MIDIFile.h 			\
Endian.h 			\
Packer.h 			\
PacketCollection.h  \
Version.h           \
FIDProperty.h 		\
stdafx.h            \
wruntime_error.h 

retroBase_sources = 	\
Adpcm.cpp 				\
Basics.cpp 				\
BitStream.cpp 			\
CommonAccess.cpp 		\
CompressedIO.cpp 		\
MemoryBitmap.cpp 		\
MobileContent.cpp 		\
MobileProperty.cpp 		\
FIDProperty.cpp 		\
MyString.cpp 			\
Endian.cpp 				\
Packer.cpp 				\
MIDIFile.cpp			\
MIDIFileWriter.cpp 		\
Modules.cpp 			\
PacketCollection.cpp	\
wruntime_error.cpp

lib_LTLIBRARIES = libretrobase.la

AM_CXXFLAGS = @RETROBASE_CPPFLAGS@ @CXXFLAGS@
libretrobase_la_SOURCES = $(retroBase_sources)
#libretrobase_la_LDFLAGS = -no-undefined
libretrobase_la_LIBADD = -lz
