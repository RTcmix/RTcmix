include ../insts.conf

OBJS = DEL1.o reset.o
CFLAGS += -fPIC
CXXFLAGS += -fPIC 
PROGS = DEL1 libDEL1.so

all: libDEL1.so DEL1

libDEL1.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) \
	$(GENLIB) $(SYSLIBS)

DEL1: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H)

clean:
	rm -f *.o $(OBJS) $(PROGS)
