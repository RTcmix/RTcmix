include ../../makefile.conf

# HAVE_PORTAUDIO = TRUE	# uncomment if you have it

PROGS = scoreplayer
ifdef HAVE_PORTAUDIO
PROGS += embedtest patest scoretrigger
endif

#CFLAGS = -g -Wall -target x86_64-apple-macos11.6
INCLUDES = -I../../src/rtcmix -I/opt/local/include
ifdef HAVE_PORTAUDIO
INCLUDES += -I/usr/local/include
LDFLAGS += -L/usr/local/lib -lportaudio
endif

all: $(PROGS)

# NB: doesn't link to rtcmix, but we leave LDFLAGS alone for patest
patest: patest.c
	$(CC) $(CFLAGS) -o patest patest.c $(INCLUDES) $(LDFLAGS)

embedtest: embedtest.c
	$(CC) $(CFLAGS) -o embedtest embedtest.c $(INCLUDES) $(LDFLAGS)

embedtest2: embedtest2.c
	$(CC) $(CFLAGS) -o embedtest2 embedtest2.c $(INCLUDES) $(LDFLAGS)

scoreplayer: scoreplayer.c
ifdef HAVE_PORTAUDIO
	$(CC) $(CFLAGS) -o scoreplayer scoreplayer.c $(INCLUDES) $(LDFLAGS) -DHAVE_PORTAUDIO
else
	$(CC) $(CFLAGS) -o scoreplayer scoreplayer.c $(INCLUDES) $(LDFLAGS) 
endif

scoretrigger: scoretrigger.c
	$(CC) $(CFLAGS) -o scoretrigger scoretrigger.c $(INCLUDES) -lcurses $(LDFLAGS)

uninstall:

clean:
	$(RM) -r *.o $(PROGS) *.dSYM
