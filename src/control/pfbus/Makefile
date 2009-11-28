include ../../../makefile.conf

CONNLIB = libpfbusconn.so

OBJS = PFBusPfield.o pfbusglue.o
SRCS = PFBusPfield.cpp pfbusglue.cpp

CXXFLAGS += -I. -I../../rtcmix

all: $(CONNLIB)

$(CONNLIB): depend $(OBJS)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(OBJS)

pfbusglue.o: pfbusglue.cpp PFBusPfield.h
	$(CXX) $(CXXFLAGS) -c -o pfbusglue.o pfbusglue.cpp

depend:
	-$(SHELL) -ec '$(CXX) -M $(CXXFLAGS) $(SRCS)' > depend

-include depend

install: all
	$(INSTALL) $(CONNLIB) $(LIBDESTDIR)

uninstall:
	$(RM) $(LIBDESTDIR)/$(CONNLIB)

clean:
	$(RM) *.o $(LIBDESTDIR)/$(CONNLIB) $(CONNLIB)

cleanall: clean
	$(RM) depend

