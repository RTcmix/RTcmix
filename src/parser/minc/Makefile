include ../../../makefile.conf

INCLUDES += -I../../include -I../../rtcmix
OBJS = y.tab.o builtin.o callextfunc.o error.o sym.o trees.o utils.o yywrap.o
SRCS = builtin.c callextfunc.cpp error.c sym.c trees.c utils.c yywrap.c 
MINC = libminc.a

LSRC = minc.l
LEX = flex
LEXFLAGS = -l #-d    # -d for debug output

YSRC = minc.y
YACC = yacc
#YACC = bison -y
YFLAGS = -d #-v

all: $(MINC)

$(MINC): depend $(OBJS)
	$(LD) -r $(OBJS) -o $(MINC)

lex.yy.c: $(LSRC)
	$(LEX) $(LEXFLAGS) $(LSRC)

y.tab.c: lex.yy.c $(YSRC)
	$(YACC) $(YFLAGS) $(YSRC)

depend:
	-$(SHELL) -ec '$(CC) -M $(CFLAGS) $(SRCS)' > depend

-include depend

install:
uninstall:

clean:
	$(RM) *.o $(MINC) lex.yy.c y.tab.c y.tab.h y.output depend
