include ../insts.conf

OBJS = gravy.o reset.o minout.o
CMIXOBJS =  ../../sys/cmix.o ../../lib/genlib.a ../../rtstuff/rtprofile.o
CFLAGS += -fPIC
CXXFLAGS += -fPIC 
PROGS = gravy libgravy.so

all: gravy libgravy.so

libgravy.so:  ../../H/ugens.h  $(OBJS)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) \
	../../lib/genlib.a $(SYSLIBS)

gravy:  ../../H/ugens.h  $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

clean:
	rm -f *.o $(OBJS) $(PROGS)
