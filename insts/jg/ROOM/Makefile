include ../package.conf

OBJS = ROOM.o roomset.o
CFLAGS += -fPIC
CXXFLAGS += -fPIC -Wall
PROGS = libROOM.so ROOM

all: libROOM.so #ROOM

libROOM.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) \
	$(GENLIB) $(SYSLIBS)

ROOM: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H) ROOM.h roomset.h

clean:
	$(RM) $(OBJS) $(PROGS)
