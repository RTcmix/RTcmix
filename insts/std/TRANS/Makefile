include ../package.conf

OBJS = TRANS.o
CMIXOBJS += $(PROFILE_O)
#CFLAGS += -fPIC
#CXXFLAGS += -fPIC 
PROGS = TRANS libTRANS.so

all: libTRANS.so

standalone: TRANS

libTRANS.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) \
	$(GENLIB) $(SYSLIBS)

TRANS: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H) TRANS.h

clean:
	$(RM) *.o $(OBJS) $(PROGS)
