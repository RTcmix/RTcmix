# This builds and installs both PLACE and MOVE

include ../package.conf

CURDIR = $(CMIXDIR)/insts/std/MOVE
COMMON_OBJS = BASE.o cmixfuns.o setup.o common.o
COMMON_HDRS = cmixfuns.h setup.h common.h
OBJS = PLACE.o placeprof.o $(COMMON_OBJS)
MOBJS = MOVE.o moveprof.o path.o $(COMMON_OBJS)
#PROF = -pg
PROF =
ifeq ($(ARCH),LINUX)
   CXXFLAGS += -funroll-loops $(PROF)
endif
PROGS = PLACE libPLACE.so MOVE libMOVE.so

.PHONY: all standalone install dso_install standalone_install \
uninstall dso_uninstall standalone_uninstall clean cleanall

ifeq ($(BUILDTYPE), STANDALONE)
  all: libPLACE.so libMOVE.so
else
  all: $(OBJS) $(MOBJS)
endif

standalone: PLACE MOVE

libPLACE.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

PLACE: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS) $(PROF)

$(OBJS): $(INSTRUMENT_H) $(COMMON_HDRS) PLACE.h

libMOVE.so: $(MOBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(MOBJS) $(GENLIB) $(SYSLIBS)

MOVE: $(MOBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(MOBJS) $(CMIXOBJS) $(LDFLAGS) $(PROF)

$(MOBJS): $(INSTRUMENT_H) $(COMMON_HDRS) MOVE.h

BASE.o : $(INSTRUMENT_H) $(COMMON_HDRS) BASE.h

install: dso_install

dso_install: all
	$(INSTALL) $(CURDIR)/libPLACE.so $(LIBDESTDIR)
	$(INSTALL) $(CURDIR)/libMOVE.so $(LIBDESTDIR)

standalone_install: standalone
	$(INSTALL) $(CURDIR)/PLACE $(DESTDIR)
	$(INSTALL) $(CURDIR)/MOVE $(DESTDIR)

uninstall: dso_uninstall standalone_uninstall

dso_uninstall:
	$(RM) $(LIBDESTDIR)/libPLACE.so
	$(RM) $(LIBDESTDIR)/libMOVE.so

standalone_uninstall:
	$(RM) $(DESTDIR)/PLACE
	$(RM) $(DESTDIR)/MOVE

clean:
	$(RM) $(OBJS) $(MOBJS) $(PROGS)

cleanall: clean
	$(RM) depend
