# be sure that the file "package.conf" has the correct
# directory/makefile for your RTcmix installation listed
# ... i.e.  "include /usr/local/src/RTcmix/makefile.conf

include ../package.conf

NAME = VWAVE

OBJS = $(NAME).o
CMIXOBJS += $(PROFILE_O)
PROGS = $(NAME) lib$(NAME).so

all: lib$(NAME).so

standalone:  $(NAME)

lib$(NAME).so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

$(NAME): $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H) $(NAME).h

clean:
	$(RM) *.o $(OBJS) $(PROGS)

