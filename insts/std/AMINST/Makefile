include ../package.conf

OBJS = AMINST.o
CMIXOBJS += $(PROFILE_O)
CFLAGS += -fPIC
CXXFLAGS += -fPIC 
PROGS = AMINST libAMINST.so

all: libAMINST.so AMINST

libAMINST.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) \
	$(GENLIB) $(SYSLIBS)

AMINST: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H)

clean:
	rm -f *.o $(OBJS) $(PROGS)
