include ../../../makefile.conf

OBJS = RTcmixMouse.o RTMousePField.o glue.o
SRCS = RTcmixMouse.cpp RTMousePField.cpp glue.cpp
ifeq ($(ARCH), MACOSX)
   OBJS += OSXMouse.o
   SRCS += OSXMouse.cpp
   APP = MouseWindow
   GUILIBS = -framework Carbon
else
   OBJS += XMouse.o
   SRCS += XMouse.cpp
   APP = 
   GUILIBS = -L$(XLIBDIR) -lX11
endif
CXXFLAGS += -I. -I../../rtcmix
LIBMOUSE = libmouseconn.so

TEST = test
PFIELD = ../../rtcmix/PField.o ../../rtcmix/RefCounted.o
GENLIB = -Wl,-rpath ../../../lib -L../../../lib -lgen

all: $(LIBMOUSE) $(APP)

$(LIBMOUSE): depend $(OBJS)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(OBJS) $(GUILIBS)

glue.o: depend glue.cpp RTMousePField.h
	$(CXX) $(CXXFLAGS) -c -o glue.o glue.cpp

$(APP).o: $(APP).cpp
	$(CXX) $(CXXFLAGS) -c -o $(APP).o $(APP).cpp

$(APP): $(APP).o
	$(CXX) $(CXXFLAGS) $(APP).o -o $(APP) $(GUILIBS)
	mv -f $(APP) $(APP).app/Contents/MacOS

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
ifeq ($(ARCH), MACOSX)
	ditto $(APP).app $(LIBDESTDIR)/$(APP).app
endif

uninstall:
	$(RM) $(LIBDESTDIR)/$(LIBMOUSE)
ifeq ($(ARCH), MACOSX)
	$(RM) -r $(LIBDESTDIR)/$(APP).app
endif

clean:
	$(RM) *.o $(LIBMOUSE) $(APP) $(TEST) depend
ifeq ($(ARCH), MACOSX)
	$(RM) $(APP).app/Contents/MacOS/$(APP)
endif

cleanall: clean

