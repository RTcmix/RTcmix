include ../package.conf

RTOBJS = START.o BEND.o FRET.o START1.o BEND1.o FRET1.o VSTART1.o \
	 VFRET1.o rtprofile.o
COBJS =  randfill.o sset.o strum.o squisher.o delayset.o delayclean.o \
	dist.o delay.o
CMIXOBJS += $(PROFILE_O)
PROGS = STRUM libSTRUM.so

all: libSTRUM.so

standalone: STRUM

libSTRUM.so: $(RTOBJS) $(COBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(RTOBJS) $(COBJS) $(GENLIB) $(SYSLIBS)

STRUM: $(RTOBJS) $(COBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(RTOBJS) $(CMIXOBJS) $(COBJS) $(LDFLAGS)

$(RTOBJS):	$(INSTRUMENT_H)

clean:
	$(RM) *.o $(OBJS) $(PROGS)
