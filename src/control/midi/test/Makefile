include ../../../../makefile.conf

PTLIB = ../portmidi/porttime/libporttime.a

ifeq ($(ARCH), MACOSX)
	PMLIB = ../portmidi/pm_mac/libportmidi.a
	AUDIOLIB = -framework CoreFoundation -framework CoreAudio -framework CoreMIDI
endif
ifeq ($(ARCH), LINUX)
	PMLIB = ../portmidi/pm_linux/libportmidi.a
	AUDIOLIB = -lasound
endif

CFLAGS += -I../portmidi/pm_common -I../portmidi/porttime

all: monitor

monitor: monitor.o
	$(CC) $(CFLAGS) monitor.o -o monitor $(PMLIB) $(PTLIB) $(AUDIOLIB) -lpthread

clean:
	$(RM) *.o monitor
