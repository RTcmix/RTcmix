include ../package.conf

OBJS = DELAY.o
CMIXOBJS += $(PROFILE_O)
#CFLAGS += -fPIC
#CXXFLAGS += -fPIC 
PROGS = DELAY libDELAY.so

all: libDELAY.so

standalone: DELAY

libDELAY.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

DELAY: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H)

clean:
	$(RM) *.o $(OBJS) $(PROGS)
