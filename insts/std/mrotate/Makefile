include ../insts.conf

OBJS = mrotate.o reset.o
CMIXOBJS =  ../../sys/cmix.o ../../lib/genlib.a ../../rtstuff/rtprofile.o
CFLAGS += -fPIC
CXXFLAGS += -fPIC 
PROGS = mrotate libmrotate.so

all: mrotate libmrotate.so

libmrotate.so:  ../../H/ugens.h  $(OBJS)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) \
	../../lib/genlib.a $(SYSLIBS)

mrotate:  ../../H/ugens.h  $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

clean:
	rm -f *.o $(OBJS) $(PROGS)
