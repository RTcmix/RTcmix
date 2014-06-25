include ../package.conf

INCLUDES += -I../../../src/rtcmix	# for globals.h

OBJS = happykeys.o
IMBCMIXOBJS += $(PROFILE_O)
PROGS = happykeys

happykeys: $(OBJS) $(IMBCMIXOBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(DYN) $(OBJS) $(IMBCMIXOBJS) $(CXXFLAGS) $(LDFLAGS) -lcurses

clean:
	$(RM) *.o $(OBJS) $(PROGS)
