# Change this if needed
include package.conf

RTLIBDIRS = FMINST CLAR COMBIT FMINST SCULPT STEREO WAVESHAPE \
METAFLUTE MULTICOMB AM AMINST IIR DEL1 DELAY PANECHO STRUM TRANS FIR

CMIXLIBDIRS = clar combit fir fminst iir move multicomb noise place sculpt \
numtest strum wah wahwah wow wshape metaflute trans am aminst pvoc rotate \
mrotate convolve gravy sgran vwavetable del1 delay panecho flange # lpc

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
	  ln -fs `pwd`/lib$$DIR.so $(LIBDESTDIR); ) \
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

dso_clean:
	@for DIR in $(LIBDIRS); \
	do \
	  ( cd $$DIR; make clean; \
	  rm -f $(LIBDESTDIR)/lib$$DIR.so; ) \
	done

standalone_clean:
	@for DIR in $(LIBDIRS); \
	do \
	  ( cd $$DIR; make clean; \
	  rm -f $(DESTDIR)/$$DIR; \
	done
