# Change this if needed
include package.conf

RTLIBDIRS = AM AMINST CLAR COMBIT DELAY DEL1 FIR FMINST IIR LPCPLAY METAFLUTE \
MOVE MULTICOMB PANECHO PLACE REVMIX SCULPT STEREO STRUM TRANS TRANSBEND \
WAVESHAPE

CMIXLIBDIRS = convolve gravy mrotate noise numtest pvoc rotate \
sgran stgran vwavetable wah wahwah wow

LIBDIRS = $(RTLIBDIRS)  $(CMIXLIBDIRS) 

all:
	@for DIR in $(LIBDIRS); \
	do \
	  ( cd $$DIR; echo "making in $$DIR..."; \
	  $(MAKE) $(MFLAGS); echo "done in $$DIR"; ) \
	done

install:
	@for DIR in $(LIBDIRS); \
	do \
	  ( cd $$DIR; echo "making install in $$DIR..."; \
	  $(INSTALL) `pwd`/lib$$DIR.so $(LIBDESTDIR); ) \
	done
	@# temporary workaround to install PLACE dso:
	@$(INSTALL) `pwd`/MOVE/libPLACE.so $(LIBDESTDIR)

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
	@# temporary workaround to clean PLACE dso:
	$(RM) $(LIBDESTDIR)/libPLACE.so

dso_clean:
	@for DIR in $(LIBDIRS); \
	do \
	  ( cd $$DIR; $(MAKE) clean; \
	  $(RM) $(LIBDESTDIR)/lib$$DIR.so; ) \
	done

standalone_clean:
	@for DIR in $(LIBDIRS); \
	do \
	  ( cd $$DIR; $(MAKE) clean; \
	  $(RM) $(DESTDIR)/$$DIR; \
	done
