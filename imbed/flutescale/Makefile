include ../package.conf

INCLUDES += -I../../../src/rtcmix	# for globals.h

OBJS = flutescale.o
IMBCMIXOBJS += $(PROFILE_O)
PROGS = flutescale

flutescale: $(OBJS) $(IMBCMIXOBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(DYN) $(OBJS) $(IMBCMIXOBJS) $(CXXFLAGS) $(LDFLAGS)

clean:
	$(RM) *.o $(OBJS) $(PROGS)
