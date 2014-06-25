include ../package.conf

INCLUDES += -I../../../src/rtcmix	# for globals.h

OBJS = wanderfreq.o
IMBCMIXOBJS += $(PROFILE_O)
PROGS = wanderfreq

wanderfreq: $(OBJS) $(IMBCMIXOBJS)
	cd MYWAVETABLE; make
	$(CXX) $(CXXFLAGS) -o $@ $(DYN) $(OBJS) $(IMBCMIXOBJS) $(CXXFLAGS) $(LDFLAGS)

wanderfreq.o:	wanderfreq.cpp $(INCLUDEDIR)/RTcmix.h $(INCLUDEDIR)/Instrument.h

clean:
	cd MYWAVETABLE; make clean
	$(RM) *.o $(OBJS) $(PROGS)
