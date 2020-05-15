include ../../makefile.conf

ifeq ($(BUILDTYPE), STANDALONE)
	DIRS = common midi mouse display datafile pfbus
else
	DIRS = maxmsp pfbus
endif

ifeq ($(OSC_SUPPORT), TRUE)
	DIRS += osc
endif

all: $(DIRS) $(OBJS)

$(DIRS)::
	@echo "making all in control..."
	$(MAKE) $(MFLAGS) -C $@

.PHONY: install clean cleanall $(DIRS)

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
