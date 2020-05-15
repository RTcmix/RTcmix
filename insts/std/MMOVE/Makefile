# This builds and installs both PLACE and MOVE

include ../package.conf

CURDIR = $(CMIXDIR)/insts/std/MMOVE
MOVEDIR = $(CMIXDIR)/insts/std/MOVE
EXTERN_OBJS = $(MOVEDIR)/cmixfuns.o $(MOVEDIR)/common.o
EXTERN_HDRS = $(MOVEDIR)/cmixfuns.h $(MOVEDIR)/common.h $(INSTRUMENT_H)
COMMON_OBJS = MBASE.o RVB.o msetup.o
COMMON_HDRS = msetup.h $(EXTERN_HEADERS)
POBJS = MPLACE.o placeprof.o $(COMMON_OBJS) $(EXTERN_OBJS)
MOBJS = MMOVE.o moveprof.o path.o $(COMMON_OBJS) $(EXTERN_OBJS)
DOBJS = DMOVE.o dmoveprof.o path.o $(COMMON_OBJS) $(EXTERN_OBJS)
#PROF = -pg
PROF =
ifeq ($(ARCH),LINUX)
   CXXFLAGS += -funroll-loops $(PROF)
endif
CXXFLAGS += -I. -I$(MOVEDIR) $(MINE)
CFLAGS += -I. -I$(MOVEDIR) $(MINE)

PROGS = MPLACE libMPLACE.so MMOVE libMMOVE.so DMOVE libDMOVE.so

.PHONY: all standalone install dso_install standalone_install \
uninstall dso_uninstall standalone_uninstall clean cleanall

ifeq ($(BUILDTYPE), STANDALONE)
  all: libMPLACE.so libMMOVE.so libDMOVE.so
else
  all: $(POBJS) $(MOBJS) $(DOBJS)
endif

standalone: MPLACE MMOVE DMOVE

$(EXTERN_OBJS):
	cd $(MOVEDIR); make

libMPLACE.so: $(POBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(POBJS) $(GENLIB) $(SYSLIBS)

MPLACE: $(POBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(POBJS) $(CMIXOBJS) $(LDFLAGS) $(PROF)

$(POBJS): $(COMMON_HDRS) MBASE.h MPLACE.h

libMMOVE.so: $(MOBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(MOBJS) $(GENLIB) $(SYSLIBS)

MMOVE: $(MOBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(MOBJS) $(CMIXOBJS) $(LDFLAGS) $(PROF)

$(MOBJS): $(COMMON_HDRS) MBASE.h MMOVE.h

libDMOVE.so: $(DOBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(DOBJS) $(GENLIB) $(SYSLIBS)

DMOVE: $(DOBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(DOBJS) $(CMIXOBJS) $(LDFLAGS) $(PROF)

$(DOBJS): $(COMMON_HDRS) MBASE.h DMOVE.h

MBASE.o : $(COMMON_HDRS) MBASE.cpp MBASE.h

install: dso_install

dso_install: all
	$(INSTALL) $(CURDIR)/libMPLACE.so $(LIBDESTDIR)
	$(INSTALL) $(CURDIR)/libMMOVE.so $(LIBDESTDIR)
	$(INSTALL) $(CURDIR)/libDMOVE.so $(LIBDESTDIR)

standalone_install: standalone
	$(INSTALL) $(CURDIR)/MPLACE $(DESTDIR)
	$(INSTALL) $(CURDIR)/MMOVE $(DESTDIR)
	$(INSTALL) $(CURDIR)/DMOVE $(DESTDIR)

uninstall: dso_uninstall standalone_uninstall

dso_uninstall:
	$(RM) $(LIBDESTDIR)/libMPLACE.so
	$(RM) $(LIBDESTDIR)/libMMOVE.so
	$(RM) $(LIBDESTDIR)/libDMOVE.so

standalone_uninstall:
	$(RM) $(DESTDIR)/MPLACE
	$(RM) $(DESTDIR)/MMOVE
	$(RM) $(DESTDIR)/DMOVE

clean:
	$(RM) $(POBJS) $(MOBJS) $(DOBJS) $(PROGS) $(TRASH)

cleanall: clean
	$(RM) depend
