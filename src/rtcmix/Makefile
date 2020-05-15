include ../../makefile.conf

# NOTES FOR UNDERSTANDING THIS MAKEFILE
#
# Here are some of the macros used in this file, and their meanings
#
# COMMON_OBJS	-	object files used by all versions
# LIB_OBJS		-	object files for all sub-libraries (needed by static builds)
# INST_OBJS		-	object files for all instruments (needed by embedded builds)
# MAIN_OBJS 	-	object files needed by objects requiring a main() routine
# M_O			-	the Minc parser object (comes from makefile.conf)
# PROFILE_O		-	the profile.o object (comes from makefile.conf)

# RTLIB_LDFLAGS	-	flags passed to loader stage for $(RTLIB)
#
# NOTE:
# BUILDTYPE is the top-level macro for determining what kind of build you get.
# It is always defined in your personal site.conf.
# This Makefile now only distinguishes STANDALONE from all others.  See makefile.conf
# for the build-type-specific logic

AUDIODIR = ../audio
SNDLIBDIR = ../sndlib
CONFDIR = ../conf

CXXFLAGS += $(SOUND_DRIVER)
INCLUDES += -I$(INCLUDEDIR) -I../include -I$(AUDIODIR) -I$(SNDLIBDIR) -I./heap -I../conf

# Source file listings

COMMON_CSRCS = \
check_byte_order.c \
command_line.c \
funderflow.c \
getsample.c \
infile.c \
io.c \
load_utils.c \
merror.c \
message.c \
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
ug_intro.c

GEN_CSRCS = \
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
filter.cpp \
Instrument.cpp \
intraverse.cpp \
loader.cpp \
lfo.cpp \
minc_info.cpp \
minc_functions.cpp \
DataFile.cpp \
MMPrint.cpp \
Option.cpp \
PFBusData.cpp \
PField.cpp \
PFieldSet.cpp \
Random.cpp \
RawDataFile.cpp \
RefCounted.cpp \
RTcmix.cpp \
InputFile.cpp \
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
sound_sample_buf_read.cpp \
table.cpp \
tableutils.cpp \
modtable.cpp \
ops.cpp \
utils.cpp \
rt_ug_intro.cpp \
connection.cpp \
converter.cpp \
monitor.cpp

# Build-based additions to local source files

ifneq ($(BUILDTYPE), STANDALONE)
	COMMON_CPPSRCS += rtprofile.cpp
endif

ifeq ($(MULTI_THREAD_SUPPORT), TRUE)
   COMMON_CPPSRCS += TaskManager.cpp RTThread.cpp
   CXXFLAGS += -DRT_THREAD_COUNT=$(THREAD_COUNT) 
endif

ifeq ($(NPLAY_SUPPORT), TRUE)
   COMMON_CPPSRCS += setnetplay.cpp
endif

ifeq ($(EMBEDDED_USE_MMOVE), TRUE)
	CXXFLAGS += -DUSE_MMOVE
	CFLAGS += -DUSE_MMOVE
endif

MAIN_CPPSRCS = RTcmixMain.cpp main.cpp

IMBEDSRCS =

CPPOBJS = $(patsubst %.cpp,%.o,$(COMMON_CPPSRCS))

COBJS = $(patsubst %.c,%.o,$(COMMON_CSRCS))

GENOBJS = $(patsubst %.c,%.o,$(GEN_CSRCS))

RTHEAPOBJS = heap/heap.o heap/queue.o heap/rtQueue.o

MIX_OBJS = ../../insts/base/MIX/MIX.o

MAIN_OBJS = RTcmixMain.o main.o

# The initial set of objects used by all builds
COMMON_OBJS = $(CPPOBJS) $(COBJS) $(GENOBJS) $(RTHEAPOBJS) $(MIX_OBJS) $(AUDIODIR)/audio_devices.o

ifeq ($(ARCH), MACOSX_10.0)
   COMMON_OBJS += osx_workaround.o
endif


# NB: all non-STANDALONE builds include all the instrument objects directly in the library
ifneq ($(BUILDTYPE), STANDALONE)
	include maxmspstuff/INBASEOBJS
	include maxmspstuff/INSTDOBJS
	include maxmspstuff/INJGOBJS
	include maxmspstuff/INSTKOBJS
	include maxmspstuff/INMAXMSPOBJS
	include maxmspstuff/INBGGOBJS
	include maxmspstuff/INJOELOBJS
	include maxmspstuff/INNEILOBJS
	INST_OBJS = $(INBASEOBJS) $(INSTDOBJS) $(INJGOBJS) $(INSTKOBJS) \
		$(INMAXMSPOBJS) $(INBGGOBJS) $(INJOELOBJS) $(INNEILOBJS)
endif

# Sub-libraries.  Not used for static builds.
LIBGEN = ../../genlib/libgen.a
LIBCONF = $(CONFDIR)/libconf.a
LIBMINC = ../parser/minc/libminc.a
SNDLIB = $(SNDLIBDIR)/sndlib.a
# NB: the last two defs will be empty if the features are not configured
LIBRTAUDIO = $(AUDIODIR)/librtaudio.a $(FFTW_LIBS) $(JACK_LIBS)
LIBINLET = ../control/maxmsp/libinlet.a
LIBPFBUS = ../control/pfbus/libpfbus.a
LIBJGOBJ = ../../insts/jg/objlib/objlib.a
LIBSTK = ../../insts/stk/stklib/stklib.a


# Libraries created differently if static vs. dynamic
ifeq ($(RTLIBTYPE), STATIC)
	# In order to build a static lib, ALL objects must be linked -- no sub-libraries possible
	LIBS =
	include iosstuff/IOSLIBOBJS
	LIB_OBJS = $(LIBRTAUDIOOBJS) $(LIBCONFOBJS) $(LIBGENOBJS) $(LIBINLETOBJS) \
		$(LIBPFBUSOBJS) $(LIBMINCOBJS) $(LIBOBJOBJS) $(LIBRTHEAPOBJS) \
		$(LIBSNDOBJS) $(LIBSTKOBJS)
	LOAD = $(AR) rv
	RANLIB = ranlib
	# Nothing to link for static lib
	SYS_LDFLAGS =
else
	# List of sublibraries for dynamic build.
	LIBS = $(LIBRTAUDIO) $(SNDLIB) $(LIBCONF) $(LIBMINC) $(LIBGEN)
	ifneq ($(BUILDTYPE), STANDALONE)
		LIBS += $(LIBINLET) $(LIBPFBUS) $(LIBSTK) $(LIBJGOBJ)
	endif
	LOAD = $(CXX) $(RTLIB_LDFLAGS) $(ARCH_BITFLAGS) -o
	RANLIB = touch
endif

PROGS = MIX

# Object lists are different for standalone vs. embedded
ifeq ($(BUILDTYPE), STANDALONE)
	TARGETS = $(RTLIB) $(MAIN_OBJS) RTsockfuncs.o $(PROGS)
else
	COMMON_OBJS += $(M_O) $(MAIN_OBJS)
	TARGETS = $(COMMON_OBJS) $(MAIN_OBJS) $(RTLIB)
endif

PERL_CCFLAGS=

ifeq ($(PERL_SUPPORT), TRUE)
   IMBEDSRCS += pRTcmix.cpp
   TARGETS += pRTcmix.o
   PROGS += PMIX
	ifeq ($(ARCH),MACOSX)
		PERL_CCFLAGS=`sh ../parser/perl/print_perl_includes.sh $(PERL) $(BUILD_ARCH)`
	else
		PERL_CCFLAGS=`$(PERL) -MExtUtils::Embed -e ccopts`
	endif
endif
ifeq ($(PYTHON_SUPPORT), TRUE)
   IMBEDSRCS += pyRTcmix.cpp
   TARGETS += pyRTcmix.o
   PROGS += PYMIX
   ARCHFLAGS += -DPYTHON
   PYTHONLIBS = `$(PYTHON) ../parser/python/print_libpython.py`
   ifeq ($(PYTHON_EXT_STATIC_LINK), TRUE)
      PYEXT = ../parser/python/rtcmixmodule.o
   endif
endif

# This workaround converts the linker path "/usr/local/lib" to "/usr/lib" for machines
# which have perl installed in /usr/bin, but have the broken config which adds the
# wrong linker path anyway.

ifeq ($(PERL_IN_LOCAL), TRUE)
	ifeq ($(ARCH),MACOSX)
		PERL_LDFLAGS = `$(PERL) -MExtUtils::Embed -e ldopts | sed 's/-arch i386//' | sed 's/-arch ppc//'`
	else
		PERL_LDFLAGS = `$(PERL) -MExtUtils::Embed -e ldopts`
	endif
else
	ifeq ($(ARCH),MACOSX)
		PERL_LDFLAGS = `$(PERL) -MExtUtils::Embed -e ldopts | sed 's/usr\/local/usr/' | sed 's/-arch i386//' | sed 's/-arch ppc//'`
	else
		PERL_LDFLAGS = `$(PERL) -MExtUtils::Embed -e ldopts | sed 's/usr\/local/usr/'`
	endif
endif

# rules to allow complete lib compile from this dir only

# This lets me compile without copying to LIBDIR
targets: $(TARGETS)

# These are needed by all builds

$(LIBCONF):
	(cd ../conf; $(MAKE) $(MFLAGS) all;)

$(LIBMINC):
	(cd ../parser; $(MAKE) $(MFLAGS) all;)

$(P_O):
	(cd ../parser; $(MAKE) $(MFLAGS) all;)

$(PY_O):
	(cd ../parser; $(MAKE) $(MFLAGS) all;)

$(SNDLIB):
	(cd $(SNDLIBDIR); $(MAKE) $(MFLAGS) all;)

$(LIBRTAUDIO):
	(cd $(AUDIODIR); $(MAKE) $(MFLAGS) all;)

$(MIX_OBJS):	
	(cd ../../insts/base/MIX; make all)

# This target attempts to do builds in all the directories outside of the current one.
ifneq ($(BUILDTYPE), STANDALONE)
everything:		$(SNDLIB) $(LIBRTAUDIO) $(MIX_OBJS) $(LIBMINC) $(LIBCONF)
#	$(LIBINLET):
		(cd ../control/maxmsp; $(MAKE) $(MFLAGS) all;)

#	$(LIBPFBUS):
		(cd ../control/pfbus; $(MAKE) $(MFLAGS) all;)

#	$(INBASEOBJS):
		(cd ../../insts/base; $(MAKE) $(MFLAGS) all;)

#	$(INSTDOBJS):
		(cd ../../insts/std; $(MAKE) $(MFLAGS) all;)

#	$(INJGOBJS):
		@echo building in ../../insts/jg
		(cd ../../insts/jg; $(MAKE) $(MFLAGS) all;)

#	$(INSTKOBJS):
		@echo building in ../../insts/stk
		(cd ../../insts/stk; $(MAKE) $(MFLAGS) all;)

#	$(INMAXMSPOBJS):
		(cd ../../insts/maxmsp; $(MAKE) $(MFLAGS) all;)

#	$(INBGGOBJS):
		(cd ../../insts/bgg; $(MAKE) $(MFLAGS) all;)

#	$(INJOELOBJS):
		(cd ../../insts/joel; $(MAKE) $(MFLAGS) all;)

#	$(INNEILOBJS):
		(cd ../../insts/neil; $(MAKE) $(MFLAGS) all;)

endif

# These objects need additional flags, which are spelled out here

loader.o: loader.cpp
	$(CXX) $(CFLAGS) -DSHAREDLIBDIR=\"$(LIBDESTDIR)\" -c loader.cpp

connection.o: connection.cpp
	$(CXX) $(CXXFLAGS) -DSHAREDLIBDIR=\"$(LIBDESTDIR)\" -c connection.cpp

addcheckfunc.o: addcheckfunc.cpp
	$(CXX) $(CXXFLAGS) -DSHAREDLIBDIR=\"$(LIBDESTDIR)\" -c addcheckfunc.cpp

RTcmix.o: RTcmix.cpp
	$(CXX) $(CXXFLAGS) -DSHAREDLIBDIR=\"$(LIBDESTDIR)\" -c RTcmix.cpp

intraverse.o: intraverse.cpp
	$(CXX) $(CXXFLAGS) -DRT_THREAD_COUNT=$(THREAD_COUNT) -c intraverse.cpp

MMPrint.o: MMPrint.cpp MMPrint.h
	$(CXX) $(CXXFLAGS) -DSHAREDLIBDIR=\"$(LIBDESTDIR)\" -c $< -o $@

Option.o: Option.cpp Option.h
	$(CXX) $(CXXFLAGS) -DSHAREDLIBDIR=\"$(LIBDESTDIR)\" -c $< -o $@

PFBusData.o: PFBusData.cpp PFBusData.h
	$(CXX) $(CXXFLAGS) -DSHAREDLIBDIR=\"$(LIBDESTDIR)\" -c $< -o $@

heap/rtHeap.o:	heap/heap.o heap/queue.o heap/rtQueue.o
	@echo compiling heap.
	(cd heap; $(MAKE) $(MFLAGS) all;)

# Main build rules

all: $(TARGETS)
ifeq ($(BUILDTYPE), STANDALONE)
		$(INSTALL) main.o $(LIBDIR)
		$(INSTALL) RTcmixMain.o $(LIBDIR)
		$(INSTALL) RTsockfuncs.o $(LIBDIR)
  ifeq ($(PERL_SUPPORT), TRUE)
		$(INSTALL) pRTcmix.o $(LIBDIR)
  endif
  ifeq ($(PYTHON_SUPPORT), TRUE)
		$(INSTALL) pyRTcmix.o $(LIBDIR)
  endif
  ifdef RTLIB_INSTALL_DIR
		@if test ! -d $(RTLIB_INSTALL_DIR) ; then mkdir -p $(RTLIB_INSTALL_DIR); fi;
		$(INSTALL) $(RTLIB) $(RTLIB_INSTALL_DIR)
  else
		@echo RTLIB_INSTALL_DIR is not set - not installing $(RTLIB)
  endif
endif

# rules for library and three executables we build

ifeq ($(BUILDTYPE), STANDALONE)
$(RTLIB): depend $(COMMON_OBJS) $(LIBS)
	$(LOAD) $@ $(COMMON_OBJS) $(INST_OBJS) $(LIB_OBJS) $(LIBS) $(SYSLIBS)
	$(RANLIB) $@
MIX: $(MAIN_OBJS) $(M_O) $(LIBS) $(PROFILE_O)
	$(CXX) $(CXXFLAGS) -o MIX $(DYN) $(MAIN_OBJS) $(PROFILE_O) $(M_O) -L. $(LDFLAGS) $(SYS_LDFLAGS)

install:	all
	@if test ! -d $(DESTDIR); then mkdir -p $(DESTDIR); fi;
	$(INSTALL) MIX $(DESTDIR)/CMIX
ifeq ($(PERL_SUPPORT), TRUE)
	$(INSTALL) PMIX $(DESTDIR)/PCMIX
endif
ifeq ($(PYTHON_SUPPORT), TRUE)
	$(INSTALL) PYMIX $(DESTDIR)/PYCMIX
endif


else

# Note: Some of these macros will be empty for dynamic builds; some will be empty for static
$(RTLIB): depend $(COMMON_OBJS) $(LIBS)
	$(LOAD) $@ $(COMMON_OBJS) $(INST_OBJS) $(LIB_OBJS) $(LIBS) $(SYS_LDFLAGS)
	$(RANLIB) $@

install:	all
  ifdef RTLIB_INSTALL_DIR
		@if test ! -d $(RTLIB_INSTALL_DIR) ; then mkdir -p $(RTLIB_INSTALL_DIR); fi;
		$(INSTALL) $(RTLIB) $(RTLIB_INSTALL_DIR)
  else
		@echo RTLIB_INSTALL_DIR is not set - not installing $(RTLIB)
  endif

endif

PMIX: $(MAIN_OBJS) $(P_O) $(LIBS) $(PROFILE_O)
ifeq ($(PERL_SUPPORT), TRUE)
	$(CXX) $(CXXFLAGS) -o PMIX $(DYN) $(MAIN_OBJS) $(PROFILE_O) \
		$(P_O) -L. $(LDFLAGS) $(PERL_LDFLAGS) $(SYS_LDFLAGS)
else
	@echo "You haven't configured with PERL_SUPPORT!"
endif

PYMIX: $(MAIN_OBJS) $(PY_O) $(LIBS) $(PROFILE_O)
ifeq ($(PYTHON_SUPPORT), TRUE)
	$(CXX) $(CXXFLAGS) -o PYMIX $(DYN) $(MAIN_OBJS) $(PROFILE_O) \
		$(PY_O) -L. $(LDFLAGS) $(PYEXT) $(PYTHONLIBS) $(SYS_LDFLAGS)
else
	@echo "You haven't configured with PYTHON_SUPPORT!"
endif

.PHONY: all standalone install standalone_install \
		uninstall standalone_uninstall clean cleanall

depend:
ifeq ($(ARCH),SGI)
# Note: -MM is a gcc extension. If no gcc, use -M.
# Unfortunately, this doesn't work...   -JGG
#	$(SHELL) -ec '$(CC) -M $(CFLAGS) $(SRCS) $(CPPSRCS)' > depend
else
	-$(SHELL) -ec '$(CC) -M -DSHAREDLIBDIR=\"$(LIBDESTDIR)\" $(CFLAGS) $(COMMON_CSRCS) $(GEN_CSRCS) $(COMMON_CPPSRCS) $(IMBEDSRCS) $(MAIN_CPPSRCS) $(PERL_CCFLAGS)' > ./depend
endif

-include depend

uninstall: standalone_uninstall
	$(RM) $(LIBDIR)/$(RTLIB)
	$(RM) $(LIBDIR)/main.o
	$(RM) $(LIBDIR)/RTcmixMain.o
	$(RM) $(LIBDIR)/RTsockfuncs.o
	$(RM) $(LIBDIR)/pRTcmix.o
	$(RM) $(LIBDIR)/pyRTcmix.o

standalone_uninstall:
	$(RM) $(DESTDIR)/CMIX
	$(RM) $(DESTDIR)/PCMIX
	$(RM) $(DESTDIR)/PYCMIX

clean:
	(cd heap; $(MAKE) $(MFLAGS) clean;)
	$(RM) $(COMMON_OBJS) $(TARGETS) core
	$(RM) $(RTLIB)

cleanall: clean
	$(RM) depend

# Cool dependency debug magic.  Please leave here.
#OLD_SHELL := $(SHELL)
#SHELL = $(warning [$@ ($^) ($?)])$(OLD_SHELL)

# DAS Added these to make config debugging easier
check_macros:
	@echo BUILDTYPE = $(BUILDTYPE)
	@echo AUDIODRIVER = $(AUDIODRIVER)
	@echo MFLAGS = $(MFLAGS)
	@echo COMMON_OBJS = $(COMMON_OBJS)
	@echo INST_OBJS = $(INST_OBJS)
	@echo LIBS = $(LIBS)
	@echo LIB_OBJS = $(LIB_OBJS)
	@echo ARCHFLAGS = $(ARCHFLAGS)
	@echo ARCH_RTLDFLAGS = $(ARCH_RTLDFLAGS)
	@echo SYS_LDFLAGS = $(SYS_LDFLAGS)
	@echo CMIX_FLAGS = $(CMIX_FLAGS)
	@echo RTLIB = $(RTLIB)
	@echo RTLIB_LDFLAGS = $(RTLIB_LDFLAGS)
	@echo RTLIBTYPE = $(RTLIBTYPE)
	@echo SHARED_LDFLAGS = $(SHARED_LDFLAGS)
	@echo DYNAMIC_LDFLAGS = $(DYNAMIC_LDFLAGS)
