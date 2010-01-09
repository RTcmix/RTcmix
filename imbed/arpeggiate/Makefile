include ../package.conf

OBJS = arpeggiate.o
INCLUDES += -I../../../src/rtcmix	# for globals.h

IMBCMIXOBJS += $(PROFILE_O)
PROGS = arpeggiate

arpeggiate: $(OBJS) $(IMBCMIXOBJS)
	$(CXX) -o $@ $(DYN) $(OBJS) $(IMBCMIXOBJS) $(CXXFLAGS) $(LDFLAGS)

clean:
	$(RM) *.o $(OBJS) $(PROGS)
