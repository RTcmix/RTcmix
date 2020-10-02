include ../../makefile.conf

OBJS = RTOSCListener.o
SRCS = RTOSCListener.cpp

CXXFLAGS += -I. -I../parser $(LIBLO_CFLAGS)

LIBOSCLSTNR = libosclstnr.dylib


all: libosclstnr.dylib
#all: $(LIBOSCLSTNR)
#For some reason when I try to use the $(LIBOSCLSTNR) macro
#in the all target, it says everything is made and doesn't
#do anything.

$(LIBOSCLSTNR): $(OBJS)
	$(RM) $@
	$(CXX) $(DYNAMIC_LDFLAGS) -o $@ $(OBJS) $(LIBLO_LIBS)

RTOSCListener.o: RTOSCListener.cpp RTOSCListener.h
	$(CXX) $(CXXFLAGS) -c -o $@ RTOSCListener.cpp 

install: all
	$(INSTALL) $(LIBOSCLSTNR) $(LIBDESTDIR)

uninstall:
	$(RM) $(LIBDESTDIR)/$(LIBOSCLSTNR)

clean:
	$(RM) *.o $(LIBOSCLSTNR)

cleanall: clean
	$(RM) depend

