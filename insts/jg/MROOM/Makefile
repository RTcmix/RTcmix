include ../package.conf

OBJS = MROOM.o timeset.o
#CFLAGS += -fPIC
#CXXFLAGS += -fPIC
PROGS = libMROOM.so MROOM

all: libMROOM.so

standalone: MROOM

libMROOM.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

MROOM: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H) MROOM.h timeset.h

clean:
	$(RM) $(OBJS) $(PROGS)
