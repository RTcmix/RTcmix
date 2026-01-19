include ../../makefile.conf

OSCLIB = libosclstnr.a

OBJECTS = RTOSCListener.o
SRCS = RTOSCListener.cpp
SCRIPT = osc_server_init.sco
SCRIPT_PATH = $(LIBDIR)/$(SCRIPT)

CXXFLAGS += -I. -I../parser $(LIBLO_CFLAGS)

all: $(OSCLIB)

$(OSCLIB): $(OBJECTS)
	$(RM) $@
	ar q $@ $(OBJECTS)
	$(RANLIB) $@

RTOSCListener.o: RTOSCListener.cpp RTOSCListener.h
	$(CXX) $(CXXFLAGS) -DSCRIPT_PATH=\"$(SCRIPT_PATH)\" -c -o $@ RTOSCListener.cpp

install: all
	$(INSTALL) $(SCRIPT) $(LIBDIR)

uninstall:
	$(RM) $(LIBDIR)/$(SCRIPT)

clean:
	$(RM) *.o $(OSCLIB)

cleanall: clean
	$(RM) depend

