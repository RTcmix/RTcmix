include ../package.conf

OBJS = MIX.o
CMIXOBJS += $(PROFILE_O)
CFLAGS += -fPIC
CXXFLAGS += -fPIC 
PROGS = MIX

all: MIX

MIX: $(OBJS) $(CMIXOBJS)
	$(CXX) -o MIX -rdynamic $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H)

clean:
	rm -f *.o $(OBJS) $(PROGS)

