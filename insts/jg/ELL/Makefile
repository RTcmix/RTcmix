include ../package.conf

NAME = ELL

OBJS = $(NAME).o ellipse.o setell.o
PROGS = lib$(NAME).so $(NAME)

all: lib$(NAME).so

standalone: $(NAME)

lib$(NAME).so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

$(NAME): $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H) $(NAME).h elldefs.h

clean:
	$(RM) $(OBJS) $(PROGS)
