include ../package.conf

OBJS = wanderfreq.o
IMBCMIXOBJS += $(PROFILE_O)
PROGS = wanderfreq

wanderfreq: $(OBJS) $(IMBCMIXOBJS)
	cd MYWAVETABLE; make
	$(CXX) -o $@ $(DYN) $(OBJS) $(IMBCMIXOBJS) $(LDFLAGS)

wanderfreq.o:	wanderfreq.C ../../H/RTcmix.h ../../H/Instrument.h

clean:
	cd MYWAVETABLE; make clean
	$(RM) *.o $(OBJS) $(PROGS)
