include ../package.conf

OBJLIBDIR = ../objlib
OBJLIB_A = $(OBJLIBDIR)/objlib.a
OBJLIB_H = $(OBJLIBDIR)/objlib.h

OBJS = JFIR.o
CMIXOBJS += $(PROFILE_O)
#CFLAGS += -fPIC
#CXXFLAGS += -fPIC -I$(OBJLIBDIR) -O3 -Wall
CXXFLAGS += -I$(OBJLIBDIR)
PROGS = libJFIR.so JFIR

all: libJFIR.so

standalone: JFIR

libJFIR.so: $(OBJS) $(GENLIB) $(OBJLIB_A)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) \
	$(GENLIB) $(SYSLIBS) $(OBJLIB_A)

JFIR: $(OBJS) $(CMIXOBJS) $(OBJLIB_A)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(OBJLIB_A) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H) JFIR.h

clean:
	$(RM) $(OBJS) $(PROGS)

