# Note: this parser requires bison.  It will not work with yacc!

include ../../makefile.conf

CONF = conf

OBJS = $(CONF).tab.o Config.o
SRCS = Config.cpp
LIBCONF = lib$(CONF).a
TEST = test

SYMPREFIX = conf

LSRC = $(CONF).l
LEX = flex
LEXFLAGS = -l -P$(SYMPREFIX) #-d    # -d for debug output

YSRC = $(CONF).ypp
YACC = bison -t -p $(SYMPREFIX)
YFLAGS = -d #-v      # -v for conf.output file

all: $(LIBCONF)

$(LIBCONF): depend $(OBJS)
	$(LD) -r $(OBJS) -o $(LIBCONF)

$(TEST).o: depend $(TEST).cpp
	$(CXX) $(CXXFLAGS) -c -o $(TEST).o $(TEST).cpp

$(TEST): depend $(TEST).o $(LIBCONF)
	$(CXX) $(CXXFLAGS) $(TEST).o -o $(TEST) $(LIBCONF) -lm

lex.yy.c: $(LSRC)
	$(LEX) $(LEXFLAGS) $(LSRC)

conf.tab.cpp: lex.yy.c $(YSRC)
	$(YACC) $(YFLAGS) $(YSRC)
	@# these are in case we're using an older bison (1.28 in OS X 10.2.8)
	@if test -f conf.ypp.tab.c; then mv conf.ypp.tab.c conf.tab.cpp; fi;
	@if test -f conf.ypp.tab.h; then mv conf.ypp.tab.h conf.tab.hpp; fi;

depend:
	-$(SHELL) -ec '$(CXX) -M $(CXXFLAGS) $(TEST).cpp $(SRCS)' > depend

-include depend

install:
uninstall:

clean:
	$(RM) *.o $(LIBCONF) $(TEST) *$(SYMPREFIX).c *.tab.cpp *.tab.hpp $(CONF).c \
		*.output depend
