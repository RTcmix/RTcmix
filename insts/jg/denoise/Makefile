include ../package.conf

OBJS = denoise.o cfast.o cfsst.o cfr2tr.o cfr4tr.o cfr4syn.o cford1.o cford2.o
CMIXOBJS += $(RTPROFILE_O)
CFLAGS += -fPIC -Wall -O3
CXXFLAGS += -fPIC -Wall -O3
PROGS = libdenoise.so denoise

all: libdenoise.so

standalone: denoise

libdenoise.so: $(UGENS_H) $(OBJS)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) \
	$(GENLIB) $(SYSLIBS)

denoise: $(UGENS_H) $(CMIXOBJS) $(OBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

clean:
	$(RM) $(OBJS) $(PROGS)
