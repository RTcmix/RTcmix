include package.conf

ifeq ($(BUILDTYPE), STANDALONE)
	RTLIBDIRS = AM AMINST CLAR COMBIT DELAY DEL1 DUMP FIR FMINST HOLO IIR \
	LPCPLAY MARAGRAN METAFLUTE MOCKBEND MOVE MULTICOMB NOISE PANECHO PVOC \
	REVMIX SCULPT STEREO STRUM STRUM2 STRUMFB TRANS TRANSBEND WAVESHAPE \
	MMOVE SCRUB LOOP

	CMIXLIBDIRS = convolve gravy mrotate numtest rotate sgran stgran wow
else
	RTLIBDIRS = AM AMINST CLAR COMBIT DELAY DEL1 DUMP FIR FMINST HOLO IIR \
	LPCPLAY MARAGRAN METAFLUTE MOCKBEND MOVE MULTICOMB NOISE PANECHO PHASER \
	PVOC.maxmsp REVMIX SCULPT STEREO STRUM STRUM2 STRUMFB TRANS TRANSBEND \
	WAVESHAPE MMOVE SCRUB LOOP
endif

LIBDIRS = $(RTLIBDIRS) $(CMIXLIBDIRS)

.PHONY: all progs dsos standalone install dso_install standalone_install \
uninstall dso_uninstall standalone_uninstall clean cleanall


all:
	@for DIR in $(LIBDIRS); \
	do \
	  ( if test -d $$DIR; then cd $$DIR; echo "making all in $$DIR..."; \
	  $(MAKE) $(MFLAGS); echo "done in $$DIR"; fi ) \
	done

progs: all

dsos: all

standalone:
	@for DIR in $(LIBDIRS); \
	do \
	  ( if test -d $$DIR; then cd $$DIR; echo "making standalone in $$DIR..."; \
	  $(MAKE) $(MFLAGS) standalone; echo "done in $$DIR"; fi ) \
	done

install:
	@for DIR in $(LIBDIRS); \
	do \
	  ( if test -d $$DIR; then cd $$DIR; echo "making install in $$DIR..."; \
	  $(MAKE) $(MFLAGS) install; fi ) \
	done

dso_install:
	@for DIR in $(LIBDIRS); \
	do \
	( if test -d $$DIR; then cd $$DIR; echo "making dso_install in $$DIR..."; \
	  $(MAKE) $(MFLAGS) dso_install; fi ) \
	done

standalone_install:
	@for DIR in $(LIBDIRS); \
	do \
	( if test -d $$DIR; then cd $$DIR; \
	  echo "making standalone_install in $$DIR..."; \
	  $(MAKE) $(MFLAGS) standalone_install; fi ) \
	done

uninstall:
	@for DIR in $(LIBDIRS); \
	do \
	( if test -d $$DIR; then cd $$DIR; echo "making uninstall in $$DIR..."; \
	  $(MAKE) $(MFLAGS) uninstall; fi ) \
	done

dso_uninstall:
	@for DIR in $(LIBDIRS); \
	do \
	( if test -d $$DIR; then cd $$DIR; echo "making dso_uninstall in $$DIR..."; \
	  $(MAKE) $(MFLAGS) dso_uninstall; fi ) \
	done

standalone_uninstall:
	@for DIR in $(LIBDIRS); \
	do \
	( if test -d $$DIR; then cd $$DIR; \
	  echo "making standalone_uninstall in $$DIR..."; \
	  $(MAKE) $(MFLAGS) standalone_uninstall; fi ) \
	done

clean:
	@for DIR in $(LIBDIRS); \
	do \
	  ( if test -d $$DIR; then cd $$DIR; echo "making clean in $$DIR..."; \
		$(MAKE) $(MFLAGS) clean; fi ) \
	done

cleanall: clean uninstall

