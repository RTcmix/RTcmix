include ../package.conf

OBJS = arpeggiate.o
IMBCMIXOBJS += $(PROFILE_O)
PROGS = arpeggiate

arpeggiate: $(OBJS) $(IMBCMIXOBJS)
	$(CXX) -o $@ $(DYN) $(OBJS) $(IMBCMIXOBJS) $(LDFLAGS)

clean:
	$(RM) *.o $(OBJS) $(PROGS)
