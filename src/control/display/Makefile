include ../../../makefile.conf

OBJS = RTcmixDisplay.o DisplayPField.o glue.o
SRCS = RTcmixDisplay.cpp DisplayPField.cpp glue.cpp

# For OS X, we have to use a helper app, which communicates with RTcmix
# over a socket.  This is because you can't sublaunch a GUI window from
# a command-line program in OS X the way you can in X.  -JGG

ifeq ($(ARCH), MACOSX)
   OBJS += OSXDisplay.o
   SRCS += OSXDisplay.cpp
   APP = DisplayWindow
   GUILIBS = -framework Carbon
   CXXFLAGS += -DAPP_PATH=\"$(DESTDIR)/$(APP).app\"
else
   OBJS += XDisplay.o
   SRCS += XDisplay.cpp
   APP = 
   GUILIBS = -L$(XLIBDIR) -lX11
endif
CXXFLAGS += -I. -I../../rtcmix
LIBDISPLAY = libdisplayconn.so

TEST = test
PFIELD = ../PField.o ../RefCounted.o
GENLIB = -Wl,-rpath ../../../lib -L../../../lib -lgen

all: $(LIBDISPLAY) $(APP)

$(LIBDISPLAY): depend $(OBJS)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(OBJS) $(GUILIBS)

glue.o: depend glue.cpp DisplayPField.h
	$(CXX) $(CXXFLAGS) -c -o glue.o glue.cpp

$(APP).o: $(APP).cpp
	$(CXX) $(CXXFLAGS) -c -o $(APP).o $(APP).cpp

# NB: Create OS X app bundle hierarchy each time, because bundle won't
# work if it contains CVS admin directories!  -JGG
$(APP): $(APP).o
	$(CXX) $(CXXFLAGS) $(APP).o -o $(APP) $(GUILIBS)
	install -d $(APP).app/Contents/MacOS
	mv -f $(APP) $(APP).app/Contents/MacOS
	cp -f osx_bundle_items/PkgInfo $(APP).app/Contents
	cp -f osx_bundle_items/Info.plist $(APP).app/Contents

$(TEST).o: depend $(TEST).cpp DisplayPField.h
	$(CXX) $(CXXFLAGS) -c -o $(TEST).o $(TEST).cpp

$(TEST): depend $(TEST).o $(LIBDISPLAY)
	$(CXX) $(CXXFLAGS) $(TEST).o -o $(TEST) $(LIBDISPLAY) $(PFIELD) $(GENLIB) \
		$(GUILIBS) -lpthread

depend:
	-$(SHELL) -ec '$(CXX) -M $(CXXFLAGS) $(TEST).cpp $(SRCS)' > depend

-include depend

install: all
	$(INSTALL) $(LIBDISPLAY) $(LIBDESTDIR)
ifeq ($(ARCH), MACOSX)
	ditto $(APP).app $(DESTDIR)/$(APP).app
endif

uninstall:
	$(RM) $(LIBDESTDIR)/$(LIBDISPLAY)
ifeq ($(ARCH), MACOSX)
	$(RM) -r $(DESTDIR)/$(APP).app
endif

clean:
	$(RM) *.o $(LIBDISPLAY) $(APP) $(TEST) depend
ifeq ($(ARCH), MACOSX)
	$(RM) -r $(APP).app
endif

cleanall: clean

