include ../package.conf

OBJS = happykeys.o
IMBCMIXOBJS += $(PROFILE_O)
PROGS = happykeys

happykeys: $(OBJS) $(IMBCMIXOBJS)
	$(CXX) -o $@ $(DYN) $(OBJS) $(IMBCMIXOBJS) $(LDFLAGS) -lcurses

clean:
	$(RM) *.o $(OBJS) $(PROGS)
