include ../insts.conf

OBJS = ELL.o ellipse.o setell.o extramath.o
CFLAGS += -fPIC -Wall -O3
CXXFLAGS += -fPIC -Wall -O3
SYSLIBS += -lf2c
PROGS = libELL.so ELL

all: libELL.so #ELL

libELL.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) \
	$(GENLIB) $(SYSLIBS)

ELL: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H) ELL.h elldefs.h

clean:
	$(RM) $(OBJS) $(PROGS)
