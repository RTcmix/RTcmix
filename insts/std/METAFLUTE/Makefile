include ../insts.conf

OBJS = SFLUTE.o BSFLUTE.o VSFLUTE.o LSFLUTE.o sflcfuncs.o rtprofile.o
CMIXOBJS = ../../sys/cmix.o ../../lib/genlib.a
CFLAGS += -fPIC
CXXFLAGS += -fPIC 
PROGS = METAFLUTE libMETAFLUTE.so

all: METAFLUTE libMETAFLUTE.so

libMETAFLUTE.so: $(OBJS) ../../lib/genlib.a
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) \
	../../lib/genlib.a $(SYSLIBS)

METAFLUTE: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): ../../rtstuff/Instrument.h

clean:
	rm -f *.o $(OBJS) $(PROGS)
