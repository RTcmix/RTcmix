include ../package.conf

OBJLIBDIR = ../objlib
OBJLIB_A = $(OBJLIBDIR)/objlib.a
OBJLIB_H = $(OBJLIBDIR)/objlib.h

OBJS = COMPLIMIT.o
CMIXOBJS += $(PROFILE_O)
CXXFLAGS += -I$(OBJLIBDIR) -Wall
PROGS = libCOMPLIMIT.so COMPLIMIT

all: libCOMPLIMIT.so

standalone: COMPLIMIT

libCOMPLIMIT.so: $(OBJS) $(GENLIB) $(OBJLIB_A)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) \
	$(GENLIB) $(SYSLIBS) $(OBJLIB_A)

COMPLIMIT: $(OBJS) $(CMIXOBJS) $(OBJLIB_A)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(OBJLIB_A) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H) COMPLIMIT.h

clean:
	$(RM) $(OBJS) $(PROGS)
