include ../package.conf

OBJS = INPUTSIG.o NOISE.o BUZZ.o PULSE.o cfuncs.o rtprofile.o
HEADERS = INPUTSIG.h NOISE.h BUZZ.h PULSE.h
#CFLAGS += -fPIC
#CXXFLAGS += -fPIC 
PROGS = IIR libIIR.so

all: libIIR.so

standalone: IIR

libIIR.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

IIR: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H) $(HEADERS)

clean:
	$(RM) *.o $(OBJS) $(PROGS)
