include ../package.conf

OBJS = MIX.o
PROGS = MIX PMIX

CMIXOBJS += $(PROFILE_O)
M_O = ../../Minc/m.o
P_O = ../../Minc/p.o

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

MIX: $(OBJS) $(CMIXOBJS) $(M_O)
	$(CXX) -o MIX $(DYN) $(OBJS) $(CMIXOBJS) $(M_O) $(LDFLAGS)

PMIX: $(OBJS) $(CMIXOBJS) $(P_O)
	$(CXX) -o PMIX $(DYN) $(OBJS) $(CMIXOBJS) $(P_O) $(LDFLAGS) $(PERL_LDFLAGS)

$(OBJS): $(INSTRUMENT_H)

clean:
	$(RM) *.o $(OBJS) $(PROGS)

