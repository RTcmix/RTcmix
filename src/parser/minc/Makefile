include ../../makefile.conf

OBJS = y.tab.o builtin.o callextfunc.o error.o sym.o trees.o utils.o yywrap.o
SRCS = builtin.c callextfunc.c error.c sym.c trees.c utils.c yywrap.c 
MINC = libminc.a
TEST = test

LSRC = minc.l
LEX = flex
LEXFLAGS = -l #-d    # -d for debug output

YSRC = minc.y
YACC = yacc
#YACC = bison -y
YFLAGS = -d #-v

CFLAGS += -pedantic

all: $(MINC)

$(MINC): depend $(OBJS)
	$(LD) -r $(OBJS) -o $(MINC)

$(TEST): depend $(TEST).o $(MINC)
	$(CC) $(TEST).o -o $(TEST) $(MINC) -lm

lex.yy.c: $(LSRC)
	$(LEX) $(LEXFLAGS) $(LSRC)

y.tab.c: lex.yy.c $(YSRC)
	$(YACC) $(YFLAGS) $(YSRC)

depend:
	-$(SHELL) -ec '$(CC) -M $(CFLAGS) $(TEST).c $(SRCS)' > depend

-include depend

clean:
	$(RM) *.o $(MINC) $(TEST) lex.yy.c y.tab.c y.tab.h y.output depend
