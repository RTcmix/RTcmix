include ../insts.conf

OBJS = wow.o
CMIXOBJS += $(RTPROFILE_O)
CFLAGS += -fPIC
CXXFLAGS += -fPIC 
PROGS = wow libwow.so

all: libwow.so wow

libwow.so:  $(UGENS_H) $(OBJS)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) \
	$(GENLIB) $(SYSLIBS)

wow:  $(UGENS_H) $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

clean:
	rm -f *.o $(OBJS) $(PROGS)
