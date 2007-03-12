include ../../../makefile.conf

OBJS = RTcmixMouse.o RTMousePField.o glue.o ../common/RTcmixWindow.o
SRCS = RTcmixMouse.cpp RTMousePField.cpp glue.cpp

# For OS X, we have to use a helper app, which communicates with RTcmix
# over a socket.  This is because you can't sublaunch a GUI window from
# a command-line program in OS X the way you can in X.  -JGG

ifeq ($(ARCH), MACOSX)
   OBJS += OSXMouse.o
   SRCS += OSXMouse.cpp
   APP = MouseWindow
   GUILIBS = -framework Carbon
   # When installing binary pkg in /Applications, don't hardcode path to app;
   # when running from elsewhere, you *must* hardcode path.
   ifeq ($(PKG), TRUE)
   else
      CXXFLAGS += -DAPP_PATH=\"$(DESTDIR)/$(APP).app\"
   endif
else
   OBJS += XMouse.o
   SRCS += XMouse.cpp
   APP = 
   GUILIBS = -L$(XLIBDIR) -lX11
endif
CXXFLAGS += -I. -I ../common -I../../rtcmix
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

# NB: Create OS X app bundle hierarchy each time, because bundle won't
# work if it contains CVS admin directories!  -JGG
$(APP): $(APP).o
	$(CXX) $(CXXFLAGS) $(APP).o -o $(APP) $(GUILIBS)
	install -d $(APP).app/Contents/MacOS
	mv -f $(APP) $(APP).app/Contents/MacOS
	cp -f osx_bundle_items/PkgInfo $(APP).app/Contents
	cp -f osx_bundle_items/Info.plist $(APP).app/Contents

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
	ditto $(APP).app $(DESTDIR)/$(APP).app
endif

uninstall:
	$(RM) $(LIBDESTDIR)/$(LIBMOUSE)
ifeq ($(ARCH), MACOSX)
	$(RM) -r $(DESTDIR)/$(APP).app
endif

clean:
	$(RM) *.o $(LIBMOUSE) $(APP) $(TEST)
ifeq ($(ARCH), MACOSX)
	$(RM) -r $(APP).app
endif

cleanall: clean
	$(RM) depend

