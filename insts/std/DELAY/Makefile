include ../insts.conf

OBJS = DELAY.o reset.o
CFLAGS += -fPIC
CXXFLAGS += -fPIC 
PROGS = DELAY libDELAY.so

all: libDELAY.so DELAY

libDELAY.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) \
	$(GENLIB) $(SYSLIBS)

DELAY: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H)

clean:
	rm -f *.o $(OBJS) $(PROGS)
