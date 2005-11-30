include ../../makefile.conf

DIRS = common midi mouse display datafile
ifeq ($(OSC_SUPPORT), TRUE)
	DIRS += osc
endif

all: $(DIRS) $(OBJS)
	@for DIR in $(DIRS); \
	do \
	  ( cd $$DIR; $(MAKE) $(MFLAGS) all; ) \
	done

.PHONY: install clean

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

cleanall: clean

