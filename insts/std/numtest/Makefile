include ../insts.conf

OBJS = numtest.o
CMIXOBJS =  ../../sys/cmix.o ../../lib/genlib.a ../../rtstuff/rtprofile.o
CFLAGS += -fPIC
CXXFLAGS += -fPIC 
PROGS = numtest

all:  numtest

numtest:  ../../H/ugens.h  $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

clean:
	rm -f *.o $(OBJS) $(PROGS)
