include ../../makefile.conf

SNDLIB = ../../sys/sndlibsupport.o ../../sndlib/sndlib.a

#CFLAGS += -Wall -O2

PROG = resample
OBJS = resample.o resamplesubs.o
CFILES = resample.c resamplesubs.c filterkit.c
HFILES = resample.h filterkit.h stdefs.h
FILTERS = smallfilter.h largefilter.h

all: $(PROG)

resample: $(HFILES) $(FILTERS) $(OBJS) libfilterkit.a $(SNDLIB)
	$(CC) -o resample $(OBJS) libfilterkit.a $(SNDLIB) -lm

libfilterkit.a: $(HFILES) filterkit.o
	ar rc libfilterkit.a filterkit.o
	ranlib libfilterkit.a

clean:
	$(RM) *.o libfilterkit.a $(PROG)

