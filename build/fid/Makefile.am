INCLUDES = -I../retroBase

noinst_HEADERS =    \
 fid.h XMFFile.h MFIFile.h stdafx.h

fid_sources = \
 fid.cpp XMFFile.cpp MFIFile.cpp 
 
bin_PROGRAMS = fid

AM_CXXFLAGS = @CXXFLAGS@ -DRETROCODEC_LIB_PATH=\"@RETROCODEC_INSTALL_PATH@\" -DRETROVALIDATOR_LIB_PATH=\"@RETROVALIDATOR_INSTALL_PATH@\"
fid_SOURCES = $(fid_sources)
fid_LDADD = @RETROLIBS@ -ldl
fid_LDFLAGS = @LDFLAGS@ @RETROLDFLAGS@
