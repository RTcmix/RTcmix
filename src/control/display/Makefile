include ../../../makefile.conf

OBJS = RTcmixDisplay.o DisplayPField.o glue.o ../common/RTcmixWindow.o
SRCS = RTcmixDisplay.cpp DisplayPField.cpp glue.cpp

# For OS X, we have to use a helper app, which communicates with RTcmix
# over a socket.  This is because you can't sublaunch a GUI window from
# a command-line program in OS X the way you can in X.  -JGG

ifeq ($(ARCH), MACOSX)
   OBJS += OSXDisplay.o
   SRCS += OSXDisplay.cpp
   APP = DisplayWindow
   GUILIBS = -framework Carbon
   # When installing binary pkg in /Applications, don't hardcode path to app;
   # when running from elsewhere, you *must* hardcode path.
   ifeq ($(PKG), TRUE)
   else
      CXXFLAGS += -DAPP_PATH=\"$(DESTDIR)/$(APP).app\"
   endif
else
   OBJS += XDisplay.o
   SRCS += XDisplay.cpp
   APP = 
   GUILIBS = -L$(XLIBDIR) -lX11
endif
CXXFLAGS += -I. -I../common -I../../rtcmix
LIBDISPLAY = libdisplayconn.so

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

depend:
	-$(SHELL) -ec '$(CXX) -M $(CXXFLAGS) $(SRCS)' > depend

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
	$(RM) *.o $(LIBDISPLAY) $(APP)
ifeq ($(ARCH), MACOSX)
	$(RM) -r $(APP).app
endif

cleanall: clean
	$(RM) depend

