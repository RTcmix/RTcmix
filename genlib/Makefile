include ../makefile.conf

INCLUDES += -I../src/include -I../src/rtcmix

LIB = libgen.$(SHLIB_SUFFIX)

CFILES = \
allpass.c \
allpole.c \
ampdb.c \
ballpole.c \
bbuzz.c \
boost.c \
boscili.c \
bpluck.c \
breson.c \
brrand.c \
buzz.c \
comb.c \
combset.c \
delget.c \
delput.c \
delset.c \
dliget.c \
evp.c \
evset.c \
hcomb.c \
hplset.c \
hpluck.c \
oscil.c \
oscili.c \
osciln.c \
oscilni.c \
pitchconv.c \
pluck.c \
pluckset.c \
randf.c \
random.c \
reson.c \
resonz.c \
reverb.c \
rrand.c \
rsnset.c \
rvbset.c \
setline.c \
spray.c \
table.c \
tablei.c \
tableset.c \
wshape.c

OBJECTS = \
allpass.o \
allpole.o \
ampdb.o \
ballpole.o \
bbuzz.o \
boost.o \
boscili.o \
bpluck.o \
breson.o \
brrand.o \
buzz.o \
comb.o \
combset.o \
delget.o \
delput.o \
delset.o \
dliget.o \
hcomb.o \
hplset.o \
hpluck.o \
evp.o \
evset.o \
oscil.o \
oscili.o \
osciln.o \
oscilni.o \
pitchconv.o \
pluck.o \
pluckset.o \
randf.o \
random.o \
randfuncs.o \
reson.o \
resonz.o \
reverb.o \
rrand.o \
rsnset.o \
rvbset.o \
setline.o \
spray.o \
table.o \
tablei.o \
tableset.o \
wshape.o

OCFILES = \
Oallpass.cpp \
Oallpassi.cpp \
Obalance.cpp \
Obucket.cpp \
Ocomb.cpp \
Ocombi.cpp \
Odcblock.cpp \
Odelay.cpp \
Odelayi.cpp \
Odistort.cpp \
Oequalizer.cpp \
Offt.cpp \
Oonepole.cpp \
Ooscil.cpp \
Ooscili.cpp \
Oreson.cpp \
Orand.cpp \
Orms.cpp \
Ortgetin.cpp \
Ostrum.cpp

OOBJECTS = \
Oallpass.o \
Oallpassi.o \
Obalance.o \
Obucket.o \
Ocomb.o \
Ocombi.o \
Odcblock.o \
Odelay.o \
Odelayi.o \
Odistort.o \
Oequalizer.o \
Offt.o \
Oonepole.o \
Ooscil.o \
Ooscili.o \
Orand.o \
Oreson.o \
Orms.o \
Ortgetin.o \
Ostrum.o

ifeq ($(FFTW_SUPPORT), TRUE)
	CXXFLAGS += $(FFTW_CFLAGS)
else
	OCFILES += FFTReal.cpp
	OOBJECTS += FFTReal.o
endif

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
	-$(SHELL) -ec '$(CXX) -M $(CXXFLAGS) $(OCFILES)' >> depend
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


