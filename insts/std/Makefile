# Change this if needed
include insts.conf

INSTDIR = `pwd`

RTLIBDIRS = FMINST CLAR COMBIT FMINST SCULPT STEREO WAVESHAPE \
METAFLUTE MULTICOMB AM AMINST IIR DEL1 DELAY PANECHO STRUM

CMIXLIBDIRS = clar combit fir fminst iir move multicomb noise place sculpt \
numtest strum wah wahwah wow wshape metaflute trans am aminst pvoc rotate \
mrotate convolve gravy sgran vwavetable del1 delay panecho flange # lpc

LIBDIRS = $(RTLIBDIRS)  $(CMIXLIBDIRS) 

all:
	@for DIR in $(LIBDIRS); \
	do \
	( cd $$DIR; echo "making in $$DIR..."; \
	$(MAKE) $(MFLAGS); echo "done in $$DIR" ); \
	done

install:
	@for DIR in $(LIBDIRS); \
	do \
	( cd $$DIR; echo "making install in $$DIR..."; \
	ln -fs $(INSTDIR)/$$DIR/lib$$DIR.so $(LIBDESTDIR); ) \
	done
	@for DIR in $(LIBDIRS); \
	do \
	( cd $$DIR; echo "making install in $$DIR..."; \
	rm -f $(DESTDIR)/$$DIR; \
	ln -fs $(INSTDIR)/$$DIR/$$DIR $(DESTDIR); ) \
	done

clean:
	@for DIR in $(LIBDIRS); \
	do \
	( cd $$DIR; make clean; \
	rm -f $(DESTDIR)/$$DIR; \
	rm -f $(LIBDESTDIR)/lib$$DIR.so; ) \
	done

