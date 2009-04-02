#target name
OUTFILEBASE=retroapple

#include retroBase folder in linker search path
LIBS:=$(LIBS) -L../retroBase

#install folder for this library
BIN=../code/codecs/

#include folders
INC:=$(INC) -I../retroBase

#libraries
LIBS:=$(LIBS) -lretrobase

#binary object list
OBJFILES=AIFFile.o AIFProperty.o retroApple.o

include ../retroBase/Makefile.Library
