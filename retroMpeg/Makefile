#target name
OUTFILEBASE=retrompeg

#install folder for this library
INSTALLDIR=$(INSTALL_CODEC_DIR)

#compiler flags
CFLAGS=-DUSEAMR_FLOAT -DID3LIB_LINKOPTION=3

#install folder for this library
BIN=../code/codecs/

#include folders
INC:=$(INC) -I../retroBase -I/usr/local/include -I/usr/include -I/usr/local/include/lame

#libraries
LIBS:=$(LIBS) -L/usr/local/lib -L/usr/lib -L../retroBase -lstdc++ -lid3 -lmp4ff -lfaad -lfaac -lz -lamrwb -lmp3lame -lspc -lfipop -lmad -lretrobase -lavformat -lavcodec -lavutil -lamrnb

#binary object list
OBJFILES=AACFile.o AACProperty.o AMRFile.o AMRProperty.o BitStream.o MP3File.o MP3Property.o MP4File.o MP4Property.o WMAFile.o OGGFile.o RAFile.o FFMPEGFile.o FFMPEGProperty.o SWFFile.o SWFProperty.o retroMpeg.o

include ../retroBase/Makefile.Library
