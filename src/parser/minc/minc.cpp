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
     TOK_FLOAT_DECL = 274,
     TOK_STRING_DECL = 275,
     TOK_HANDLE_DECL = 276,
     TOK_LIST_DECL = 277,
     TOK_MAP_DECL = 278,
     TOK_FUNC_DECL = 279,
     TOK_IDENT = 280,
     TOK_NUM = 281,
     TOK_ARG_QUERY = 282,
     TOK_ARG = 283,
     TOK_NOT = 284,
     TOK_IF = 285,
     TOK_ELSE = 286,
     TOK_FOR = 287,
     TOK_WHILE = 288,
     TOK_RETURN = 289,
     TOK_TRUE = 290,
     TOK_FALSE = 291,
     TOK_STRING = 292
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
#define TOK_FLOAT_DECL 274
#define TOK_STRING_DECL 275
#define TOK_HANDLE_DECL 276
#define TOK_LIST_DECL 277
#define TOK_MAP_DECL 278
#define TOK_FUNC_DECL 279
#define TOK_IDENT 280
#define TOK_NUM 281
#define TOK_ARG_QUERY 282
#define TOK_ARG 283
#define TOK_NOT 284
#define TOK_IF 285
#define TOK_ELSE 286
#define TOK_FOR 287
#define TOK_WHILE 288
#define TOK_RETURN 289
#define TOK_TRUE 290
#define TOK_FALSE 291
#define TOK_STRING 292




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
double minc_memflush();
#else
// in args.cpp
const char *lookup_token(const char *token, bool printWarning);
#endif
#ifdef __cplusplus
}
#endif
#undef MDEBUG	/* turns on yacc debugging below */

#ifdef MDEBUG
//int yydebug=1;
#define MPRINT(x) rtcmix_print("YACC: %s\n", x)
#define MPRINT1(x,y) rtcmix_print("YACC: " x "\n", y)
#define MPRINT2(x,y,z) rtcmix_print("YACC: " x "\n", y, z)
#else
#define MPRINT(x)
#define MPRINT1(x,y)
#define MPRINT2(x,y,z)
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
static int      slevel = 0;     /* > 0 if we are in a struct decl block */
static int      xblock = 0;		/* 1 if we are entering a block preceeded by if(), else(), while(), or for() */
static bool     preserve_symbols = false;   /* what to do with symbol table at end of parse */
static void 	cleanup();
static void 	incrLevel();
static void		decrLevel();
static Node * declare(MincDataType type);
static Node * declareStruct(const char *typeName);
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
#line 241 "minc.cpp"

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
#define YYFINAL  86
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   678

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  55
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  38
/* YYNRULES -- Number of rules.  */
#define YYNRULES  143
/* YYNRULES -- Number of states.  */
#define YYNSTATES  258

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   292

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    54,     2,     2,
      48,    49,    21,    19,    53,    20,    50,    22,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    47,
      15,     4,    16,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    51,     2,    52,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    44,     2,    46,     2,     2,     2,     2,
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
      18,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    45
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     6,     8,    11,    14,    18,    20,
      22,    24,    26,    28,    30,    32,    34,    39,    46,    51,
      62,    64,    66,    68,    70,    73,    76,    79,    82,    85,
      88,    91,    94,    97,   100,   103,   106,   109,   110,   115,
     118,   121,   125,   128,   131,   134,   137,   140,   144,   147,
     148,   150,   154,   159,   161,   165,   170,   172,   174,   176,
     180,   184,   188,   192,   196,   200,   203,   206,   211,   215,
     222,   228,   230,   234,   236,   238,   242,   244,   246,   249,
     253,   257,   261,   265,   269,   273,   277,   281,   283,   285,
     287,   291,   295,   299,   303,   307,   311,   315,   317,   319,
     324,   327,   332,   336,   338,   340,   342,   344,   347,   349,
     352,   355,   358,   361,   364,   367,   370,   372,   376,   379,
     385,   386,   389,   392,   395,   398,   401,   405,   408,   412,
     416,   420,   424,   428,   433,   435,   439,   443,   446,   448,
     451,   453,   459,   466
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      56,     0,    -1,    -1,    57,    -1,    58,    -1,    58,    47,
      -1,    57,    58,    -1,    57,    58,    47,    -1,    74,    -1,
      62,    -1,    63,    -1,    64,    -1,    65,    -1,    66,    -1,
      67,    -1,    68,    -1,    37,    69,    79,    58,    -1,    37,
      69,    79,    58,    38,    58,    -1,    40,    69,    79,    58,
      -1,    39,    69,    48,    58,    47,    79,    47,    58,    49,
      58,    -1,    59,    -1,    92,    -1,    61,    -1,    84,    -1,
       1,    26,    -1,     1,    27,    -1,     1,    28,    -1,     1,
      29,    -1,     1,    30,    -1,     1,    31,    -1,     1,    37,
      -1,     1,    40,    -1,     1,    39,    -1,     1,    44,    -1,
       1,    38,    -1,     1,    41,    -1,     1,    47,    -1,    -1,
      44,    60,    57,    46,    -1,    44,    46,    -1,    41,    80,
      -1,    41,    80,    47,    -1,    26,    75,    -1,    27,    75,
      -1,    28,    75,    -1,    29,    75,    -1,    30,    75,    -1,
      25,    76,    75,    -1,    31,    75,    -1,    -1,    76,    -1,
      70,    50,    76,    -1,    70,    51,    80,    52,    -1,    76,
      -1,    70,    50,    76,    -1,    70,    51,    80,    52,    -1,
      80,    -1,    79,    -1,    72,    -1,    73,    53,    72,    -1,
      76,     4,    80,    -1,    76,     8,    80,    -1,    76,     7,
      80,    -1,    76,    10,    80,    -1,    76,     9,    80,    -1,
       5,    76,    -1,     6,    76,    -1,    71,    48,    73,    49,
      -1,    71,    48,    49,    -1,    70,    51,    80,    52,     4,
      80,    -1,    70,    50,    76,     4,    80,    -1,    76,    -1,
      75,    53,    76,    -1,    32,    -1,    80,    -1,    77,    53,
      80,    -1,    45,    -1,    80,    -1,    36,    79,    -1,    79,
      12,    79,    -1,    79,    11,    79,    -1,    79,    14,    79,
      -1,    80,    13,    80,    -1,    80,    15,    80,    -1,    80,
      16,    80,    -1,    80,    18,    80,    -1,    80,    17,    80,
      -1,    42,    -1,    43,    -1,    74,    -1,    80,    23,    80,
      -1,    80,    21,    80,    -1,    80,    22,    80,    -1,    80,
      19,    80,    -1,    80,    20,    80,    -1,    80,    54,    80,
      -1,    48,    79,    49,    -1,    78,    -1,    33,    -1,    44,
      69,    77,    46,    -1,    44,    46,    -1,    70,    51,    80,
      52,    -1,    70,    50,    76,    -1,    34,    -1,    35,    -1,
      42,    -1,    43,    -1,    20,    80,    -1,    76,    -1,    26,
      76,    -1,    27,    76,    -1,    28,    76,    -1,    29,    76,
      -1,    30,    76,    -1,    31,    76,    -1,    83,    76,    -1,
      81,    -1,    82,    53,    81,    -1,    25,    76,    -1,    83,
      44,    82,    46,    85,    -1,    -1,    26,    76,    -1,    27,
      76,    -1,    28,    76,    -1,    29,    76,    -1,    30,    76,
      -1,    25,    76,    76,    -1,    31,    76,    -1,    26,    76,
      90,    -1,    27,    76,    90,    -1,    28,    76,    90,    -1,
      29,    76,    90,    -1,    30,    76,    90,    -1,    25,    76,
      76,    90,    -1,    86,    -1,    88,    53,    86,    -1,    48,
      88,    49,    -1,    48,    49,    -1,    69,    -1,    57,    61,
      -1,    61,    -1,    87,    89,    44,    91,    46,    -1,     1,
      87,    89,    44,    57,    46,    -1,     1,    87,    89,    44,
      46,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   101,   101,   101,   105,   106,   107,   108,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   125,   130,   135,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   166,   166,   173,
     180,   190,   204,   209,   214,   219,   224,   229,   234,   241,
     245,   246,   247,   250,   251,   252,   256,   257,   261,   262,
     266,   267,   268,   269,   270,   272,   275,   279,   282,   285,
     289,   296,   297,   301,   305,   306,   310,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   333,
     334,   335,   336,   337,   338,   339,   340,   341,   342,   347,
     348,   350,   351,   352,   355,   358,   359,   360,   364,   373,
     374,   375,   376,   377,   378,   379,   388,   390,   396,   401,
     409,   421,   423,   425,   427,   429,   431,   433,   441,   443,
     445,   447,   449,   451,   457,   458,   468,   469,   474,   483,
     486,   493,   503,   504
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
  "CASTTOKEN", "TOK_STRUCT_DECL", "TOK_FLOAT_DECL", "TOK_STRING_DECL",
  "TOK_HANDLE_DECL", "TOK_LIST_DECL", "TOK_MAP_DECL", "TOK_FUNC_DECL",
  "TOK_IDENT", "TOK_NUM", "TOK_ARG_QUERY", "TOK_ARG", "TOK_NOT", "TOK_IF",
  "TOK_ELSE", "TOK_FOR", "TOK_WHILE", "TOK_RETURN", "TOK_TRUE",
  "TOK_FALSE", "'{'", "TOK_STRING", "'}'", "';'", "'('", "')'", "'.'",
  "'['", "']'", "','", "'%'", "$accept", "prg", "stml", "stmt", "bstml",
  "@1", "ret", "fdecl", "sdecl", "hdecl", "ldecl", "mdecl", "structdecl",
  "funcdecl", "level", "obj", "func", "fexp", "fexpl", "rstmt", "idl",
  "id", "expl", "str", "bexp", "exp", "mbr", "mbrl", "structname",
  "structdef", "struct", "arg", "funcname", "argl", "fargl", "function",
  "fstml", "fdef", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,    61,   259,   260,   261,   262,   263,
     264,   265,   266,   267,   268,    60,    62,   269,   270,    43,
      45,    42,    47,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   123,   292,   125,    59,    40,    41,
      46,    91,    93,    44,    37
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    55,    56,    56,    57,    57,    57,    57,    58,    58,
      58,    58,    58,    58,    58,    58,    58,    58,    58,    58,
      58,    58,    58,    58,    58,    58,    58,    58,    58,    58,
      58,    58,    58,    58,    58,    58,    58,    60,    59,    59,
      61,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    70,    70,    71,    71,    71,    72,    72,    73,    73,
      74,    74,    74,    74,    74,    74,    74,    74,    74,    74,
      74,    75,    75,    76,    77,    77,    78,    79,    79,    79,
      79,    79,    79,    79,    79,    79,    79,    79,    79,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    81,
      81,    81,    81,    81,    81,    81,    82,    82,    83,    84,
      85,    86,    86,    86,    86,    86,    86,    86,    87,    87,
      87,    87,    87,    87,    88,    88,    89,    89,    90,    91,
      91,    92,    92,    92
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     1,     1,     2,     2,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     4,     6,     4,    10,
       1,     1,     1,     1,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     0,     4,     2,
       2,     3,     2,     2,     2,     2,     2,     3,     2,     0,
       1,     3,     4,     1,     3,     4,     1,     1,     1,     3,
       3,     3,     3,     3,     3,     2,     2,     4,     3,     6,
       5,     1,     3,     1,     1,     3,     1,     1,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     1,     1,     1,
       3,     3,     3,     3,     3,     3,     3,     1,     1,     4,
       2,     4,     3,     1,     1,     1,     1,     2,     1,     2,
       2,     2,     2,     2,     2,     2,     1,     3,     2,     5,
       0,     2,     2,     2,     2,     2,     3,     2,     3,     3,
       3,     3,     3,     4,     1,     3,     3,     2,     1,     2,
       1,     5,     6,     5
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    73,    49,    49,    49,     0,    37,     0,     0,     4,
      20,    22,     9,    10,    11,    12,    13,    14,    15,     0,
       0,     8,    50,     0,    23,     0,    21,     0,    24,    25,
      26,    27,    28,    29,    30,    34,    32,    31,    35,    33,
      36,     0,    65,    66,   118,    42,    71,    43,    71,    44,
      71,    45,    71,    46,    71,    48,    71,     0,     0,     0,
       0,    98,   103,   104,   105,   106,    49,    76,     0,     0,
      89,   108,    97,    40,    39,     0,     1,     6,     5,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    49,    49,    49,    49,    49,     0,    47,    71,     0,
     138,   128,   129,   130,   131,   132,     0,    87,    88,     0,
      77,     0,     0,   107,   100,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    41,     0,     0,     7,    51,     0,
      68,    58,     0,    57,    77,    60,    62,    61,    64,    63,
       0,     0,     0,     0,     0,     0,     0,   116,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   137,   134,     0,
       0,    49,     0,   133,    72,    78,     0,     0,     0,    16,
       0,     0,     0,     0,     0,     0,    18,     0,    74,    96,
     102,     0,    93,    94,    91,    92,    90,    95,    38,     0,
      52,    67,     0,   118,   109,   110,   111,   112,   113,   114,
     120,     0,   115,     0,   121,   122,   123,   124,   125,   127,
     136,     0,     0,    22,     0,   143,     0,    80,    79,    81,
       0,    82,    83,    84,    86,    85,     0,    99,     0,   101,
      70,     0,    59,   119,   117,   126,   135,    22,   141,   142,
      17,     0,    75,    69,     0,     0,     0,    19
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    17,    18,    19,    20,    85,    21,    22,    23,    24,
      25,    26,    27,    28,   110,    79,    30,   141,   142,    80,
      55,    81,   187,    82,   143,   120,   157,   158,    33,    34,
     243,   168,    35,   169,    99,   111,   224,    36
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -157
static const yytype_int16 yypact[] =
{
     423,   624,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  -157,  -157,  -157,  -157,   515,     2,    53,   465,    -2,
    -157,  -157,  -157,  -157,  -157,  -157,  -157,  -157,  -157,    44,
      14,  -157,    60,    33,  -157,    38,  -157,    -1,    -1,    -1,
      -1,    -1,    -1,  -157,  -157,  -157,  -157,  -157,  -157,  -157,
    -157,    38,  -157,  -157,    -1,     3,    68,     3,    68,     3,
      68,     3,    68,     3,    68,     3,  -157,   398,    77,   398,
     515,  -157,  -157,  -157,  -157,  -157,    57,  -157,   398,    73,
    -157,    42,  -157,   305,  -157,   539,  -157,    81,  -157,    -1,
     515,   162,   515,   515,   515,   515,   515,   339,   611,    85,
      -1,  -157,  -157,  -157,  -157,  -157,    87,     3,    68,    -1,
    -157,  -157,  -157,  -157,  -157,  -157,   398,   461,   580,   502,
     591,   539,   502,    80,  -157,   515,    90,    -1,   515,   515,
     515,   515,   515,   515,  -157,   515,   218,  -157,    32,   251,
    -157,  -157,   -19,   106,   569,   605,   605,   605,   605,   605,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  -157,    35,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  -157,  -157,    12,
     539,  -157,   317,  -157,  -157,  -157,   398,   398,   398,    97,
     515,   515,   515,   515,   515,    94,  -157,    45,   605,  -157,
      24,   361,    84,    84,    -7,    -7,    80,   605,  -157,   515,
      39,  -157,   398,  -157,  -157,  -157,  -157,  -157,  -157,  -157,
    -157,   339,  -157,    -1,  -157,  -157,  -157,  -157,  -157,  -157,
    -157,   647,   539,    91,   102,  -157,   370,   100,   131,  -157,
     539,   605,   605,   605,   605,   605,   398,  -157,   515,    28,
     605,   515,  -157,  -157,  -157,  -157,  -157,   103,  -157,  -157,
    -157,    99,   605,   605,   539,   108,   539,  -157
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -157,  -157,   -73,    65,  -157,  -157,  -156,  -157,  -157,  -157,
    -157,  -157,  -157,  -157,     8,    11,  -157,   -44,  -157,    55,
      17,     0,  -157,  -157,   -34,   206,   -42,  -157,   -96,  -157,
    -157,   -50,   172,  -157,   124,   -45,  -157,  -157
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -141
static const yytype_int16 yytable[] =
{
      32,   159,    52,    53,    54,    56,    58,    60,    62,    64,
      66,    29,   136,   112,   223,   113,   133,   114,    32,   115,
      67,    68,    69,    57,    59,    61,    63,    65,   199,    29,
     201,    11,   241,   119,   202,   122,   199,   100,   101,   102,
     103,   104,   105,   241,   126,    88,    92,   135,    84,    93,
      94,    95,    96,    86,   108,    31,   109,   112,   113,   114,
     115,   220,    91,   173,    92,   221,   247,    93,    94,    95,
      96,   107,   -54,    31,   -51,   -51,   -55,    97,   -52,   -52,
     -54,   210,   175,    87,   125,    32,    98,   -55,   211,   138,
     -53,   237,   -50,   -50,    89,    90,    29,   222,   238,   226,
     171,   176,   177,   124,   178,   131,   132,   133,   -53,   174,
     176,   177,   177,   178,   178,   159,   -49,   176,   177,    32,
     178,    32,    32,   127,   128,   121,   173,   190,   137,   170,
      29,   172,    29,    29,   135,   230,    32,  -140,   135,   189,
      31,   236,   227,   228,   229,   178,   254,    29,   248,  -139,
     203,   204,   205,   206,   207,   208,   209,   256,   242,   212,
     213,   214,   215,   216,   217,   218,   219,     2,     3,   244,
      32,   246,    32,    51,    31,   106,    31,    31,     0,     0,
       0,    29,    70,    29,   179,     0,   185,   186,     0,     0,
       0,    31,     0,     0,    11,    71,    72,    73,   116,     0,
       0,    87,   251,     0,   117,   118,    76,    77,     0,     0,
      78,   140,     0,   245,     0,     0,     0,     0,     0,     1,
       0,    83,    32,     2,     3,    31,    32,    31,     0,     0,
      32,     0,     0,    29,     0,     0,     0,    29,     0,     0,
       0,    29,     0,     4,     5,     6,     7,     8,     9,    10,
      11,     0,     0,     0,    32,    12,    32,    13,    14,    15,
       0,     0,    16,     0,   198,    29,     0,    29,     0,     0,
     129,   130,   131,   132,   133,     0,   123,    31,     0,     0,
       0,    31,     0,     0,     0,    31,     0,    87,     0,     0,
       0,    87,     0,     0,     0,   250,   139,   144,   145,   146,
     147,   148,   149,   200,     0,   135,     0,     0,     0,    31,
       0,    31,     0,     0,     0,     0,     0,     0,     1,   255,
       0,   257,     2,     3,   129,   130,   131,   132,   133,     0,
       0,   188,     0,     0,   191,   192,   193,   194,   195,   196,
       0,   197,     4,     5,     6,     7,     8,     9,    10,    11,
       0,     0,   134,     0,    12,     0,    13,    14,    15,   135,
       0,    16,     0,   225,   150,   151,   152,   153,   154,   155,
     156,     1,     0,     0,     0,     2,     3,     0,     0,     0,
     129,   130,   131,   132,   133,     0,   231,   232,   233,   234,
     235,     0,     0,     0,     0,     4,     5,     6,     7,     8,
       9,    10,    11,     2,     3,   240,     0,    12,   144,    13,
      14,    15,     0,   239,    16,   135,   249,     0,    70,     0,
       0,     0,     0,    -2,     1,     0,     0,     0,     2,     3,
      11,    71,    72,    73,   116,     0,     0,     0,     0,     0,
     117,   118,    76,    77,   252,     0,    78,   253,     4,     5,
       6,     7,     8,     9,    10,    11,     0,     0,     0,     0,
      12,     0,    13,    14,    15,    -3,     1,    16,     0,     0,
       2,     3,     0,     0,  -105,     0,  -105,  -105,  -105,  -105,
    -105,  -105,  -105,  -105,  -105,     0,     0,     0,     0,     0,
       4,     5,     6,     7,     8,     9,    10,    11,     0,     0,
       0,     0,    12,     1,    13,    14,    15,     2,     3,    16,
       0,     0,     0,   176,   177,  -105,   178,     0,     0,     0,
       2,     3,     0,     0,     0,     0,     0,     4,     5,     6,
       7,     8,     9,    10,    11,    70,     0,     0,     0,    12,
       1,    13,    14,    15,     2,     3,    16,    11,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,    75,    76,
      77,     0,     0,    78,     4,     5,     6,     7,     8,     9,
      10,    11,     0,     0,     0,     0,    12,     0,    13,    14,
      15,     0,   180,    16,   181,   182,   183,   184,   129,   130,
     131,   132,   133,  -106,     0,  -106,  -106,  -106,  -106,  -106,
    -106,  -106,  -106,  -106,   180,     0,   181,   182,   183,   184,
     129,   130,   131,   132,   133,     0,     0,     0,   -56,     0,
       0,     0,   -56,   135,   129,   130,   131,   132,   133,     0,
       0,     0,     0,     0,  -106,     0,   160,   161,   162,   163,
     164,   165,   166,     0,     0,   135,     0,     0,     0,    37,
      38,    39,    40,    41,    42,    43,     0,     0,     0,   135,
     167,    44,    45,    46,    47,    48,     0,     0,    49,     0,
       0,    50,   160,   161,   162,   163,   164,   165,   166
};

static const yytype_int16 yycheck[] =
{
       0,    97,     2,     3,     4,     5,     6,     7,     8,     9,
      10,     0,    85,    58,   170,    60,    23,    62,    18,    64,
      12,    13,    14,     6,     7,     8,     9,    10,     4,    18,
      49,    32,     4,    67,    53,    69,     4,    37,    38,    39,
      40,    41,    42,     4,    78,    47,     4,    54,    46,     7,
       8,     9,    10,     0,    54,     0,    53,   102,   103,   104,
     105,    49,    48,   108,     4,    53,   222,     7,     8,     9,
      10,    54,    48,    18,    50,    51,    48,    44,    50,    51,
      48,    46,   116,    18,    76,    85,    48,    48,    53,    89,
      48,    46,    50,    51,    50,    51,    85,   170,    53,   172,
     100,    11,    12,    46,    14,    21,    22,    23,    48,   109,
      11,    12,    12,    14,    14,   211,    48,    11,    12,   119,
      14,   121,   122,    50,    51,    48,   171,   127,    47,    44,
     119,    44,   121,   122,    54,    38,   136,    46,    54,    49,
      85,    47,   176,   177,   178,    14,    47,   136,    46,    46,
     150,   151,   152,   153,   154,   155,   156,    49,   202,   159,
     160,   161,   162,   163,   164,   165,   166,     5,     6,   211,
     170,   221,   172,     1,   119,    51,   121,   122,    -1,    -1,
      -1,   170,    20,   172,   119,    -1,   121,   122,    -1,    -1,
      -1,   136,    -1,    -1,    32,    33,    34,    35,    36,    -1,
      -1,   136,   236,    -1,    42,    43,    44,    45,    -1,    -1,
      48,    49,    -1,   213,    -1,    -1,    -1,    -1,    -1,     1,
      -1,    15,   222,     5,     6,   170,   226,   172,    -1,    -1,
     230,    -1,    -1,   222,    -1,    -1,    -1,   226,    -1,    -1,
      -1,   230,    -1,    25,    26,    27,    28,    29,    30,    31,
      32,    -1,    -1,    -1,   254,    37,   256,    39,    40,    41,
      -1,    -1,    44,    -1,    46,   254,    -1,   256,    -1,    -1,
      19,    20,    21,    22,    23,    -1,    70,   222,    -1,    -1,
      -1,   226,    -1,    -1,    -1,   230,    -1,   222,    -1,    -1,
      -1,   226,    -1,    -1,    -1,   230,    90,    91,    92,    93,
      94,    95,    96,    52,    -1,    54,    -1,    -1,    -1,   254,
      -1,   256,    -1,    -1,    -1,    -1,    -1,    -1,     1,   254,
      -1,   256,     5,     6,    19,    20,    21,    22,    23,    -1,
      -1,   125,    -1,    -1,   128,   129,   130,   131,   132,   133,
      -1,   135,    25,    26,    27,    28,    29,    30,    31,    32,
      -1,    -1,    47,    -1,    37,    -1,    39,    40,    41,    54,
      -1,    44,    -1,    46,    25,    26,    27,    28,    29,    30,
      31,     1,    -1,    -1,    -1,     5,     6,    -1,    -1,    -1,
      19,    20,    21,    22,    23,    -1,   180,   181,   182,   183,
     184,    -1,    -1,    -1,    -1,    25,    26,    27,    28,    29,
      30,    31,    32,     5,     6,   199,    -1,    37,   202,    39,
      40,    41,    -1,    52,    44,    54,    46,    -1,    20,    -1,
      -1,    -1,    -1,     0,     1,    -1,    -1,    -1,     5,     6,
      32,    33,    34,    35,    36,    -1,    -1,    -1,    -1,    -1,
      42,    43,    44,    45,   238,    -1,    48,   241,    25,    26,
      27,    28,    29,    30,    31,    32,    -1,    -1,    -1,    -1,
      37,    -1,    39,    40,    41,     0,     1,    44,    -1,    -1,
       5,     6,    -1,    -1,    13,    -1,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    -1,    -1,    -1,    -1,    -1,
      25,    26,    27,    28,    29,    30,    31,    32,    -1,    -1,
      -1,    -1,    37,     1,    39,    40,    41,     5,     6,    44,
      -1,    -1,    -1,    11,    12,    54,    14,    -1,    -1,    -1,
       5,     6,    -1,    -1,    -1,    -1,    -1,    25,    26,    27,
      28,    29,    30,    31,    32,    20,    -1,    -1,    -1,    37,
       1,    39,    40,    41,     5,     6,    44,    32,    33,    34,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    44,
      45,    -1,    -1,    48,    25,    26,    27,    28,    29,    30,
      31,    32,    -1,    -1,    -1,    -1,    37,    -1,    39,    40,
      41,    -1,    13,    44,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    13,    -1,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    13,    -1,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    -1,    -1,    -1,    49,    -1,
      -1,    -1,    53,    54,    19,    20,    21,    22,    23,    -1,
      -1,    -1,    -1,    -1,    54,    -1,    25,    26,    27,    28,
      29,    30,    31,    -1,    -1,    54,    -1,    -1,    -1,    25,
      26,    27,    28,    29,    30,    31,    -1,    -1,    -1,    54,
      49,    37,    38,    39,    40,    41,    -1,    -1,    44,    -1,
      -1,    47,    25,    26,    27,    28,    29,    30,    31
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     5,     6,    25,    26,    27,    28,    29,    30,
      31,    32,    37,    39,    40,    41,    44,    56,    57,    58,
      59,    61,    62,    63,    64,    65,    66,    67,    68,    70,
      71,    74,    76,    83,    84,    87,    92,    25,    26,    27,
      28,    29,    30,    31,    37,    38,    39,    40,    41,    44,
      47,    87,    76,    76,    76,    75,    76,    75,    76,    75,
      76,    75,    76,    75,    76,    75,    76,    69,    69,    69,
      20,    33,    34,    35,    42,    43,    44,    45,    48,    70,
      74,    76,    78,    80,    46,    60,     0,    58,    47,    50,
      51,    48,     4,     7,     8,     9,    10,    44,    48,    89,
      76,    76,    76,    76,    76,    76,    89,    75,    76,    53,
      69,    90,    90,    90,    90,    90,    36,    42,    43,    79,
      80,    48,    79,    80,    46,    69,    79,    50,    51,    19,
      20,    21,    22,    23,    47,    54,    57,    47,    76,    80,
      49,    72,    73,    79,    80,    80,    80,    80,    80,    80,
      25,    26,    27,    28,    29,    30,    31,    81,    82,    83,
      25,    26,    27,    28,    29,    30,    31,    49,    86,    88,
      44,    76,    44,    90,    76,    79,    11,    12,    14,    58,
      13,    15,    16,    17,    18,    58,    58,    77,    80,    49,
      76,    80,    80,    80,    80,    80,    80,    80,    46,     4,
      52,    49,    53,    76,    76,    76,    76,    76,    76,    76,
      46,    53,    76,    76,    76,    76,    76,    76,    76,    76,
      49,    53,    57,    61,    91,    46,    57,    79,    79,    79,
      38,    80,    80,    80,    80,    80,    47,    46,    53,    52,
      80,     4,    72,    85,    81,    76,    86,    61,    46,    46,
      58,    79,    80,    80,    47,    58,    49,    58
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
      case 57: /* "stml" */
#line 95 "minc.y"
	{ MPRINT1("yydestruct deleting node %p\n", (yyvaluep->node)); delete (yyvaluep->node); };
#line 1455 "minc.cpp"
	break;
      case 58: /* "stmt" */
#line 95 "minc.y"
	{ MPRINT1("yydestruct deleting node %p\n", (yyvaluep->node)); delete (yyvaluep->node); };
#line 1460 "minc.cpp"
	break;
      case 59: /* "bstml" */
#line 95 "minc.y"
	{ MPRINT1("yydestruct deleting node %p\n", (yyvaluep->node)); delete (yyvaluep->node); };
#line 1465 "minc.cpp"
	break;
      case 61: /* "ret" */
#line 95 "minc.y"
	{ MPRINT1("yydestruct deleting node %p\n", (yyvaluep->node)); delete (yyvaluep->node); };
#line 1470 "minc.cpp"
	break;
      case 62: /* "fdecl" */
#line 95 "minc.y"
	{ MPRINT1("yydestruct deleting node %p\n", (yyvaluep->node)); delete (yyvaluep->node); };
#line 1475 "minc.cpp"
	break;
      case 63: /* "sdecl" */
#line 95 "minc.y"
	{ MPRINT1("yydestruct deleting node %p\n", (yyvaluep->node)); delete (yyvaluep->node); };
#line 1480 "minc.cpp"
	break;
      case 64: /* "hdecl" */
#line 95 "minc.y"
	{ MPRINT1("yydestruct deleting node %p\n", (yyvaluep->node)); delete (yyvaluep->node); };
#line 1485 "minc.cpp"
	break;
      case 65: /* "ldecl" */
#line 95 "minc.y"
	{ MPRINT1("yydestruct deleting node %p\n", (yyvaluep->node)); delete (yyvaluep->node); };
#line 1490 "minc.cpp"
	break;
      case 66: /* "mdecl" */
#line 95 "minc.y"
	{ MPRINT1("yydestruct deleting node %p\n", (yyvaluep->node)); delete (yyvaluep->node); };
#line 1495 "minc.cpp"
	break;
      case 67: /* "structdecl" */
#line 95 "minc.y"
	{ MPRINT1("yydestruct deleting node %p\n", (yyvaluep->node)); delete (yyvaluep->node); };
#line 1500 "minc.cpp"
	break;
      case 68: /* "funcdecl" */
#line 95 "minc.y"
	{ MPRINT1("yydestruct deleting node %p\n", (yyvaluep->node)); delete (yyvaluep->node); };
#line 1505 "minc.cpp"
	break;
      case 70: /* "obj" */
#line 95 "minc.y"
	{ MPRINT1("yydestruct deleting node %p\n", (yyvaluep->node)); delete (yyvaluep->node); };
#line 1510 "minc.cpp"
	break;
      case 72: /* "fexp" */
#line 95 "minc.y"
	{ MPRINT1("yydestruct deleting node %p\n", (yyvaluep->node)); delete (yyvaluep->node); };
#line 1515 "minc.cpp"
	break;
      case 73: /* "fexpl" */
#line 95 "minc.y"
	{ MPRINT1("yydestruct deleting node %p\n", (yyvaluep->node)); delete (yyvaluep->node); };
#line 1520 "minc.cpp"
	break;
      case 74: /* "rstmt" */
#line 95 "minc.y"
	{ MPRINT1("yydestruct deleting node %p\n", (yyvaluep->node)); delete (yyvaluep->node); };
#line 1525 "minc.cpp"
	break;
      case 77: /* "expl" */
#line 95 "minc.y"
	{ MPRINT1("yydestruct deleting node %p\n", (yyvaluep->node)); delete (yyvaluep->node); };
#line 1530 "minc.cpp"
	break;
      case 78: /* "str" */
#line 95 "minc.y"
	{ MPRINT1("yydestruct deleting node %p\n", (yyvaluep->node)); delete (yyvaluep->node); };
#line 1535 "minc.cpp"
	break;
      case 79: /* "bexp" */
#line 95 "minc.y"
	{ MPRINT1("yydestruct deleting node %p\n", (yyvaluep->node)); delete (yyvaluep->node); };
#line 1540 "minc.cpp"
	break;
      case 80: /* "exp" */
#line 95 "minc.y"
	{ MPRINT1("yydestruct deleting node %p\n", (yyvaluep->node)); delete (yyvaluep->node); };
#line 1545 "minc.cpp"
	break;
      case 81: /* "mbr" */
#line 95 "minc.y"
	{ MPRINT1("yydestruct deleting node %p\n", (yyvaluep->node)); delete (yyvaluep->node); };
#line 1550 "minc.cpp"
	break;
      case 82: /* "mbrl" */
#line 95 "minc.y"
	{ MPRINT1("yydestruct deleting node %p\n", (yyvaluep->node)); delete (yyvaluep->node); };
#line 1555 "minc.cpp"
	break;
      case 84: /* "structdef" */
#line 95 "minc.y"
	{ MPRINT1("yydestruct deleting node %p\n", (yyvaluep->node)); delete (yyvaluep->node); };
#line 1560 "minc.cpp"
	break;
      case 86: /* "arg" */
#line 95 "minc.y"
	{ MPRINT1("yydestruct deleting node %p\n", (yyvaluep->node)); delete (yyvaluep->node); };
#line 1565 "minc.cpp"
	break;
      case 87: /* "funcname" */
#line 95 "minc.y"
	{ MPRINT1("yydestruct deleting node %p\n", (yyvaluep->node)); delete (yyvaluep->node); };
#line 1570 "minc.cpp"
	break;
      case 88: /* "argl" */
#line 95 "minc.y"
	{ MPRINT1("yydestruct deleting node %p\n", (yyvaluep->node)); delete (yyvaluep->node); };
#line 1575 "minc.cpp"
	break;
      case 89: /* "fargl" */
#line 95 "minc.y"
	{ MPRINT1("yydestruct deleting node %p\n", (yyvaluep->node)); delete (yyvaluep->node); };
#line 1580 "minc.cpp"
	break;
      case 91: /* "fstml" */
#line 95 "minc.y"
	{ MPRINT1("yydestruct deleting node %p\n", (yyvaluep->node)); delete (yyvaluep->node); };
#line 1585 "minc.cpp"
	break;
      case 92: /* "fdef" */
#line 95 "minc.y"
	{ MPRINT1("yydestruct deleting node %p\n", (yyvaluep->node)); delete (yyvaluep->node); };
#line 1590 "minc.cpp"
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
#line 101 "minc.y"
    { MPRINT("prg:"); program = (yyvsp[(1) - (1)].node); cleanup(); return 0; ;}
    break;

  case 4:
#line 105 "minc.y"
    { MPRINT("stml:	stmt"); (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 5:
#line 106 "minc.y"
    { MPRINT("stml:	stmt;"); (yyval.node) = (yyvsp[(1) - (2)].node); ;}
    break;

  case 6:
#line 107 "minc.y"
    { MPRINT("stml:	stml stmt"); (yyval.node) = new NodeSeq((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node)); ;}
    break;

  case 7:
#line 108 "minc.y"
    { MPRINT("stml:	stml stmt;"); (yyval.node) = new NodeSeq((yyvsp[(1) - (3)].node), (yyvsp[(2) - (3)].node)); ;}
    break;

  case 8:
#line 112 "minc.y"
    { MPRINT("rstmt");	(yyval.node) = go((yyvsp[(1) - (1)].node)); ;}
    break;

  case 16:
#line 120 "minc.y"
    {	xblock = 1; MPRINT("IF bexp stmt");
								decrLevel();
								(yyval.node) = go(new NodeIf((yyvsp[(3) - (4)].node), (yyvsp[(4) - (4)].node)));
								xblock = 0;
							;}
    break;

  case 17:
#line 125 "minc.y"
    { xblock = 1;
								decrLevel();
								(yyval.node) = go(new NodeIfElse((yyvsp[(3) - (6)].node), (yyvsp[(4) - (6)].node), (yyvsp[(6) - (6)].node)));
								xblock = 0;
							;}
    break;

  case 18:
#line 130 "minc.y"
    { xblock = 1;	MPRINT("WHILE bexp stmt");
								decrLevel();
								(yyval.node) = go(new NodeWhile((yyvsp[(3) - (4)].node), (yyvsp[(4) - (4)].node)));
								xblock = 0;
							;}
    break;

  case 19:
#line 135 "minc.y"
    { xblock = 1;
								decrLevel();
								(yyval.node) = go(new NodeFor((yyvsp[(4) - (10)].node), (yyvsp[(6) - (10)].node), (yyvsp[(8) - (10)].node), (yyvsp[(10) - (10)].node)));
								xblock = 0;
							;}
    break;

  case 24:
#line 144 "minc.y"
    { flerror = 1; (yyval.node) = new NodeNoop(); ;}
    break;

  case 25:
#line 145 "minc.y"
    { flerror = 1; (yyval.node) = new NodeNoop(); ;}
    break;

  case 26:
#line 146 "minc.y"
    { flerror = 1; (yyval.node) = new NodeNoop(); ;}
    break;

  case 27:
#line 147 "minc.y"
    { flerror = 1; (yyval.node) = new NodeNoop(); ;}
    break;

  case 28:
#line 148 "minc.y"
    { flerror = 1; (yyval.node) = new NodeNoop(); ;}
    break;

  case 29:
#line 149 "minc.y"
    { flerror = 1; (yyval.node) = new NodeNoop(); ;}
    break;

  case 30:
#line 150 "minc.y"
    { flerror = 1; (yyval.node) = new NodeNoop(); ;}
    break;

  case 31:
#line 151 "minc.y"
    { flerror = 1; (yyval.node) = new NodeNoop(); ;}
    break;

  case 32:
#line 152 "minc.y"
    { flerror = 1; (yyval.node) = new NodeNoop(); ;}
    break;

  case 33:
#line 153 "minc.y"
    { flerror = 1; (yyval.node) = new NodeNoop(); ;}
    break;

  case 34:
#line 154 "minc.y"
    { flerror = 1; (yyval.node) = new NodeNoop(); ;}
    break;

  case 35:
#line 155 "minc.y"
    { flerror = 1; (yyval.node) = new NodeNoop(); ;}
    break;

  case 36:
#line 156 "minc.y"
    { flerror = 1; (yyval.node) = new NodeNoop(); ;}
    break;

  case 37:
#line 166 "minc.y"
    { if (!xblock) incrLevel(); ;}
    break;

  case 38:
#line 168 "minc.y"
    { 	MPRINT("bstml: { stml }"); MPRINT2("level = %d, xblock = %d", level, xblock);
									if (!xblock) { decrLevel(); }
									(yyval.node) = go(new NodeBlock((yyvsp[(3) - (4)].node)));
								;}
    break;

  case 39:
#line 173 "minc.y"
    { 	MPRINT("bstml: {}");
								(yyval.node) = go(new NodeBlock(new NodeNoop()));
								;}
    break;

  case 40:
#line 180 "minc.y"
    {	MPRINT("ret exp");
								MPRINT1("called at level %d", level);
								if (flevel == 0) {
									minc_die("return statements not allowed in main score");
									(yyval.node) = new NodeNoop();
								}
								else {
									(yyval.node) = new NodeRet((yyvsp[(2) - (2)].node));
								}
							;}
    break;

  case 41:
#line 190 "minc.y"
    {	MPRINT("ret exp;");
								MPRINT1("called at level %d", level);
								if (flevel == 0) {
									minc_die("return statements not allowed in main score");
									(yyval.node) = new NodeNoop();
								}
								else {
									(yyval.node) = new NodeRet((yyvsp[(2) - (3)].node));
								}
							;}
    break;

  case 42:
#line 204 "minc.y"
    { 	MPRINT("fdecl");
								(yyval.node) = go(declare(MincFloatType));
								idcount = 0;
							;}
    break;

  case 43:
#line 209 "minc.y"
    { 	MPRINT("sdecl");
								(yyval.node) = go(declare(MincStringType));
								idcount = 0;
							;}
    break;

  case 44:
#line 214 "minc.y"
    { 	MPRINT("hdecl");
								(yyval.node) = go(declare(MincHandleType));
								idcount = 0;
							;}
    break;

  case 45:
#line 219 "minc.y"
    { 	MPRINT("ldecl");
								(yyval.node) = go(declare(MincListType));
								idcount = 0;
							;}
    break;

  case 46:
#line 224 "minc.y"
    {     MPRINT("mdecl");
                                (yyval.node) = go(declare(MincMapType));
                                idcount = 0;
                              ;}
    break;

  case 47:
#line 229 "minc.y"
    {     MPRINT("structdecl");
                                            (yyval.node) = go(declareStruct((yyvsp[(2) - (3)].str)));
                                            idcount = 0;
                                        ;}
    break;

  case 48:
#line 234 "minc.y"
    {     MPRINT("funcdecl");
                                            (yyval.node) = go(declare(MincFunctionType));
                                            idcount = 0;
                                        ;}
    break;

  case 49:
#line 241 "minc.y"
    { incrLevel(); ;}
    break;

  case 50:
#line 245 "minc.y"
    {       MPRINT("obj: id");          (yyval.node) = new NodeLoadSym((yyvsp[(1) - (1)].str)); ;}
    break;

  case 51:
#line 246 "minc.y"
    {       MPRINT("obj: obj.id");      (yyval.node) = new NodeMember((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].str));  ;}
    break;

  case 52:
#line 247 "minc.y"
    {       MPRINT("obj: obj[exp]");    (yyval.node) = new NodeSubscriptRead((yyvsp[(1) - (4)].node), (yyvsp[(3) - (4)].node)); ;}
    break;

  case 53:
#line 250 "minc.y"
    {       MPRINT("func: id");         (yyval.node) = new NodeLoadFuncSym((yyvsp[(1) - (1)].str)); ;}
    break;

  case 54:
#line 251 "minc.y"
    {       MPRINT("func: obj.id");     (yyval.node) = new NodeMember((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].str)); ;}
    break;

  case 55:
#line 252 "minc.y"
    {       MPRINT("func: obj[exp]");   (yyval.node) = new NodeSubscriptRead((yyvsp[(1) - (4)].node), (yyvsp[(3) - (4)].node)); ;}
    break;

  case 56:
#line 256 "minc.y"
    { MPRINT("fexp: exp"); (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 57:
#line 257 "minc.y"
    { MPRINT("fexp: bexp"); (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 58:
#line 261 "minc.y"
    { MPRINT("fexpl: fexp"); (yyval.node) = new NodeListElem(new NodeEmptyListElem(), (yyvsp[(1) - (1)].node)); ;}
    break;

  case 59:
#line 262 "minc.y"
    {  MPRINT("fexpl: fexpl,fexp"); (yyval.node) = new NodeListElem((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 60:
#line 266 "minc.y"
    { MPRINT("rstmt: id = exp");		(yyval.node) = new NodeStore(new NodeAutoDeclLoadSym((yyvsp[(1) - (3)].str)), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 61:
#line 267 "minc.y"
    {		(yyval.node) = new NodeOpAssign(new NodeLoadSym((yyvsp[(1) - (3)].str)), (yyvsp[(3) - (3)].node), OpPlus); ;}
    break;

  case 62:
#line 268 "minc.y"
    {		(yyval.node) = new NodeOpAssign(new NodeLoadSym((yyvsp[(1) - (3)].str)), (yyvsp[(3) - (3)].node), OpMinus); ;}
    break;

  case 63:
#line 269 "minc.y"
    {		(yyval.node) = new NodeOpAssign(new NodeLoadSym((yyvsp[(1) - (3)].str)), (yyvsp[(3) - (3)].node), OpMul); ;}
    break;

  case 64:
#line 270 "minc.y"
    {		(yyval.node) = new NodeOpAssign(new NodeLoadSym((yyvsp[(1) - (3)].str)), (yyvsp[(3) - (3)].node), OpDiv); ;}
    break;

  case 65:
#line 272 "minc.y"
    {
        (yyval.node) = new NodeOpAssign(new NodeLoadSym((yyvsp[(2) - (2)].str)), new NodeConstf(1.0), OpPlusPlus);
    ;}
    break;

  case 66:
#line 275 "minc.y"
    {
        (yyval.node) = new NodeOpAssign(new NodeLoadSym((yyvsp[(2) - (2)].str)), new NodeConstf(1.0), OpMinusMinus);
    ;}
    break;

  case 67:
#line 279 "minc.y"
    {	MPRINT("rstmt: func(fexpl)");
								(yyval.node) = new NodeCall((yyvsp[(1) - (4)].node), (yyvsp[(3) - (4)].node));
							;}
    break;

  case 68:
#line 282 "minc.y"
    {			MPRINT("rstmt: func()");
								(yyval.node) = new NodeCall((yyvsp[(1) - (3)].node), new NodeEmptyListElem());
							;}
    break;

  case 69:
#line 285 "minc.y"
    {
                                MPRINT("rstmt: obj[exp] = exp");
                                (yyval.node) = new NodeSubscriptWrite((yyvsp[(1) - (6)].node), (yyvsp[(3) - (6)].node), (yyvsp[(6) - (6)].node));
                            ;}
    break;

  case 70:
#line 289 "minc.y"
    {
                                MPRINT("rstmt: obj.id = exp");
                                (yyval.node) = new NodeStore(new NodeMember((yyvsp[(1) - (5)].node), (yyvsp[(3) - (5)].str)), (yyvsp[(5) - (5)].node));
                            ;}
    break;

  case 71:
#line 296 "minc.y"
    { MPRINT("idl: id"); idlist[idcount++] = (yyvsp[(1) - (1)].str); ;}
    break;

  case 72:
#line 297 "minc.y"
    { MPRINT("idl: idl,id"); idlist[idcount++] = (yyvsp[(3) - (3)].str); ;}
    break;

  case 73:
#line 301 "minc.y"
    { MPRINT("id"); (yyval.str) = strsave(yytext); ;}
    break;

  case 74:
#line 305 "minc.y"
    { MPRINT("expl: exp"); (yyval.node) = new NodeListElem(new NodeEmptyListElem(), (yyvsp[(1) - (1)].node)); ;}
    break;

  case 75:
#line 306 "minc.y"
    { MPRINT("expl: expl,exp"); (yyval.node) = new NodeListElem((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 76:
#line 310 "minc.y"
    {
								char *s = yytext + 1;
								s[strlen(s) - 1] = '\0';
								(yyval.node) = new NodeString(strsave(s));
							;}
    break;

  case 77:
#line 318 "minc.y"
    { MPRINT("bexp"); (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 78:
#line 319 "minc.y"
    { MPRINT("!bexp"); (yyval.node) = new NodeNot((yyvsp[(2) - (2)].node)); ;}
    break;

  case 79:
#line 320 "minc.y"
    { (yyval.node) = new NodeAnd((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 80:
#line 321 "minc.y"
    { (yyval.node) = new NodeOr((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 81:
#line 322 "minc.y"
    { (yyval.node) = new NodeRelation(OpEqual, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 82:
#line 323 "minc.y"
    { (yyval.node) = new NodeRelation(OpNotEqual, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 83:
#line 324 "minc.y"
    { (yyval.node) = new NodeRelation(OpLess, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 84:
#line 325 "minc.y"
    { (yyval.node) = new NodeRelation(OpGreater, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 85:
#line 326 "minc.y"
    { (yyval.node) = new NodeRelation(OpLessEqual, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 86:
#line 327 "minc.y"
    { (yyval.node) = new NodeRelation(OpGreaterEqual, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 87:
#line 328 "minc.y"
    { (yyval.node) = new NodeRelation(OpEqual, new NodeConstf(1.0), new NodeConstf(1.0)); ;}
    break;

  case 88:
#line 329 "minc.y"
    { (yyval.node) = new NodeRelation(OpNotEqual, new NodeConstf(1.0), new NodeConstf(1.0)); ;}
    break;

  case 89:
#line 333 "minc.y"
    { MPRINT("exp: rstmt"); (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 90:
#line 334 "minc.y"
    { (yyval.node) = new NodeOp(OpPow, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 91:
#line 335 "minc.y"
    { (yyval.node) = new NodeOp(OpMul, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 92:
#line 336 "minc.y"
    { (yyval.node) = new NodeOp(OpDiv, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 93:
#line 337 "minc.y"
    { (yyval.node) = new NodeOp(OpPlus, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 94:
#line 338 "minc.y"
    { (yyval.node) = new NodeOp(OpMinus, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 95:
#line 339 "minc.y"
    { (yyval.node) = new NodeOp(OpMod, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 96:
#line 340 "minc.y"
    { MPRINT("exp: (bexp)"); (yyval.node) = (yyvsp[(2) - (3)].node); ;}
    break;

  case 97:
#line 341 "minc.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 98:
#line 342 "minc.y"
    {
							double f = atof(yytext);
							(yyval.node) = new NodeConstf(f);
						;}
    break;

  case 99:
#line 347 "minc.y"
    { MPRINT("exp: {expl}");	decrLevel(); (yyval.node) = new NodeList((yyvsp[(3) - (4)].node)); ;}
    break;

  case 100:
#line 348 "minc.y"
    { MPRINT("exp: {}");	(yyval.node) = new NodeList(new NodeEmptyListElem()); ;}
    break;

  case 101:
#line 350 "minc.y"
    {	MPRINT("exp: obj[exp]");    (yyval.node) = new NodeSubscriptRead((yyvsp[(1) - (4)].node), (yyvsp[(3) - (4)].node)); ;}
    break;

  case 102:
#line 351 "minc.y"
    {   MPRINT("exp: obj.id");    (yyval.node) = new NodeMember((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].str)); ;}
    break;

  case 103:
#line 352 "minc.y"
    {
                            (yyval.node) = parseArgumentQuery(yytext, &flerror);
						;}
    break;

  case 104:
#line 355 "minc.y"
    {
                            (yyval.node) = parseScoreArgument(yytext, &flerror);
						;}
    break;

  case 105:
#line 358 "minc.y"
    { (yyval.node) = new NodeConstf(1.0); ;}
    break;

  case 106:
#line 359 "minc.y"
    { (yyval.node) = new NodeConstf(0.0); ;}
    break;

  case 107:
#line 360 "minc.y"
    {
								/* NodeConstf is a dummy; makes exct_operator work */
								(yyval.node) = new NodeOp(OpNeg, (yyvsp[(2) - (2)].node), new NodeConstf(0.0));
							;}
    break;

  case 108:
#line 364 "minc.y"
    {
								MPRINT("exp: id");    (yyval.node) = new NodeLoadSym((yyvsp[(1) - (1)].str));
							;}
    break;

  case 109:
#line 373 "minc.y"
    { MPRINT("mbr");  (yyval.node) = new NodeMemberDecl((yyvsp[(2) - (2)].str), MincFloatType); ;}
    break;

  case 110:
#line 374 "minc.y"
    { MPRINT("mbr");    (yyval.node) = new NodeMemberDecl((yyvsp[(2) - (2)].str), MincStringType); ;}
    break;

  case 111:
#line 375 "minc.y"
    { MPRINT("mbr");    (yyval.node) = new NodeMemberDecl((yyvsp[(2) - (2)].str), MincHandleType); ;}
    break;

  case 112:
#line 376 "minc.y"
    { MPRINT("mbr");  (yyval.node) = new NodeMemberDecl((yyvsp[(2) - (2)].str), MincListType); ;}
    break;

  case 113:
#line 377 "minc.y"
    { MPRINT("mbr");   (yyval.node) = new NodeMemberDecl((yyvsp[(2) - (2)].str), MincMapType); ;}
    break;

  case 114:
#line 378 "minc.y"
    { MPRINT("mbr");   (yyval.node) = new NodeMemberDecl((yyvsp[(2) - (2)].str), MincFunctionType); ;}
    break;

  case 115:
#line 379 "minc.y"
    { MPRINT("mbr");   (yyval.node) = new NodeMemberDecl((yyvsp[(2) - (2)].str), MincStructType, (yyvsp[(1) - (2)].str)); ;}
    break;

  case 116:
#line 388 "minc.y"
    { MPRINT("mbrl: mbr");
            (yyval.node) = new NodeSeq(new NodeNoop(), (yyvsp[(1) - (1)].node)); ;}
    break;

  case 117:
#line 390 "minc.y"
    { MPRINT("mbrl: mbrl,mbr");
            (yyval.node) = new NodeSeq((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 118:
#line 396 "minc.y"
    { MPRINT("structname"); (yyval.str) = (yyvsp[(2) - (2)].str); ;}
    break;

  case 119:
#line 401 "minc.y"
    { MPRINT("structdef");
        --slevel; MPRINT1("slevel => %d", slevel);
        (yyval.node) = go(new NodeStructDef((yyvsp[(1) - (5)].str), (yyvsp[(3) - (5)].node)));
    ;}
    break;

  case 120:
#line 409 "minc.y"
    {    MPRINT("struct"); if (slevel > 0) { minc_die("nested struct decls not allowed"); }
                    slevel++; MPRINT1("slevel => %d", slevel);
                ;}
    break;

  case 121:
#line 421 "minc.y"
    { MPRINT("arg");
                                    (yyval.node) = new NodeDecl((yyvsp[(2) - (2)].str), MincFloatType); ;}
    break;

  case 122:
#line 423 "minc.y"
    { MPRINT("arg");
                                    (yyval.node) = new NodeDecl((yyvsp[(2) - (2)].str), MincStringType); ;}
    break;

  case 123:
#line 425 "minc.y"
    { MPRINT("arg");
                                    (yyval.node) = new NodeDecl((yyvsp[(2) - (2)].str), MincHandleType); ;}
    break;

  case 124:
#line 427 "minc.y"
    { MPRINT("arg");
                                    (yyval.node) = new NodeDecl((yyvsp[(2) - (2)].str), MincListType); ;}
    break;

  case 125:
#line 429 "minc.y"
    { MPRINT("arg");
                                    (yyval.node) = new NodeDecl((yyvsp[(2) - (2)].str), MincMapType); ;}
    break;

  case 126:
#line 431 "minc.y"
    { MPRINT("arg");
                                    (yyval.node) = new NodeStructDecl((yyvsp[(3) - (3)].str), (yyvsp[(2) - (3)].str)); ;}
    break;

  case 127:
#line 433 "minc.y"
    { MPRINT("arg");
                                    (yyval.node) = new NodeDecl((yyvsp[(2) - (2)].str), MincFunctionType); ;}
    break;

  case 128:
#line 441 "minc.y"
    { MPRINT("funcname");
									(yyval.node) = go(new NodeFuncDecl(strsave((yyvsp[(2) - (3)].str)), MincFloatType)); ;}
    break;

  case 129:
#line 443 "minc.y"
    { MPRINT("funcname");
									(yyval.node) = go(new NodeFuncDecl(strsave((yyvsp[(2) - (3)].str)), MincStringType)); ;}
    break;

  case 130:
#line 445 "minc.y"
    { MPRINT("funcname");
									(yyval.node) = go(new NodeFuncDecl(strsave((yyvsp[(2) - (3)].str)), MincHandleType)); ;}
    break;

  case 131:
#line 447 "minc.y"
    { MPRINT("funcname");
									(yyval.node) = go(new NodeFuncDecl(strsave((yyvsp[(2) - (3)].str)), MincListType)); ;}
    break;

  case 132:
#line 449 "minc.y"
    { MPRINT("funcname");
                                    (yyval.node) = go(new NodeFuncDecl(strsave((yyvsp[(2) - (3)].str)), MincMapType)); ;}
    break;

  case 133:
#line 451 "minc.y"
    { MPRINT("funcname");
                                    (yyval.node) = go(new NodeFuncDecl(strsave((yyvsp[(3) - (4)].str)), MincStructType)); ;}
    break;

  case 134:
#line 457 "minc.y"
    { MPRINT("argl: arg"); (yyval.node) = new NodeArgListElem(new NodeEmptyListElem(), (yyvsp[(1) - (1)].node)); ;}
    break;

  case 135:
#line 458 "minc.y"
    { MPRINT("argl: argl,arg"); (yyval.node) = new NodeArgListElem((yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node)); ;}
    break;

  case 136:
#line 468 "minc.y"
    { MPRINT("fargl: (argl)"); (yyval.node) = new NodeArgList((yyvsp[(2) - (3)].node)); ;}
    break;

  case 137:
#line 469 "minc.y"
    { MPRINT("fargl: (NULL)"); (yyval.node) = new NodeArgList(new NodeEmptyListElem()); ;}
    break;

  case 138:
#line 474 "minc.y"
    {	MPRINT("function"); if (flevel > 0) {
								minc_die("nested function decls not allowed");
							}
							flevel++; MPRINT1("flevel => %d", flevel);
						 ;}
    break;

  case 139:
#line 483 "minc.y"
    {	MPRINT("fstml: stml,ret");
									(yyval.node) = new NodeFuncSeq((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node));
							;}
    break;

  case 140:
#line 486 "minc.y"
    {	MPRINT("fstml: ret");
									(yyval.node) = new NodeFuncSeq(new NodeEmptyListElem(), (yyvsp[(1) - (1)].node));
							;}
    break;

  case 141:
#line 493 "minc.y"
    {
									MPRINT("fdef");
									decrLevel();
									--flevel; MPRINT1("flevel => %d", flevel);
									go(new NodeFuncDef((yyvsp[(1) - (5)].node), (yyvsp[(2) - (5)].node), (yyvsp[(4) - (5)].node)));
									/* because we're just a decl, and the tree is stored
									   in the Symbol, we do not return a Node to the parser.
									 */
									(yyval.node) = new NodeNoop();
								;}
    break;

  case 142:
#line 503 "minc.y"
    { minc_die("%s(): function body must end with 'return <exp>' statement", (yyvsp[(2) - (6)].node)); flerror = 1; (yyval.node) = new NodeNoop(); ;}
    break;

  case 143:
#line 504 "minc.y"
    { minc_die("%s(): function body must end with 'return <exp>' statement", (yyvsp[(2) - (5)].node)); flerror = 1; (yyval.node) = new NodeNoop(); ;}
    break;


/* Line 1267 of yacc.c.  */
#line 2677 "minc.cpp"
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


#line 507 "minc.y"


static void 	incrLevel()
{
	++level; MPRINT1("level => %d", level);
}

static void		decrLevel()
{
	--level; MPRINT1("level => %d", level);
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

static Node * declareStruct(const char *typeName)
{
    MPRINT2("declareStruct(typeName='%s', idcount=%d)", typeName, idcount);
    assert(idcount > 0);
    Node * t = new NodeNoop();    // end of the list
    for (int i = 0; i < idcount; i++) {
        Node * decl = new NodeStructDecl(idlist[i], typeName);
        t = new NodeSeq(t, decl);
    }
    return t;
}

static Node * parseArgumentQuery(const char *text, int *pOutErr)
{
#ifndef EMBEDDED
    /* ?argument will return 1.0 if defined, else 0.0 */
    const char *token = text + 1;    // strip off '?'
    // returns NULL silently if not found
    const char *value = lookup_token(token, false);
    return new NodeConstf(value != NULL ? 1.0 : 0.0);
#else
    minc_warn("Argument variables not supported");
    *pOutErr = 1;
    return new NodeNoop();
#endif
}

static Node * parseScoreArgument(const char *text, int *pOutErr)
{
#ifndef EMBEDDED
    const char *token = text + 1;    // strip off '$'
    const char *value = lookup_token(token, true);        // returns NULL with warning
    if (value != NULL) {
        // We store this as a number constant if it can be coaxed into a number,
        // else we store this as a string constant.
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
        catch(const RTException &rtex) {
            delete t1;
            t1 = NULL;
            cleanup();
            rterror("parser", "caught fatal exception: '%s' - bailing out", rtex.mesg());
            throw;
        }
		catch(...) {
			MPRINT1("caught exception - deleting node %p and cleaning up", t1);
			delete t1;
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
	rtcmix_debug("cleanup", "yy_init = %d", yy_init);
	MPRINT1("Freeing program tree %p", program);
    delete program;
	program = NULL;
	/* Reset all static state */
	comments = 0;	// from lex.yy.c
	cpcomments = 0;
	xblock = 0;
	idcount = 0;
	flerror = 0;
	flevel = 0;
    slevel = 0;
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

#warning DAS Make sure yylex_destroy() works
#define USE_YYLEX_DESTROY

// BGG mm -- for dynamic memory mgmt (double return for UG_INTRO() macro)
double minc_memflush()
{
	rtcmix_debug("minc_memflush", "Freeing parser memory");
	delete program;
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

