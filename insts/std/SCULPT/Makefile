include ../package.conf

OBJS = SCULPT.o
CMIXOBJS += $(PROFILE_O)
PROGS = SCULPT libSCULPT.so

all: libSCULPT.so

standalone: SCULPT

libSCULPT.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

SCULPT: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H)

clean:
	$(RM) *.o $(OBJS) $(PROGS)
