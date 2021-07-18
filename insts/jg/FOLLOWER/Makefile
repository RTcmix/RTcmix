include ../package.conf

CURDIR = $(CMIXDIR)/insts/jg/FOLLOWER
OBJLIBDIR = ../objlib
OBJLIB_A = $(OBJLIBDIR)/objlib.a
OBJLIB_H = $(OBJLIBDIR)/objlib.h

COMMON_OBJS = FOLLOWER_BASE.o
COMMON_HEADERS = FOLLOWER_BASE.h
FOLLOWER_OBJS = FOLLOWER.o $(COMMON_OBJS)
FOLLOWGATE_OBJS = FOLLOWGATE.o $(COMMON_OBJS)
FOLLOWBUTTER_OBJS = FOLLOWBUTTER.o $(COMMON_OBJS)
CMIXOBJS += $(PROFILE_O)
CXXFLAGS += -I$(OBJLIBDIR)
PROGS = libFOLLOWER.so FOLLOWER libFOLLOWGATE.so FOLLOWGATE \
libFOLLOWBUTTER.so FOLLOWBUTTER

.PHONY: all standalone install dso_install standalone_install \
uninstall dso_uninstall standalone_uninstall clean cleanall

ifeq ($(BUILDTYPE), STANDALONE)
  all: libFOLLOWER.so libFOLLOWGATE.so libFOLLOWBUTTER.so
else
  all: $(FOLLOWER_OBJS) $(FOLLOWGATE_OBJS) $(FOLLOWBUTTER_OBJS)
endif

$(OBJLIB_A):
	@( cd $(OBJLIBDIR); echo "making in objlib..."; \
	  $(MAKE) $(MFLAGS); echo "done in objlib" ); \

standalone: FOLLOWER FOLLOWGATE FOLLOWBUTTER

################################################################### FOLLOWER ###
libFOLLOWER.so: $(FOLLOWER_OBJS) $(GENLIB) $(OBJLIB_A)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(FOLLOWER_OBJS) $(GENLIB) $(SYSLIBS) \
		$(OBJLIB_A)

FOLLOWER: $(FOLLOWER_OBJS) $(CMIXOBJS) $(OBJLIB_A)
	$(CXX) -o $@ $(FOLLOWER_OBJS) $(CMIXOBJS) $(OBJLIB_A) $(LDFLAGS)

$(FOLLOWER_OBJS): $(INSTRUMENT_H) $(OBJLIB_H) $(COMMON_HEADERS) FOLLOWER.h

################################################################# FOLLOWGATE ###
libFOLLOWGATE.so: $(FOLLOWGATE_OBJS) $(GENLIB) $(OBJLIB_A)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(FOLLOWGATE_OBJS) $(GENLIB) $(SYSLIBS) \
		$(OBJLIB_A)

FOLLOWGATE: $(FOLLOWGATE_OBJS) $(CMIXOBJS) $(OBJLIB_A)
	$(CXX) -o $@ $(FOLLOWGATE_OBJS) $(CMIXOBJS) $(OBJLIB_A) $(LDFLAGS)

$(FOLLOWGATE_OBJS): $(INSTRUMENT_H) $(OBJLIB_H) $(COMMON_HEADERS) FOLLOWGATE.h

############################################################### FOLLOWBUTTER ###
libFOLLOWBUTTER.so: $(FOLLOWBUTTER_OBJS) $(GENLIB) $(OBJLIB_A)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(FOLLOWBUTTER_OBJS) $(GENLIB) $(SYSLIBS) \
		$(OBJLIB_A)

FOLLOWBUTTER: $(FOLLOWBUTTER_OBJS) $(CMIXOBJS) $(OBJLIB_A)
	$(CXX) -o $@ $(FOLLOWBUTTER_OBJS) $(CMIXOBJS) $(OBJLIB_A) $(LDFLAGS)

$(FOLLOWBUTTER_OBJS): $(INSTRUMENT_H) $(OBJLIB_H) $(COMMON_HEADERS) \
		FOLLOWBUTTER.h

################################################################# BASE CLASS ###
FOLLOWER_BASE.o: $(INSTRUMENT_H) $(COMMON_HEADERS)

################################################## install, uninstall, clean ###
install: dso_install

dso_install: all
	$(INSTALL) $(CURDIR)/libFOLLOWER.so $(LIBDESTDIR)
	$(INSTALL) $(CURDIR)/libFOLLOWGATE.so $(LIBDESTDIR)
	$(INSTALL) $(CURDIR)/libFOLLOWBUTTER.so $(LIBDESTDIR)

standalone_install: standalone
	$(INSTALL) $(CURDIR)/FOLLOWER $(DESTDIR)
	$(INSTALL) $(CURDIR)/FOLLOWGATE $(DESTDIR)
	$(INSTALL) $(CURDIR)/FOLLOWBUTTER $(DESTDIR)

uninstall: dso_uninstall standalone_uninstall

dso_uninstall:
	$(RM) $(LIBDESTDIR)/libFOLLOWER.so
	$(RM) $(LIBDESTDIR)/libFOLLOWGATE.so
	$(RM) $(LIBDESTDIR)/libFOLLOWBUTTER.so

standalone_uninstall:
	$(RM) $(DESTDIR)/FOLLOWER
	$(RM) $(DESTDIR)/FOLLOWGATE
	$(RM) $(DESTDIR)/FOLLOWBUTTER

clean:
	$(RM) $(FOLLOWER_OBJS) $(FOLLOWGATE_OBJS) $(FOLLOWBUTTER_OBJS) $(PROGS)

cleanall: clean
	$(RM) depend

