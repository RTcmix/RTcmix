include ../package.conf

NAME = sgran

OBJS = $(NAME).o
CMIXOBJS += $(RTPROFILE_O)
ifeq ($(ARCH),LINUX)
   CFLAGS += -funroll-loops
endif
PROGS = $(NAME) lib$(NAME).so

all: lib$(NAME).so

standalone: $(NAME)

lib$(NAME).so:  $(UGENS_H) $(OBJS)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

$(NAME):  $(UGENS_H) $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

clean:
	$(RM) *.o $(OBJS) $(PROGS)
