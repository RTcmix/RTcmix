include ../insts.conf

OBJS = TRANS.o reset.o
CMIXOBJS = ../../sys/cmix.o ../../lib/genlib.a
CFLAGS += -fPIC
CXXFLAGS += -fPIC 
PROGS = TRANS libTRANS.so

all: TRANS libTRANS.so

libTRANS.so: $(OBJS) ../../lib/genlib.a
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) \
	../../lib/genlib.a $(SYSLIBS)

TRANS: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): ../../rtstuff/Instrument.h

clean:
	rm -f *.o $(OBJS) $(PROGS)
