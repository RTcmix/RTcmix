include ../package.conf

OBJS = crudelooch.o randfuncs.o
IMBCMIXOBJS += $(PROFILE_O)
PROGS = crudelooch

crudelooch: $(OBJS) $(IMBCMIXOBJS)
	$(CXX) -o $@ $(DYN) $(OBJS) $(IMBCMIXOBJS) $(LDFLAGS)

clean:
	$(RM) *.o $(OBJS) $(PROGS)
