include ../insts.conf   

OBJS = WAVESHAPE.o
CMIXOBJS = ../../sys/cmix.o ../../lib/genlib.a ../../rtstuff/profile.o
CFLAGS += -fPIC
CXXFLAGS += -fPIC 
PROGS = WAVESHAPE libWAVESHAPE.so

all:  WAVESHAPE libWAVESHAPE.so

libWAVESHAPE.so: $(OBJS) ../../lib/genlib.a
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) \
	../../lib/genlib.a $(SYSLIBS)

WAVESHAPE: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): ../../rtstuff/Instrument.h

clean:
	rm -f *.o $(OBJS) $(PROGS)
