PROGS = patest embedtest scoreplayer scoretrigger

CFLAGS = -g -Wall
LDFLAGS = -L/usr/local/lib -lportaudio -L../../lib -lrtcmix_embedded
INCLUDES = -I/usr/local/include -I../../src/rtcmix

all: $(PROGS)

# NB: doesn't link to rtcmix, but we leave LDFLAGS alone here
patest: patest.c
	$(CC) $(CFLAGS) -o patest patest.c $(INCLUDES) $(LDFLAGS)

embedtest: embedtest.c
	$(CC) $(CFLAGS) -o embedtest embedtest.c $(INCLUDES) $(LDFLAGS)

scoreplayer: scoreplayer.c
	$(CC) $(CFLAGS) -o scoreplayer scoreplayer.c $(INCLUDES) $(LDFLAGS)

scoretrigger: scoretrigger.c
	$(CC) $(CFLAGS) -o scoretrigger scoretrigger.c $(INCLUDES) -lcurses $(LDFLAGS)

uninstall:

clean:
	$(RM) -r *.o $(PROGS) *.dSYM
