include ../makefile.conf

PLAY = cmixplay
NPLAY = nplay

PLAY_HDRS = ../H/sndlibsupport.h ../sys/AudioDevice.h
AUDIO_DEV_OBJS = ../sys/AudioDevice.o ../sys/AudioDeviceImpl.o \
   ../sys/AudioIODevice.o ../sys/ThreadedAudioDevice.o \
   ../sys/audio_dev_creator.o ../sys/NetAudioDevice.o \
   ../sys/check_byte_order.o ../sys/sndlibsupport.o

ifeq ($(ARCH),LINUX)
   PROGS = $(PLAY) playraw full-duplex-test
   RESAMPLE = resample
   AUDIO_DEV_OBJS += ../sys/ALSAAudioDevice.o ../sys/OSSAudioDevice.o
#  SYSLIBS = -lm   # only this, overriding makefile.conf
endif
ifeq ($(ARCH),MACOSX)
   PROGS = $(PLAY) #playraw full-duplex-test
   RESAMPLE = resample
   AUDIO_DEV_OBJS += ../sys/OSXAudioDevice.o
   SYSLIBS = -framework CoreAudio
endif

# nplay won't compile with current RTcmix; it will be replaced soon
#ifeq ($(NPLAY_SUPPORT), TRUE)
#   PROGS += $(NPLAY)
#endif

SYSOBJS = ../sys/check_byte_order.o ../sys/sndlibsupport.o ../sys/audio_port.o


all: $(PROGS) $(RESAMPLE)

$(PLAY):  $(PLAY).o $(AUDIO_DEV_OBJS)
	$(CXX) -o $(PLAY) $(PLAY).o $(SYSLIBS) $(AUDIO_DEV_OBJS) $(SNDLIB)

$(PLAY).o:  $(PLAY).cpp $(PLAY_HDRS)
	$(CXX) $(CXXFLAGS) $(SOUND_DRIVER) -c $< -o $@

$(NPLAY):  $(NPLAY).o $(SYSOBJS)
	$(CC) -o $(NPLAY) $(SOUND_DRIVER) $(NPLAY).o $(SYSOBJS) $(SNDLIB) $(SYSLIBS)

$(NPLAY).o:  $(NPLAY).c ../H/audio_port.h ../H/sndlibsupport.h
	$(CC) $(CFLAGS) $(SOUND_DRIVER) -c $< -o $@

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

