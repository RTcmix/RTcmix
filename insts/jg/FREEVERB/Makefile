include ../package.conf

NAME = FREEVERB

OBJS = $(NAME).o allpass.o comb.o revmodel.o
HDRS = allpass.hpp comb.hpp denormals.h revmodel.hpp tuning.h
CMIXOBJS += $(PROFILE_O)
CXXFLAGS += -I. -Wall 
PROGS = $(NAME) lib$(NAME).so

all: lib$(NAME).so

standalone: $(NAME)

lib$(NAME).so: $(OBJS) $(GENLIB)
	$(CXX) $(SHARED_LDFLAGS) -Wl,-soname,$@ -o $@ $(OBJS) $(GENLIB) $(SYSLIBS)

$(NAME): $(OBJS) $(CMIXOBJS)
	$(CXX) -o $@ $(OBJS) $(CMIXOBJS) $(LDFLAGS)

$(OBJS): $(INSTRUMENT_H) $(NAME).h $(HDRS)

clean:
	$(RM) $(OBJS) $(PROGS)

