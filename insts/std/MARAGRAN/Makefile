include ../package.conf

DIRNAME = MARAGRAN
NAME = SGRANR
NAME2 = STGRANR

CURDIR = $(CMIXDIR)/insts/std/$(DIRNAME)
OBJS = $(NAME).o
OBJS2 = $(NAME2).o
CMIXOBJS = += $(PROFILE_O)
PROGS = lib$(NAME).so $(NAME) lib$(NAME2).so $(NAME2)

.PHONY: all standalone install dso_install standalone_install \
uninstall dso_uninstall standalone_uninstall clean cleanall

ifeq ($(BUILDTYPE), STANDALONE)
  all: lib$(NAME).so lib$(NAME2).so
else
  all: $(OBJS) $(OBJS2)
endif

standalone: $(NAME) $(NAME2)

lib$(NAME).so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

lib$(NAME2).so: $(OBJS2) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(OBJS2) $(GENLIB) $(SYSLIBS)

$(NAME): $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(NAME2): $(OBJS2) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS2) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H) $(NAME).h

$(OBJS2): $(INSTRUMENT_H) $(NAME2).h

install: dso_install

dso_install: lib$(NAME).so lib$(NAME2).so
	$(INSTALL) $(CURDIR)/lib$(NAME).so $(LIBDESTDIR)
	$(INSTALL) $(CURDIR)/lib$(NAME2).so $(LIBDESTDIR)

standalone_install: standalone
	$(INSTALL) $(CURDIR)/$(NAME) $(DESTDIR)
	$(INSTALL) $(CURDIR)/$(NAME2) $(DESTDIR)

uninstall: dso_uninstall standalone_uninstall

dso_uninstall:
	$(RM) $(LIBDESTDIR)/lib$(NAME).so $(LIBDESTDIR)/lib$(NAME2).so

standalone_uninstall:
	$(RM) $(DESTDIR)/$(NAME) $(DESTDIR)/$(NAME2)

clean:
	$(RM) $(OBJS) $(OBJS2) $(PROGS)

cleanall: clean
	$(RM) depend
