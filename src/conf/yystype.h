#ifndef _YYSTYPE_H_
#define _YYSTYPE_H_ 1

/* We have to do this because bisons older than 1.35 -- specifically 1.28,
   which is included in OS X 10.2.8 -- do not generate compilable code when
   we use the %union directive.  This solves the problem.   -JGG
*/

typedef union {
   double num;
   bool val;
   char *str;
} YYSTYPE;

#endif // _YYSTYPE_H_ 1
