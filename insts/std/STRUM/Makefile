include ../insts.conf

RTOBJS = START.o BEND.o FRET.o START1.o BEND1.o FRET1.o VSTART1.o \
	 VFRET1.o rtprofile.o
COBJS =  randfill.o sset.o strum.o squisher.o delayset.o delayclean.o \
	dist.o delay.o
CMIXOBJS = ../../sys/cmix.o ../../lib/genlib.a ../../rtstuff/profile.o
CFLAGS += -fPIC
CXXFLAGS += -fPIC 
PROGS = STRUM libSTRUM.so

all: STRUM libSTRUM.so

libSTRUM.so: $(RTOBJS) $(COBJS) ../../lib/genlib.a
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(RTOBJS) $(COBJS) \
	../../lib/genlib.a $(SYSLIBS)

STRUM: $(RTOBJS) $(COBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(RTOBJS) $(CMIXOBJS) $(COBJS) $(LDFLAGS)

$(RTOBJS):	../../rtstuff/Instrument.h

clean:
	rm -f *.o $(OBJS) $(PROGS)
