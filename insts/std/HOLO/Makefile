include ../package.conf

OBJS = HOLO.o
CMIXOBJS += $(PROFILE_O)
#CFLAGS += -fPIC
#CXXFLAGS += -fPIC 
PROGS = HOLO libHOLO.so

all: libHOLO.so

standalone: HOLO

libHOLO.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

HOLO: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H)

clean:
	$(RM) *.o $(OBJS) $(PROGS)
