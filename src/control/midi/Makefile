include ../../../makefile.conf

OBJS = RTcmixMIDI.o RTMidiPField.o glue.o
SRCS = RTcmixMIDI.cpp RTMidiPField.cpp glue.cpp

EXTRA_PMINCLUDES =
PMLIB =
PTLIB =

ifneq ($(USE_EXTERNAL_PMLIB), TRUE)
	PTLIB = portmidi/porttime/libporttime.a
	EXTRA_PMINCLUDES = -Iportmidi/pm_common -Iportmidi/porttime
endif

ifeq ($(ARCH), MACOSX)
	ifneq ($(USE_EXTERNAL_PMLIB), TRUE)
		PMLIB = portmidi/pm_mac/libportmidi.a
	endif
	MIDISYSLIB = -framework CoreFoundation -framework CoreAudio -framework CoreMIDI
endif
ifeq ($(ARCH), LINUX)
	ifneq ($(USE_EXTERNAL_PMLIB), TRUE)
		PMLIB = portmidi/pm_linux/libportmidi.a
	endif
#	MIDISYSLIB = -lasound
endif

CXXFLAGS += -I. -I../../rtcmix $(EXTRA_PMINCLUDES)

LIBMIDI = libmidiconn.so

all: $(LIBMIDI)

$(LIBMIDI): depend $(OBJS) $(PMLIB) $(PTLIB)
	$(RM) $@
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(OBJS) $(PMLIB) $(PTLIB) $(MIDISYSLIB)

glue.o: depend glue.cpp RTMidiPField.h
	$(CXX) $(CXXFLAGS) -c -o glue.o glue.cpp

$(PMLIB)::
	( cd portmidi; $(MAKE) $(MFLAGS) all; )

depend:
	-$(SHELL) -ec '$(CXX) -M $(CXXFLAGS) $(SRCS)' > depend

-include depend

install: all
	$(INSTALL) $(LIBMIDI) $(LIBDESTDIR)

uninstall:
	$(RM) $(LIBDESTDIR)/$(LIBMIDI)

clean:
	( cd portmidi; $(MAKE) $(MFLAGS) clean; )
	$(RM) *.o $(LIBMIDI)

cleanall: clean
	$(RM) depend

