include ../package.conf

OBJS = mixit.o randfuncs.o
IMBCMIXOBJS += $(PROFILE_O)
PROGS = mixit

mixit: $(OBJS) $(IMBCMIXOBJS)
	$(CXX) -o $@ $(DYN) $(OBJS) $(IMBCMIXOBJS) $(LDFLAGS)

clean:
	$(RM) *.o $(OBJS) $(PROGS)
