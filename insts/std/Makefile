# Change this if needed
include package.conf

RTLIBDIRS = COMBIT STEREO FMINST STRUM PANECHO AMINST

#RTLIBDIRS = FMINST CLAR COMBIT SCULPT STEREO WAVESHAPE \
#METAFLUTE MULTICOMB AM AMINST IIR DEL1 DELAY PANECHO STRUM FIR #TRANS

#CMIXLIBDIRS = clar combit fir fminst iir move multicomb noise place sculpt \
#numtest strum wah wahwah wow wsh metaflute trans am aminst pvoc rotate \
#mrotate convolve gravy sgran vwavetable del1 delay panecho flange # lpc

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
