# NOTE: MIX is built as the main RTcmix executable -- called CMIX, PCMIX,
# or PYCMIX, depending on the parser -- and never as a DSO.

include ../package.conf

OBJS = MIX.o
PROGS = MIX PMIX PYMIX

ifeq ($(ARCH), MACOSX_10.0)
   WORKAROUND = osx_workaround.o
endif

PERL_LDFLAGS = `perl -MExtUtils::Embed -e ldopts`

TARGETS = MIX
ifeq ($(PERL_SUPPORT), TRUE)
  TARGETS += PMIX
endif
ifeq ($(PYTHON_SUPPORT), TRUE)
  TARGETS += PYMIX
  LIBPYTHON = `python $(CMIXDIR)/Python/print_libpython.py`
  ifeq ($(ARCH), LINUX)
     # This is necessary for Python 2.0 in Slackware Linux 8.0, and probably
     # other distributions; comment out if it gives you trouble with earlier
     # Pythons.  -JGG
     LIBPYTHON += -lutil
  endif
endif

.PHONY: all standalone install dso_install standalone_install \
uninstall dso_uninstall standalone_uninstall clean cleanall

all: $(TARGETS)

standalone: all

MIX: $(OBJS) $(CMIX_O) $(M_O) $(GENLIB) $(PROFILE_O) $(WORKAROUND)
	$(CXX) -o MIX $(WORKAROUND) $(DYN) $(OBJS) $(CMIX_O) $(PROFILE_O) \
		$(M_O) $(GENLIB) $(LDFLAGS)

PMIX: $(OBJS) $(CMIX_O) $(P_O) $(GENLIB) $(PROFILE_O) $(WORKAROUND)
	$(CXX) -o PMIX $(WORKAROUND) $(DYN) $(OBJS) $(CMIX_O) $(PROFILE_O) \
		$(P_O) $(LDFLAGS) $(GENLIB) $(PERL_LDFLAGS)

PYMIX: $(OBJS) $(CMIX_O) $(PY_O) $(GENLIB) $(PROFILE_O) $(WORKAROUND)
	$(CXX) -o PYMIX $(WORKAROUND) $(DYN) $(OBJS) $(CMIX_O) $(PROFILE_O) \
		$(PY_O) $(LDFLAGS) $(GENLIB) $(LIBPYTHON)

$(OBJS): $(INSTRUMENT_H) MIX.h

install: standalone_install

dso_install: standalone_install		# otherwise can't run the dso's

standalone_install: all
	$(INSTALL) MIX $(DESTDIR)/CMIX
ifeq ($(PERL_SUPPORT), TRUE)
	$(INSTALL) PMIX $(DESTDIR)/PCMIX
endif
ifeq ($(PYTHON_SUPPORT), TRUE)
	$(INSTALL) PYMIX $(DESTDIR)/PYCMIX
endif

uninstall: standalone_uninstall

dso_uninstall: # nothing to do

standalone_uninstall:
	$(RM) $(DESTDIR)/CMIX
	$(RM) $(DESTDIR)/PCMIX
	$(RM) $(DESTDIR)/PYCMIX

clean:
	$(RM) $(OBJS) $(PROGS) $(WORKAROUND)

cleanall: clean uninstall

