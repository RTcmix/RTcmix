include ../package.conf

OBJS = FMINST.o
CMIXOBJS += $(PROFILE_O)
PROGS = FMINST libFMINST.so

all: libFMINST.so

standalone: FMINST

libFMINST.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

FMINST: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H)

clean:
	$(RM) *.o $(OBJS) $(PROGS)
