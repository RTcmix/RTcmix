include ../../../makefile.conf

OBJS = RTcmixMIDI.o RTMidiPField.o glue.o
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

LIBMIDI = libmidiconn.$(SHLIB_SUFFIX)

all: $(LIBMIDI)

$(LIBMIDI): depend $(OBJS)
	$(RM) $@
	$(CXX) $(DYNAMIC_LDFLAGS) -o $@ $(OBJS) $(PMLIB) $(PTLIB) $(MIDISYSLIB)

glue.o: depend glue.cpp RTMidiPField.h
	$(CXX) $(CXXFLAGS) -c -o glue.o glue.cpp

depend:
	-$(SHELL) -ec '$(CXX) -M $(CXXFLAGS) $(SRCS)' > depend

-include depend

install: all
	$(INSTALL) $(LIBMIDI) $(LIBDESTDIR)

uninstall:
	$(RM) $(LIBDESTDIR)/$(LIBMIDI)

clean:
	$(RM) *.o $(LIBMIDI) depend

cleanall: clean

