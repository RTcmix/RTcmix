include ../insts.conf

OBJS = DEL1.o reset.o
CMIXOBJS = ../../sys/cmix.o ../../lib/genlib.a
CFLAGS += -fPIC
CXXFLAGS += -fPIC 
PROGS = DEL1 libDEL1.so

all: DEL1 libDEL1.so

libDEL1.so: $(OBJS) ../../lib/genlib.a
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) \
	../../lib/genlib.a $(SYSLIBS)

DEL1: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): ../../rtstuff/Instrument.h

clean:
	rm -f *.o $(OBJS) $(PROGS)
