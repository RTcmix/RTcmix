include ../package.conf

OBJLIBDIR = ../objlib
OBJLIB_A = $(OBJLIBDIR)/objlib.a
OBJLIB_H = $(OBJLIBDIR)/objlib.h

OBJS = FLANGE.o
CMIXOBJS += $(PROFILE_O)
CFLAGS += -fPIC
CXXFLAGS += -fPIC -I$(OBJLIBDIR) -Wall
PROGS = FLANGE libFLANGE.so

all: libFLANGE.so

standalone: FLANGE

libFLANGE.so: $(OBJS) $(GENLIB) $(OBJLIB_A)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) \
	$(GENLIB) $(SYSLIBS) $(OBJLIB_A)

FLANGE: $(OBJS) $(CMIXOBJS) $(OBJLIB_A)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(OBJLIB_A) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H) FLANGE.h

clean:
	$(RM) $(OBJS) $(PROGS)

