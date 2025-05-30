include ../package.conf

NAME = PVOC

CURDIR = $(CMIXDIR)/insts/std/$(NAME)
OBJS = PVOC.o lpa.o lpamp.o makewindows.o fft.o fold.o overlapadd.o setup.o

INCLUDES += -I$(CMIXDIR)/src/rtcmix
CXXFLAGS +=  -DSHAREDLIBDIR=\"$(LIBDESTDIR)\"

ifeq ($(BUILDTYPE), STANDALONE)
	BUILD_FILTERS=1
endif

ifdef BUILD_FILTERS
	FILTERS = libPVNewFilter.so libPVRandBend.so libPVRandBands.so libPVTransBend.so libPVFilterBands.so libPVBandThreshold.so
endif

PROGS = $(NAME) lib$(NAME).so $(FILTERS)

.PHONY: all standalone install dso_install standalone_install \
uninstall dso_uninstall standalone_uninstall clean cleanall

ifeq ($(BUILDTYPE), STANDALONE)
  all: lib$(NAME).so $(FILTERS)
else
  all: $(OBJS)
endif

standalone: $(NAME)

lib$(NAME).so:	$(UGENS_H) $(OBJS)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

ifeq ($(BUILDTYPE), STANDALONE)

libPVFilterTest.so:	PVFilter.o PVFilterTest.o
	$(CXX) $(SHARED_LDFLAGS) -o $@ PVFilter.o PVFilterTest.o setup.o

libPVNewFilter.so:	PVFilter.o PVNewFilter.o
	$(CXX) $(SHARED_LDFLAGS) -o $@ PVFilter.o PVNewFilter.o setup.o

libPVRandBend.so:	PVFilter.o PVRandBend.o
	$(CXX) $(SHARED_LDFLAGS) -o $@ PVFilter.o PVRandBend.o setup.o

libPVRandBands.so:	PVFilter.o PVRandBands.o
	$(CXX) $(SHARED_LDFLAGS) -o $@ PVFilter.o PVRandBands.o setup.o

libPVTransBend.so:	PVFilter.o PVTransBend.o
	$(CXX) $(SHARED_LDFLAGS) -o $@ PVFilter.o PVTransBend.o setup.o

libPVFilterBands.so:	PVFilter.o PVFilterBands.o
	$(CXX) $(SHARED_LDFLAGS) -o $@ PVFilter.o PVFilterBands.o setup.o

libPVBandThreshold.so:	PVFilter.o PVBandThreshold.o
	$(CXX) $(SHARED_LDFLAGS) -o $@ PVFilter.o PVBandThreshold.o setup.o

endif

$(NAME):	$(UGENS_H) $(OBJS) $(CMIXOBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

install: dso_install

dso_install: lib$(NAME).so $(FILTERS)
	$(INSTALL) $(CURDIR)/lib$(NAME).so $(LIBDESTDIR)
ifdef BUILD_FILTERS
	$(INSTALL) $(CURDIR)/libPVNewFilter.so $(LIBDESTDIR)
	$(INSTALL) $(CURDIR)/libPVRandBend.so $(LIBDESTDIR)
	$(INSTALL) $(CURDIR)/libPVRandBands.so $(LIBDESTDIR)
	$(INSTALL) $(CURDIR)/libPVTransBend.so $(LIBDESTDIR)
	$(INSTALL) $(CURDIR)/libPVFilterBands.so $(LIBDESTDIR)
	$(INSTALL) $(CURDIR)/libPVBandThreshold.so $(LIBDESTDIR)
endif

standalone_install: standalone
	$(INSTALL) $(CURDIR)/$(NAME) $(DESTDIR)

uninstall: dso_uninstall standalone_uninstall

dso_uninstall:
	$(RM) $(LIBDESTDIR)/lib$(NAME).so
ifdef BUILD_FILTERS
	$(RM) $(LIBDESTDIR)/libPVNewFilter.so
	$(RM) $(LIBDESTDIR)/libPVRandBend.so
	$(RM) $(LIBDESTDIR)/libPVRandBands.so
	$(RM) $(LIBDESTDIR)/libPVTransBend.so
	$(RM) $(LIBDESTDIR)/libPVFilterBands.so
	$(RM) $(LIBDESTDIR)/libPVBandThreshold.so
endif

standalone_uninstall:
	$(RM) $(DESTDIR)/$(NAME)

clean:
	$(RM) $(OBJS) $(PROGS) $(FILTERS) PVFilter.o PVFilterTest.o PVNewFilter.o PVRandBend.o PVRandBands.o PVTransBend.o PVFilterBands.o

cleanall: clean
	$(RM) depend
