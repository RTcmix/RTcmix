include ../package.conf

OBJS = STEREO.o
CMIXOBJS += $(PROFILE_O)
#CFLAGS += -fPIC
#CXXFLAGS += -fPIC 
PROGS = STEREO libSTEREO.so

all: libSTEREO.so

standalone: STEREO

libSTEREO.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

STEREO: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H)

clean:
	$(RM) *.o $(OBJS) $(PROGS)
