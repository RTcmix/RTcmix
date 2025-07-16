include ../package.conf

NAME = MIDI

CURDIR = $(CMIXDIR)/insts/doug/$(NAME)
OBJS = MIDIBase.o PROGRAM.o CONTROLLER.o PITCHBEND.o SYSEX.o NOTE.o setup.o
CMIXOBJS +=
PROGS = lib$(NAME).so $(NAME)
CXXFLAGS += -DSHAREDLIBDIR=\"$(LIBDESTDIR)\"

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

$(OBJS): $(INSTRUMENT_H) MIDIBase.h PROGRAM.h CONTROLLER.h NOTE.h SYSEX.h PITCHBEND.h

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

