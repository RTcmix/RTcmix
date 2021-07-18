include ../package.conf

NAME = RESYNTH

CURDIR = $(CMIXDIR)/insts/jg/$(NAME)
PARTIALPLAYER = Frame.o PartialFramePlayer.o PartialVoice.o RandGen.o \
	Sine.o
PARTIALPLAYER_H = Frame.h List.h PartialFramePlayer.h PartialVoice.h \
	Interpolator.h RandGen.h Sine.h
OBJS = $(NAME).o $(PARTIALPLAYER)
CMIXOBJS += $(PROFILE_O)
PROGS = lib$(NAME).so $(NAME)

.PHONY: all standalone install dso_install standalone_install \
uninstall dso_uninstall standalone_uninstall clean cleanall

all: lib$(NAME).so

standalone: $(NAME)

lib$(NAME).so: $(OBJS) SDIFfile.o $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(OBJS) SDIFfile.o $(GENLIB) $(SYSLIBS) $(SDIF_LIBS)

$(NAME): $(OBJS) SDIFfile.o $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) SDIFfile.o $(CMIXOBJS) $(LDFLAGS) $(SDIF_LIBS)

# separately, to pull in SDIF_CFLAGS for this only
SDIFfile.o: SDIFfile.cpp SDIFfile.h
	$(CXX) $(CXXFLAGS) $(SDIF_CFLAGS) -c -o SDIFfile.o SDIFfile.cpp

$(OBJS): $(INSTRUMENT_H) $(NAME).h $(PARTIALPLAYER_H)

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
	$(RM) -r $(OBJS) SDIFfile.o $(PROGS) test test.o test.dSYM RESYNTH.dSYM

cleanall: clean
	$(RM) depend

test: test.cpp Interpolator.h List.h
	g++ -g -o test test.cpp
