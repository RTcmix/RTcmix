# change these if needed
include package.conf

INSTDIR = insts.base

RTLIBDIRS = MIX WAVETABLE

CMIXLIBDIRS = mix wavetable

LIBDIRS = $(RTLIBDIRS)  $(CMIXLIBDIRS) 
DSODIRS = WAVETABLE $(CMIXLIBDIRS)

all:
	@for DIR in $(LIBDIRS); \
	do \
	  ( cd $$DIR; echo "making in $$DIR..."; \
	  $(MAKE) $(MFLAGS); echo "done in $$DIR"; ) \
	done

install:
	@for DIR in $(DSODIRS); \
	do \
	  ( cd $$DIR; echo "making install in $$DIR..."; \
	  ln -fs `pwd`/lib$$DIR.so $(LIBDESTDIR) ); \
	done
	@ln -fs `pwd`/MIX/MIX $(DESTDIR)/CMIX

dsos:  all

dso_install:  install

standalone:
	@for DIR in $(LIBDIRS); \
	do \
	  ( cd $$DIR; echo "making in $$DIR..."; \
	  $(MAKE) $(MFLAGS) standalone; echo "done in $$DIR" ); \
	done

standalone_install:
	@for DIR in $(LIBDIRS); \
	do \
	( cd $$DIR; echo "making install in $$DIR..."; \
	  rm -f $(DESTDIR)/$$DIR; \
	  ln -fs `pwd`/$$DIR $(DESTDIR); ) \
	done

clean:
	@for DIR in $(LIBDIRS); \
	do \
	  ( cd $$DIR; make clean; \
	  rm -f $(DESTDIR)/$$DIR; \
	  rm -f $(LIBDESTDIR)/lib$$DIR.so; ) \
	done
	rm -f $(DESTDIR)/CMIX

dso_clean:
	@for DIR in $(LIBDIRS); \
	do \
	  ( cd $$DIR; make clean; \
	  rm -f $(LIBDESTDIR)/lib$$DIR.so; ) \
	done
	rm -f $(DESTDIR)/CMIX

standalone_clean:
	@for DIR in $(LIBDIRS); \
	do \
	  ( cd $$DIR; make clean; \
	  rm -f $(DESTDIR)/$$DIR; \
	done

