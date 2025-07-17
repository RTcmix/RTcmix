/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     LOWPRIO = 258,
     TOK_PLUSPLUS = 259,
     TOK_MINUSMINUS = 260,
     TOK_MINUSEQU = 261,
     TOK_PLUSEQU = 262,
     TOK_DIVEQU = 263,
     TOK_MULEQU = 264,
     TOK_OR = 265,
     TOK_AND = 266,
     TOK_UNEQU = 267,
     TOK_EQU = 268,
     TOK_GTREQU = 269,
     TOK_LESSEQU = 270,
     TOK_POW = 271,
     CASTTOKEN = 272,
     TOK_STRUCT_DECL = 273,
     TOK_BASE_DECL = 274,
     TOK_FLOAT_DECL = 275,
     TOK_STRING_DECL = 276,
     TOK_HANDLE_DECL = 277,
     TOK_LIST_DECL = 278,
     TOK_MAP_DECL = 279,
     TOK_MFUNC_DECL = 280,
     TOK_METHOD = 281,
     TOK_IDENT = 282,
     TOK_NUM = 283,
     TOK_ARG_QUERY = 284,
     TOK_ARG = 285,
     TOK_NOT = 286,
     TOK_IF = 287,
     TOK_ELSE = 288,
     TOK_FOR = 289,
     TOK_WHILE = 290,
     TOK_RETURN = 291,
     TOK_TRUE = 292,
     TOK_FALSE = 293,
     TOK_STRING = 294
   };
#endif
/* Tokens.  */
#define LOWPRIO 258
#define TOK_PLUSPLUS 259
#define TOK_MINUSMINUS 260
#define TOK_MINUSEQU 261
#define TOK_PLUSEQU 262
#define TOK_DIVEQU 263
#define TOK_MULEQU 264
#define TOK_OR 265
#define TOK_AND 266
#define TOK_UNEQU 267
#define TOK_EQU 268
#define TOK_GTREQU 269
#define TOK_LESSEQU 270
#define TOK_POW 271
#define CASTTOKEN 272
#define TOK_STRUCT_DECL 273
#define TOK_BASE_DECL 274
#define TOK_FLOAT_DECL 275
#define TOK_STRING_DECL 276
#define TOK_HANDLE_DECL 277
#define TOK_LIST_DECL 278
#define TOK_MAP_DECL 279
#define TOK_MFUNC_DECL 280
#define TOK_METHOD 281
#define TOK_IDENT 282
#define TOK_NUM 283
#define TOK_ARG_QUERY 284
#define TOK_ARG 285
#define TOK_NOT 286
#define TOK_IF 287
#define TOK_ELSE 288
#define TOK_FOR 289
#define TOK_WHILE 290
#define TOK_RETURN 291
#define TOK_TRUE 292
#define TOK_FALSE 293
#define TOK_STRING 294




/* Copy the first part of user declarations.  */
#line 5 "minc.y"

#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "rename.h"
#include "bison_version.h"
#include "minc_internal.h"
#include "Node.h"
#include "Symbol.h"
#include "lex.yy.c"
#ifdef __cplusplus
extern "C" {
#endif
#ifdef EMBEDDED
double minc_memflush(double p[], int n_args);
#else
// in args.cpp
const char *lookup_token(const char *token, bool printWarning);
#endif
#ifdef __cplusplus
}
#endif
#undef MDEBUG	/* turns on yacc debugging below */
#undef DEBUG_ID    /* turns on printing of each ID found (assumes MDEBUG) */

#ifdef MDEBUG
// yydebug=1;
#define MPRINT(x) rtcmix_print("YACC: %s\n", x)
#define MPRINT1(x,y) rtcmix_print("YACC: " x "\n", y)
#define MPRINT2(x,y,z) rtcmix_print("YACC: " x "\n", y, z)
#else
#define MPRINT(x)
#define MPRINT1(x,y)
#define MPRINT2(x,y,z)
#endif

#ifdef DEBUG_ID
#define MPRINT_ID(id) MPRINT1("id\t\t\(%s)", yytext)
#else
#define MPRINT_ID(id) MPRINT("id")
#endif

#undef YYDEBUG
#define YYDEBUG 1
#define MAXTOK_IDENTLIST 200
#define TRUE 1
#define FALSE 0

static Node *	program;
static int		idcount = 0;
static char		*idlist[MAXTOK_IDENTLIST];  
static int		flerror;		/* set if there was an error during parsing */
static int		level = 0;		/* keeps track whether we are in a sub-block */
static int		flevel = 0;		/* > 0 if we are in a function decl block */
static const char *   sCurrentStructname = NULL;   /* set to struct name if we are in a struct decl block */
static int      xblock = 0;		/* 1 if we are entering a block preceeded by if(), else(), while(), or for() */
static bool     preserve_symbols = false;   /* what to do with symbol table at end of parse */
static void 	cleanup();
static void 	incrLevel();
static void		decrLevel();
static void     incrFunctionLevel();
static void     decrFunctionLevel();
static void     setStructName(const char *name);
static Node * declare(MincDataType type);
static Node * declareStructs(const char *typeName);
static Node * initializeStruct(const char *typeName, Node *initList);
static Node * parseArgumentQuery(const char *text, int *pOutErr);
static Node * parseScoreArgument(const char *text, int *pOutErr);
static Node * go(Node * t1);



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 256 "minc.cpp"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  83
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   686

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  59
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  46
/* YYNRULES -- Number of rules.  */
#define YYNRULES  153
/* YYNRULES -- Number of states.  */
#define YYNSTATES  295

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   294

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    58,     2,     2,
      52,    53,    21,    19,    54,    20,    57,    22,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    26,    51,
      15,     4,    16,    25,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    55,     2,    56,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    48,     2,    50,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    17,
      18,    23,    24,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    49
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     6,     8,    11,    14,    18,    20,
      22,    24,    26,    28,    30,    32,    34,    36,    41,    48,
      53,    64,    66,    68,    70,    72,    73,    78,    81,    84,
      88,    90,    94,    96,    99,   102,   105,   108,   111,   114,
     115,   119,   121,   125,   130,   135,   137,   139,   141,   145,
     149,   152,   155,   158,   162,   167,   173,   177,   181,   185,
     189,   193,   198,   203,   208,   213,   217,   221,   224,   227,
     229,   231,   238,   244,   246,   250,   252,   254,   257,   261,
     265,   269,   273,   277,   281,   285,   289,   291,   293,   295,
     297,   301,   305,   309,   313,   317,   321,   325,   327,   329,
     331,   333,   336,   338,   340,   342,   344,   347,   350,   353,
     356,   359,   362,   365,   368,   371,   373,   377,   380,   383,
     388,   394,   398,   404,   407,   410,   413,   416,   419,   422,
     426,   429,   432,   435,   438,   441,   445,   448,   451,   454,
     457,   460,   464,   467,   469,   473,   477,   480,   483,   485,
     489,   493,   500,   506
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      60,     0,    -1,    -1,    61,    -1,    62,    -1,    62,    51,
      -1,    61,    62,    -1,    61,    62,    51,    -1,    84,    -1,
      68,    -1,    69,    -1,    70,    -1,    71,    -1,    72,    -1,
      94,    -1,    95,    -1,    73,    -1,    41,    74,    87,    62,
      -1,    41,    74,    87,    62,    42,    62,    -1,    44,    74,
      87,    62,    -1,    43,    74,    52,    62,    51,    87,    51,
      62,    53,    62,    -1,    63,    -1,   103,    -1,    65,    -1,
      93,    -1,    -1,    48,    64,    61,    50,    -1,    48,    50,
      -1,    45,    88,    -1,    45,    88,    51,    -1,    67,    -1,
      66,    54,    67,    -1,    36,    -1,    29,    66,    -1,    30,
      66,    -1,    31,    66,    -1,    32,    66,    -1,    33,    66,
      -1,    34,    66,    -1,    -1,    55,    88,    56,    -1,    67,
      -1,    76,    57,    67,    -1,    76,    55,    88,    56,    -1,
      48,    74,    85,    50,    -1,    88,    -1,    87,    -1,    78,
      -1,    79,    54,    78,    -1,    52,    79,    53,    -1,    52,
      53,    -1,    67,    80,    -1,    81,    80,    -1,    76,    75,
      80,    -1,    76,    57,    67,    80,    -1,    88,    25,    88,
      26,    88,    -1,    67,     4,    88,    -1,    76,     8,    88,
      -1,    76,     7,    88,    -1,    76,    10,    88,    -1,    76,
       9,    88,    -1,    76,    75,     8,    88,    -1,    76,    75,
       7,    88,    -1,    76,    75,    10,    88,    -1,    76,    75,
       9,    88,    -1,     5,    76,    75,    -1,     6,    76,    75,
      -1,     5,    76,    -1,     6,    76,    -1,    81,    -1,    82,
      -1,    76,    55,    88,    56,     4,    88,    -1,    76,    57,
      67,     4,    88,    -1,    88,    -1,    85,    54,    88,    -1,
      49,    -1,    88,    -1,    40,    87,    -1,    87,    12,    87,
      -1,    87,    11,    87,    -1,    87,    14,    87,    -1,    88,
      13,    88,    -1,    88,    15,    88,    -1,    88,    16,    88,
      -1,    88,    18,    88,    -1,    88,    17,    88,    -1,    46,
      -1,    47,    -1,    84,    -1,    83,    -1,    88,    23,    88,
      -1,    88,    21,    88,    -1,    88,    22,    88,    -1,    88,
      19,    88,    -1,    88,    20,    88,    -1,    88,    58,    88,
      -1,    52,    87,    53,    -1,    86,    -1,    37,    -1,    76,
      -1,    77,    -1,    48,    50,    -1,    38,    -1,    39,    -1,
      46,    -1,    47,    -1,    20,    88,    -1,    29,    67,    -1,
      30,    67,    -1,    31,    67,    -1,    32,    67,    -1,    33,
      67,    -1,    34,    67,    -1,    91,    67,    -1,    35,   104,
      -1,    89,    -1,    90,    54,    89,    -1,    27,    67,    -1,
      28,    67,    -1,    91,    48,    90,    50,    -1,    91,    92,
      48,    90,    50,    -1,    27,    67,    66,    -1,    27,    67,
      66,     4,    77,    -1,    29,    67,    -1,    30,    67,    -1,
      31,    67,    -1,    32,    67,    -1,    34,    67,    -1,    33,
      67,    -1,    27,    67,    67,    -1,    29,    67,    -1,    30,
      67,    -1,    31,    67,    -1,    32,    67,    -1,    33,    67,
      -1,    27,    67,    67,    -1,    29,    67,    -1,    30,    67,
      -1,    31,    67,    -1,    32,    67,    -1,    33,    67,    -1,
      27,    67,    67,    -1,    34,    67,    -1,    98,    -1,    99,
      54,    98,    -1,    52,    99,    53,    -1,    52,    53,    -1,
      61,    65,    -1,    65,    -1,    48,   101,    50,    -1,    96,
     100,   102,    -1,     1,    96,   100,    48,    61,    50,    -1,
       1,    96,   100,    48,    50,    -1,    97,   100,   102,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   118,   118,   118,   122,   123,   124,   125,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   143,   148,
     153,   158,   159,   160,   161,   171,   171,   178,   185,   195,
     208,   209,   213,   218,   223,   228,   233,   238,   244,   248,
     251,   254,   255,   256,   260,   264,   265,   269,   270,   274,
     275,   279,   280,   281,   286,   290,   294,   295,   298,   299,
     300,   305,   308,   311,   314,   320,   323,   328,   331,   335,
     336,   339,   344,   351,   352,   356,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   379,   380,
     381,   382,   383,   384,   385,   386,   387,   388,   389,   394,
     395,   396,   397,   398,   400,   401,   402,   414,   415,   416,
     417,   418,   419,   420,   421,   428,   429,   434,   439,   444,
     448,   456,   460,   466,   468,   470,   472,   474,   476,   478,
     485,   487,   489,   491,   493,   495,   503,   505,   507,   509,
     511,   513,   515,   521,   522,   532,   533,   538,   541,   548,
     554,   558,   559,   565
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "LOWPRIO", "'='", "TOK_PLUSPLUS",
  "TOK_MINUSMINUS", "TOK_MINUSEQU", "TOK_PLUSEQU", "TOK_DIVEQU",
  "TOK_MULEQU", "TOK_OR", "TOK_AND", "TOK_UNEQU", "TOK_EQU", "'<'", "'>'",
  "TOK_GTREQU", "TOK_LESSEQU", "'+'", "'-'", "'*'", "'/'", "TOK_POW",
  "CASTTOKEN", "'?'", "':'", "TOK_STRUCT_DECL", "TOK_BASE_DECL",
  "TOK_FLOAT_DECL", "TOK_STRING_DECL", "TOK_HANDLE_DECL", "TOK_LIST_DECL",
  "TOK_MAP_DECL", "TOK_MFUNC_DECL", "TOK_METHOD", "TOK_IDENT", "TOK_NUM",
  "TOK_ARG_QUERY", "TOK_ARG", "TOK_NOT", "TOK_IF", "TOK_ELSE", "TOK_FOR",
  "TOK_WHILE", "TOK_RETURN", "TOK_TRUE", "TOK_FALSE", "'{'", "TOK_STRING",
  "'}'", "';'", "'('", "')'", "','", "'['", "']'", "'.'", "'%'", "$accept",
  "prg", "stml", "stmt", "bstml", "@1", "ret", "idl", "id", "fdecl",
  "sdecl", "hdecl", "ldecl", "mapdecl", "mfuncdecl", "level", "subscript",
  "obj", "expblk", "fexp", "fexpl", "func", "fcall", "mcall", "ternary",
  "rstmt", "expl", "str", "bexp", "exp", "mbr", "mbrl", "structname",
  "basename", "structdef", "structdecl", "structinit", "funcname",
  "methodname", "arg", "argl", "fargl", "fstml", "fblock", "funcdef",
  "methoddef", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,    61,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   268,    60,    62,   269,   270,    43,
      45,    42,    47,   271,   272,    63,    58,   273,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   123,   294,
     125,    59,    40,    41,    44,    91,    93,    46,    37
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    59,    60,    60,    61,    61,    61,    61,    62,    62,
      62,    62,    62,    62,    62,    62,    62,    62,    62,    62,
      62,    62,    62,    62,    62,    64,    63,    63,    65,    65,
      66,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    76,    76,    77,    78,    78,    79,    79,    80,
      80,    81,    81,    81,    82,    83,    84,    84,    84,    84,
      84,    84,    84,    84,    84,    84,    84,    84,    84,    84,
      84,    84,    84,    85,    85,    86,    87,    87,    87,    87,
      87,    87,    87,    87,    87,    87,    87,    87,    88,    88,
      88,    88,    88,    88,    88,    88,    88,    88,    88,    88,
      88,    88,    88,    88,    88,    88,    88,    89,    89,    89,
      89,    89,    89,    89,    89,    90,    90,    91,    92,    93,
      93,    94,    95,    96,    96,    96,    96,    96,    96,    96,
      97,    97,    97,    97,    97,    97,    98,    98,    98,    98,
      98,    98,    98,    99,    99,   100,   100,   101,   101,   102,
     103,   103,   103,   104
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     1,     1,     2,     2,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     4,     6,     4,
      10,     1,     1,     1,     1,     0,     4,     2,     2,     3,
       1,     3,     1,     2,     2,     2,     2,     2,     2,     0,
       3,     1,     3,     4,     4,     1,     1,     1,     3,     3,
       2,     2,     2,     3,     4,     5,     3,     3,     3,     3,
       3,     4,     4,     4,     4,     3,     3,     2,     2,     1,
       1,     6,     5,     1,     3,     1,     1,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     1,     1,     1,     1,
       3,     3,     3,     3,     3,     3,     3,     1,     1,     1,
       1,     2,     1,     1,     1,     1,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     1,     3,     2,     2,     4,
       5,     3,     5,     2,     2,     2,     2,     2,     2,     3,
       2,     2,     2,     2,     2,     3,     2,     2,     2,     2,
       2,     3,     2,     1,     3,     3,     2,     2,     1,     3,
       3,     6,     5,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    32,    39,    39,    39,     0,    25,     0,     0,     4,
      21,    23,    41,     9,    10,    11,    12,    13,    16,     0,
      69,    70,     8,     0,    24,    14,    15,     0,    22,     0,
       0,     0,     0,     0,     0,     0,     0,    41,    67,    68,
     117,    33,    30,    34,    30,    35,    30,    36,    30,    37,
      30,    38,    30,     0,     0,     0,     0,    98,   102,   103,
     104,   105,    39,    75,     0,    99,   100,    89,    88,    97,
      28,    27,     0,     1,     6,     5,     0,     0,    51,     0,
       0,     0,     0,     0,     0,     0,    52,     0,     0,     0,
       0,     0,     0,   123,   124,   125,   126,   128,   127,     0,
       0,     0,    65,    66,   121,    30,     0,     0,    86,    87,
       0,    76,     0,     0,   106,   101,     0,     0,     0,     0,
       0,     0,     0,     0,    29,     0,     0,     7,    56,    50,
      47,     0,    46,    76,    58,    57,    60,    59,     0,    42,
       0,     0,     0,     0,    53,   118,     0,     0,     0,     0,
       0,     0,     0,     0,   115,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   146,   143,     0,     0,   150,
     129,     0,     0,    42,     0,    31,    77,     0,     0,     0,
      17,     0,     0,     0,     0,     0,     0,    19,     0,    73,
      96,    93,    94,    91,    92,    90,     0,    95,    26,    49,
       0,    43,     0,    54,    62,    61,    64,    63,   117,   107,
     108,   109,   110,   111,   112,     0,     0,     0,     0,     0,
       0,     0,   114,   119,     0,   113,     0,     0,   136,   137,
     138,   139,   140,   142,   145,     0,     0,    23,     0,   152,
       0,    40,    39,   122,    79,    78,    80,     0,    81,    82,
      83,    85,    84,     0,    44,     0,     0,    48,     0,    72,
       0,   130,   131,   132,   133,   134,     0,   116,   120,   141,
     144,    23,   149,   151,    18,     0,    74,    55,    71,   135,
     153,     0,     0,     0,    20
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    17,    18,    19,    20,    82,    21,    51,    22,    23,
      24,    25,    26,    27,    28,   126,    95,    75,    76,   140,
     141,    88,    30,    31,    77,    78,   198,    79,   142,   121,
     164,   165,    33,    99,    34,    35,    36,    37,   231,   176,
     177,   101,   248,   179,    38,   232
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -170
static const yytype_int16 yypact[] =
{
     463,   634,   -11,   -11,   -11,   -11,   -11,   -11,   -11,   -11,
     -11,  -170,  -170,  -170,  -170,   378,     2,    33,   483,   -12,
    -170,  -170,    12,  -170,  -170,  -170,  -170,  -170,  -170,    19,
      29,  -170,  -170,   -16,  -170,  -170,  -170,    48,  -170,   -11,
     -11,   -11,   -11,   -11,   -11,   -11,    48,  -170,   -33,   -33,
     -11,    70,    85,    70,    86,    70,    99,    70,   100,    70,
     101,    70,   114,   579,   115,   579,   378,  -170,  -170,  -170,
    -170,  -170,    89,  -170,   579,    19,  -170,  -170,  -170,  -170,
     298,  -170,   128,  -170,   119,  -170,   378,   515,  -170,   378,
     378,   378,   378,   378,   -11,    59,  -170,   -11,   625,   131,
     448,   132,   -11,  -170,  -170,  -170,  -170,  -170,  -170,   134,
     378,   -11,  -170,  -170,     9,   133,   -11,   579,    25,   416,
     564,   521,   128,   564,    -5,  -170,   378,    87,   378,   378,
     378,   378,   378,   378,  -170,   378,   310,  -170,   628,  -170,
    -170,    17,   163,   178,   628,   628,   628,   628,   354,    13,
     378,   378,   378,   378,  -170,  -170,   -11,   -11,   -11,   -11,
     -11,   -11,   -11,   652,  -170,   -40,   -11,   625,   -11,   -11,
     -11,   -11,   -11,   -11,   -11,  -170,  -170,    36,   128,  -170,
    -170,   361,   613,  -170,   140,  -170,  -170,   579,   579,   579,
     142,   378,   378,   378,   378,   378,   139,  -170,   -39,   628,
    -170,    63,    63,   107,   107,    -5,   620,   628,  -170,  -170,
     579,    27,   378,  -170,   628,   628,   628,   628,  -170,  -170,
    -170,  -170,  -170,  -170,  -170,   -11,   -11,   -11,   -11,   -11,
     -11,    48,  -170,  -170,   625,  -170,    81,   -11,  -170,  -170,
    -170,  -170,  -170,  -170,  -170,   643,   128,   152,   154,  -170,
     417,    -6,  -170,  -170,    66,   191,  -170,   128,   628,   628,
     628,   628,   628,   579,  -170,   378,   378,  -170,   378,   628,
     -11,  -170,  -170,  -170,  -170,  -170,   132,  -170,  -170,  -170,
    -170,   156,  -170,  -170,  -170,   136,   628,   149,   628,  -170,
    -170,   128,   162,   128,  -170
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -170,  -170,   -74,   -17,  -170,  -170,  -169,   106,    52,  -170,
    -170,  -170,  -170,  -170,  -170,    96,    69,     0,    32,     7,
    -170,   -23,  -170,  -170,  -170,     5,  -170,  -170,   -44,   177,
      -7,    61,   -92,  -170,  -170,  -170,  -170,   229,  -170,   -10,
    -170,   -42,  -170,   -38,  -170,  -170
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -149
static const yytype_int16 yytable[] =
{
      29,    84,    48,    49,   109,    32,   166,    96,   136,   247,
     233,   264,    97,   184,   234,   265,    86,   212,    29,   120,
     133,   123,   110,    32,   111,    11,    89,    90,    91,    92,
     127,   268,    98,    83,   -40,   -40,   -40,   -40,  -104,    85,
    -104,  -104,  -104,  -104,  -104,  -104,  -104,  -104,  -104,   -43,
    -104,   -43,    81,   135,    47,    47,    50,    52,    54,    56,
      58,    60,    62,   116,    87,    87,   150,   151,   152,   153,
     209,   210,   154,   186,    93,   166,    94,   281,   188,   -40,
     189,    87,    29,  -104,   130,   131,   132,    32,   133,   244,
     245,   102,   103,   104,   105,   106,   107,   108,   187,   188,
     100,   189,   115,   190,   246,   196,   197,   250,    63,    64,
      65,    87,    53,    55,    57,    59,    61,   112,   113,    84,
      29,   135,    29,    29,   116,    32,   213,    32,    32,     1,
     132,   278,   133,     2,     3,   234,    29,  -123,  -124,   125,
     200,    32,   166,   254,   255,   256,   149,   187,   188,   155,
     189,  -125,  -126,  -128,   180,     4,   114,     5,     6,     7,
       8,     9,    10,   183,    11,   135,  -127,   122,   185,    12,
     137,    13,    14,    15,   187,   188,    16,   189,    29,   167,
     178,    29,   181,    32,   257,  -129,    32,   291,   252,   276,
     263,   191,    80,   192,   193,   194,   195,   128,   129,   130,
     131,   132,  -148,   133,   282,   189,  -147,   135,   218,   219,
     220,   221,   222,   223,   224,   293,   253,   267,   235,   285,
     237,   238,   239,   240,   241,   242,   243,   277,   236,    84,
      46,   -45,   -45,    84,     0,   280,   135,     0,   290,     0,
     284,     0,     0,   124,     0,     0,    29,     0,     0,     0,
      29,    32,     0,     0,     0,    32,     0,    29,     0,     0,
       0,     0,    32,   138,   143,     0,   144,   145,   146,   147,
     148,     0,     0,     0,   292,     0,   294,   270,   271,   272,
     273,   274,   275,     0,     0,     0,     0,   182,     0,   279,
       0,    29,     0,    29,     0,     0,    32,     0,    32,     0,
       0,     0,     0,   199,     0,   201,   202,   203,   204,   205,
     206,     1,   207,     0,     0,     2,     3,   128,   129,   130,
     131,   132,   289,   133,     0,     0,     0,   214,   215,   216,
     217,     0,     0,     0,     0,     0,     0,     4,     0,     5,
       6,     7,     8,     9,    10,     0,    11,     0,     0,   134,
       0,    12,     0,    13,    14,    15,   135,     0,    16,     0,
     208,     0,     1,     0,     0,     0,     2,     3,   258,   259,
     260,   261,   262,   128,   129,   130,   131,   132,     0,   133,
       0,     0,     0,     2,     3,     0,     0,   143,     4,   269,
       5,     6,     7,     8,     9,    10,     0,    11,    66,     0,
       0,     0,    12,     0,    13,    14,    15,     0,     0,    16,
     211,   249,   135,     0,    11,    67,    68,    69,     1,     0,
       0,     0,     2,     3,    70,    71,    72,    73,     0,  -105,
      74,  -105,  -105,  -105,  -105,  -105,  -105,  -105,  -105,  -105,
       0,  -105,   286,   287,     4,   288,     5,     6,     7,     8,
       9,    10,     0,    11,     0,     0,     0,     0,    12,     0,
      13,    14,    15,    -2,     1,    16,     0,   283,     2,     3,
       0,     0,     0,     0,  -105,   168,     0,   169,   170,   171,
     172,   173,   174,    -3,     1,     0,     0,     0,     2,     3,
       4,     0,     5,     6,     7,     8,     9,    10,     0,    11,
       0,   175,     0,     0,    12,     0,    13,    14,    15,     0,
       4,    16,     5,     6,     7,     8,     9,    10,     0,    11,
       2,     3,     0,     0,    12,     0,    13,    14,    15,     0,
       0,    16,     0,     0,   191,    66,   192,   193,   194,   195,
     128,   129,   130,   131,   132,     0,   133,     0,     0,     0,
       0,    11,    67,    68,    69,   117,     0,     0,     0,     0,
       0,   118,   119,    72,    73,     1,     0,    74,   139,     2,
       3,     0,     0,     0,     0,   187,   188,     0,   189,   135,
       0,     0,     0,     0,     2,     3,     0,     0,     0,     0,
       0,     4,     0,     5,     6,     7,     8,     9,    10,    66,
      11,     0,     0,     0,     0,    12,     0,    13,    14,    15,
       0,     0,    16,     0,     0,    11,    67,    68,    69,   117,
       0,     0,     0,     0,     0,   118,   119,    72,    73,     0,
       0,    74,   128,   129,   130,   131,   132,     0,   133,   128,
     129,   130,   131,   132,     0,   133,   266,   128,   129,   130,
     131,   132,   156,   133,   157,   158,   159,   160,   161,   162,
     163,    39,     0,    40,    41,    42,    43,    44,    45,   251,
     168,   135,   169,   170,   171,   172,   173,   174,   135,   225,
       0,   226,   227,   228,   229,   230,   135
};

static const yytype_int16 yycheck[] =
{
       0,    18,     2,     3,    46,     0,    98,    30,    82,   178,
      50,    50,    28,     4,    54,    54,     4,     4,    18,    63,
      25,    65,    55,    18,    57,    36,     7,     8,     9,    10,
      74,     4,    48,     0,     7,     8,     9,    10,    13,    51,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    55,
      25,    57,    50,    58,     2,     3,     4,     5,     6,     7,
       8,     9,    10,    54,    52,    52,     7,     8,     9,    10,
      53,    54,    95,   117,    55,   167,    57,   246,    12,    52,
      14,    52,    82,    58,    21,    22,    23,    82,    25,    53,
      54,    39,    40,    41,    42,    43,    44,    45,    11,    12,
      52,    14,    50,   120,   178,   122,   123,   181,    12,    13,
      14,    52,     6,     7,     8,     9,    10,    48,    49,   136,
     120,    58,   122,   123,    54,   120,   149,   122,   123,     1,
      23,    50,    25,     5,     6,    54,   136,    52,    52,    50,
      53,   136,   234,   187,   188,   189,    94,    11,    12,    97,
      14,    52,    52,    52,   102,    27,    50,    29,    30,    31,
      32,    33,    34,   111,    36,    58,    52,    52,   116,    41,
      51,    43,    44,    45,    11,    12,    48,    14,   178,    48,
      48,   181,    48,   178,    42,    52,   181,    51,    48,   231,
      51,    13,    15,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    50,    25,    50,    14,    50,    58,   156,   157,
     158,   159,   160,   161,   162,    53,   184,   210,   166,   263,
     168,   169,   170,   171,   172,   173,   174,   234,   167,   246,
       1,    53,    54,   250,    -1,   245,    58,    -1,   276,    -1,
     257,    -1,    -1,    66,    -1,    -1,   246,    -1,    -1,    -1,
     250,   246,    -1,    -1,    -1,   250,    -1,   257,    -1,    -1,
      -1,    -1,   257,    86,    87,    -1,    89,    90,    91,    92,
      93,    -1,    -1,    -1,   291,    -1,   293,   225,   226,   227,
     228,   229,   230,    -1,    -1,    -1,    -1,   110,    -1,   237,
      -1,   291,    -1,   293,    -1,    -1,   291,    -1,   293,    -1,
      -1,    -1,    -1,   126,    -1,   128,   129,   130,   131,   132,
     133,     1,   135,    -1,    -1,     5,     6,    19,    20,    21,
      22,    23,   270,    25,    -1,    -1,    -1,   150,   151,   152,
     153,    -1,    -1,    -1,    -1,    -1,    -1,    27,    -1,    29,
      30,    31,    32,    33,    34,    -1,    36,    -1,    -1,    51,
      -1,    41,    -1,    43,    44,    45,    58,    -1,    48,    -1,
      50,    -1,     1,    -1,    -1,    -1,     5,     6,   191,   192,
     193,   194,   195,    19,    20,    21,    22,    23,    -1,    25,
      -1,    -1,    -1,     5,     6,    -1,    -1,   210,    27,   212,
      29,    30,    31,    32,    33,    34,    -1,    36,    20,    -1,
      -1,    -1,    41,    -1,    43,    44,    45,    -1,    -1,    48,
      56,    50,    58,    -1,    36,    37,    38,    39,     1,    -1,
      -1,    -1,     5,     6,    46,    47,    48,    49,    -1,    13,
      52,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      -1,    25,   265,   266,    27,   268,    29,    30,    31,    32,
      33,    34,    -1,    36,    -1,    -1,    -1,    -1,    41,    -1,
      43,    44,    45,     0,     1,    48,    -1,    50,     5,     6,
      -1,    -1,    -1,    -1,    58,    27,    -1,    29,    30,    31,
      32,    33,    34,     0,     1,    -1,    -1,    -1,     5,     6,
      27,    -1,    29,    30,    31,    32,    33,    34,    -1,    36,
      -1,    53,    -1,    -1,    41,    -1,    43,    44,    45,    -1,
      27,    48,    29,    30,    31,    32,    33,    34,    -1,    36,
       5,     6,    -1,    -1,    41,    -1,    43,    44,    45,    -1,
      -1,    48,    -1,    -1,    13,    20,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    -1,    25,    -1,    -1,    -1,
      -1,    36,    37,    38,    39,    40,    -1,    -1,    -1,    -1,
      -1,    46,    47,    48,    49,     1,    -1,    52,    53,     5,
       6,    -1,    -1,    -1,    -1,    11,    12,    -1,    14,    58,
      -1,    -1,    -1,    -1,     5,     6,    -1,    -1,    -1,    -1,
      -1,    27,    -1,    29,    30,    31,    32,    33,    34,    20,
      36,    -1,    -1,    -1,    -1,    41,    -1,    43,    44,    45,
      -1,    -1,    48,    -1,    -1,    36,    37,    38,    39,    40,
      -1,    -1,    -1,    -1,    -1,    46,    47,    48,    49,    -1,
      -1,    52,    19,    20,    21,    22,    23,    -1,    25,    19,
      20,    21,    22,    23,    -1,    25,    26,    19,    20,    21,
      22,    23,    27,    25,    29,    30,    31,    32,    33,    34,
      35,    27,    -1,    29,    30,    31,    32,    33,    34,    56,
      27,    58,    29,    30,    31,    32,    33,    34,    58,    27,
      -1,    29,    30,    31,    32,    33,    58
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     5,     6,    27,    29,    30,    31,    32,    33,
      34,    36,    41,    43,    44,    45,    48,    60,    61,    62,
      63,    65,    67,    68,    69,    70,    71,    72,    73,    76,
      81,    82,    84,    91,    93,    94,    95,    96,   103,    27,
      29,    30,    31,    32,    33,    34,    96,    67,    76,    76,
      67,    66,    67,    66,    67,    66,    67,    66,    67,    66,
      67,    66,    67,    74,    74,    74,    20,    37,    38,    39,
      46,    47,    48,    49,    52,    76,    77,    83,    84,    86,
      88,    50,    64,     0,    62,    51,     4,    52,    80,     7,
       8,     9,    10,    55,    57,    75,    80,    28,    48,    92,
      52,   100,    67,    67,    67,    67,    67,    67,    67,   100,
      55,    57,    75,    75,    66,    67,    54,    40,    46,    47,
      87,    88,    52,    87,    88,    50,    74,    87,    19,    20,
      21,    22,    23,    25,    51,    58,    61,    51,    88,    53,
      78,    79,    87,    88,    88,    88,    88,    88,    88,    67,
       7,     8,     9,    10,    80,    67,    27,    29,    30,    31,
      32,    33,    34,    35,    89,    90,    91,    48,    27,    29,
      30,    31,    32,    33,    34,    53,    98,    99,    48,   102,
      67,    48,    88,    67,     4,    67,    87,    11,    12,    14,
      62,    13,    15,    16,    17,    18,    62,    62,    85,    88,
      53,    88,    88,    88,    88,    88,    88,    88,    50,    53,
      54,    56,     4,    80,    88,    88,    88,    88,    67,    67,
      67,    67,    67,    67,    67,    27,    29,    30,    31,    32,
      33,    97,   104,    50,    54,    67,    90,    67,    67,    67,
      67,    67,    67,    67,    53,    54,    61,    65,   101,    50,
      61,    56,    48,    77,    87,    87,    87,    42,    88,    88,
      88,    88,    88,    51,    50,    54,    26,    78,     4,    88,
      67,    67,    67,    67,    67,    67,   100,    89,    50,    67,
      98,    65,    50,    50,    62,    87,    88,    88,    88,    67,
     102,    51,    62,    53,    62
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {
      case 61: /* "stml" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1495 "minc.cpp"
	break;
      case 62: /* "stmt" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1500 "minc.cpp"
	break;
      case 63: /* "bstml" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1505 "minc.cpp"
	break;
      case 65: /* "ret" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1510 "minc.cpp"
	break;
      case 68: /* "fdecl" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1515 "minc.cpp"
	break;
      case 69: /* "sdecl" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1520 "minc.cpp"
	break;
      case 70: /* "hdecl" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1525 "minc.cpp"
	break;
      case 71: /* "ldecl" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1530 "minc.cpp"
	break;
      case 72: /* "mapdecl" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1535 "minc.cpp"
	break;
      case 73: /* "mfuncdecl" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1540 "minc.cpp"
	break;
      case 75: /* "subscript" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1545 "minc.cpp"
	break;
      case 76: /* "obj" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1550 "minc.cpp"
	break;
      case 77: /* "expblk" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1555 "minc.cpp"
	break;
      case 78: /* "fexp" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1560 "minc.cpp"
	break;
      case 79: /* "fexpl" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1565 "minc.cpp"
	break;
      case 83: /* "ternary" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1570 "minc.cpp"
	break;
      case 84: /* "rstmt" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1575 "minc.cpp"
	break;
      case 85: /* "expl" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1580 "minc.cpp"
	break;
      case 86: /* "str" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1585 "minc.cpp"
	break;
      case 87: /* "bexp" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1590 "minc.cpp"
	break;
      case 88: /* "exp" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1595 "minc.cpp"
	break;
      case 89: /* "mbr" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1600 "minc.cpp"
	break;
      case 90: /* "mbrl" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1605 "minc.cpp"
	break;
      case 93: /* "structdef" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1610 "minc.cpp"
	break;
      case 94: /* "structdecl" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1615 "minc.cpp"
	break;
      case 95: /* "structinit" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1620 "minc.cpp"
	break;
      case 96: /* "funcname" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1625 "minc.cpp"
	break;
      case 97: /* "methodname" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1630 "minc.cpp"
	break;
      case 98: /* "arg" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1635 "minc.cpp"
	break;
      case 99: /* "argl" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1640 "minc.cpp"
	break;
      case 100: /* "fargl" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1645 "minc.cpp"
	break;
      case 101: /* "fstml" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1650 "minc.cpp"
	break;
      case 102: /* "fblock" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1655 "minc.cpp"
	break;
      case 103: /* "funcdef" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1660 "minc.cpp"
	break;
      case 104: /* "methoddef" */
#line 112 "minc.y"
	{ MPRINT1("yydestruct unref'ing node %p\n", (yyvaluep->node)); RefCounted::unref((yyvaluep->node)); };
#line 1665 "minc.cpp"
	break;

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 3:
#line 118 "minc.y"
    { MPRINT("prg:"); program = (yyvsp[(1) - (1)].node); program->ref(); cleanup(); return 0; ;}
    break;

  case 4:
#line 122 "minc.y"
    { MPRINT("stml:	stmt"); (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 5:
#line 123 "minc.y"
    { MPRINT("stml:	stmt;"); (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 6:
#line 124 "minc.y"
    { MPRINT("stml:	stml stmt"); (yyval.node) = new NodeSeq((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node)); ;}
    break;

  case 7:
#line 125 "minc.y"
    { MPRINT("stml:	stml stmt;"); (yyval.node) = new NodeSeq((yyvsp[(1) - (3)].node), (yyvsp[(2) - (3)].node)); ;}
    break;

  case 8:
#line 129 "minc.y"
    { MPRINT("stmt: rstmt");	(yyval.node) = go((yyvsp[(1) - (1)].node)); ;}
    break;

  case 17:
#line 138 "minc.y"
    {	xblock = 1; MPRINT("IF bexp stmt");
								decrLevel();
								(yyval.node) = go(new NodeIf((yyvsp[(3) - (4)].node), (yyvsp[(4) - (4)].node)));
								xblock = 0;
							;}
    break;

  case 18:
#line 143 "minc.y"
    { xblock = 1;
								decrLevel();
								(yyval.node) = go(new NodeIfElse((yyvsp[(3) - (6)].node), (yyvsp[(4) - (6)].node), (yyvsp[(6) - (6)].node)));
								xblock = 0;
							;}
    break;

  case 19:
#line 148 "minc.y"
    { xblock = 1;	MPRINT("WHILE bexp stmt");
								decrLevel();
								(yyval.node) = go(new NodeWhile((yyvsp[(3) - (4)].node), (yyvsp[(4) - (4)].node)));
								xblock = 0;
							;}
    break;

  case 20:
#line 153 "minc.y"
    { xblock = 1;
								decrLevel();
								(yyval.node) = go(new NodeFor((yyvsp[(4) - (10)].node), (yyvsp[(6) - (10)].node), (yyvsp[(8) - (10)].node), (yyvsp[(10) - (10)].node)));
								xblock = 0;
							;}
    break;

  case 21:
#line 158 "minc.y"
    { MPRINT("stmt: bstml"); (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 22:
#line 159 "minc.y"
    { MPRINT("stmt: funcdef"); (yyval.node) = go((yyvsp[(1) - (1)].node)); ;}
    break;

  case 23:
#line 160 "minc.y"
    { MPRINT("stmt: ret"); (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 24:
#line 161 "minc.y"
    { MPRINT("stmt: structdef"); (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 25:
#line 171 "minc.y"
    { if (!xblock) incrLevel(); ;}
    break;

  case 26:
#line 173 "minc.y"
    { 	MPRINT("bstml: { stml }"); MPRINT2("level = %d, xblock = %d", level, xblock);
									if (!xblock) { decrLevel(); }
									(yyval.node) = go(new NodeBlock((yyvsp[(3) - (4)].node)));
								;}
    break;

  case 27:
#line 178 "minc.y"
    { 	MPRINT("bstml: {}");
								(yyval.node) = go(new NodeBlock(new NodeNoop()));
								;}
    break;

  case 28:
#line 185 "minc.y"
    {	MPRINT("ret exp");
								MPRINT1("\tcalled at level %d", level);
								if (flevel == 0) {
									minc_die("return statements not allowed in main score");
									(yyval.node) = new NodeNoop();
								}
								else {
									(yyval.node) = new NodeRet((yyvsp[(2) - (2)].node));
								}
							;}
    break;

  case 29:
#line 195 "minc.y"
    {	MPRINT("ret exp;");
								MPRINT1("\tcalled at level %d", level);
								if (flevel == 0) {
									minc_die("return statements not allowed in main score");
									(yyval.node) = new NodeNoop();
								}
								else {
									(yyval.node) = new NodeRet((yyvsp[(2) - (3)].node));
								}
							;}
    break;

  case 30:
#line 208 "minc.y"
    { MPRINT("idl: id"); idlist[idcount++] = (yyvsp[(1) - (1)].str); ;}
    break;

  case 31:
#line 209 "minc.y"
    { MPRINT("idl: idl,id"); idlist[idcount++] = (yyvsp[(3) - (3)].str); ;}
    break;

  case 32:
#line 213 "minc.y"
    { MPRINT_ID(yytext); (yyval.str) = strsave(yytext); ;}
    break;

  case 33:
#line 218 "minc.y"
    { 	MPRINT("stmt: fdecl");
								(yyval.node) = go(declare(MincFloatType));
								idcount = 0;
							;}
    break;

  case 34:
#line 223 "minc.y"
    { 	MPRINT("stmt: sdecl");
								(yyval.node) = go(declare(MincStringType));
								idcount = 0;
							;}
    break;

  case 35:
#line 228 "minc.y"
    { 	MPRINT("stmt: hdecl");
								(yyval.node) = go(declare(MincHandleType));
								idcount = 0;
							;}
    break;

  case 36:
#line 233 "minc.y"
    { 	MPRINT("stmt: ldecl");
								(yyval.node) = go(declare(MincListType));
								idcount = 0;
							;}
    break;

  case 37:
#line 238 "minc.y"
    {     MPRINT("stmt: mapdecl");
                                (yyval.node) = go(declare(MincMapType));
                                idcount = 0;
                              ;}
    break;

  case 38:
#line 244 "minc.y"
    {     MPRINT("stmt: mfuncdecl"); (yyval.node) = go(declare(MincFunctionType)); idcount = 0; ;}
    break;

  case 39:
#line 248 "minc.y"
    { incrLevel(); ;}
    break;

  case 40:
#line 251 "minc.y"
    {       MPRINT("subscript: [exp]"); (yyval.node) = (yyvsp[(2) - (3)].node); ;}
    break;

  case 41:
#line 254 "minc.y"
    {       MPRINT("obj: id");          (yyval.node) = new NodeLoadSym((yyvsp[(1) - (1)].str)); ;}
    break;

  case 42:
#line 255 "minc.y"
    {       MPRINT("obj: obj.id");      (yyval.node) = new NodeMemberAccess((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].str));  ;}
    break;

  case 43:
#line 256 "minc.y"
    {       MPRINT("obj: obj[exp]");    (yyval.node) = new NodeSubscriptRead((yyvsp[(1) - (4)].node), (yyvsp[(3) - (4)].node)); ;}
    break;

  case 44:
#line 260 "minc.y"
    { MPRINT("expblk: {expl}");    decrLevel(); (yyval.node) = new NodeList((yyvsp[(3) - (4)].node)); ;}
    break;

  case 45:
#line 264 "minc.y"
    { MPRINT("fexp: exp"); (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 46:
#line 265 "minc.y"
    { MPRINT("fexp: bexp"); (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 47:
#line 269 "minc.y"
    { MPRINT("fexpl: fexp"); (yyval.node) = new NodeListElem(new NodeEmptyListElem(), (yyvsp[(1) - (1)].node)); ;}
    break;

  case 48:
#line 270 "minc.y"
    {  MPRINT("fexpl: fexpl,fexp"); (yyval.node) = new NodeListElem((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 49:
#line 274 "minc.y"
    {    MPRINT("func: (fexpl)"); (yyval.node) = (yyvsp[(2) - (3)].node); ;}
    break;

  case 50:
#line 275 "minc.y"
    {     MPRINT("func: ()"); (yyval.node) = new NodeEmptyListElem();  ;}
    break;

  case 51:
#line 279 "minc.y"
    {    MPRINT("fcall: id func"); (yyval.node) = new NodeFunctionCall(new NodeLoadSym((yyvsp[(1) - (2)].str)), (yyvsp[(2) - (2)].node)); ;}
    break;

  case 52:
#line 280 "minc.y"
    {    MPRINT("fcall: fcall func"); (yyval.node) = new NodeFunctionCall((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node)); ;}
    break;

  case 53:
#line 281 "minc.y"
    { MPRINT("fcall: obj subscript func"); (yyval.node) = new NodeFunctionCall(new NodeSubscriptRead((yyvsp[(1) - (3)].node), (yyvsp[(2) - (3)].node)), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 54:
#line 286 "minc.y"
    {  MPRINT("mcall: obj.id func"); (yyval.node) = new NodeMethodCall((yyvsp[(1) - (4)].node), (yyvsp[(3) - (4)].str), (yyvsp[(4) - (4)].node)); ;}
    break;

  case 55:
#line 290 "minc.y"
    { (yyval.node) = new NodeTernary((yyvsp[(1) - (5)].node), (yyvsp[(3) - (5)].node), (yyvsp[(5) - (5)].node)); ;}
    break;

  case 56:
#line 294 "minc.y"
    { MPRINT("rstmt: id = exp");		(yyval.node) = new NodeStore(new NodeAutoDeclLoadSym((yyvsp[(1) - (3)].str)), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 57:
#line 295 "minc.y"
    {	MPRINT("rstmt: obj TOK_PLUSEQU exp");
	                        (yyval.node) = new NodeOpAssign((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpPlus);
	;}
    break;

  case 58:
#line 298 "minc.y"
    {	(yyval.node) = new NodeOpAssign((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpMinus); ;}
    break;

  case 59:
#line 299 "minc.y"
    {		(yyval.node) = new NodeOpAssign((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpMul); ;}
    break;

  case 60:
#line 300 "minc.y"
    {		(yyval.node) = new NodeOpAssign((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), OpDiv); ;}
    break;

  case 61:
#line 305 "minc.y"
    { MPRINT("rstmt: obj subscript TOK_PLUSEQU exp");
     	(yyval.node) = new NodeSubscriptOpAssign((yyvsp[(1) - (4)].node), (yyvsp[(2) - (4)].node), (yyvsp[(4) - (4)].node), OpPlus);
	;}
    break;

  case 62:
#line 308 "minc.y"
    { MPRINT("rstmt: obj subscript TOK_MINUSEQU exp");
     	(yyval.node) = new NodeSubscriptOpAssign((yyvsp[(1) - (4)].node), (yyvsp[(2) - (4)].node), (yyvsp[(4) - (4)].node), OpMinus);
	;}
    break;

  case 63:
#line 311 "minc.y"
    { MPRINT("rstmt: obj subscript TOK_MULEQU exp");
     	(yyval.node) = new NodeSubscriptOpAssign((yyvsp[(1) - (4)].node), (yyvsp[(2) - (4)].node), (yyvsp[(4) - (4)].node), OpMul);
	;}
    break;

  case 64:
#line 314 "minc.y"
    { MPRINT("rstmt: obj subscript TOK_DIVEQU exp");
     	(yyval.node) = new NodeSubscriptOpAssign((yyvsp[(1) - (4)].node), (yyvsp[(2) - (4)].node), (yyvsp[(4) - (4)].node), OpDiv);
	;}
    break;

  case 65:
#line 320 "minc.y"
    { MPRINT("rstmt: TOK_PLUSPLUS obj subscript");
 	    (yyval.node) = new NodeSubscriptOpAssign((yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].node), new NodeConstf(1.0), OpPlus);
 	;}
    break;

  case 66:
#line 323 "minc.y"
    { MPRINT("rstmt: TOK_MINUSMINUS obj subscript");
 	    (yyval.node) = new NodeSubscriptOpAssign((yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].node), new NodeConstf(1.0), OpMinus);
 	;}
    break;

  case 67:
#line 328 "minc.y"
    { MPRINT("rstmt: TOK_PLUSPLUS obj");
        (yyval.node) = new NodeOpAssign((yyvsp[(2) - (2)].node), new NodeConstf(1.0), OpPlusPlus);
    ;}
    break;

  case 68:
#line 331 "minc.y"
    { MPRINT("rstmt: TOK_MINUSMINUS obj");
        (yyval.node) = new NodeOpAssign((yyvsp[(2) - (2)].node), new NodeConstf(1.0), OpMinusMinus);
    ;}
    break;

  case 69:
#line 335 "minc.y"
    {  MPRINT("rstmt: fcall"); (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 70:
#line 336 "minc.y"
    {  MPRINT("rstmt: mcall"); (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 71:
#line 339 "minc.y"
    {
                                MPRINT("rstmt: obj[exp] = exp");
                                (yyval.node) = new NodeSubscriptWrite((yyvsp[(1) - (6)].node), (yyvsp[(3) - (6)].node), (yyvsp[(6) - (6)].node));
                            ;}
    break;

  case 72:
#line 344 "minc.y"
    {
                                MPRINT("rstmt: obj.id = exp");
                                (yyval.node) = new NodeStore(new NodeMemberAccess((yyvsp[(1) - (5)].node), (yyvsp[(3) - (5)].str)), (yyvsp[(5) - (5)].node), /* allowOverwrite = */ false);
                            ;}
    break;

  case 73:
#line 351 "minc.y"
    { MPRINT("expl: exp"); (yyval.node) = new NodeListElem(new NodeEmptyListElem(), (yyvsp[(1) - (1)].node)); ;}
    break;

  case 74:
#line 352 "minc.y"
    { MPRINT("expl: expl,exp"); (yyval.node) = new NodeListElem((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 75:
#line 356 "minc.y"
    {
								char *s = yytext + 1;
								s[strlen(s) - 1] = '\0';
								(yyval.node) = new NodeString(strsave(s));
							;}
    break;

  case 76:
#line 364 "minc.y"
    { MPRINT("bexp: exp"); (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 77:
#line 365 "minc.y"
    { MPRINT("!bexp"); (yyval.node) = new NodeNot((yyvsp[(2) - (2)].node)); ;}
    break;

  case 78:
#line 366 "minc.y"
    { (yyval.node) = new NodeAnd((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 79:
#line 367 "minc.y"
    { (yyval.node) = new NodeOr((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 80:
#line 368 "minc.y"
    { (yyval.node) = new NodeRelation(OpEqual, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 81:
#line 369 "minc.y"
    { (yyval.node) = new NodeRelation(OpNotEqual, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 82:
#line 370 "minc.y"
    { (yyval.node) = new NodeRelation(OpLess, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 83:
#line 371 "minc.y"
    { (yyval.node) = new NodeRelation(OpGreater, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 84:
#line 372 "minc.y"
    { (yyval.node) = new NodeRelation(OpLessEqual, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 85:
#line 373 "minc.y"
    { (yyval.node) = new NodeRelation(OpGreaterEqual, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 86:
#line 374 "minc.y"
    { (yyval.node) = new NodeRelation(OpEqual, new NodeConstf(1.0), new NodeConstf(1.0)); ;}
    break;

  case 87:
#line 375 "minc.y"
    { (yyval.node) = new NodeRelation(OpNotEqual, new NodeConstf(1.0), new NodeConstf(1.0)); ;}
    break;

  case 88:
#line 379 "minc.y"
    { MPRINT("exp: rstmt"); (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 89:
#line 380 "minc.y"
    {  MPRINT("exp: ternary"); (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 90:
#line 381 "minc.y"
    { (yyval.node) = new NodeOp(OpPow, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 91:
#line 382 "minc.y"
    { (yyval.node) = new NodeOp(OpMul, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 92:
#line 383 "minc.y"
    { (yyval.node) = new NodeOp(OpDiv, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 93:
#line 384 "minc.y"
    { (yyval.node) = new NodeOp(OpPlus, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 94:
#line 385 "minc.y"
    { (yyval.node) = new NodeOp(OpMinus, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 95:
#line 386 "minc.y"
    { (yyval.node) = new NodeOp(OpMod, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 96:
#line 387 "minc.y"
    { MPRINT("exp: (bexp)"); (yyval.node) = (yyvsp[(2) - (3)].node); ;}
    break;

  case 97:
#line 388 "minc.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 98:
#line 389 "minc.y"
    {
							double f = atof(yytext);
							(yyval.node) = new NodeConstf(f);
						;}
    break;

  case 99:
#line 394 "minc.y"
    { MPRINT("exp: obj");   (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 100:
#line 395 "minc.y"
    { MPRINT("exp: expblk");   (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 101:
#line 396 "minc.y"
    { MPRINT("exp: {}");	(yyval.node) = new NodeList(new NodeEmptyListElem()); ;}
    break;

  case 102:
#line 397 "minc.y"
    { (yyval.node) = parseArgumentQuery(yytext, &flerror); ;}
    break;

  case 103:
#line 398 "minc.y"
    { (yyval.node) = parseScoreArgument(yytext, &flerror); ;}
    break;

  case 104:
#line 400 "minc.y"
    { (yyval.node) = new NodeConstf(1.0); ;}
    break;

  case 105:
#line 401 "minc.y"
    { (yyval.node) = new NodeConstf(0.0); ;}
    break;

  case 106:
#line 402 "minc.y"
    { MPRINT("exp: rstmt: '-' exp");
								/* NodeConstf is a dummy; makes exct_operator work */
								(yyval.node) = new NodeOp(OpNeg, (yyvsp[(2) - (2)].node), new NodeConstf(0.0));
							;}
    break;

  case 107:
#line 414 "minc.y"
    { MPRINT("mbr: decl");  (yyval.node) = new NodeMemberDecl((yyvsp[(2) - (2)].str), MincFloatType); ;}
    break;

  case 108:
#line 415 "minc.y"
    { MPRINT("mbr: decl");    (yyval.node) = new NodeMemberDecl((yyvsp[(2) - (2)].str), MincStringType); ;}
    break;

  case 109:
#line 416 "minc.y"
    { MPRINT("mbr: decl");    (yyval.node) = new NodeMemberDecl((yyvsp[(2) - (2)].str), MincHandleType); ;}
    break;

  case 110:
#line 417 "minc.y"
    { MPRINT("mbr: decl");  (yyval.node) = new NodeMemberDecl((yyvsp[(2) - (2)].str), MincListType); ;}
    break;

  case 111:
#line 418 "minc.y"
    { MPRINT("mbr: decl");   (yyval.node) = new NodeMemberDecl((yyvsp[(2) - (2)].str), MincMapType); ;}
    break;

  case 112:
#line 419 "minc.y"
    { MPRINT("mbr: decl");   (yyval.node) = new NodeMemberDecl((yyvsp[(2) - (2)].str), MincFunctionType); ;}
    break;

  case 113:
#line 420 "minc.y"
    { MPRINT("mbr: struct decl");   (yyval.node) = new NodeMemberDecl((yyvsp[(2) - (2)].str), MincStructType, (yyvsp[(1) - (2)].str)); ;}
    break;

  case 114:
#line 421 "minc.y"
    { MPRINT("mbr: methoddef"); (yyval.node) = (yyvsp[(2) - (2)].node); ;}
    break;

  case 115:
#line 428 "minc.y"
    { MPRINT("mbrl: mbr"); (yyval.node) = new NodeSeq(new NodeNoop(), (yyvsp[(1) - (1)].node)); ;}
    break;

  case 116:
#line 429 "minc.y"
    { MPRINT("mbrl: mbrl,mbr"); (yyval.node) = new NodeSeq((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 117:
#line 434 "minc.y"
    { MPRINT("structname"); (yyval.str) = (yyvsp[(2) - (2)].str); setStructName((yyvsp[(2) - (2)].str)); ;}
    break;

  case 118:
#line 439 "minc.y"
    { MPRINT("basename"); (yyval.str) = (yyvsp[(2) - (2)].str);  ;}
    break;

  case 119:
#line 444 "minc.y"
    { MPRINT("structdef");
                                        setStructName(NULL);
                                        (yyval.node) = go(new NodeStructDef((yyvsp[(1) - (4)].str), (yyvsp[(3) - (4)].node)));
                                    ;}
    break;

  case 120:
#line 448 "minc.y"
    { MPRINT("structdef (with base class)");
                                               setStructName(NULL);
                                               (yyval.node) = go(new NodeStructDef((yyvsp[(1) - (5)].str), (yyvsp[(4) - (5)].node), (yyvsp[(2) - (5)].str)));
                                             ;}
    break;

  case 121:
#line 456 "minc.y"
    { MPRINT("structdecl"); (yyval.node) = go(declareStructs((yyvsp[(2) - (3)].str))); idcount = 0; ;}
    break;

  case 122:
#line 460 "minc.y"
    {   MPRINT("stmt: structinit: struct <type> id = expblk"); (yyval.node) = go(initializeStruct((yyvsp[(2) - (5)].str), (yyvsp[(5) - (5)].node))); idcount = 0; ;}
    break;

  case 123:
#line 466 "minc.y"
    { MPRINT("funcname"); incrFunctionLevel();
                                    (yyval.node) = new NodeFuncDecl((yyvsp[(2) - (2)].str), MincFloatType); ;}
    break;

  case 124:
#line 468 "minc.y"
    { MPRINT("funcname"); incrFunctionLevel();
                                    (yyval.node) = new NodeFuncDecl((yyvsp[(2) - (2)].str), MincStringType); ;}
    break;

  case 125:
#line 470 "minc.y"
    { MPRINT("funcname");  incrFunctionLevel();
                                    (yyval.node) = new NodeFuncDecl((yyvsp[(2) - (2)].str), MincHandleType); ;}
    break;

  case 126:
#line 472 "minc.y"
    { MPRINT("funcname: returns list");  incrFunctionLevel();
                                    (yyval.node) = new NodeFuncDecl((yyvsp[(2) - (2)].str), MincListType); ;}
    break;

  case 127:
#line 474 "minc.y"
    { MPRINT("funcname: returns mfunction"); incrFunctionLevel();
                                    (yyval.node) = new NodeFuncDecl((yyvsp[(2) - (2)].str), MincFunctionType); ;}
    break;

  case 128:
#line 476 "minc.y"
    { MPRINT("funcname: returns map");  incrFunctionLevel();
                                    (yyval.node) = new NodeFuncDecl((yyvsp[(2) - (2)].str), MincMapType); ;}
    break;

  case 129:
#line 478 "minc.y"
    { MPRINT("funcname: returns struct");  incrFunctionLevel();
                                    (yyval.node) = new NodeFuncDecl((yyvsp[(3) - (3)].str), MincStructType); ;}
    break;

  case 130:
#line 485 "minc.y"
    { MPRINT("methodname"); incrFunctionLevel();
                            (yyval.node) = new NodeMethodDecl((yyvsp[(2) - (2)].str), sCurrentStructname, MincFloatType); ;}
    break;

  case 131:
#line 487 "minc.y"
    { MPRINT("methodname"); incrFunctionLevel();
                            (yyval.node) = new NodeMethodDecl((yyvsp[(2) - (2)].str), sCurrentStructname, MincStringType); ;}
    break;

  case 132:
#line 489 "minc.y"
    { MPRINT("methodname");  incrFunctionLevel();
                            (yyval.node) = new NodeMethodDecl((yyvsp[(2) - (2)].str), sCurrentStructname, MincHandleType); ;}
    break;

  case 133:
#line 491 "minc.y"
    { MPRINT("methodname: returns list");  incrFunctionLevel();
                            (yyval.node) = new NodeMethodDecl((yyvsp[(2) - (2)].str), sCurrentStructname, MincListType); ;}
    break;

  case 134:
#line 493 "minc.y"
    { MPRINT("methodname: returns map");  incrFunctionLevel();
                            (yyval.node) = new NodeMethodDecl((yyvsp[(2) - (2)].str), sCurrentStructname, MincMapType); ;}
    break;

  case 135:
#line 495 "minc.y"
    { MPRINT("methodname: returns struct");  incrFunctionLevel();
                            (yyval.node) = new NodeMethodDecl((yyvsp[(3) - (3)].str), sCurrentStructname, MincStructType); ;}
    break;

  case 136:
#line 503 "minc.y"
    { MPRINT("arg");
                                    (yyval.node) = new NodeDecl((yyvsp[(2) - (2)].str), MincFloatType); ;}
    break;

  case 137:
#line 505 "minc.y"
    { MPRINT("arg");
                                    (yyval.node) = new NodeDecl((yyvsp[(2) - (2)].str), MincStringType); ;}
    break;

  case 138:
#line 507 "minc.y"
    { MPRINT("arg");
                                    (yyval.node) = new NodeDecl((yyvsp[(2) - (2)].str), MincHandleType); ;}
    break;

  case 139:
#line 509 "minc.y"
    { MPRINT("arg");
                                    (yyval.node) = new NodeDecl((yyvsp[(2) - (2)].str), MincListType); ;}
    break;

  case 140:
#line 511 "minc.y"
    { MPRINT("arg");
                                    (yyval.node) = new NodeDecl((yyvsp[(2) - (2)].str), MincMapType); ;}
    break;

  case 141:
#line 513 "minc.y"
    { MPRINT("arg: structname");
                                    (yyval.node) = new NodeStructDecl((yyvsp[(3) - (3)].str), (yyvsp[(2) - (3)].str)); ;}
    break;

  case 142:
#line 515 "minc.y"
    { MPRINT("arg: mfunction");
                                    (yyval.node) = new NodeDecl((yyvsp[(2) - (2)].str), MincFunctionType); ;}
    break;

  case 143:
#line 521 "minc.y"
    { MPRINT("argl: arg"); (yyval.node) = new NodeArgListElem(new NodeEmptyListElem(), (yyvsp[(1) - (1)].node)); ;}
    break;

  case 144:
#line 522 "minc.y"
    { MPRINT("argl: argl,arg"); (yyval.node) = new NodeArgListElem((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 145:
#line 532 "minc.y"
    { MPRINT("fargl: (argl)"); (yyval.node) = new NodeArgList((yyvsp[(2) - (3)].node)); ;}
    break;

  case 146:
#line 533 "minc.y"
    { MPRINT("fargl: ()"); (yyval.node) = new NodeArgList(new NodeEmptyListElem()); ;}
    break;

  case 147:
#line 538 "minc.y"
    {	MPRINT("fstml: stml ret");
									(yyval.node) = new NodeFuncBodySeq((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node));
							;}
    break;

  case 148:
#line 541 "minc.y"
    {	MPRINT("fstml: ret");
									(yyval.node) = new NodeFuncBodySeq(new NodeEmptyListElem(), (yyvsp[(1) - (1)].node));
							;}
    break;

  case 149:
#line 548 "minc.y"
    {     MPRINT("fblock"); (yyval.node) = (yyvsp[(2) - (3)].node); ;}
    break;

  case 150:
#line 554 "minc.y"
    { MPRINT("funcdef");
                                    decrFunctionLevel();
									(yyval.node) = new NodeFuncDef((yyvsp[(1) - (3)].node), (yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].node));
								;}
    break;

  case 151:
#line 558 "minc.y"
    { minc_die("%s(): function body must end with 'return <exp>' statement", (yyvsp[(2) - (6)].node)); flerror = 1; (yyval.node) = new NodeNoop(); ;}
    break;

  case 152:
#line 559 "minc.y"
    { minc_die("%s(): function body must end with 'return <exp>' statement", (yyvsp[(2) - (5)].node)); flerror = 1; (yyval.node) = new NodeNoop(); ;}
    break;

  case 153:
#line 565 "minc.y"
    { MPRINT("methoddef");
                                    decrFunctionLevel();
                                    (yyval.node) = new NodeMethodDef((yyvsp[(1) - (3)].node), (yyvsp[(2) - (3)].node), (yyvsp[(3) - (3)].node));
                                ;}
    break;


/* Line 1267 of yacc.c.  */
#line 2814 "minc.cpp"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 571 "minc.y"


static void 	incrLevel()
{
	++level; MPRINT1("level => %d", level);
}

static void		decrLevel()
{
	--level; MPRINT1("level => %d", level);
}

static void     incrFunctionLevel()
{
    if (flevel > 0) { minc_die("nested function decls not allowed"); }
    ++flevel; MPRINT1("flevel => %d", flevel);
    incrLevel();
}

static void        decrFunctionLevel()
{
    decrLevel();
    --flevel; MPRINT1("flevel => %d", flevel);
}

static void     setStructName(const char *name)
{
    if (name == NULL || sCurrentStructname == NULL) {   // never override one with another
        sCurrentStructname = name;
        if (name) {
            MPRINT1("storing struct name '%s'", name);
        }
        else {
            MPRINT("clearing stored struct name");
        }
    }
}

// N.B. Because we have no need for <id>'s to exist in our tree other than for the purpose
// of declaring variables, we shortcut here and do not rely on the recursive parser.  We
// create our own sequence of declaration nodes.

static Node * declare(MincDataType type)
{
	MPRINT2("declare(type=%d, idcount=%d)", type, idcount);
	assert(idcount > 0);
	
	Node * t = new NodeNoop();	// end of the list
	for (int i = 0; i < idcount; i++) {
		Node * decl = new NodeDecl(idlist[i], type);
		t = new NodeSeq(t, decl);
	}
	return t;
}

static Node * declareStructs(const char *typeName)
{
    MPRINT2("declareStructs(typeName='%s', idcount=%d)", typeName, idcount);
    assert(idcount > 0);
    Node * t = new NodeNoop();    // end of the list
    for (int i = 0; i < idcount; i++) {
        Node * decl = new NodeStructDecl(idlist[i], typeName);
        t = new NodeSeq(t, decl);
    }
    return t;
}

static Node * initializeStruct(const char *typeName, Node *initList)
{
    MPRINT2("initializeStruct(typeName='%s', initList=%p)", typeName, initList);
    assert(idcount > 0);
    if (idcount > 1) {
        minc_die("Only one struct variable can be initialized at a time");
    }
    Node * end = new NodeNoop();    // end of the list
    Node * decl = new NodeStructDecl(idlist[0], typeName, initList);
    return new NodeSeq(end, decl);
}

static Node * parseArgumentQuery(const char *text, int *pOutErr)
{
#ifndef EMBEDDED
    /* ?argument will return 1.0 if defined, else 0.0 */
    const char *token = strsave(text + 1);    // strip off '?'
    // returns NULL silently if not found
    const char *value = lookup_token(token, false);
    return new NodeConstf(value != NULL ? 1.0 : 0.0);
#else
    minc_warn("Argument variables not supported");
    *pOutErr = 1;
    return new NodeNoop();
#endif
}

static Node * parseListArgument(const char *text, int *pOutErr)
{
    Node *listNode = NULL;
    Node *listElem = new NodeEmptyListElem();
    ++text;         // get past open brace
    bool done = false;
    try {
        bool isNumber = false;      // until proven otherwise
        bool firstSign = true;     // watch for extra '-'
        int elemIndex = 0;          // start of text for each element
        for (int n = 0; !done; ++n) {
            switch (text[n]) {
            case '-':
                if (isNumber && !firstSign) {
                    minc_warn("Malformed list argument ignored");
                    throw 1;
                }
                firstSign = false;
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '.':
                isNumber = true;
                break;
            case '}':
                done = true;
                // fall through
            case ',':
                firstSign = true;      // clear this
                if (isNumber) {
                    double f = atof(&text[elemIndex]);
                    listElem = new NodeListElem(listElem, new NodeConstf(f));
                }
                else {
                    // Make copy so we can edit it.
                    char *vcopy = strdup(&text[elemIndex]);
                    char *term = strchr(vcopy, ',');
                    if (!term) term = strchr(vcopy, '}');
                    if (!term) {
                        minc_warn("Malformed list argument ignored");
                        free(vcopy);
                        throw 1;
                    }
                    *term = '\0';
                    if (strlen(vcopy) > 0) {
                        // strsave the list argument before adding to node
                        listElem = new NodeListElem(listElem, new NodeString(strsave(vcopy)));
                    }
                    free(vcopy);
                }
                // Advance the index
                elemIndex = n + 1;
                break;
            case '\0':
                minc_warn("Malformed list argument ignored");
                throw 1;
                break;
            default:
                isNumber = false;
                break;
            }
        }
        listNode = new NodeList(listElem);
    } catch (int &ii) {
        RefCounted::unref(listElem);
        *pOutErr = 1;
    }
    return listNode;
}

static Node * parseScoreArgument(const char *text, int *pOutErr)
{
#ifndef EMBEDDED
    const char *token = strsave(text + 1);    // strip off '$'
    const char *value = lookup_token(token, true);        // returns NULL with warning
    if (value != NULL) {
        // Value can be a list (e.g., "{1,2,3}"), a float, or a string
        
        // Check first for a list
        if (value[0] == '{') {
            Node *n = parseListArgument(value, pOutErr);
            return (n) ? n : new NodeNoop();
        }
        // Check to see if it can be coaxed into a number,
        int i, is_number = 1;
        for(i = 0; value[i] != '\0'; ++i) {
            if ('-' == value[i] && i == 0)
                continue;    // allow initial sign
            if (! (isdigit(value[i]) || '.' == value[i]) ) {
                is_number = 0;
                break;
            }
        }
        if (is_number) {
            double f = atof(value);
            return new NodeConstf(f);
        }
        // else we store this as a string constant.
        else {
            // Strip off extra "" if present
            if (value[0] == '"' && value[strlen(value)-1] == '"') {
                char *vcopy = strsave(value+1);
                *strrchr(vcopy, '"') = '\0';
                return new NodeString(vcopy);
            }
            else {
                return new NodeString(strsave(value));
            }
        }
    }
    else {
        *pOutErr = 1;
        return new NodeNoop();
    }
#else
    minc_warn("Argument variables not supported");
    *pOutErr = 1;
    return new NodeNoop();
#endif
}

static Node *
go(Node * t1)
{
	if (level == 0) {
		MPRINT1("--> go(%p)", t1);
		try {
			t1->exct();
		}
        catch(const RTException rtex) {
            MPRINT1("caught fatal exception: '%s' - cleaning up and re-throwing", rtex.mesg());
            t1->unref();
            t1 = NULL;
            cleanup();
            throw;
        }
		catch(...) {
			MPRINT("caught other exception - cleaning up and re-throwing");
			t1->unref();
			t1 = NULL;
			cleanup();
			throw;
		}
		MPRINT1("<-- go(%p)", t1);
	}
	return t1;
}

int yywrap()
{
	return 1;
}

static void cleanup()
{
	rtcmix_debug("cleanup", "Freeing program tree %p", program);
    RefCounted::unref(program);
	program = NULL;
	/* Reset all static state */
	comments = 0;	// from lex.yy.c
	cpcomments = 0;
	xblock = 0;
	idcount = 0;
	flerror = 0;
	flevel = 0;
    sCurrentStructname = NULL;
	level = 0;
	include_stack_index = 0;
    if (!preserve_symbols) {
        free_symbols();
    }
#ifndef EMBEDDED
	/* BGG mm -- I think this buffer gets reused, so we don't delete it */
	yy_delete_buffer(YY_CURRENT_BUFFER);
	YY_CURRENT_BUFFER_LVALUE = NULL;
#endif
}

void preserveSymbols(bool preserve)
{
    preserve_symbols = preserve;
}

void reset_parser()
{
    rtcmix_debug("reset_parser", "resetting line number");
    flerror = 0;
    // Reset the line # every time a new score buffer is received
    yyset_lineno(1);
    // Special exported function from minc.l
    yy_clear_includes();
}

#ifdef EMBEDDED

// BGGx -- no #warning in VS!
// warning DAS Make sure yylex_destroy() works
#define USE_YYLEX_DESTROY

// BGG mm -- for dynamic memory mgmt (double return for UG_INTRO() macro)
double minc_memflush(double p[], int n_args)
{
	rtcmix_debug("minc_memflush", "Freeing parser memory");
    RefCounted::unref(program);
	program = NULL;
	free_symbols();
#ifdef USE_YYLEX_DESTROY
	yylex_destroy();
#else
	yy_delete_buffer(YY_CURRENT_BUFFER);
	YY_CURRENT_BUFFER_LVALUE = NULL;
	
	yy_init = 1;    /* whether we need to initialize */
	yy_start = 0;   /* start state number */
#endif
	return 1.0;
}

#endif

