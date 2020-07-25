include ../../makefile.conf

ifdef $(PD_INCLUDES)
    PDINCLUDEDIR=$(PD_INCLUDES)
endif

# Makefile to build class 'rtcmix' for Pure Data.
# Needs Makefile.pdlibbuilder as helper makefile for platform-dependent build
# settings and rules.

# library name
lib.name = rtcmix~

# input source file (class name == source file basename)
class.sources = rtcmix~.c

# all extra files to be included in binary distribution of the library
datafiles = rtcmix~-help.pd rtcmix~-meta.pd README.md

# include Makefile.pdlibbuilder from submodule directory 'pd-lib-builder'
PDLIBBUILDER_DIR=pd-lib-builder/
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder
