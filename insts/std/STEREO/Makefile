include ../package.conf

OBJS = STEREO.o
CMIXOBJS += $(PROFILE_O)
PROGS = STEREO libSTEREO.so

all: libSTEREO.so

standalone: STEREO

libSTEREO.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

STEREO: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H) STEREO.h

clean:
	$(RM) *.o $(OBJS) $(PROGS)
