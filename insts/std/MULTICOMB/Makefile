include ../package.conf

OBJS = MULTICOMB.o
CMIXOBJS += $(PROFILE_O)
#CFLAGS += -fPIC
#CXXFLAGS += -fPIC 
PROGS = MULTICOMB libMULTICOMB.so

all: libMULTICOMB.so

standalone: MULTICOMB

libMULTICOMB.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

MULTICOMB: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H)

clean:
	$(RM) *.o $(OBJS) $(PROGS)
