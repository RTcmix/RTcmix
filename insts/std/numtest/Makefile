include ../package.conf

OBJS = numtest.o
CMIXOBJS += $(RTPROFILE_O)
#CFLAGS += -fPIC
#CXXFLAGS += -fPIC 
PROGS = numtest libnumtest.so

all:  libnumtest.so

standalone: numtest

libnumtest.so:  $(UGENS_H) $(OBJS)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

numtest:  $(UGENS_H) $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

clean:
	$(RM) *.o $(OBJS) $(PROGS)
