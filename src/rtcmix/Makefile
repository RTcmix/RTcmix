include ../../makefile.conf

CXXFLAGS += $(SOUND_DRIVER)
INCLUDES += -I$(INCLUDEDIR) -I../include -I../audio -I../sndlib -I./heap -I../conf

COMMON_CSRCS = \
buffers.c \
check_byte_order.c \
command_line.c \
funderflow.c \
infile.c \
io.c \
loader.c \
merror.c \
message.c \
minc_functions.c \
minc_info.c \
minout.c \
m_system.c \
printsf.c \
profile.c \
resetamp.c \
rtinput.c \
rtoutput.c \
rtsetparams.c \
sampfunc.c \
sfcopy.c \
sfprint.c \
sfstats.c \
sound.c \
soundio.c \
tempo.c \
ug_intro.c \
gen/fdump.c \
gen/floc.c \
gen/fnscl.c \
gen/fplot.c \
gen/fsize.c \
gen/gen10.c \
gen/gen17.c \
gen/gen18.c \
gen/gen1.c \
gen/gen20.c \
gen/gen24.c \
gen/gen25.c \
gen/gen2.c \
gen/gen3.c \
gen/gen4.c \
gen/gen5.c \
gen/gen6.c \
gen/gen7.c \
gen/gen9.c \
gen/makegen.c \
gen/modgens.c


COMMON_CPPSRCS = \
addcheckfunc.cpp \
addrtInst.cpp \
bus_config.cpp \
checkInsts.cpp \
dispatch.cpp \
Instrument.cpp \
intraverse.cpp \
Option.cpp \
PField.cpp \
PFieldSet.cpp \
Random.cpp \
RefCounted.cpp \
rtcmix_types.cpp \
rtgetin.cpp \
rtgetsamps.cpp \
rtprofile.cpp \
rtsendsamps.cpp \
rtsetinput.cpp \
rtsetoutput.cpp \
rtwritesamps.cpp \
setnetplay.cpp \
set_option.cpp \
sockit.cpp \
table.cpp

IMBEDSRCS = RTcmix.cpp

CPPOBJS = $(patsubst %.cpp,%.o,$(COMMON_CPPSRCS))

COBJS = $(patsubst %.c,%.o,$(COMMON_CSRCS))

COMMON_OBJS = $(CPPOBJS) $(COBJS) heap/rtHeap.o

PIMBED_OBJS = $(COMMON_OBJS) pRTcmix.o

ifeq ($(NPLAY_SUPPORT), TRUE)
   COMMON_OBJS += setnetplay.o
   CPPSRCS += setnetplay.cpp
endif

ifeq ($(ARCH), MACOSX_10.0)
   COMMON_OBJS += osx_workaround.o
endif

RTLIB = librtcmix.a

LIBCONF = ../conf/libconf.a
LIBMINC = ../parser/minc/libminc.a
AUDIOLIB = ../audio/librtaudio.a
SNDLIB = ../sndlib/sndlib.a

LIBS = $(RTLIB) $(LIBMINC) $(AUDIOLIB) $(SNDLIB) $(LIBCONF) $(LIBDIR)/libgen.a

PROGS = MIX
TARGETS = $(RTLIB) RTcmix.o main.o $(PROGS)

ifeq ($(PERL_SUPPORT), TRUE)
IMBEDSRCS += pRTcmix.cpp
TARGETS += pRTcmix.o
PROGS += PMIX
endif
ifeq ($(PYTHON_SUPPORT), TRUE)
ARCHFLAGS += -DPYTHON
PROGS += PYMIX
PYTHON_INCLUDE_DIR = `$(PYTHON) ../parser/python/print_python_includedir.py`
LIBPYTHON = `$(PYTHON) ../parser/python/print_libpython.py`
ifeq ($(ARCH), LINUX)
	# This is necessary for Python 2.0 in Slackware Linux 8.0, and probably
	# other distributions; comment out if it gives you trouble with earlier
	# Pythons.  -JGG
	LIBPYTHON += -lutil
endif
endif

PERL_LDFLAGS = `$(PERL) -MExtUtils::Embed -e ldopts`

all: $(TARGETS)

# rules to allow complete lib compile from this dir only

$(LIBCONF):
	(cd ../conf; $(MAKE) $(MFLAGS) all;)

$(LIBMINC):
	(cd ../parser/minc; $(MAKE) $(MFLAGS) all;)

$(SNDLIB):
	(cd ../sndlib; $(MAKE) $(MFLAGS) all;)

$(AUDIOLIB):
	(cd ../audio; $(MAKE) $(MFLAGS) all;)

# rules for librtcmix.a

$(RTLIB): depend $(COMMON_OBJS) $(LIBMINC) $(LIBCONF)
	ar q $@ $(COMMON_OBJS) 

Option.o: Option.cpp Option.h
	$(CXX) $(CXXFLAGS) -DSHAREDLIBDIR=\"$(LIBDESTDIR)\" -c $< -o $@

heap/rtHeap.o:
	(cd heap; $(MAKE) $(MFLAGS) all;)

# rules for three executables we build

MIX: $(CMIX_O) $(M_O) $(LIBS) $(PROFILE_O) $(WORKAROUND)
	$(CXX) -o MIX $(WORKAROUND) $(DYN) $(CMIX_O) $(PROFILE_O) \
		$(M_O) $(LIBS) $(LDFLAGS)

PMIX: $(CMIX_O) $(P_O) $(LIBS) $(PROFILE_O) $(WORKAROUND)
	$(CXX) -o PMIX $(WORKAROUND) $(DYN) $(CMIX_O) $(PROFILE_O) \
		$(P_O) $(LIBS) $(LDFLAGS) $(PERL_LDFLAGS)

PYMIX: $(CMIX_O) $(PY_O) $(LIBS) $(PROFILE_O) $(WORKAROUND)
	$(CXX) -o PYMIX $(WORKAROUND) $(DYN) $(CMIX_O) $(PROFILE_O) \
		$(PY_O) $(LIBS) $(LDFLAGS) $(LIBPYTHON)

.PHONY: all standalone install dso_install standalone_install \
		uninstall dso_uninstall standalone_uninstall clean cleanall

depend:
ifeq ($(ARCH),LINUX)
	-$(SHELL) -ec '$(CC) -M $(CFLAGS) $(COMMON_CSRCS) $(COMMON_CPPSRCS) $(IMBEDSRCS) `$(PERL) -MExtUtils::Embed -e ccopts`' > depend
endif
ifeq ($(ARCH),SGI)
# Note: -MM is a gcc extension. If no gcc, use -M.
# Unfortunately, this doesn't work...   -JGG
#	$(SHELL) -ec '$(CC) -M $(CFLAGS) $(SRCS) $(CPPSRCS)' > depend
endif

-include depend

install:	all
	@if test ! -d $(LIBDIR); then mkdir $(LIBDIR); fi;
	$(INSTALL) $(RTLIB) $(LIBDIR)
	$(INSTALL) main.o $(LIBDIR)
	$(INSTALL) RTcmix.o $(LIBDIR)
	@if test ! -d $(DESTDIR); then mkdir $(DESTDIR); fi;
	$(INSTALL) MIX $(DESTDIR)/CMIX
ifeq ($(PERL_SUPPORT), TRUE)
	$(INSTALL) pRTcmix.o $(LIBDIR)
	$(INSTALL) PMIX $(DESTDIR)/PCMIX
endif
ifeq ($(PYTHON_SUPPORT), TRUE)
	$(INSTALL) PYMIX $(DESTDIR)/PYCMIX
endif

standalone_install: all
	$(INSTALL) MIX $(DESTDIR)/CMIX
ifeq ($(PERL_SUPPORT), TRUE)
	$(INSTALL) PMIX $(DESTDIR)/PCMIX
endif
ifeq ($(PYTHON_SUPPORT), TRUE)
	$(INSTALL) PYMIX $(DESTDIR)/PYCMIX
endif

standalone_uninstall:
	$(RM) $(DESTDIR)/CMIX
	$(RM) $(DESTDIR)/PCMIX
	$(RM) $(DESTDIR)/PYCMIX

clean:
	(cd heap; $(MAKE) $(MFLAGS) clean;)
	$(RM) *.o $(TARGETS) core depend

cleanall: clean

