include ../package.conf

OBJS = TRANSBEND.o
CMIXOBJS += $(PROFILE_O)
#CFLAGS += -fPIC
#CXXFLAGS += -fPIC 
PROGS = TRANSBEND libTRANSBEND.so

all: libTRANSBEND.so

standalone: TRANSBEND

libTRANSBEND.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) \
	$(GENLIB) $(SYSLIBS)

TRANSBEND: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H) TRANSBEND.h

clean:
	$(RM) *.o $(OBJS) $(PROGS)
