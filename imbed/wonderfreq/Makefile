include ../package.conf

INCLUDES += -I../../../src/rtcmix	# for globals.h

OBJS = wonderfreq.o
IMBCMIXOBJS += $(PROFILE_O)
PROGS = wonderfreq

wonderfreq: $(OBJS) $(IMBCMIXOBJS) MYWAVETABLE/MYWAVETABLE.so
	$(CXX) -o $@ $(DYN) $(OBJS) $(IMBCMIXOBJS) $(LDFLAGS)

MYWAVETABLE/MYWAVETABLE.so:
	cd MYWAVETABLE; make

wonderfreq.o:	wonderfreq.cpp $(INCLUDEDIR)/RTcmix.h $(INCLUDEDIR)/Instrument.h

clean:
	cd MYWAVETABLE; make clean
	$(RM) *.o $(OBJS) $(PROGS)
