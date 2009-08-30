include ../../makefile.conf

SNDLIB += ../../src/audio/sndlibsupport.o
INCLUDES += -I$(AUDIODIR) -I$(SRCDIR)/sndlib -I$(SRCDIR)/rtcmix

PROG = resample
OBJS = resample.o resamplesubs.o
CFILES = resample.c resamplesubs.c filterkit.c
HFILES = resample.h filterkit.h stdefs.h
FILTERS = smallfilter.h largefilter.h

all: $(PROG)

$(PROG): $(HFILES) $(FILTERS) $(OBJS) libfilterkit.a $(SNDLIB)
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS) libfilterkit.a $(SNDLIB) -lm

libfilterkit.a: $(HFILES) filterkit.o
	ar rc libfilterkit.a filterkit.o
	ranlib libfilterkit.a

install:
	@echo "making install..."	
	@$(INSTALL) $(CMIXDIR)/utils/resample/$(PROG) $(DESTDIR)

uninstall:
	@echo "making uninstall..."	
	$(RM) $(DESTDIR)/resample

clean:
	$(RM) *.o libfilterkit.a $(PROG)

