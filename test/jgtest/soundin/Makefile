include ../../../insts.jg/package.conf

OBJLIBDIR = ../../../insts.jg/objlib
OBJLIB_A = $(OBJLIBDIR)/objlib.a
OBJLIB_H = $(OBJLIBDIR)/objlib.h

OBJS = soundin.o getfilename.o
CXXFLAGS += -I$(OBJLIBDIR)
PROGS = soundin

all: soundin

soundin: $(OBJS) $(CMIXOBJS) $(OBJLIB_A)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(OBJLIB_A) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H) soundin.h getfilename.h

clean:
	$(RM) $(OBJS) $(PROGS)

