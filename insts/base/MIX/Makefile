include ../package.conf

OBJS = MIX.o
PROGS = MIX PMIX

ifeq ($(ARCH),LINUX)
   DYN = -rdynamic
else
   DYN =
endif

PERL_LDFLAGS = `perl -MExtUtils::Embed -e ldopts`

ifeq ($(PERL_SUPPORT), TRUE)
all: MIX PMIX
else
all: MIX
endif

MIX: $(OBJS) $(CMIX_O) $(M_O) $(GENLIB) $(PROFILE_O)
	$(CXX) -o MIX $(DYN) $(OBJS) $(CMIX_O) $(PROFILE_O) $(M_O) $(GENLIB) \
		$(LDFLAGS)

PMIX: $(OBJS) $(CMIX_O) $(P_O) $(GENLIB) $(PROFILE_O)
	$(CXX) -o PMIX $(DYN) $(OBJS) $(CMIX_O) $(PROFILE_O) $(P_O) $(LDFLAGS) \
		$(GENLIB) $(PERL_LDFLAGS)

$(OBJS): $(INSTRUMENT_H)

clean:
	$(RM) *.o $(OBJS) $(PROGS)

