include ../package.conf

NAME = HAR

CURDIR = $(CMIXDIR)/insts.vccm/$(NAME)
OBJS = HOLD.o STORE.o FADE_HOLD.o rtprofile.o
CMIXOBJS += $(PROFILE_O)
PROGS = lib$(NAME).so $(NAME)

.PHONY: all standalone install dso_install standalone_install \
uninstall dso_uninstall standalone_uninstall clean cleanall

all: lib$(NAME).so

standalone: $(NAME)

lib$(NAME).so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

$(NAME): $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H) HOLD.h FADE_HOLD.h STORE.h

install: dso_install

dso_install: lib$(NAME).so
	$(INSTALL) $(CURDIR)/lib$(NAME).so $(LIBDESTDIR)/libi$(NAME).so

standalone_install: standalone
	$(INSTALL) $(CURDIR)/$(NAME) $(DESTDIR)/i$(NAME)

uninstall: dso_uninstall standalone_uninstall

dso_uninstall:
	$(RM) $(LIBDESTDIR)/libi$(NAME).so

standalone_uninstall:
	$(RM) $(DESTDIR)/i$(NAME)

clean:
	$(RM) $(OBJS) $(PROGS)

cleanall: clean uninstall

