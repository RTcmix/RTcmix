include ../package.conf

NAME = REVMIX

OBJS = $(NAME).o
CMIXOBJS += $(PROFILE_O)
CXXFLAGS += -I. -Wall 
PROGS = $(NAME) lib$(NAME).so

all: lib$(NAME).so

standalone: $(NAME)

lib$(NAME).so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

$(NAME): $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H) $(NAME).h

clean:
	$(RM) $(OBJS) $(PROGS)

