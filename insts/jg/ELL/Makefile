include ../package.conf

OBJS = ELL.o ellipse.o setell.o extramath.o
#CFLAGS += -fPIC
#CXXFLAGS += -fPIC
SYSLIBS += -lf2c
PROGS = libELL.so ELL

all: libELL.so

standalone: ELL

libELL.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

ELL: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H) ELL.h elldefs.h

clean:
	$(RM) $(OBJS) $(PROGS)
