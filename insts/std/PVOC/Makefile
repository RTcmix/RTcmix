include ../package.conf

NAME = PVOC

CURDIR = $(CMIXDIR)/insts/std/$(NAME)
OBJS = PVOC.o lpa.o lpamp.o makewindows.o fft.o fold.o overlapadd.o setup.o

INCLUDES += -I$(CMIXDIR)/src/rtcmix
CXXFLAGS +=  -DSHAREDLIBDIR=\"$(LIBDESTDIR)\"

FILTERS = libPVRandBend.so

PROGS = $(NAME) lib$(NAME).so $(FILTERS)

.PHONY: all standalone install dso_install standalone_install \
uninstall dso_uninstall standalone_uninstall clean cleanall

all: lib$(NAME).so $(FILTERS)

standalone: $(NAME)

lib$(NAME).so:	$(UGENS_H) $(OBJS)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

libPVFilterTest.so:	PVFilter.o PVFilterTest.o
	$(CXX) $(SHARED_LDFLAGS) -o $@ PVFilter.o PVFilterTest.o setup.o

libPVRandBend.so:	PVFilter.o PVRandBend.o
	$(CXX) $(SHARED_LDFLAGS) -o $@ PVFilter.o PVRandBend.o setup.o

$(NAME):	$(UGENS_H) $(OBJS) $(CMIXOBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

install: dso_install

dso_install: lib$(NAME).so
	$(INSTALL) $(CURDIR)/lib$(NAME).so $(LIBDESTDIR)
	$(INSTALL) $(CURDIR)/libPVRandBend.so $(LIBDESTDIR)

standalone_install: standalone
	$(INSTALL) $(CURDIR)/$(NAME) $(DESTDIR)

uninstall: dso_uninstall standalone_uninstall

dso_uninstall:
	$(RM) $(LIBDESTDIR)/lib$(NAME).so
	$(RM) $(LIBDESTDIR)/libPVRandBend.so

standalone_uninstall:
	$(RM) $(DESTDIR)/$(NAME)

clean:
	$(RM) $(OBJS) $(PROGS) $(FILTERS) PVFilter.o PVFilterTest.o PVRandBend.o

cleanall: clean uninstall

