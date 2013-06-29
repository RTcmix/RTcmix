include package.conf

ifeq ($(BUILDTYPE), STANDALONE)
	RTLIBDIRS = BUTTER COMPLIMIT CONVOLVE1 DCBLOCK DECIMATE DISTORT ELL EQ \
	FILTERBANK FILTSWEEP FLANGE FOLLOWER FREEVERB GRANSYNTH GRANULATE JCHOR \
	JDELAY JFIR JGRAN MOOGVCF MROOM MULTEQ MULTIWAVE NPAN PAN QPAN REV \
	REVERBIT ROOM SHAPE SPECTACLE SPECTACLE2 SPLITTER SROOM VOCODE2 \
	VOCODE3 VOCODESYNTH WAVY WIGGLE

	CMIXLIBDIRS = bender denoise splice transpan
else
	RTLIBDIRS = BUTTER COMPLIMIT CONVOLVE1 DCBLOCK DECIMATE DISTORT ELL EQ \
	FILTERBANK FILTSWEEP FLANGE FOLLOWER FREEVERB GRANSYNTH JCHOR JDELAY JFIR \
	JGRAN MOOGVCF MROOM MULTEQ MULTIWAVE PAN REV REVERBIT ROOM SHAPE \
	SPECTACLE SPECTACLE2 SPLITTER VOCODE2 VOCODE3 VOCODESYNTH WAVY WIGGLE
endif

LIBDIRS = $(RTLIBDIRS) $(CMIXLIBDIRS)

.PHONY: all objlib progs dsos standalone install dso_install standalone_install\
uninstall dso_uninstall standalone_uninstall clean cleanall

all: objlib
	@for DIR in $(LIBDIRS); \
	do \
	  ( if test -d $$DIR; then cd $$DIR; echo "making all in $$DIR..."; \
	  $(MAKE) $(MFLAGS) all; echo "done in $$DIR"; fi ) \
	done

progs: all

dsos: all

objlib:
	@( cd objlib; echo "making in objlib..."; \
	  $(MAKE) $(MFLAGS); echo "done in objlib" ); \

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
	  ( if test -d $$DIR; then cd $$DIR; \
	  echo "making dso_uninstall in $$DIR..."; \
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
	@( cd objlib; echo "making clean in objlib..."; \
		$(MAKE) $(MFLAGS) clean; )

cleanall: clean uninstall

