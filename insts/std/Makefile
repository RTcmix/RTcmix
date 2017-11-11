include package.conf

RTLIBDIRS = AM AMINST CLAR COMBIT DELAY DEL1 DUMP FIR FMINST HOLO IIR \
LPCPLAY MARAGRAN METAFLUTE MOCKBEND MOVE MULTICOMB NOISE PANECHO PHASER PVOC \
REVMIX SCULPT STEREO STRUM STRUM2 STRUMFB TRANS TRANSBEND WAVESHAPE \
MMOVE SCRUB LOOP

ifeq ($(BUILDTYPE), STANDALONE)
	CMIXLIBDIRS = convolve gravy mrotate numtest rotate sgran stgran wow
endif

LIBDIRS = $(RTLIBDIRS) $(CMIXLIBDIRS)

.PHONY: all progs dsos standalone install dso_install standalone_install \
uninstall dso_uninstall standalone_uninstall clean cleanall $(LIBDIRS)


all: $(LIBDIRS)

$(LIBDIRS)::
	@echo "making all in std..."
	$(MAKE) $(MFLAGS) -C $@

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

cleanall:
	@for DIR in $(LIBDIRS); \
	do \
	  ( if test -d $$DIR; then cd $$DIR; echo "making cleanll in $$DIR..."; \
		$(MAKE) $(MFLAGS) cleanall; fi ) \
	done

