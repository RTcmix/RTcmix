include ../insts.conf

OBJS = rotate.o reset.o
CMIXOBJS =  ../../sys/cmix.o ../../lib/genlib.a ../../rtstuff/rtprofile.o
CFLAGS += -fPIC
CXXFLAGS += -fPIC 
PROGS = rotate librotate.so

all: rotate librotate.so

librotate.so:  ../../H/ugens.h  $(OBJS)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) \
	../../lib/genlib.a $(SYSLIBS)

rotate:  ../../H/ugens.h  $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

clean:
	rm -f *.o $(OBJS) $(PROGS)
