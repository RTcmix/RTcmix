include ../package.conf

OBJS = HOLD.o STORE.o FADE.o rtprofile.o
PROGS = HAR libHAR.so
CMIXOBJS += $(PROFILE_O)

all: libHAR.so

standalone: HAR

libHAR.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

HAR: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H) $(HEADERS)

clean:
	$(RM) *.o $(OBJS) $(PROGS)

