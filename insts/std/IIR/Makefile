include ../insts.conf

OBJS = INPUTSIG.o NOISE.o BUZZ.o PULSE.o cfuncs.o rtprofile.o
CMIXOBJS += $(PROFILE_O)
CFLAGS += -fPIC
CXXFLAGS += -fPIC 
PROGS = IIR libIIR.so

all: libIIR.so IIR

libIIR.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) \
	$(GENLIB) $(SYSLIBS)

IIR: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H)

clean:
	rm -f *.o $(OBJS) $(PROGS)
