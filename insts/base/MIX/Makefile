include ../package.conf

OBJS = MIX.o
CMIXOBJS += $(PROFILE_O)
PROGS = MIX
ifeq ($(ARCH),LINUX)
   DYN = -rdynamic
else
   DYN =
endif

all: MIX

MIX: $(OBJS) $(CMIXOBJS)
	$(CXX) -o MIX $(DYN) $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H)

clean:
	$(RM) *.o $(OBJS) $(PROGS)

