include ../package.conf

OBJLIBDIR = ../objlib
OBJLIB_A = $(OBJLIBDIR)/objlib.a
OBJLIB_H = $(OBJLIBDIR)/objlib.h

COMMON_OBJS = FOLLOWER_BASE.o
COMMON_HEADERS = FOLLOWER_BASE.h
FOLLOWER_OBJS = FOLLOWER.o $(COMMON_OBJS)
CMIXOBJS += $(PROFILE_O)
CXXFLAGS += -I$(OBJLIBDIR)
PROGS = libFOLLOWER.so FOLLOWER

.PHONY: all standalone install dso_install standalone_install \
uninstall dso_uninstall standalone_uninstall clean cleanall

all: libFOLLOWER.so

$(OBJLIB_A):
	@( cd $(OBJLIBDIR); echo "making in objlib..."; \
	  $(MAKE) $(MFLAGS); echo "done in objlib" ); \

standalone: FOLLOWER

libFOLLOWER.so: $(FOLLOWER_OBJS) $(GENLIB) $(OBJLIB_A)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(FOLLOWER_OBJS) $(GENLIB) $(SYSLIBS) \
		$(OBJLIB_A)

FOLLOWER: $(FOLLOWER_OBJS) $(CMIXOBJS) $(OBJLIB_A)
	$(CXX) -o $@ $(FOLLOWER_OBJS) $(CMIXOBJS) $(OBJLIB_A) $(LDFLAGS)

$(FOLLOWER_OBJS): $(INSTRUMENT_H) $(OBJLIB_H) $(COMMON_HEADERS) FOLLOWER.h

FOLLOWER_BASE.o: $(INSTRUMENT_H) $(COMMON_HEADERS)

install: dso_install

dso_install: all
	$(INSTALL) libFOLLOWER.so $(LIBDESTDIR)

standalone_install: standalone
	$(INSTALL) FOLLOWER $(DESTDIR)

uninstall: dso_uninstall standalone_uninstall

dso_uninstall:
	$(RM) $(LIBDESTDIR)/libFOLLOWER.so

standalone_uninstall:
	$(RM) $(DESTDIR)/FOLLOWER

clean:
	$(RM) $(FOLLOWER_OBJS) $(PROGS)

cleanall: clean uninstall

