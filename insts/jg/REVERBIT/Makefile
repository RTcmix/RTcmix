include ../package.conf

OBJS = REVERBIT.o
CMIXOBJS += $(PROFILE_O)
#CFLAGS += -fPIC
#CXXFLAGS += -fPIC
PROGS = libREVERBIT.so REVERBIT

all: libREVERBIT.so

standalone: REVERBIT

libREVERBIT.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

REVERBIT: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H) REVERBIT.h

clean:
	$(RM) $(OBJS) $(PROGS)
