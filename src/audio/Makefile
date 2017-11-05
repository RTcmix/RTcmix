include ../../makefile.conf

AUDIOLIB = librtaudio.a
AUDIO_DEFINES = -D$(AUDIODRIVER)

# where we are versus the dylib or lib build directory
RELATIVEDIR = ../audio

INCLUDES += -I../sndlib -I../rtcmix -I$(INCLUDEDIR)
CFLAGS += -DCMIX_LEGACY

OBJECTS =  AudioDevice.o AudioIODevice.o AudioDeviceImpl.o \
		   ThreadedAudioDevice.o AudioOutputGroupDevice.o \
		   DualOutputAudioDevice.o AudioFileDevice.o audio_devices.o \
		   audio_dev_creator.o sndlibsupport.o

ifeq ($(ARCH),LINUX)
   ifeq ($(AUDIODRIVER), EMBEDDEDAUDIO)
   		OBJECTS += EmbeddedAudioDevice.o
   else ifeq ($(AUDIODRIVER), ALSA))
		OBJECTS +=  OSSAudioDevice.o SinglePortOSSAudioDevice.o \
			MultiPortOSSAudioDevice.o ALSAAudioDevice.o TestAudioDevice.o
   else ifeq ($(AUDIODRIVER), OSS))
		OBJECTS +=  OSSAudioDevice.o SinglePortOSSAudioDevice.o \
			MultiPortOSSAudioDevice.o
   endif
# Add this to enable the TestAudioDevice class
#		CXXFLAGS += -DTEST_AUDIO_DEVICE
#		OBJECTS += TestAudioDevice.o
endif

ifeq ($(ARCH),MACOSX)
   ifeq ($(AUDIODRIVER), APPLE)
		OBJECTS += AppleAudioDevice.o
   else ifeq ($(AUDIODRIVER), EMBEDDEDAUDIO)
   		OBJECTS += EmbeddedAudioDevice.o
   endif
endif

ifeq ($(ARCH),SGI)
	OBJECTS += SGIAudioDevice.o
endif

ifeq ($(JACK_SUPPORT), TRUE)
	OBJECTS += JackAudioDevice.o
	CXXFLAGS += $(JACK_CFLAGS)
endif

ifeq ($(NPLAY_SUPPORT), TRUE)
	OBJECTS += NetAudioDevice.o
endif

SNDLIB = ../sndlib/sndlib.a

all:	$(AUDIOLIB)

$(AUDIOLIB):	$(OBJECTS)
	$(RM) $@
	ar q $@ $(OBJECTS)
	$(RANLIB) $@

.c.o:
	$(CC) $(CFLAGS) $(AUDIO_DEFINES) -c $< -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) $(AUDIO_DEFINES) -c $< -o $@

depend:
	$(SHELL) -ec '$(CC) -MM $(CFLAGS) $(AUDIO_DEFINES) *.c *.cpp' > depend

-include depend

install: all
uninstall:

clean:
	$(RM) $(OBJECTS) $(AUDIOLIB)

cleanall: clean
	$(RM) depend

test:
	@echo ARCH = ${ARCH}
	@echo BUILDTYPE = ${BUILDTYPE}
	@echo OBJECTS = ${OBJECTS}

printobjs:
	@for OBJ in $(OBJECTS); \
	do \
  		( echo $(RELATIVEDIR)/$$OBJ ); \
	done

