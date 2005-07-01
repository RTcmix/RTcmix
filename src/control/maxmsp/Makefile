include ../../../makefile.conf

OBJS = RTInletPField.o inletglue.o
SRCS = RTInletPField.cpp inletglue.cpp

CXXFLAGS += -I. -I../../rtcmix
LIBINLET = libinlet.a

all: $(LIBINLET)

$(LIBINLET): $(OBJS)
	$(LD) -r $(OBJS) -o $@

inletglue.o: inletglue.cpp RTInletPField.h
	$(CXX) $(CXXFLAGS) -c -o inletglue.o inletglue.cpp

install: all
#	$(INSTALL) $(LIBINLET) $(LIBDIR)

uninstall:
#	$(RM) $(LIBDIR)/$(LIBINLET)

clean:
	$(RM) *.o $(LIBINLET)

cleanall: clean

