include ../package.conf

OBJS = CLAR.o cfuncs.o
CMIXOBJS += $(PROFILE_O)
PROGS = CLAR libCLAR.so

all: libCLAR.so

standalone: CLAR

libCLAR.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

CLAR: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H)

clean:
	$(RM) *.o $(OBJS) $(PROGS)
