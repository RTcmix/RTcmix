include ../makefile.conf

INCLUDES += -I../src/include -I../src/rtcmix

LIB = libgen.$(SHLIB_SUFFIX)
CFILES =  oscil.c oscili.c osciln.c oscilni.c table.c tablei.c evp.c reson.c \
	  rsnset.c evset.c tableset.c cpspch.c cpsoct.c octpch.c buzz.c\
	  rrand.c boost.c allpole.c delset.c delput.c delget.c dliget.c\
	  reverb.c rvbset.c comb.c combset.c allpass.c \
	  pchoct.c octcps.c pchcps.c boscili.c bbuzz.c ballpole.c brrand.c\
	  hpluck.c hcomb.c setline.c random.c ampdb.c\
	  breson.c resonz.c pluck.c hplset.c pluckset.c randf.c spray.c wshape.c bpluck.c\
	  pchmidi.c midipch.c
OCFILES = Ooscili.cpp Orand.cpp Odelayi.cpp Ocomb.cpp Ocombi.cpp Oequalizer.cpp Oonepole.cpp

OBJECTS = oscil.o oscili.o osciln.o oscilni.o table.o tablei.o evp.o reson.o \
	  rsnset.o evset.o tableset.o cpspch.o cpsoct.o octpch.o buzz.o\
	  rrand.o boost.o allpole.o delset.o delput.o delget.o dliget.o\
	  reverb.o rvbset.o comb.o combset.o allpass.o\
	  pchoct.o octcps.o pchcps.o boscili.o bbuzz.o ballpole.o brrand.o\
	  hpluck.o hcomb.o setline.o random.o ampdb.o\
	  breson.o resonz.o pluck.o hplset.o pluckset.o randf.o spray.o wshape.o bpluck.o\
	  pchmidi.o midipch.o
OOBJECTS = Ooscili.o Orand.o Odelayi.o Ocomb.o Ocombi.o Oequalizer.o Oonepole.o

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

.cpp.o:
	$(CXX) -c $(CXXFLAGS) -o $@ $<
	
all : $(LIB)
	@if test ! -d $(LIBDIR); then mkdir $(LIBDIR); fi;
	$(INSTALL) $(LIB) $(LIBDIR)

install:	all

uninstall:
	$(RM) $(LIBDIR)/$(LIB)

$(LIB):	$(OBJECTS) $(OOBJECTS)
	$(RM) $@
	$(CXX) $(DYNAMIC_LDFLAGS) -o $@ $(OBJECTS) $(OOBJECTS)

depend:
ifeq ($(ARCH),LINUX)
	-$(SHELL) -ec '$(CC) -M $(CFLAGS) $(CFILES)' > depend
endif
ifeq ($(ARCH),SGI)
# Note: -MM is a gcc extension. If no gcc, use -M.
# Unfortunately, this doesn't work...   -JGG
#	$(SHELL) -ec '$(CC) -M $(CFLAGS) $(SRCS) $(CPPSRCS)' > depend
endif

-include depend

clean:
	$(RM) *.o *~ $(LIB)

cleanall: clean
	$(RM) depend
	$(RM) $(LIBDIR)/$(LIB)


