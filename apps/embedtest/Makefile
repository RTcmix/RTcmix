PROGS = patest embedtest

HEADERS = portaudio.h RTcmix_API.h

CFLAGS = -g -Wall
LDFLAGS = -L/usr/local/lib -lportaudio -L../../lib -lrtcmix_embedded
INCLUDES = -I/usr/local/include -I../../include

all: $(PROGS)

patest: patest.c
	$(CC) $(CFLAGS) -o patest patest.c $(INCLUDES) $(LDFLAGS)

embedtest: embedtest.c
	$(CC) $(CFLAGS) -o embedtest embedtest.c $(INCLUDES) $(LDFLAGS)

clean:
	$(RM) -r *.o $(PROGS) *.dSYM

