include ../package.conf

INCLUDES += -I../../../src/rtcmix	# for globals.h

OBJS = mixit.o randfuncs.o
IMBCMIXOBJS += $(PROFILE_O)
PROGS = mixit

mixit: $(OBJS) $(IMBCMIXOBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(DYN) $(OBJS) $(IMBCMIXOBJS) $(CXXFLAGS) $(LDFLAGS)

clean:
	$(RM) *.o $(OBJS) $(PROGS)
