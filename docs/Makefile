# RTcmix  - Copyright (C) 2000  The RTcmix Development Team
# See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
# the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
#
# RTcmix help system makefile by John Gibson
# 
include ../makefile.conf

DIRS = pod

all:
	@for DIR in $(DIRS); \
	do \
		( cd $$DIR; $(MAKE) $(MFLAGS) all; ) \
	done

install: all

uninstall:
	@for DIR in $(DIRS); \
	do \
		( cd $$DIR; $(MAKE) $(MFLAGS) uninstall; ) \
	done

clean:
	@echo "'clean' does not affect docs"

cleanall: uninstall
	@for DIR in $(DIRS); \
	do \
		( cd $$DIR; $(MAKE) $(MFLAGS) clean; ) \
	done
