include ../../makefile.conf

IMBDIRS = flutescale arpeggiate crudelooch happykeys mixit wanderfreq

all: $(IMBDIRS)

$(IMBDIRS)::
	$(MAKE) $(MFLAGS) -C $@;

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

