include ../package.conf

NAME = stgran

OBJS = $(NAME).o
CMIXOBJS += $(RTPROFILE_O)
ifeq ($(ARCH),LINUX)
   CFLAGS += -funroll-loops
endif
#LDFLAGS += -lefence

PROGS = $(NAME) lib$(NAME).so

all: lib$(NAME).so

standalone: $(NAME)

lib$(NAME).so:  $(UGENS_H) $(OBJS)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

$(NAME):  $(UGENS_H) $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

clean:
	$(RM) $(OBJS) $(PROGS)
