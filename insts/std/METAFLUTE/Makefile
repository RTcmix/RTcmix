include ../package.conf

OBJS = SFLUTE.o BSFLUTE.o VSFLUTE.o LSFLUTE.o sflcfuncs.o rtprofile.o
CMIXOBJS += $(PROFILE_O)
CFLAGS += -fPIC
CXXFLAGS += -fPIC 
PROGS = METAFLUTE libMETAFLUTE.so

all: libMETAFLUTE.so

standalone: METAFLUTE

libMETAFLUTE.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) \
	$(GENLIB) $(SYSLIBS)

METAFLUTE: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H)

clean:
	rm -f *.o $(OBJS) $(PROGS)
