include ../../../makefile.conf

INCLUDES += -I../../include -I../../rtcmix
OBJS = minc.o builtin.o callextfunc.o error.o sym.o Node.o utils.o handle.o
SRCS = builtin.cpp callextfunc.cpp error.cpp sym.cpp Node.cpp utils.cpp handle.cpp
MINC = libminc.a

LSRC = minc.l
LEX = flex
LEXFLAGS = -l #-d    # -d for debug output

YSRC = minc.y
YACC = bison
YFLAGS = -d -o minc.cpp # -t -v

all: $(MINC)

ifeq ($(BUILDTYPE), STANDALONE)

OBJS += args.o
SRCS += args.cpp

$(MINC): depend $(OBJS)
	$(LD) -r $(OBJS) -o $(MINC)

else

ARCHFLAGS += -DEMBEDDED

$(MINC): $(OBJS)
	$(RM) $@
	ar q $@ $(OBJS)
	$(RANLIB) $@

endif

lex.yy.c: $(LSRC) minc_internal.h rename.h
	$(LEX) $(LEXFLAGS) $(LSRC)

minc.cpp: lex.yy.c $(YSRC) minc_internal.h rename.h
	$(YACC) $(YFLAGS) $(YSRC)

depend:
	-$(SHELL) -ec '$(CC) -M $(CFLAGS) $(SRCS)' > depend

-include depend

install: all
uninstall:

clean:
	$(RM) *.o $(MINC) lex.yy.c minc.cpp minc.hpp y.output

cleanall: clean
	$(RM) depend

