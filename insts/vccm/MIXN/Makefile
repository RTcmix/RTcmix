include ../package.conf

NAME = MIXN

CURDIR = $(CMIXDIR)/insts/vccm/$(NAME)
OBJS = $(NAME).o funcs.o
PROGS = lib$(NAME).so $(NAME)

.PHONY: all standalone install dso_install standalone_install \
uninstall dso_uninstall standalone_uninstall clean cleanall

all: lib$(NAME).so

standalone: $(NAME)

lib$(NAME).so: $(OBJS) $(GENLIB) $(OBJLIB_A)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(OBJS) $(GENLIB) $(SYSLIBS) $(OBJLIB_A)

$(NAME): $(OBJS) $(CMIXOBJS) $(OBJLIB_A)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(OBJLIB_A) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H) $(OBJLIB_H) $(NAME).h 

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

