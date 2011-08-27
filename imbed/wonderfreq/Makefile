include ../package.conf

OBJS = wonderfreq.o
IMBCMIXOBJS += $(PROFILE_O)
PROGS = wonderfreq

wonderfreq: $(OBJS) $(IMBCMIXOBJS) MYWAVETABLE/MYWAVETABLE.so
	$(CXX) $(CXXFLAGS) -o $@ $(DYN) $(OBJS) $(IMBCMIXOBJS) $(LDFLAGS)

MYWAVETABLE/MYWAVETABLE.so:
	cd MYWAVETABLE; make

wonderfreq.o:	wonderfreq.cpp $(INCLUDEDIR)/RTcmix.h $(INCLUDEDIR)/Instrument.h

clean:
	cd MYWAVETABLE; make clean
	$(RM) *.o $(OBJS) $(PROGS)
