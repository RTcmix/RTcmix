include ../package.conf

OBJS = PANECHO.o
CMIXOBJS += $(PROFILE_O)
PROGS = PANECHO libPANECHO.so

all: libPANECHO.so

standalone: PANECHO

libPANECHO.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

PANECHO: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H)

clean:
	$(RM) *.o $(OBJS) $(PROGS)
