include ../package.conf

OBJS = rotate.o
CMIXOBJS += $(RTPROFILE_O)
CFLAGS += -fPIC
CXXFLAGS += -fPIC 
PROGS = rotate librotate.so

all: librotate.so

standalone: rotate

librotate.so:  $(UGENS_H) $(OBJS)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) \
	$(GENLIB) $(SYSLIBS)

rotate:  $(UGENS_H) $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

clean:
	rm -f *.o $(OBJS) $(PROGS)
