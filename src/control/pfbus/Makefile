include ../../../makefile.conf

ifeq ($(BUILDTYPE), STANDALONE)
	CONNLIB = libpfbusconn.so
else
	CONNLIB = libpfbus.a
endif

OBJS = PFBusPField.o pfbusglue.o
SRCS = PFBusPField.cpp pfbusglue.cpp

CXXFLAGS += -I. -I../../rtcmix

all: $(CONNLIB)

ifeq ($(BUILDTYPE), STANDALONE)
$(CONNLIB): depend $(OBJS)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(OBJS)
else
$(CONNLIB): $(OBJS)
	$(RM) $@
	ar q $@ $(OBJS)
	$(RANLIB) $@
endif

pfbusglue.o: pfbusglue.cpp PFBusPField.h
	$(CXX) $(CXXFLAGS) -c -o pfbusglue.o pfbusglue.cpp

depend:
	-$(SHELL) -ec '$(CXX) -M $(CXXFLAGS) $(SRCS)' > depend

-include depend

install: all
	$(INSTALL) $(CONNLIB) $(LIBDESTDIR)

uninstall:
	$(RM) $(LIBDESTDIR)/$(CONNLIB)

clean:
	$(RM) *.o *.so $(LIBDESTDIR)/$(CONNLIB) $(CONNLIB)

cleanall: clean
	$(RM) depend

