include ../insts.conf

RTOBJS = START.o BEND.o FRET.o START1.o BEND1.o FRET1.o VSTART1.o \
	 VFRET1.o rtprofile.o
COBJS =  randfill.o sset.o strum.o squisher.o delayset.o delayclean.o \
	dist.o delay.o
CMIXOBJS += $(PROFILE_O)
CFLAGS += -fPIC
CXXFLAGS += -fPIC 
PROGS = STRUM libSTRUM.so

all: libSTRUM.so STRUM

libSTRUM.so: $(RTOBJS) $(COBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(RTOBJS) $(COBJS) \
	$(GENLIB) $(SYSLIBS)

STRUM: $(RTOBJS) $(COBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(RTOBJS) $(CMIXOBJS) $(COBJS) $(LDFLAGS)

$(RTOBJS):	$(INSTRUMENT_H)

clean:
	rm -f *.o $(OBJS) $(PROGS)
