include ../package.conf

OBJS = COMBIT.o
CMIXOBJS += $(PROFILE_O)
PROGS = COMBIT libCOMBIT.so

all: libCOMBIT.so

standalone: COMBIT

libCOMBIT.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

COMBIT: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H)

clean:
	$(RM) *.o $(OBJS) $(PROGS)
