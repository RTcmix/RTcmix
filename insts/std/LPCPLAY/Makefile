include ../package.conf

NAME = LPCPLAY

CURDIR = $(CMIXDIR)/insts/std/$(NAME)

# NOTE: Using lib versions for some now -- optimize them!
#LPCPLAY_O = allpole.o readjust.o mp.o shift.o buzz.o bmultf.o lpcheader.o rand.o
LPCPLAY_O = DataSet.o setup.o shift.o buzz.o bmultf.o lpcheader.o rand.o stabilize.o Complex.o

HEADERS = lp.h setup.h lpcheader.h

OBJS = $(NAME).o $(LPCPLAY_O)

CXXFLAGS += -I. -DUSE_HEADERS
CFLAGS += -I. -DUSE_HEADERS

PROGS = $(NAME) lib$(NAME).so

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

$(OBJS): $(INSTRUMENT_H) $(NAME).h $(HEADERS) ../../../include/RefCounted.h DataSet.h

# For now, compile these with c++ because of compiler bug
#
buzz.o: buzz.c lp.h
	$(CXX) $(CXXFLAGS) -c buzz.c
#
rand.o: rand.c lp.h
	$(CXX) $(CXXFLAGS) -c rand.c

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
