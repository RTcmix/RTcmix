include ../insts.conf

OBJS = SROOM.o
CMIXOBJS += $(PROFILE_O)
CFLAGS += -fPIC
CXXFLAGS += -fPIC -Wall -O3
PROGS = libSROOM.so SROOM

all: libSROOM.so #SROOM

libSROOM.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) \
	$(GENLIB) $(SYSLIBS)

SROOM: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H) SROOM.h

clean:
	$(RM) $(OBJS) $(PROGS)
