include ../makefile.conf

GENLIB = genlib.a
CFILES =  oscil.c oscili.c osciln.c oscilni.c table.c tablei.c evp.c reson.c \
	  rsnset.c evset.c tableset.c cpspch.c cpsoct.c octpch.c buzz.c\
	  rrand.c boost.c allpole.c delset.c delput.c delget.c dliget.c\
	  reverb.c rvbset.c comb.c combset.c allpass.c \
	  pchoct.c octcps.c pchcps.c boscili.c bbuzz.c ballpole.c brrand.c\
	  hpluck.c hcomb.c setline.c random.c ampdb.c\
	  getsample.c breson.c resonz.c pluck.c hplset.c pluckset.c randf.c spray.c wshape.c bpluck.c\
	  pchmidi.c midipch.c

OBJECTS = oscil.o oscili.o osciln.o oscilni.o table.o tablei.o evp.o reson.o \
	  rsnset.o evset.o tableset.o cpspch.o cpsoct.o octpch.o buzz.o\
	  rrand.o boost.o allpole.o delset.o delput.o delget.o dliget.o\
	  reverb.o rvbset.o comb.o combset.o allpass.o\
	  pchoct.o octcps.o pchcps.o boscili.o bbuzz.o ballpole.o brrand.o\
	  hpluck.o hcomb.o setline.o random.o ampdb.o\
	  getsample.o breson.o resonz.o pluck.o hplset.o pluckset.o randf.o spray.o wshape.o bpluck.o\
	  pchmidi.o midipch.o

IFILES =  ../H/ugens.h

RT_SOCKET_STUFF = RTsockfuncs.o randfuncs.o 

all : genlib.a $(RT_SOCKET_STUFF)

genlib.a:	$(OBJECTS)
	rm -f genlib.a
	ar rv $(GENLIB) $(OBJECTS)
#	$(RANLIB) $(GENLIB)


# $(CFILES): $(IFILES)

RTsockfuncs.o: RTsockfuncs.c
	$(CC) $(CFLAGS) -c RTsockfuncs.c

randfuncs.o: randfuncs.c
	$(CC) $(CFLAGS) -c randfuncs.c

clean:
	rm -f *.o *~ *\# $(OBJECTS) $(GENLIB) $(RT_SOCKET_STUFF)
