include ../package.conf

OBJS = wanderfreq.o
IMBCMIXOBJS += $(PROFILE_O)
PROGS = wanderfreq

wanderfreq: $(OBJS) $(IMBCMIXOBJS)
	cd MYWAVETABLE; make
	$(CXX) -o $@ $(DYN) $(OBJS) $(IMBCMIXOBJS) $(LDFLAGS)

clean:
	cd MYWAVETABLE; make clean
	$(RM) *.o $(OBJS) $(PROGS)
