include ../package.conf

# NOTE: STRUM is a family of instruments, none of which is called "STRUM."

NAME = STRUM

CURDIR = $(CMIXDIR)/insts/std/$(NAME)
OBJS = START.o BEND.o FRET.o START1.o BEND1.o FRET1.o VSTART1.o VFRET1.o \
rtprofile.o randfill.o sset.o strum.o squisher.o delayset.o delayclean.o \
dist.o delay.o
HEADERS = START.h BEND.h FRET.h START1.h BEND1.h FRET1.h VFRET1.h VSTART1.h \
delayq.h strums.h
CMIXOBJS += $(PROFILE_O)
PROGS = lib$(NAME).so $(NAME)

.PHONY: all standalone install dso_install standalone_install \
uninstall dso_uninstall standalone_uninstall clean cleanall

ifeq ($(BUILDTYPE), STANDALONE)
  all: lib$(NAME).so
else
  all: $(OBJS)
endif

standalone: $(NAME)

lib$(NAME).so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

$(NAME): $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H) $(HEADERS)

install: dso_install

dso_install: lib$(NAME).so
	$(INSTALL) $(CURDIR)/lib$(NAME).so $(LIBDESTDIR)

standalone_install: standalone
	$(INSTALL) $(CURDIR)/$(NAME) $(DESTDIR)

uninstall: dso_uninstall standalone_uninstall

dso_uninstall:
	$(RM) $(LIBDESTDIR)/lib$(NAME).so

standalone_uninstall:
	$(RM) $(DESTDIR)/$(NAME)

clean:
	$(RM) $(OBJS) $(PROGS)

cleanall: clean
	$(RM) depend
