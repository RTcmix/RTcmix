include ../../makefile.conf

AUDIOLIB = librtaudio.a

INCLUDES += -I../sndlib -I../rtcmix -I$(INCLUDEDIR)
CFLAGS += -DCMIX_LEGACY

OBJECTS =  AudioDevice.o AudioIODevice.o AudioDeviceImpl.o \
		   ThreadedAudioDevice.o AudioFileDevice.o NetAudioDevice.o \
		   audio_dev_creator.o audio_devices.o sndlibsupport.o

ifeq ($(ARCH),LINUX)
OBJECTS +=  OSSAudioDevice.o ALSAAudioDevice.o 
endif
ifeq ($(ARCH),MACOSX)
OBJECTS += OSXAudioDevice.o
endif
ifeq ($(ARCH),SGI)
OBJECTS += SGIAudioDevice.o
endif

SNDLIB = ../sndlib/sndlib.a

all:	$(AUDIOLIB)

$(AUDIOLIB):	$(OBJECTS)
	$(RM) $@
	ar q $@ $(OBJECTS)
ifeq ($(ARCH),MACOSX)
	$(RANLIB) $@
endif

.c.o:
	$(CC) $(CFLAGS) $(SOUND_DRIVER) -c $< -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) $(SOUND_DRIVER) -c $< -o $@

depend:
ifeq ($(ARCH),LINUX)
	$(SHELL) -ec '$(CC) -MM $(CFLAGS) $(SOUND_DRIVER) *.c *.cpp' > depend
endif
ifeq ($(ARCH),SGI)
# Note: -MM is a gcc extension. If no gcc, use -M.
# Unfortunately, this doesn't work...   -JGG
#	$(SHELL) -ec '$(CC) -M $(CFLAGS) $(SRCS) $(CPPSRCS)' > depend
endif

-include depend

install:
uninstall:

clean:
	$(RM) $(OBJECTS) $(AUDIOLIB)

cleanall: clean
	$(RM) depend

