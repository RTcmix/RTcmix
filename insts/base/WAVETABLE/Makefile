include ../insts.conf

OBJS = WAVETABLE.o reset.o 
CMIXOBJS = ../../sys/cmix.o ../../lib/genlib.a 
CFLAGS = -fPIC
CXXFLAGS = -fPIC 
PROGS = WAVETABLE libWAVETABLE.so

all: WAVETABLE libWAVETABLE.so

libWAVETABLE.so: $(OBJS) ../../lib/genlib.a
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) \
	../../lib/genlib.a $(SYSLIBS)

WAVETABLE: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): ../../rtstuff/Instrument.h

clean:
	rm -f *.o $(OBJS) $(PROGS)
