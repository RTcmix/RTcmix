# Makefile for ALSA probe tool
#

PROG = alsaprobe

CFLAGS = -g
LDFLAGS = -lasound

all: $(PROG)

$(PROG): $(PROG).c
	$(CC) $(CFLAGS) -o $(PROG) $(PROG).c $(LDFLAGS)

clean:
	$(RM) *.o $(PROG)
