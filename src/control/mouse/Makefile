include ../../../makefile.conf

OBJS = RTcmixMouse.o RTMousePField.o
SRCS = RTcmixMouse.cpp RTMousePField.cpp glue.cpp
ifeq ($(ARCH), MACOSX)
   OBJS += OSXMouse.o
   SRCS += OSXMouse.cpp
   GUILIBS = 
else
   OBJS += XMouse.o
   SRCS += XMouse.cpp
   GUILIBS = -L/usr/X11/lib -lX11
endif
CXXFLAGS += -I. -I../../rtcmix
LIBMOUSE = libmouse.a
TEST = test
PFIELD = ../../rtcmix/PField.o ../../rtcmix/RefCounted.o
GENLIB = ../../../lib/libgen.so

all: $(LIBMOUSE) glue.o

# FIXME: make this a dso
$(LIBMOUSE): depend $(OBJS)
	$(LD) -r $(OBJS) -o $(LIBMOUSE)

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

uninstall:

clean:
	$(RM) *.o $(LIBMOUSE) $(TEST) depend

cleanall: clean

