include ../insts.conf

OBJS = MIX.o cfuncs.o
CMIXOBJS = ../../sys/cmix.o ../../lib/genlib.a
CFLAGS += -fPIC
CXXFLAGS += -fPIC 
PROGS = MIX

all: MIX

MIX: $(OBJS) $(CMIXOBJS)
	$(CXX) -o MIX -rdynamic $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): ../../rtstuff/Instrument.h

clean:
	rm -f *.o $(OBJS) $(PROGS)
