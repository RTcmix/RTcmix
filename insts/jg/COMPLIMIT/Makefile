include ../package.conf

NAME = COMPLIMIT

OBJS = $(NAME).o
CMIXOBJS += $(PROFILE_O)
ifeq ($(ARCH),LINUX)
   CXXFLAGS += -Wall
endif
PROGS = lib$(NAME).so $(NAME)

all: lib$(NAME).so

standalone: $(NAME)

libCOMPLIMIT.so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

COMPLIMIT: $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H) $(NAME).h

clean:
	$(RM) $(OBJS) $(PROGS)
