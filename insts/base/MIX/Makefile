include ../package.conf

OBJS = MIX.o
PROGS = MIX PMIX PYMIX

ifeq ($(ARCH),LINUX)
   DYN = -rdynamic
else
   DYN =
endif

PERL_LDFLAGS = `perl -MExtUtils::Embed -e ldopts`

TARGETS = MIX
ifeq ($(PERL_SUPPORT), TRUE)
TARGETS += PMIX
endif
ifeq ($(PYTHON_SUPPORT), TRUE)
TARGETS += PYMIX
LIBPYTHON = `python $(CMIXDIR)/Python/print_libpython.py`

# This is necessary for Python 2.0 in Slackware Linux 8.0, and probably
# other distributions; comment out if it gives you trouble with earlier
# Pythons.  -JGG
LIBPYTHON += -lutil
endif

all: $(TARGETS)

MIX: $(OBJS) $(CMIX_O) $(M_O) $(GENLIB) $(PROFILE_O)
	$(CXX) -o MIX $(DYN) $(OBJS) $(CMIX_O) $(PROFILE_O) $(M_O) $(GENLIB) \
		$(LDFLAGS)

PMIX: $(OBJS) $(CMIX_O) $(P_O) $(GENLIB) $(PROFILE_O)
	$(CXX) -o PMIX $(DYN) $(OBJS) $(CMIX_O) $(PROFILE_O) $(P_O) $(LDFLAGS) \
		$(GENLIB) $(PERL_LDFLAGS)

PYMIX: $(OBJS) $(CMIX_O) $(PY_O) $(GENLIB) $(PROFILE_O)
	$(CXX) -o PYMIX $(DYN) $(OBJS) $(CMIX_O) $(PROFILE_O) $(PY_O) $(LDFLAGS) \
		$(GENLIB) $(LIBPYTHON)

$(OBJS): $(INSTRUMENT_H)

clean:
	$(RM) *.o $(OBJS) $(PROGS)

