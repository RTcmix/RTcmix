include ../../makefile.conf

CXXFLAGS += $(SOUND_DRIVER)
INCLUDES += -I$(INCLUDEDIR) -I../include -I../audio -I../sndlib -I./heap -I../conf

COMMON_CSRCS = \
check_byte_order.c \
command_line.c \
funderflow.c \
getsample.c \
infile.c \
io.c \
loader.c \
merror.c \
message.c \
minc_functions.c \
minout.c \
m_system.c \
printsf.c \
profile.c \
resetamp.c \
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
buffers.cpp \
bus_config.cpp \
checkInsts.cpp \
dispatch.cpp \
Instrument.cpp \
intraverse.cpp \
minc_info.cpp \
Option.cpp \
PField.cpp \
PFieldSet.cpp \
Random.cpp \
RefCounted.cpp \
RTcmix.cpp \
rtcmix_types.cpp \
rtcmix_wrappers.cpp \
rtgetin.cpp \
rtgetsamps.cpp \
rtinput.cpp \
rtoutput.cpp \
rtsendsamps.cpp \
rtsetinput.cpp \
rtsetoutput.cpp \
rtsetparams.cpp \
rtwritesamps.cpp \
set_option.cpp \
table.cpp \
rt_ug_intro.cpp \
connection.cpp \
converter.cpp

MAIN_CPPSRCS = RTcmixMain.cpp main.cpp

# This is not linked into librtcmix
#rtprofile.cpp

IMBEDSRCS =

CPPOBJS = $(patsubst %.cpp,%.o,$(COMMON_CPPSRCS))

COBJS = $(patsubst %.c,%.o,$(COMMON_CSRCS))

MIX_INST = ../../insts/base/MIX/MIX.o

COMMON_OBJS = $(CPPOBJS) $(COBJS) heap/rtHeap.o $(MIX_INST) ../audio/audio_devices.o

PIMBED_OBJS = $(COMMON_OBJS) pRTcmix.o


ifeq ($(NPLAY_SUPPORT), TRUE)
   COMMON_OBJS += setnetplay.o
   CPPSRCS += setnetplay.cpp
endif

ifeq ($(ARCH), MACOSX_10.0)
   COMMON_OBJS += osx_workaround.o
endif

LIBCONF = ../conf/libconf.a
LIBMINC = ../parser/minc/libminc.a
AUDIOLIB = ../audio/librtaudio.a
SNDLIB = ../sndlib/sndlib.a

LIBS = $(AUDIOLIB) $(SNDLIB) $(LIBCONF)

PROGS = MIX
MAIN_OBJS = RTcmixMain.o main.o
TARGETS = $(RTLIB) $(MAIN_OBJS) RTsockfuncs.o $(PROGS)

ifeq ($(PERL_SUPPORT), TRUE)
   IMBEDSRCS += pRTcmix.cpp
   TARGETS += pRTcmix.o
   PROGS += PMIX
endif
ifeq ($(PYTHON_SUPPORT), TRUE)
   ARCHFLAGS += -DPYTHON
   PROGS += PYMIX
   PYTHONLIBS = `$(PYTHON) ../parser/python/print_libpython.py`
   ifeq ($(PYTHON_EXT_STATIC_LINK), TRUE)
      PYEXT = ../parser/python/rtcmixmodule.o
   endif
endif

# This workaround converts the linker path "/usr/local/lib" to "/usr/lib" for machines
# which have perl installed in /usr/bin, but have the broken config which adds the
# wrong linker path anyway.

ifeq ($(PERL_IN_LOCAL), TRUE)
PERL_LDFLAGS = `$(PERL) -MExtUtils::Embed -e ldopts`
else
PERL_LDFLAGS = `$(PERL) -MExtUtils::Embed -e ldopts | sed 's/usr\/local/usr/'`
endif

all: $(TARGETS)
	$(INSTALL) main.o $(LIBDIR)
	$(INSTALL) RTcmixMain.o $(LIBDIR)
	$(INSTALL) RTsockfuncs.o $(LIBDIR)
ifeq ($(PERL_SUPPORT), TRUE)
	$(INSTALL) pRTcmix.o $(LIBDIR)
endif
	$(INSTALL) $(RTLIB) $(LIBDIR)

# rules to allow complete lib compile from this dir only

$(LIBCONF):
	(cd ../conf; $(MAKE) $(MFLAGS) all;)

$(LIBMINC):
	(cd ../parser; $(MAKE) $(MFLAGS) all;)

$(P_O):
	(cd ../parser; $(MAKE) $(MFLAGS) all;)

$(PY_O):
	(cd ../parser; $(MAKE) $(MFLAGS) all;)

$(SNDLIB):
	(cd ../sndlib; $(MAKE) $(MFLAGS) all;)

$(AUDIOLIB):
	(cd ../audio; $(MAKE) $(MFLAGS) all;)

loader.o: loader.c
	$(CC) $(CFLAGS) -DSHAREDLIBDIR=\"$(LIBDESTDIR)\" -c loader.c

connection.o: connection.cpp
	$(CXX) $(CXXFLAGS) -DSHLIB_SUFFIX=\"$(SHLIB_SUFFIX)\" -DSHAREDLIBDIR=\"$(LIBDESTDIR)\" -c connection.cpp

$(RTLIB): depend $(COMMON_OBJS) $(LIBMINC) $(LIBS)
	$(CXX) $(DYNAMIC_LDFLAGS) -o $@ $(COMMON_OBJS) $(LIBMINC) $(LIBS) $(SYSLIBS)

Option.o: Option.cpp Option.h
	$(CXX) $(CXXFLAGS) -DSHAREDLIBDIR=\"$(LIBDESTDIR)\" -c $< -o $@

$(MIX_INST):	
	(cd ../../insts/base/MIX; make all)

heap/rtHeap.o:
	@echo compiling heap.
	(cd heap; $(MAKE) $(MFLAGS) all;)

# rules for three executables we build

MIX: $(MAIN_OBJS) $(M_O) $(LIBS) $(PROFILE_O)
	$(CXX) -o MIX $(DYN) $(MAIN_OBJS) $(PROFILE_O) $(M_O) -L. $(LDFLAGS)

PMIX: $(MAIN_OBJS) $(P_O) $(LIBS) $(PROFILE_O)
ifeq ($(PERL_SUPPORT), TRUE)
	$(CXX) -o PMIX $(DYN) $(MAIN_OBJS) $(PROFILE_O) \
		$(P_O) -L. $(LDFLAGS) $(PERL_LDFLAGS)
else
	@echo "You haven't configured with PERL_SUPPORT!"
endif

PYMIX: $(MAIN_OBJS) $(PY_O) $(LIBS) $(PROFILE_O)
ifeq ($(PYTHON_SUPPORT), TRUE)
	$(CXX) -o PYMIX $(DYN) $(MAIN_OBJS) $(PROFILE_O) \
		$(PY_O) -L. $(LDFLAGS) $(PYEXT) $(PYTHONLIBS)
else
	@echo "You haven't configured with PYTHON_SUPPORT!"
endif

.PHONY: all standalone install standalone_install \
		uninstall standalone_uninstall clean cleanall

depend:
ifeq ($(ARCH),LINUX)
	-$(SHELL) -ec '$(CC) -M $(CFLAGS) $(COMMON_CSRCS) $(COMMON_CPPSRCS) $(IMBEDSRCS) $(MAIN_CPPSRCS) `$(PERL) -MExtUtils::Embed -e ccopts`' > depend
endif
ifeq ($(ARCH),SGI)
# Note: -MM is a gcc extension. If no gcc, use -M.
# Unfortunately, this doesn't work...   -JGG
#	$(SHELL) -ec '$(CC) -M $(CFLAGS) $(SRCS) $(CPPSRCS)' > depend
endif

-include depend

install:	standalone_install

standalone_install: all
	@if test ! -d $(DESTDIR); then mkdir $(DESTDIR); fi;
	$(INSTALL) MIX $(DESTDIR)/CMIX
ifeq ($(PERL_SUPPORT), TRUE)
	$(INSTALL) PMIX $(DESTDIR)/PCMIX
endif
ifeq ($(PYTHON_SUPPORT), TRUE)
	$(INSTALL) PYMIX $(DESTDIR)/PYCMIX
endif

uninstall: standalone_uninstall
	$(RM) $(LIBDIR)/$(RTLIB)
	$(RM) $(LIBDIR)/main.o
	$(RM) $(LIBDIR)/RTcmixMain.o
	$(RM) $(LIBDIR)/RTsockfuncs.o
	$(RM) $(LIBDIR)/pRTcmix.o

standalone_uninstall:
	$(RM) $(DESTDIR)/CMIX
	$(RM) $(DESTDIR)/PCMIX
	$(RM) $(DESTDIR)/PYCMIX

clean:
	(cd heap; $(MAKE) $(MFLAGS) clean;)
	$(RM) $(COMMON_OBJS) $(TARGETS) core depend

cleanall: clean

