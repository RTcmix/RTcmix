include ../package.conf

OBJS = fft.o convolve.o
CMIXOBJS += $(RTPROFILE_O)
CFLAGS += -fPIC
CXXFLAGS += -fPIC 
PROGS = convolve libconvolve.so

all: libconvolve.so

standalone: convolve

libconvolve.so:  $(UGENS_H) $(OBJS)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) \
	$(GENLIB) $(SYSLIBS)

convolve: $(UGENS_H) $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

clean:
	rm -f *.o $(OBJS) $(PROGS)
