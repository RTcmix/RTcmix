# change these if needed
include package.conf
INSTDIR = insts.base

RTLIBDIRS = MIX WAVETABLE

CMIXLIBDIRS = mix wavetable

LIBDIRS = $(RTLIBDIRS)  $(CMIXLIBDIRS) 
DSODIRS = WAVETABLE $(CMIXLIBDIRS)

all:
	@for DIR in $(LIBDIRS) MIX; \
	do \
	( cd $$DIR; echo "making in $$DIR..."; \
	$(MAKE) $(MFLAGS); echo "done in $$DIR"; \
	ln -fs `pwd`/lib$$DIR.so $(LIBDESTDIR) ); \
	done
	@ln -fs `pwd`/MIX/MIX $(DESTDIR)/CMIX

clean:
	@for DIR in $(LIBDIRS); \
	do \
	( cd $$DIR; make clean; \
	rm -f $(DESTDIR)/$$DIR; \
	rm -f $(LIBDESTDIR)/lib$$DIR.so; ) \
	done
	rm -f $(DESTDIR)/CMIX

standalone:
	@for DIR in $(LIBDIRS); \
	do \
	( cd $$DIR; echo "making install in $$DIR..."; \
	  rm -f $(DESTDIR)/$$DIR; \
	  make standalone; \
	  ln -fs `pwd`/$$DIR $(DESTDIR); ) \
	done
	@ln -fs `pwd`/MIX/MIX $(DESTDIR)/CMIX

onlyDSO:
	@for DIR in $(DSODIRS); \
	do \
	( cd $$DIR; echo "removing standalone instrument in $$DIR..."; \
	rm -f $$DIR; ) \
	done

