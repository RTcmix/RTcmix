include ../package.conf

NAME = COMBFILT

CURDIR = $(CMIXDIR)/insts.vccm/$(NAME)
OBJS = $(NAME).o
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

$(OBJS): $(INSTRUMENT_H) $(NAME).h

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

