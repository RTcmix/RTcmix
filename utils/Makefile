include ../makefile.conf

PLAY = cmixplay
SETUP = setup_rtcmixrc

PROGS = $(PLAY) $(SETUP) hist cpspch pchcps rescale sndpeak sndreverse

HISTOBJ = fft.o hist.o
SYSOBJS = $(SRCDIR)/rtcmix/printsf.o $(SRCDIR)/rtcmix/check_byte_order.o \
	$(AUDIODIR)/sndlibsupport.o
PLAY_HDRS = ../src/audio/sndlibsupport.h ../src/audio/AudioDevice.h
OPTION_O = ../src/rtcmix/Option.o
LIBCONF = ../src/conf/libconf.a
INCLUDES += -I$(AUDIODIR) -I$(SRCDIR)/sndlib -I$(SRCDIR)/rtcmix

all: $(PROGS) head resample

$(PLAY): $(PLAY).o $(AUDIOLIB) $(SNDLIB)
	$(CXX) $(CXXFLAGS) -o $(PLAY) $(PLAY).o $(OPTION_O) $(AUDIOLIB) $(SNDLIB) \
		$(LIBCONF) $(SYS_LDFLAGS)

$(PLAY).o: $(PLAY).cpp $(PLAY_HDRS)
	$(CXX) $(CXXFLAGS) $(SOUND_DRIVER) -c $< -o $@

$(SETUP): $(SETUP).o $(OPTION_O) $(LIBCONF)
	$(CXX) $(CXXFLAGS) -o $(SETUP) $(SETUP).o $(OPTION_O) $(LIBCONF)

hist: $(HISTOBJ) $(SYSOBJS)
	$(CC) $(CFLAGS) -o hist $(HISTOBJ) $(SYSOBJS) $(SNDLIB) -lm

cpspch: cpspchlookup.o $(GENLIB)
	$(CC) $(CFLAGS) -o cpspch cpspchlookup.c ../genlib/cpspch.o -lm

pchcps: pchcpslookup.o $(GENLIB)
	$(CC) $(CFLAGS) -o pchcps pchcpslookup.c ../genlib/pchcps.o ../genlib/octcps.o ../genlib/pchoct.o -lm

rescale: rescale.o $(SYSOBJS)
	$(CC) $(CFLAGS) -o rescale rescale.o $(SYSOBJS) $(SNDLIB) -lm

sndpeak: sndpeak.o $(SYSOBJS)
	$(CC) $(CFLAGS) -o sndpeak sndpeak.o $(SYSOBJS) $(SNDLIB) -lm

sndreverse: sndreverse.o $(SYSOBJS)
	$(CC) $(CFLAGS) -o sndreverse sndreverse.o $(SYSOBJS) $(SNDLIB) -lm

head::
	@echo ""; echo "making head ..."
	@cd head; $(MAKE) $(MFLAGS) all
	@echo "done."; echo ""

resample::
	@echo "making resample ..."
	@cd resample; $(MAKE) $(MFLAGS) all
	@echo "done."; echo ""

install: all
	@echo "making install..."	
ifneq ($(strip $(PROGS)),)       # do only if PROGS is nonempty
	@for PROG in $(PROGS); \
	do \
	  ( $(INSTALL) $(CMIXDIR)/utils/$$PROG $(DESTDIR); ) \
	done
endif
	@cd $(DESTDIR); if test -f $(PLAY); then ln -fs $(PLAY) play; fi
	@cd head; $(MAKE) $(MFLAGS) install
	@cd resample; $(MAKE) $(MFLAGS) install

install_scripts::
	@echo "installing scripts..."	
	@cd scripts; $(INSTALL) *[1,2]* $(DESTDIR)

uninstall:
	@echo "making uninstall..."	
ifneq ($(strip $(PROGS)),)       # do only if PROGS is nonempty
	@for PROG in $(PROGS); \
	do \
	  ( $(RM) $(DESTDIR)/$$PROG; ) \
	done
endif
	$(RM) $(DESTDIR)/play
	@cd head; $(MAKE) $(MFLAGS) uninstall
	@cd resample; $(MAKE) $(MFLAGS) uninstall

clean:
	$(RM) *.o core $(PROGS)
	@cd head; $(MAKE) $(MFLAGS) clean
	@cd resample; $(MAKE) $(MFLAGS) clean

cleanall: clean uninstall

