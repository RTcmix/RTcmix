include ../../makefile.conf

IMBDIRS = flutescale arpeggiate crudelooch happykeys mixit wanderfreq

DIRS = $(IMBDIRS)

all:
	@for DIR in $(DIRS); \
	do \
	  ( cd $$DIR; echo "making in $$DIR..."; \
	  $(MAKE) $(MFLAGS); echo "done in $$DIR"; ) \
	done

install:
	@echo "nothing to be done in imbed"

uninstall:
	@echo "nothing to be done in imbed"

clean:
	@for DIR in $(DIRS); \
	do \
	  ( cd $$DIR; $(MAKE) clean; ) \
	done

cleanall: clean uninstall

