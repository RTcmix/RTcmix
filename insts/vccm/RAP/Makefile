include ../package.conf

OBJS = PLAY.o RECORD.o rtprofile.o
PROGS = RAP libRAP.so
CMIXOBJS += $(PROFILE_O)

all: libRAP.so

standalone: RAP

libRAP.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

RAP: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H) $(HEADERS)

clean:
	$(RM) *.o $(OBJS) $(PROGS)

