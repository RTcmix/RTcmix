include ../../../makefile.conf

OBJS = RTcmixMouse.o RTMousePField.o glue.o
SRCS = RTcmixMouse.cpp RTMousePField.cpp glue.cpp
ifeq ($(ARCH), MACOSX)
   OBJS += OSXMouse.o
   SRCS += OSXMouse.cpp
   GUILIBS = 
else
   OBJS += XMouse.o
   SRCS += XMouse.cpp
   GUILIBS = -L$(XLIBDIR) -lX11
endif
CXXFLAGS += -I. -I../../rtcmix
LIBMOUSE = libmouseconn.$(SHLIB_SUFFIX)
TEST = test
PFIELD = ../../rtcmix/PField.o ../../rtcmix/RefCounted.o
GENLIB = -Wl,-rpath ../../../lib -L../../../lib -lgen

all: $(LIBMOUSE)

$(LIBMOUSE): depend $(OBJS)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(OBJS) $(GUILIBS)

glue.o: depend glue.cpp RTMousePField.h
	$(CXX) $(CXXFLAGS) -c -o glue.o glue.cpp

$(TEST).o: depend $(TEST).cpp RTMousePField.h
	$(CXX) $(CXXFLAGS) -c -o $(TEST).o $(TEST).cpp

$(TEST): depend $(TEST).o $(LIBMOUSE)
	$(CXX) $(CXXFLAGS) $(TEST).o -o $(TEST) $(LIBMOUSE) $(PFIELD) $(GENLIB) \
		$(GUILIBS) -lpthread

depend:
	-$(SHELL) -ec '$(CXX) -M $(CXXFLAGS) $(TEST).cpp $(SRCS)' > depend

-include depend

install: all
	$(INSTALL) $(LIBMOUSE) $(LIBDESTDIR)

uninstall:
	$(RM) $(LIBDESTDIR)/$(LIBMOUSE)

clean:
	$(RM) *.o $(LIBMOUSE) $(TEST) depend

cleanall: clean

