include ../insts.conf

OBJS = FMINST.o
CMIXOBJS += $(PROFILE_O)
CFLAGS += -fPIC
CXXFLAGS += -fPIC
PROGS = FMINST libFMINST.so

all: libFMINST.so FMINST

libFMINST.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) \
	$(GENLIB) $(SYSLIBS)

FMINST: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H)

clean:
	rm -f *.o $(OBJS) $(PROGS)
