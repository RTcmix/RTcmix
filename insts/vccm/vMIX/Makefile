include ../package.conf

OBJS = vMIX.o FADE_vMIX.o
PROGS = vMIX libvMIX.so
CMIXOBJS += $(PROFILE_O)

all: libvMIX.so

standalone: vMIX

libvMIX.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

vMIX: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H) $(HEADERS)

clean:
	$(RM) *.o $(OBJS) $(PROGS)

