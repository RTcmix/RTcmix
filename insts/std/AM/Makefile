include ../insts.conf

OBJS = AM.o reset.o
CMIXOBJS = ../../sys/cmix.o ../../lib/genlib.a
CFLAGS += -fPIC
CXXFLAGS += -fPIC 
PROGS = AM libAM.so

all: AM libAM.so

libAM.so: $(OBJS) ../../lib/genlib.a
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) \
	../../lib/genlib.a $(SYSLIBS)

AM: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): ../../rtstuff/Instrument.h

clean:
	rm -f *.o $(OBJS) $(PROGS)
