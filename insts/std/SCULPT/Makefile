include ../insts.conf

OBJS = SCULPT.o
CMIXOBJS = ../../sys/cmix.o ../../lib/genlib.a ../../rtstuff/profile.o
CFLAGS += -fPIC
CXXFLAGS += -fPIC 
PROGS = SCULPT libSCULPT.so

all: SCULPT libSCULPT.so

libSCULPT.so: $(OBJS) ../../lib/genlib.a
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) \
	../../lib/genlib.a $(SYSLIBS)

SCULPT: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): ../../rtstuff/Instrument.h

clean:
	rm -f *.o $(OBJS) $(PROGS)
