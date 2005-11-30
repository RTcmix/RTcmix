include ../../../makefile.conf

OBJS = RTcmixOSC.o RTOscPField.o glue.o
SRCS = RTcmixOSC.cpp RTOscPField.cpp glue.cpp

# XXX get liblo include and lib paths from configure script

LIBLO = -L/usr/local/lib -llo

CXXFLAGS += -I. -I../../rtcmix -I/usr/local/include/lo

LIBOSC = liboscconn.so

all: $(LIBOSC)

$(LIBOSC): depend $(OBJS)
	$(RM) $@
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(OBJS) $(LIBLO)

glue.o: depend glue.cpp RTOscPField.h
	$(CXX) $(CXXFLAGS) -c -o glue.o glue.cpp

depend:
	-$(SHELL) -ec '$(CXX) -M $(CXXFLAGS) $(SRCS)' > depend

-include depend

install: all
	$(INSTALL) $(LIBOSC) $(LIBDESTDIR)

uninstall:
	$(RM) $(LIBDESTDIR)/$(LIBOSC)

clean:
	$(RM) *.o $(LIBOSC)

cleanall: clean
	$(RM) depend

