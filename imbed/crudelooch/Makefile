include ../package.conf

INCLUDES += -I../../../src/rtcmix	# for globals.h

OBJS = crudelooch.o randfuncs.o
IMBCMIXOBJS += $(PROFILE_O)
PROGS = crudelooch

crudelooch: $(OBJS) $(IMBCMIXOBJS)
	$(CXX) -o $@ $(DYN) $(OBJS) $(IMBCMIXOBJS) $(CXXFLAGS) $(LDFLAGS)

clean:
	$(RM) *.o $(OBJS) $(PROGS)
