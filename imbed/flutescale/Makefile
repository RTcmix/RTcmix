include ../../../makefile.conf

INCLUDES += -I../../../src/rtcmix	# for globals.h

OBJS = flutescale.o
IMBCMIXOBJS += $(PROFILE_O)
PROGS = flutescale

flutescale: $(OBJS) $(IMBCMIXOBJS)
	$(CXX) -o $@ $(DYN) $(OBJS) $(IMBCMIXOBJS) $(LDFLAGS)

clean:
	$(RM) *.o $(OBJS) $(PROGS)
