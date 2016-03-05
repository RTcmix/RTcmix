include ../package.conf

NAME = PVOC

CURDIR = $(CMIXDIR)/insts/std/$(NAME)
OBJS = PVOC.o lpa.o lpamp.o makewindows.o fft.o fold.o overlapadd.o setup.o

INCLUDES += -I$(CMIXDIR)/src/rtcmix
CXXFLAGS +=  -DSHAREDLIBDIR=\"$(LIBDESTDIR)\"

ifeq ($(BUILDTYPE), STANDALONE)
	BUILD_FILTERS=1
endif

ifdef ($(BUILD_FILTERS))
	FILTERS = libPVRandBend.$(SHLIB_SUFFIX) libPVRandBands.$(SHLIB_SUFFIX) libPVTransBend.$(SHLIB_SUFFIX)
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

libPVFilterTest.$(SHLIB_SUFFIX):	PVFilter.o PVFilterTest.o
	$(CXX) $(SHARED_LDFLAGS) -o $@ PVFilter.o PVFilterTest.o setup.o

libPVRandBend.$(SHLIB_SUFFIX):	PVFilter.o PVRandBend.o
	$(CXX) $(SHARED_LDFLAGS) -o $@ PVFilter.o PVRandBend.o setup.o

libPVRandBands.$(SHLIB_SUFFIX):	PVFilter.o PVRandBands.o
	$(CXX) $(SHARED_LDFLAGS) -o $@ PVFilter.o PVRandBands.o setup.o

libPVTransBend.$(SHLIB_SUFFIX):	PVFilter.o PVTransBend.o
	$(CXX) $(SHARED_LDFLAGS) -o $@ PVFilter.o PVTransBend.o setup.o

endif

$(NAME):	$(UGENS_H) $(OBJS) $(CMIXOBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

install: dso_install

dso_install: lib$(NAME).so $(FILTERS)
	$(INSTALL) $(CURDIR)/lib$(NAME).so $(LIBDESTDIR)
ifdef ($(BUILD_FILTERS))
	$(INSTALL) $(CURDIR)/libPVRandBend.$(SHLIB_SUFFIX) $(LIBDESTDIR)
	$(INSTALL) $(CURDIR)/libPVRandBands.$(SHLIB_SUFFIX) $(LIBDESTDIR)
	$(INSTALL) $(CURDIR)/libPVTransBend.$(SHLIB_SUFFIX) $(LIBDESTDIR)
endif

standalone_install: standalone
	$(INSTALL) $(CURDIR)/$(NAME) $(DESTDIR)

uninstall: dso_uninstall standalone_uninstall

dso_uninstall:
	$(RM) $(LIBDESTDIR)/lib$(NAME).so
ifdef ($(BUILD_FILTERS))
	$(RM) $(LIBDESTDIR)/libPVRandBend.$(SHLIB_SUFFIX)
	$(RM) $(LIBDESTDIR)/libPVRandBands.$(SHLIB_SUFFIX)
	$(RM) $(LIBDESTDIR)/libPVTransBend.$(SHLIB_SUFFIX)
endif

standalone_uninstall:
	$(RM) $(DESTDIR)/$(NAME)

clean:
	$(RM) $(OBJS) $(PROGS) $(FILTERS) PVFilter.o PVFilterTest.o PVRandBend.o PVRandBands.o PVTransBend.o

cleanall: clean uninstall

