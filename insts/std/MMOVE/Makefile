# This builds and installs both PLACE and MOVE

include ../package.conf

CURDIR = $(CMIXDIR)/insts/std/MMOVE
MOVEDIR = $(CMIXDIR)/insts/std/MOVE
EXTERN_OBJS = $(MOVEDIR)/cmixfuns.o $(MOVEDIR)/common.o
EXTERN_HDRS = $(MOVEDIR)/cmixfuns.h $(MOVEDIR)/common.h $(INSTRUMENT_H)
COMMON_OBJS = MBASE.o RVBBASE.o RVB.o msetup.o
QCOMMON_OBJS = MBASE.o RVBBASE.o QRVB.o msetup.o
COMMON_HDRS = msetup.h $(EXTERN_HEADERS)
SBASE_OBJS = MSTEREOBASE.o
MBASE_OBJS = MOVEBASE.o
POBJS = MPLACE.o $(SBASE_OBJS) placeprof.o $(COMMON_OBJS) $(EXTERN_OBJS)
MOBJS = MMOVE.o $(SBASE_OBJS) moveprof.o path.o $(COMMON_OBJS) $(EXTERN_OBJS)
DOBJS = DMOVE.o $(SBASE_OBJS) $(MBASE_OBJS) dmoveprof.o path.o $(COMMON_OBJS) $(EXTERN_OBJS)
QOBJS = QMOVE.o $(MBASE_OBJS) qmoveprof.o path.o $(QCOMMON_OBJS) $(EXTERN_OBJS)
#PROF = -pg
PROF =
# If sanitize is turned on, make sure to add this to the environment:
# DYLD_INSERT_LIBRARIES=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/16/lib/darwin/libclang_rt.asan_osx_dynamic.dylib

#OPT = -g -fsanitize=address
ifeq ($(ARCH),LINUX)
   CXXFLAGS += -funroll-loops $(PROF)
endif
CXXFLAGS += -I. -I$(MOVEDIR) $(MINE)
CFLAGS += -I. -I$(MOVEDIR) $(MINE)

PROGS = MPLACE libMPLACE.so MMOVE libMMOVE.so DMOVE libDMOVE.so QMOVE libQMOVE.so

.PHONY: all standalone install dso_install standalone_install \
uninstall dso_uninstall standalone_uninstall clean cleanall

ifeq ($(BUILDTYPE), STANDALONE)
  all: libMPLACE.so libMMOVE.so libDMOVE.so libQMOVE.so
else
  all: $(POBJS) $(MOBJS) $(DOBJS) $(QOBJS)
endif

standalone: MPLACE MMOVE DMOVE QMOVE

$(EXTERN_OBJS):
	cd $(MOVEDIR); make

libMPLACE.so: $(POBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(POBJS) $(GENLIB) $(SYSLIBS)

MPLACE: $(POBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(POBJS) $(CMIXOBJS) $(LDFLAGS) $(PROF)

$(POBJS): $(COMMON_HDRS) MBASE.h MSTEREOBASE.h MPLACE.h

libMMOVE.so: $(MOBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(MOBJS) $(GENLIB) $(SYSLIBS)

MMOVE: $(MOBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(MOBJS) $(CMIXOBJS) $(LDFLAGS) $(PROF)

$(MOBJS): $(COMMON_HDRS) MBASE.h MMOVE.h MOVEBASE.h

libDMOVE.so: $(DOBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(DOBJS) $(GENLIB) $(SYSLIBS)

DMOVE: $(DOBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(DOBJS) $(CMIXOBJS) $(LDFLAGS) $(PROF)

$(DOBJS): $(COMMON_HDRS) MBASE.h MSTEREOBASE.h MOVEBASE.h DMOVE.h

libQMOVE.so: $(QOBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(QOBJS) $(GENLIB) $(SYSLIBS)

QMOVE: $(QOBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(QOBJS) $(CMIXOBJS) $(LDFLAGS) $(PROF)

$(QOBJS): $(COMMON_HDRS) MBASE.h MOVEBASE.h QMOVE.h

MBASE.o : $(COMMON_HDRS) MBASE.cpp MBASE.h

install: dso_install

dso_install: all
	$(INSTALL) $(CURDIR)/libMPLACE.so $(LIBDESTDIR)
	$(INSTALL) $(CURDIR)/libMMOVE.so $(LIBDESTDIR)
	$(INSTALL) $(CURDIR)/libDMOVE.so $(LIBDESTDIR)
	$(INSTALL) $(CURDIR)/libQMOVE.so $(LIBDESTDIR)

standalone_install: standalone
	$(INSTALL) $(CURDIR)/MPLACE $(DESTDIR)
	$(INSTALL) $(CURDIR)/MMOVE $(DESTDIR)
	$(INSTALL) $(CURDIR)/DMOVE $(DESTDIR)
	$(INSTALL) $(CURDIR)/QMOVE $(DESTDIR)

uninstall: dso_uninstall standalone_uninstall

dso_uninstall:
	$(RM) $(LIBDESTDIR)/libMPLACE.so
	$(RM) $(LIBDESTDIR)/libMMOVE.so
	$(RM) $(LIBDESTDIR)/libDMOVE.so
	$(RM) $(LIBDESTDIR)/libQMOVE.so

standalone_uninstall:
	$(RM) $(DESTDIR)/MPLACE
	$(RM) $(DESTDIR)/MMOVE
	$(RM) $(DESTDIR)/DMOVE
	$(RM) $(DESTDIR)/QMOVE

clean:
	$(RM) $(POBJS) $(MOBJS) $(DOBJS) $(QOBJS) $(PROGS) $(TRASH)

cleanall: clean
	$(RM) depend
