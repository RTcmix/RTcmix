include ../package.conf

NAME = denoise

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

install: all
	$(INSTALL) lib$(NAME).so $(LIBDESTDIR)

dso_install: install

standalone_install: standalone
	$(INSTALL) $(NAME) $(DESTDIR)

uninstall: dso_uninstall standalone_uninstall

dso_uninstall:
	$(RM) $(LIBDESTDIR)/lib$(NAME).so

standalone_uninstall:
	$(RM) $(DESTDIR)/$(NAME)

clean:
	$(RM) $(OBJS) $(PROGS)

cleanall: clean uninstall

