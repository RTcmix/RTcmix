include ../package.conf

OBJS = MULTICOMB.o
CMIXOBJS += $(PROFILE_O)
PROGS = MULTICOMB libMULTICOMB.so

all: libMULTICOMB.so

standalone: MULTICOMB

libMULTICOMB.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

MULTICOMB: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H)

clean:
	$(RM) *.o $(OBJS) $(PROGS)
