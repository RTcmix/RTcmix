include ../insts.conf

OBJS = mrotate.o
CMIXOBJS += $(RTPROFILE_O)
CFLAGS += -fPIC
CXXFLAGS += -fPIC 
PROGS = mrotate libmrotate.so

all: libmrotate.so mrotate

libmrotate.so:  $(UGENS_H) $(OBJS)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) \
	$(GENLIB) $(SYSLIBS)

mrotate:  $(UGENS_H) $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

clean:
	rm -f *.o $(OBJS) $(PROGS)
