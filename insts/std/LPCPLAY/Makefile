include ../package.conf

NAME = LPCPLAY

# NOTE: Using lib versions for some now -- optimize them!
#LPCPLAY_O = allpole.o readjust.o mp.o shift.o buzz.o bmultf.o lpcheader.o rand.o
LPCPLAY_O = DataSet.o setup.o shift.o buzz.o bmultf.o lpcheader.o rand.o stabilize.o Complex.o

HEADERS = lp.h setup.h lpcheader.h

OBJS = $(NAME).o $(LPCPLAY_O)

CXXFLAGS += -I. -DUSE_HEADERS
CFLAGS += -I. -DUSE_HEADERS

PROGS = $(NAME) lib$(NAME).so

all: lib$(NAME).so

standalone: $(NAME)

lib$(NAME).so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

$(NAME): $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H) $(NAME).h $(HEADERS)

clean:
	$(RM) $(OBJS) $(PROGS)


# For now, compile these with c++ because of compiler bug
#
buzz.o: buzz.c lp.h
	$(CXX) $(CXXFLAGS) -c buzz.c
#
rand.o: rand.c lp.h
	$(CXX) $(CXXFLAGS) -c rand.c
