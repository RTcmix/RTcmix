#!/bin/sh
#
# Replace all the global var names in lex.conf.c with ones that don't
# conflict with our Minc parser.  The bison-generated source (conf.tab.cpp)
# uses the macros defined at the top of lex.conf.c to access these
# variables, so this substitution should not matter to it.   -JGG, 7/7/04

sed \
-e 's/yyin/__yyin/g' \
-e 's/yyleng/__yyleng/g' \
-e 's/yyout/__yyout/g' \
-e 's/yytext/__yytext/g' \
-e 's/yylineno/__yylineno/g' \
$1 > $1.tmp

mv $1.tmp $1

