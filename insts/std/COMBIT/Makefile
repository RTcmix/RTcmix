include ../insts.conf

OBJS = COMBIT.o cfuncs.o
CMIXOBJS = ../../sys/cmix.o ../../lib/genlib.a
CFLAGS += -fPIC
CXXFLAGS += -fPIC 
PROGS = COMBIT libCOMBIT.so

all: COMBIT libCOMBIT.so

libCOMBIT.so: $(OBJS) ../../lib/genlib.a
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) \
	../../lib/genlib.a $(SYSLIBS)

COMBIT: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): ../../rtstuff/Instrument.h

clean:
	rm -f *.o $(OBJS) $(PROGS)
