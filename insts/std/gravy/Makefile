include ../package.conf

OBJS = gravy.o
CMIXOBJS += $(RTPROFILE_O)
PROGS = gravy libgravy.so

all: libgravy.so

standalone:  gravy

libgravy.so:  $(UGENS_H) $(OBJS)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

gravy:  $(UGENS_H) $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

clean:
	$(RM) *.o $(OBJS) $(PROGS)
