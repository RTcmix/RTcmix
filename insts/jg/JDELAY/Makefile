include ../package.conf

OBJS = JDELAY.o
CMIXOBJS += $(PROFILE_O)
#CFLAGS += -fPIC
#CXXFLAGS += -fPIC
PROGS = libJDELAY.so JDELAY

all: libJDELAY.so

standalone: JDELAY

libJDELAY.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

JDELAY: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H) JDELAY.h

clean:
	$(RM) $(OBJS) $(PROGS)
