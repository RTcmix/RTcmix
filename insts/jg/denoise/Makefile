include ../package.conf

NAME = denoise

CURDIR = $(CMIXDIR)/insts/jg/$(NAME)
OBJS = $(NAME).o cfast.o cfsst.o cfr2tr.o cfr4tr.o cfr4syn.o cford1.o cford2.o
CMIXOBJS += $(RTPROFILE_O)
PROGS = lib$(NAME).so $(NAME)

.PHONY: all standalone install dso_install standalone_install \
uninstall dso_uninstall standalone_uninstall clean cleanall

all: lib$(NAME).so

standalone: $(NAME)

lib$(NAME).so: $(UGENS_H) $(OBJS)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

$(NAME): $(UGENS_H) $(CMIXOBJS) $(OBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(NAME).h

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

