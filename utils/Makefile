include ../makefile.conf

PLAY = cmixplay
NPLAY = nplay
SETUP = setup_rtcmixrc

PLAY_HDRS = ../src/audio/sndlibsupport.h ../src/audio/AudioDevice.h
OPTION_O = ../src/rtcmix/Option.o
LIBCONF = ../src/conf/libconf.a
LIBAUDIO = ../src/audio/librtaudio.a
INCLUDES += -I../src/audio -I../src/sndlib -I../src/rtcmix

ifeq ($(ARCH),LINUX)
   PROGS = $(PLAY) $(SETUP) # full-duplex-test playraw
   RESAMPLE = resample
#  SYSLIBS = -lm   # only this, overriding makefile.conf
endif
ifeq ($(ARCH),MACOSX)
   PROGS = $(PLAY) $(SETUP) #playraw full-duplex-test
   RESAMPLE = resample
   SYSLIBS = -framework CoreAudio
endif

# nplay won't compile with current RTcmix; it will be replaced soon
#ifeq ($(NPLAY_SUPPORT), TRUE)
#   PROGS += $(NPLAY)
#endif

SYSOBJS = ../src/rtcmix/check_byte_order.o

all: $(PROGS) $(RESAMPLE)

$(PLAY):  $(PLAY).o $(AUDIO_DEV_OBJS)
	$(CXX) -o $(PLAY) $(PLAY).o $(SYSLIBS) $(LIBAUDIO) $(SNDLIB)

$(PLAY).o:  $(PLAY).cpp $(PLAY_HDRS)
	$(CXX) $(CXXFLAGS) $(SOUND_DRIVER) -c $< -o $@

$(NPLAY):  $(NPLAY).o $(SYSOBJS)
	$(CC) -o $(NPLAY) $(SOUND_DRIVER) $(NPLAY).o $(SYSOBJS) $(SNDLIB) $(SYSLIBS)

$(NPLAY).o:  $(NPLAY).c ../H/audio_port.h ../H/sndlibsupport.h
	$(CC) $(CFLAGS) $(SOUND_DRIVER) -c $< -o $@

$(SETUP):  $(SETUP).o $(OPTION_O) $(LIBCONF)
	$(CXX) -o $(SETUP) $(SETUP).o $(SYSLIBS) $(OPTION_O) $(LIBCONF)

#$(SETUP).o:  $(SETUP).cpp $(PLAY_HDRS)
#	echo $(LIBDESTDIR)
#	$(CXX) $(CXXFLAGS) -DSHAREDLIBDIR=\"$(LIBDESTDIR)\" -c $< -o $@

playraw: playraw.o
	$(CC) -o playraw playraw.o

full-duplex-test: full-duplex-test.o
	$(CC) -o full-duplex-test full-duplex-test.o -lm

sfrecord:  sfrecord.o $(SYSOBJS)
	$(CC) -o sfrecord sfrecord.o $(SYSOBJS) $(SNDLIB) $(SYSLIBS)

$(RESAMPLE)::
	@echo "making resample ..."
	@cd $(RESAMPLE); $(MAKE) all
	@echo "done."; echo ""

install:
	@echo "making install..."	
ifneq ($(strip $(PROGS)),)       # do only if PROGS is nonempty
	@for PROG in $(PROGS); \
	do \
	  ( $(INSTALL) $(CMIXDIR)/utils/$$PROG $(DESTDIR); ) \
	done
endif
ifneq ($(strip $(RESAMPLE)),)    # do only if RESAMPLE is nonempty
	@$(INSTALL) $(CMIXDIR)/utils/resample/resample $(DESTDIR)
endif
ifneq ($(strip $(PLAY)),)        # do only if PLAY is nonempty
	@cd $(DESTDIR); ln -fs $(PLAY) play
endif

uninstall:
	@echo "making uninstall..."	
ifneq ($(strip $(PROGS)),)       # do only if PROGS is nonempty
	@for PROG in $(PROGS); \
	do \
	  ( $(RM) $(DESTDIR)/$$PROG; ) \
	done
endif
	$(RM) $(DESTDIR)/play
	$(RM) $(DESTDIR)/resample

clean:
	$(RM) *.o core $(PROGS)
ifneq ($(strip $(RESAMPLE)),)    # do only if RESAMPLE is nonempty
	@cd $(RESAMPLE); $(MAKE) clean
endif

cleanall: clean uninstall

