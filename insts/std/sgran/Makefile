include ../insts.conf

OBJS = sgran.o setline.o minout.o
CMIXOBJS =  ../../sys/cmix.o ../../lib/genlib.a ../../rtstuff/rtprofile.o
CFLAGS += -fPIC
CXXFLAGS += -fPIC 
PROGS = sgran libsgran.so

all: sgran libsgran.so

libsgran.so:  ../../H/ugens.h  $(OBJS)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) \
	../../lib/genlib.a $(SYSLIBS)

sgran:  ../../H/ugens.h  $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

clean:
	rm -f *.o $(OBJS) $(PROGS)
