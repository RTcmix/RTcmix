include ../package.conf

OBJS = SROOM.o
CMIXOBJS += $(PROFILE_O)
#CFLAGS += -fPIC
#CXXFLAGS += -fPIC
PROGS = libSROOM.so SROOM

all: libSROOM.so

standalone: SROOM

libSROOM.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

SROOM: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H) SROOM.h

clean:
	$(RM) $(OBJS) $(PROGS)
