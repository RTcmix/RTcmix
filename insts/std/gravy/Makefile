include ../package.conf

NAME = gravy

OBJS = $(NAME).o
CMIXOBJS += $(RTPROFILE_O)
PROGS = $(NAME) lib$(NAME).so

all: lib$(NAME).so

standalone:  $(NAME)

lib$(NAME).so:  $(UGENS_H) $(OBJS)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

$(NAME):  $(UGENS_H) $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

clean:
	$(RM) *.o $(OBJS) $(PROGS)
