include ../package.conf

NAME = denoise

OBJS = $(NAME).o cfast.o cfsst.o cfr2tr.o cfr4tr.o cfr4syn.o cford1.o cford2.o
CMIXOBJS += $(RTPROFILE_O)
PROGS = lib$(NAME).so $(NAME)

all: lib$(NAME).so

standalone: $(NAME)

lib$(NAME).so: $(UGENS_H) $(OBJS)
	$(CXX) $(SHARED_LDFLAGS) -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

$(NAME): $(UGENS_H) $(CMIXOBJS) $(OBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

clean:
	$(RM) $(OBJS) $(PROGS)
