include ../package.conf

# the new improved R. Luke DuBois lpc makefile.
# good luck!
# DT:  amen

LDFLAGS = -lm -ldl -lpthread
INCLUDE = -I../../H -I.
CFLAGS = $(ARCHFLAGS) -DUSE_HEADERS
PROGS = lpc
CMIX_SYS = ../../sys
CMIX_LIB = ../../lib
CMIX_RT = ../../rtstuff
LFLAGS =  $(CMIX_SYS)/cmix.o $(CMIX_LIB)/genlib.a $(CMIX_RT)/rtprofile.o $(LDFLAGS)
UTILS = merge lpcplot lpcscan restoreplot pchplot restorelpc

CC = gcc
CPP = g++

all:	lpc stabilize lpcplay newlpcplay ptrack $(UTILS)

LPC_O = lpc.o lpc.anallpc.o $(CMIX_SYS)/wheader.o $(CMIX_SYS)/printsf.o $(CMIX_SYS)/sfcodes.o alpole.o $(CMIX_SYS)/check_byte_order.o

factor.o: factor.c
	$(CC) -O -c factor.c

lpc.roottest.o: lpc.roottest.c
	$(CC) -O -c lpc.roottest.c

lpc.stable.o: lpc.stable.c
	$(CC) -O -c lpc.stable.c

lpc: 	$(LPC_O) 
	$(CC) -o lpc $(LPC_O) $(LDFLAGS)


STAB_O = newroottest.o newroottestmain.o newstable.o factor.o lpcheader.o fortran.o

stabilize:  $(STAB_O)
	$(CC) -o stabilize $(STAB_O) $(LDFLAGS)

lpcheader.o: lpcheader.h lpcheader.c
	$(CC) $(CFLAGS) -c lpcheader.c


LPCPLAY_O = allpole.o readjust.o mp.o shift.o lpcplay.o buzz.o inters.o bmultf.o lpcheader.o

NEWLPCPLAY_O = allpole.o readjust.o mp.o shift.o newlpcplay.o buzz.o inters.o bmultf.o lpcheader.o

lpcplay: ../../H/ugens.h ../../sys/cmix.o ../../rtstuff/rtprofile.o $(LPCPLAY_O)
	$(CPP) -o lpcplay $(LPCPLAY_O) $(LFLAGS)

newlpcplay: ../../H/ugens.h ../../sys/cmix.o ../../rtstuff/rtprofile.o $(NEWLPCPLAY_O)
	$(CPP) -o newlpcplay $(NEWLPCPLAY_O) $(LFLAGS)


TRACK_O = crack.o die.o  ptrack.o ptrackfuns.o sfname.o ../../sys/wheader.o ../../sys/sfcodes.o ../../sys/check_byte_order.o

ptrack:  $(TRACK_O)
	$(CC) -o ptrack $(TRACK_O) $(LDFLAGS)


merge:	merge.o lpcheader.o crack.o
	$(CC) $(CFLAGS) -o merge merge.c lpcheader.o crack.o

lpcplot: lpcplot.c lpcheader.c
	$(CC) $(CFLAGS) -o lpcplot lpcplot.c lpcheader.o

lpcscan: lpcscan.o
	$(CPP) $(CFLAGS) -o lpcscan lpcscan.o lpcheader.o $(LFLAGS)

pchplot:	pchplot.c
	$(CC) -o pchplot pchplot.c

restoreplot:	restoreplot.c
	$(CC) -o restoreplot restoreplot.c

restorelpc:	restorelpc.c
	$(CC) $(CFLAGS) -o restorelpc restorelpc.c lpcheader.o


clean:
	rm -f *.o lpc stabilize lpcplay newlpcplay ptrack $(UTILS)





