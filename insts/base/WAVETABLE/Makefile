include ../package.conf

OBJS = WAVETABLE.o
CMIXOBJS += $(PROFILE_O)
PROGS = WAVETABLE libWAVETABLE.so

all: libWAVETABLE.so

standalone:  WAVETABLE

libWAVETABLE.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

WAVETABLE: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H)

clean:
	$(RM) *.o $(OBJS) $(PROGS)
