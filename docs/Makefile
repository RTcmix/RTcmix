# RTcmix  - Copyright (C) 2000  The RTcmix Development Team
# See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
# the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
#
include ../makefile.conf

DIRS = #pod

all:
	@for DIR in $(DIRS); \
	do \
		( cd $$DIR; $(MAKE) $(MFLAGS) all; ) \
	done

install:
	@for DIR in $(DIRS); \
	do \
		( cd $$DIR; $(MAKE) $(MFLAGS) install; ) \
	done

uninstall:
	@for DIR in $(DIRS); \
	do \
		( cd $$DIR; $(MAKE) $(MFLAGS) uninstall; ) \
	done

clean:
	@for DIR in $(DIRS); \
	do \
		( cd $$DIR; $(MAKE) $(MFLAGS) clean; ) \
	done

cleanall:
	@for DIR in $(DIRS); \
	do \
		( cd $$DIR; $(MAKE) $(MFLAGS) cleanall; ) \
	done

distclean: cleanall

