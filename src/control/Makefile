include ../../makefile.conf

# BGGx
#	DIRS = common midi mouse display datafile pfbus
ifeq ($(BUILDTYPE), STANDALONE)
	DIRS = common midi datafile pfbus
else
	DIRS = maxmsp pfbus
endif

ifeq ($(OSC_SUPPORT), TRUE)
	DIRS += osc
endif

all: $(DIRS) $(OBJS)
	@for DIR in $(DIRS); \
	do \
	  ( cd $$DIR; $(MAKE) $(MFLAGS) all; ) \
	done

.PHONY: install clean cleanall

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

ifeq ($(BUILDTYPE), STANDALONE)
depend:
	@for DIR in $(DIRS); \
	do \
	  ( cd $$DIR; $(MAKE) $(MFLAGS) depend; ) \
	done
endif

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
