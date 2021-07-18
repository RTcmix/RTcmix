include ../package.conf

NAME = MMODALBAR

CURDIR = $(CMIXDIR)/insts/stk/$(NAME)
STKLIBDIR = ../stklib
STKLIB_A = $(STKLIBDIR)/stklib.a

OBJS = $(NAME).o
CMIXOBJS += $(PROFILE_O)
CXXFLAGS += -I. -I$(STKLIBDIR) -Wall 
PROGS = $(NAME) lib$(NAME).so

.PHONY: all standalone install dso_install standalone_install \
uninstall dso_uninstall standalone_uninstall clean cleanall

ifeq ($(BUILDTYPE), STANDALONE)
  all: lib$(NAME).so
else
  all: $(OBJS)
endif

standalone: $(NAME)

lib$(NAME).so: $(OBJS) $(GENLIB) $(STKLIB_A)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(OBJS) $(GENLIB) $(SYSLIBS) $(STKLIB_A)

$(NAME): $(OBJS) $(CMIXOBJS) $(STKLIB_A)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(STKLIB_A) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H) $(NAME).h

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
