include ../../../makefile.conf

OBJS = RTcmixMIDI.o RTMidiPField.o
SRCS = RTcmixMIDI.cpp RTMidiPField.cpp glue.cpp

PTLIB = portmidi/porttime/libporttime.a

ifeq ($(ARCH), MACOSX)
	PMLIB = portmidi/pm_mac/libportmidi.a
	MIDISYSLIB = -framework CoreFoundation -framework CoreAudio -framework CoreMIDI
endif
ifeq ($(ARCH), LINUX)
	PMLIB = portmidi/pm_linux/libportmidi.a
#	MIDISYSLIB = -lasound
endif

CXXFLAGS += -I. -I../../rtcmix -Iportmidi/pm_common -Iportmidi/porttime

LIBMIDI = libmidicon.a

all: $(LIBMIDI) glue.o

# FIXME: make this a dso
$(LIBMIDI): depend $(OBJS)
	$(LD) -r $(OBJS) -o $(LIBMIDI) $(PTLIB) $(PMLIB) $(MIDISYSLIB)

glue.o: depend glue.cpp RTMidiPField.h
	$(CXX) $(CXXFLAGS) -c -o glue.o glue.cpp

depend:
	-$(SHELL) -ec '$(CXX) -M $(CXXFLAGS) $(SRCS)' > depend

-include depend

install: all

uninstall:

clean:
	$(RM) *.o $(LIBMIDI) depend

cleanall: clean

