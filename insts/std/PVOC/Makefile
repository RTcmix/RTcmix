include ../package.conf

NAME = PVOC

OBJS = PVOC.o lpa.o lpamp.o makewindows.o fft.o fold.o overlapadd.o

CMIXOBJS += $(PROFILE_O)

PROGS = $(NAME) lib$(NAME).so

all: lib$(NAME).so

standalone: $(NAME)

lib$(NAME).so:	$(UGENS_H) $(OBJS)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

$(NAME):	$(UGENS_H) $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

clean:
	$(RM) *.o $(OBJS) $(PROGS)
