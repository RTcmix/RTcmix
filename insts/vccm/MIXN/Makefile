include ../package.conf

OBJS = MIXN.o
CMIXOBJS += 
#CFLAGS += -fPIC
#CXXFLAGS += -fPIC
PROGS = MIXN libMIXN.so

all: libMIXN.so

standalone: MIXN

libMIXN.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

MIXN: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H)

clean:
	$(RM) *.o $(OBJS) $(PROGS)
