include ../package.conf

OBJS = TRANS.o
CMIXOBJS += $(PROFILE_O)
PROGS = TRANS libTRANS.so

all: libTRANS.so

standalone: TRANS

libTRANS.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

TRANS: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H) TRANS.h

clean:
	$(RM) *.o $(OBJS) $(PROGS)
