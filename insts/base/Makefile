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
	  $(INSTALL) `pwd`/lib$$DIR.so $(LIBDESTDIR) ); \
	done
	@$(INSTALL) `pwd`/MIX/MIX $(DESTDIR)/CMIX
ifeq ($(PERL_SUPPORT), TRUE)
	@$(INSTALL) `pwd`/MIX/PMIX $(DESTDIR)/PCMIX
endif

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
	  $(INSTALL) `pwd`/$$DIR $(DESTDIR); ) \
	done

clean:
	@for DIR in $(LIBDIRS); \
	do \
	  ( cd $$DIR; $(MAKE) clean; \
	  $(RM) $(DESTDIR)/$$DIR; \
	  $(RM) $(LIBDESTDIR)/lib$$DIR.so; ) \
	done
	@$(RM) $(DESTDIR)/CMIX
	@$(RM) $(DESTDIR)/PCMIX

dso_clean:
	@for DIR in $(LIBDIRS); \
	do \
	  ( cd $$DIR; $(MAKE) clean; \
	  $(RM) $(LIBDESTDIR)/lib$$DIR.so; ) \
	done
	@$(RM) $(DESTDIR)/CMIX

standalone_clean:
	@for DIR in $(LIBDIRS); \
	do \
	  ( cd $$DIR; $(MAKE) clean; \
	  $(RM) $(DESTDIR)/$$DIR; ) \
	done

