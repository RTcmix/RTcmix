# This builds and installs both PLACE and MOVE

include ../package.conf

COMMON_OBJS = BASE.o cmixfuns.o setup.o common.o
OBJS = PLACE.o placeprof.o $(COMMON_OBJS)
MOBJS = MOVE.o moveprof.o path.o $(COMMON_OBJS)
#PROF = -pg
PROF =
ifeq ($(ARCH),LINUX)
   CXXFLAGS += -funroll-loops $(PROF)
endif
PROGS = PLACE libPLACE.so MOVE libMOVE.so

all: libPLACE.so libMOVE.so

standalone: PLACE MOVE

libPLACE.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) \
	$(GENLIB) $(SYSLIBS)

PLACE: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS) $(PROF)

$(OBJS): $(INSTRUMENT_H) PLACE.h cmixfuns.h setup.h common.h

libMOVE.so: $(MOBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(MOBJS) \
	$(GENLIB) $(SYSLIBS)

MOVE: $(MOBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(MOBJS) $(CMIXOBJS) $(LDFLAGS) $(PROF)

$(MOBJS): $(INSTRUMENT_H) MOVE.h cmixfuns.h setup.h common.h

BASE.o : $(INSTRUMENT_H) BASE.h cmixfuns.h setup.h common.h

clean:
	$(RM) $(OBJS) $(MOBJS) $(PROGS)

