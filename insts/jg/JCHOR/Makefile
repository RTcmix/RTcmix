include ../package.conf

OBJS = JCHOR.o
CMIXOBJS += $(PROFILE_O)
CFLAGS += -fPIC -O3
CXXFLAGS += -fPIC -Wall -O3
PROGS = libJCHOR.so JCHOR

all: libJCHOR.so

standalone: JCHOR

libJCHOR.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) \
	$(GENLIB) $(SYSLIBS)

JCHOR: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H) JCHOR.h

clean:
	$(RM) $(OBJS) $(PROGS)
