include ../package.conf

INCLUDES += -I../../../src/rtcmix	# for globals.h

OBJS = loochinspace.o randfuncs.o
IMBCMIXOBJS += $(PROFILE_O)
PROGS = crudelooch

loochinspace: $(OBJS) $(IMBCMIXOBJS)
	$(CXX) -o $@ $(DYN) $(OBJS) $(IMBCMIXOBJS) $(CXXFLAGS) $(LDFLAGS)

clean:
	$(RM) *.o $(OBJS) $(PROGS)
