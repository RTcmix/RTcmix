include ../package.conf

OBJS = sgran.o
CMIXOBJS += $(RTPROFILE_O)
ifeq ($(ARCH),LINUX)
   CFLAGS += -funroll-loops
endif
PROGS = sgran libsgran.so

all: libsgran.so

standalone: sgran

libsgran.so:  $(UGENS_H) $(OBJS)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

sgran:  $(UGENS_H) $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

clean:
	$(RM) *.o $(OBJS) $(PROGS)
