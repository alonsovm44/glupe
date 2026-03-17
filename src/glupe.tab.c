/* A Bison parser, made by GNU Bison 2.4.2.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2006, 2009-2010 Free Software
   Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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
#define YYBISON_VERSION "2.4.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 1 "src/glupe.y"

#include "ast.hpp"
#include <iostream>
#include <vector>
#include <string>

extern int yylex();
extern int yylineno;
void yyerror(ProgramNode** root, const char* s) {
    std::cerr << "Parse error at line " << yylineno << ": " << s << std::endl;
}


/* Line 189 of yacc.c  */
#line 86 "src/glupe.tab.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     T_RAW_CODE = 258,
     T_IDENTIFIER = 259,
     T_VAR_VALUE = 260,
     T_INTENT_TEXT = 261,
     T_STRING_LITERAL = 262,
     T_NUMBER = 263,
     T_PERSISTENT_VAR_START = 264,
     T_CONST_VAR_START = 265,
     T_EPHEMERAL_VAR_START = 266,
     T_BLOCK_START = 267,
     T_INLINE_START = 268,
     T_ABSTRACT = 269,
     T_ARROW = 270,
     T_COMMA = 271,
     T_LPAREN = 272,
     T_RPAREN = 273,
     T_LBRACE = 274,
     T_BLOCK_END = 275,
     T_INLINE_END = 276,
     T_NEWLINE = 277,
     T_PLUS = 278,
     T_MINUS = 279,
     T_STAR = 280,
     T_SLASH = 281,
     T_CARET = 282
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 16 "src/glupe.y"

    std::string* str;
    ASTNode* node;
    ProgramNode* prog;
    std::vector<std::string>* str_list;
    bool boolean;



/* Line 214 of yacc.c  */
#line 159 "src/glupe.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 171 "src/glupe.tab.c"

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
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
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
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
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
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   60

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  28
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  14
/* YYNRULES -- Number of rules.  */
#define YYNRULES  37
/* YYNRULES -- Number of states.  */
#define YYNSTATES  69

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   282

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     4,     7,     9,    11,    13,    15,    16,
      18,    23,    28,    33,    34,    36,    37,    41,    42,    45,
      47,    51,    53,    57,    61,    65,    69,    73,    75,    77,
      79,    81,    83,    84,    87,    96,   102,   111
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      29,     0,    -1,    -1,    29,    30,    -1,     3,    -1,    32,
      -1,    41,    -1,    22,    -1,    -1,     5,    -1,     9,     4,
      31,    22,    -1,    10,     4,    31,    22,    -1,    11,     4,
      31,    22,    -1,    -1,    14,    -1,    -1,    17,    36,    18,
      -1,    -1,    15,    36,    -1,    37,    -1,    36,    16,    37,
      -1,    38,    -1,    37,    23,    38,    -1,    37,    24,    38,
      -1,    37,    25,    38,    -1,    37,    26,    38,    -1,    37,
      27,    38,    -1,     4,    -1,     7,    -1,     8,    -1,     4,
      -1,     7,    -1,    -1,    40,     6,    -1,    12,    33,    39,
      34,    35,    19,    40,    20,    -1,    12,    33,    19,    40,
      20,    -1,    13,    33,    39,    34,    35,    19,    40,    21,
      -1,    13,    33,    19,    40,    21,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,    41,    41,    45,    55,    60,    61,    62,    66,    67,
      71,    76,    81,    89,    90,    94,    95,    99,   100,   104,
     110,   119,   120,   121,   122,   123,   124,   128,   129,   130,
     134,   135,   139,   140,   148,   156,   162,   170
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "T_RAW_CODE", "T_IDENTIFIER",
  "T_VAR_VALUE", "T_INTENT_TEXT", "T_STRING_LITERAL", "T_NUMBER",
  "T_PERSISTENT_VAR_START", "T_CONST_VAR_START", "T_EPHEMERAL_VAR_START",
  "T_BLOCK_START", "T_INLINE_START", "T_ABSTRACT", "T_ARROW", "T_COMMA",
  "T_LPAREN", "T_RPAREN", "T_LBRACE", "T_BLOCK_END", "T_INLINE_END",
  "T_NEWLINE", "T_PLUS", "T_MINUS", "T_STAR", "T_SLASH", "T_CARET",
  "$accept", "program", "element", "opt_var_value", "var_decl",
  "optional_abstract", "optional_params", "optional_parents", "expr_list",
  "algebra_expr", "algebra_term", "string_or_id", "intent_body",
  "container_decl", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    28,    29,    29,    30,    30,    30,    30,    31,    31,
      32,    32,    32,    33,    33,    34,    34,    35,    35,    36,
      36,    37,    37,    37,    37,    37,    37,    38,    38,    38,
      39,    39,    40,    40,    41,    41,    41,    41
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     1,     1,     1,     1,     0,     1,
       4,     4,     4,     0,     1,     0,     3,     0,     2,     1,
       3,     1,     3,     3,     3,     3,     3,     1,     1,     1,
       1,     1,     0,     2,     8,     5,     8,     5
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     1,     4,     0,     0,     0,    13,    13,     7,
       3,     5,     6,     8,     8,     8,    14,     0,     0,     9,
       0,     0,     0,    30,    31,    32,    15,    32,    15,    10,
      11,    12,     0,     0,    17,     0,    17,    33,    35,    27,
      28,    29,     0,    19,    21,     0,     0,    37,     0,     0,
      16,     0,     0,     0,     0,     0,    18,    32,    32,    20,
      22,    23,    24,    25,    26,     0,     0,    34,    36
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     1,    10,    20,    11,    17,    34,    46,    42,    43,
      44,    26,    32,    12
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -28
static const yytype_int8 yypact[] =
{
     -28,     1,   -28,   -28,    21,    23,    28,    -7,    -7,   -28,
     -28,   -28,   -28,    11,    11,    11,   -28,    -2,    -1,   -28,
      24,    25,    29,   -28,   -28,   -28,    16,   -28,    16,   -28,
     -28,   -28,     2,    41,    35,     3,    35,   -28,   -28,   -28,
     -28,   -28,    10,    12,   -28,    41,    33,   -28,    34,    41,
     -28,    41,    41,    41,    41,    41,    38,   -28,   -28,    12,
     -28,   -28,   -28,   -28,   -28,     9,    13,   -28,   -28
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -28,   -28,   -28,     6,   -28,    47,    30,    20,    14,     8,
     -11,    42,   -27,   -28
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
      35,     2,    23,    23,     3,    24,    24,    16,    37,    37,
       4,     5,     6,     7,     8,    37,    19,    25,    27,    37,
      21,    22,    38,     9,    47,    13,    49,    14,    50,    67,
      65,    66,    15,    33,    68,    51,    52,    53,    54,    55,
      60,    61,    62,    63,    64,    39,    29,    30,    40,    41,
      45,    31,    57,    58,    49,    18,    48,    59,    36,    56,
      28
};

static const yytype_uint8 yycheck[] =
{
      27,     0,     4,     4,     3,     7,     7,    14,     6,     6,
       9,    10,    11,    12,    13,     6,     5,    19,    19,     6,
      14,    15,    20,    22,    21,     4,    16,     4,    18,    20,
      57,    58,     4,    17,    21,    23,    24,    25,    26,    27,
      51,    52,    53,    54,    55,     4,    22,    22,     7,     8,
      15,    22,    19,    19,    16,     8,    36,    49,    28,    45,
      18
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    29,     0,     3,     9,    10,    11,    12,    13,    22,
      30,    32,    41,     4,     4,     4,    14,    33,    33,     5,
      31,    31,    31,     4,     7,    19,    39,    19,    39,    22,
      22,    22,    40,    17,    34,    40,    34,     6,    20,     4,
       7,     8,    36,    37,    38,    15,    35,    21,    35,    16,
      18,    23,    24,    25,    26,    27,    36,    19,    19,    37,
      38,    38,    38,    38,    38,    40,    40,    20,    21
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
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

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
      yyerror (root, YY_("syntax error: cannot back up")); \
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
		  Type, Value, root); \
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
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, ProgramNode** root)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, root)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    ProgramNode** root;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (root);
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, ProgramNode** root)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, root)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    ProgramNode** root;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, root);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
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
yy_reduce_print (YYSTYPE *yyvsp, int yyrule, ProgramNode** root)
#else
static void
yy_reduce_print (yyvsp, yyrule, root)
    YYSTYPE *yyvsp;
    int yyrule;
    ProgramNode** root;
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
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       , root);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule, root); \
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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, ProgramNode** root)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, root)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    ProgramNode** root;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (root);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {
      case 3: /* "T_RAW_CODE" */

/* Line 1009 of yacc.c  */
#line 24 "src/glupe.y"
	{ delete (yyvaluep->str); };

/* Line 1009 of yacc.c  */
#line 1135 "src/glupe.tab.c"
	break;
      case 4: /* "T_IDENTIFIER" */

/* Line 1009 of yacc.c  */
#line 24 "src/glupe.y"
	{ delete (yyvaluep->str); };

/* Line 1009 of yacc.c  */
#line 1144 "src/glupe.tab.c"
	break;
      case 5: /* "T_VAR_VALUE" */

/* Line 1009 of yacc.c  */
#line 24 "src/glupe.y"
	{ delete (yyvaluep->str); };

/* Line 1009 of yacc.c  */
#line 1153 "src/glupe.tab.c"
	break;
      case 6: /* "T_INTENT_TEXT" */

/* Line 1009 of yacc.c  */
#line 24 "src/glupe.y"
	{ delete (yyvaluep->str); };

/* Line 1009 of yacc.c  */
#line 1162 "src/glupe.tab.c"
	break;
      case 7: /* "T_STRING_LITERAL" */

/* Line 1009 of yacc.c  */
#line 24 "src/glupe.y"
	{ delete (yyvaluep->str); };

/* Line 1009 of yacc.c  */
#line 1171 "src/glupe.tab.c"
	break;
      case 8: /* "T_NUMBER" */

/* Line 1009 of yacc.c  */
#line 24 "src/glupe.y"
	{ delete (yyvaluep->str); };

/* Line 1009 of yacc.c  */
#line 1180 "src/glupe.tab.c"
	break;
      case 30: /* "element" */

/* Line 1009 of yacc.c  */
#line 24 "src/glupe.y"
	{ delete (yyvaluep->node); };

/* Line 1009 of yacc.c  */
#line 1189 "src/glupe.tab.c"
	break;
      case 31: /* "opt_var_value" */

/* Line 1009 of yacc.c  */
#line 24 "src/glupe.y"
	{ delete (yyvaluep->str); };

/* Line 1009 of yacc.c  */
#line 1198 "src/glupe.tab.c"
	break;
      case 32: /* "var_decl" */

/* Line 1009 of yacc.c  */
#line 24 "src/glupe.y"
	{ delete (yyvaluep->node); };

/* Line 1009 of yacc.c  */
#line 1207 "src/glupe.tab.c"
	break;
      case 34: /* "optional_params" */

/* Line 1009 of yacc.c  */
#line 24 "src/glupe.y"
	{ delete (yyvaluep->str_list); };

/* Line 1009 of yacc.c  */
#line 1216 "src/glupe.tab.c"
	break;
      case 35: /* "optional_parents" */

/* Line 1009 of yacc.c  */
#line 24 "src/glupe.y"
	{ delete (yyvaluep->str_list); };

/* Line 1009 of yacc.c  */
#line 1225 "src/glupe.tab.c"
	break;
      case 36: /* "expr_list" */

/* Line 1009 of yacc.c  */
#line 24 "src/glupe.y"
	{ delete (yyvaluep->str_list); };

/* Line 1009 of yacc.c  */
#line 1234 "src/glupe.tab.c"
	break;
      case 37: /* "algebra_expr" */

/* Line 1009 of yacc.c  */
#line 24 "src/glupe.y"
	{ delete (yyvaluep->str); };

/* Line 1009 of yacc.c  */
#line 1243 "src/glupe.tab.c"
	break;
      case 38: /* "algebra_term" */

/* Line 1009 of yacc.c  */
#line 24 "src/glupe.y"
	{ delete (yyvaluep->str); };

/* Line 1009 of yacc.c  */
#line 1252 "src/glupe.tab.c"
	break;
      case 39: /* "string_or_id" */

/* Line 1009 of yacc.c  */
#line 24 "src/glupe.y"
	{ delete (yyvaluep->str); };

/* Line 1009 of yacc.c  */
#line 1261 "src/glupe.tab.c"
	break;
      case 40: /* "intent_body" */

/* Line 1009 of yacc.c  */
#line 24 "src/glupe.y"
	{ delete (yyvaluep->str); };

/* Line 1009 of yacc.c  */
#line 1270 "src/glupe.tab.c"
	break;
      case 41: /* "container_decl" */

/* Line 1009 of yacc.c  */
#line 24 "src/glupe.y"
	{ delete (yyvaluep->node); };

/* Line 1009 of yacc.c  */
#line 1279 "src/glupe.tab.c"
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
int yyparse (ProgramNode** root);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

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
yyparse (ProgramNode** root)
#else
int
yyparse (root)
    ProgramNode** root;
#endif
#endif
{


    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

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
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
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

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
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

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
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
        case 2:

/* Line 1464 of yacc.c  */
#line 41 "src/glupe.y"
    {
        *root = new ProgramNode();
        (yyval.prog) = *root;
    ;}
    break;

  case 3:

/* Line 1464 of yacc.c  */
#line 46 "src/glupe.y"
    {
        if ((yyvsp[(2) - (2)].node)) {
            (yyvsp[(1) - (2)].prog)->addElement(std::unique_ptr<ASTNode>((yyvsp[(2) - (2)].node)));
        }
        (yyval.prog) = (yyvsp[(1) - (2)].prog);
    ;}
    break;

  case 4:

/* Line 1464 of yacc.c  */
#line 56 "src/glupe.y"
    {
        (yyval.node) = new RawCodeNode(*(yyvsp[(1) - (1)].str));
        delete (yyvsp[(1) - (1)].str);
    ;}
    break;

  case 5:

/* Line 1464 of yacc.c  */
#line 60 "src/glupe.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 6:

/* Line 1464 of yacc.c  */
#line 61 "src/glupe.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 7:

/* Line 1464 of yacc.c  */
#line 62 "src/glupe.y"
    { (yyval.node) = nullptr; ;}
    break;

  case 8:

/* Line 1464 of yacc.c  */
#line 66 "src/glupe.y"
    { (yyval.str) = new std::string(""); ;}
    break;

  case 9:

/* Line 1464 of yacc.c  */
#line 67 "src/glupe.y"
    { (yyval.str) = (yyvsp[(1) - (1)].str); ;}
    break;

  case 10:

/* Line 1464 of yacc.c  */
#line 72 "src/glupe.y"
    {
        (yyval.node) = new VariableNode(*(yyvsp[(2) - (4)].str), *(yyvsp[(3) - (4)].str), VarType::PERSISTENT);
        delete (yyvsp[(2) - (4)].str); delete (yyvsp[(3) - (4)].str);
    ;}
    break;

  case 11:

/* Line 1464 of yacc.c  */
#line 77 "src/glupe.y"
    {
        (yyval.node) = new VariableNode(*(yyvsp[(2) - (4)].str), *(yyvsp[(3) - (4)].str), VarType::CONSTANT);
        delete (yyvsp[(2) - (4)].str); delete (yyvsp[(3) - (4)].str);
    ;}
    break;

  case 12:

/* Line 1464 of yacc.c  */
#line 82 "src/glupe.y"
    {
        (yyval.node) = new VariableNode(*(yyvsp[(2) - (4)].str), *(yyvsp[(3) - (4)].str), VarType::EPHEMERAL);
        delete (yyvsp[(2) - (4)].str); delete (yyvsp[(3) - (4)].str);
    ;}
    break;

  case 13:

/* Line 1464 of yacc.c  */
#line 89 "src/glupe.y"
    { (yyval.boolean) = false; ;}
    break;

  case 14:

/* Line 1464 of yacc.c  */
#line 90 "src/glupe.y"
    { (yyval.boolean) = true; ;}
    break;

  case 15:

/* Line 1464 of yacc.c  */
#line 94 "src/glupe.y"
    { (yyval.str_list) = new std::vector<std::string>(); ;}
    break;

  case 16:

/* Line 1464 of yacc.c  */
#line 95 "src/glupe.y"
    { (yyval.str_list) = (yyvsp[(2) - (3)].str_list); ;}
    break;

  case 17:

/* Line 1464 of yacc.c  */
#line 99 "src/glupe.y"
    { (yyval.str_list) = new std::vector<std::string>(); ;}
    break;

  case 18:

/* Line 1464 of yacc.c  */
#line 100 "src/glupe.y"
    { (yyval.str_list) = (yyvsp[(2) - (2)].str_list); ;}
    break;

  case 19:

/* Line 1464 of yacc.c  */
#line 105 "src/glupe.y"
    {
        (yyval.str_list) = new std::vector<std::string>();
        (yyval.str_list)->push_back(*(yyvsp[(1) - (1)].str));
        delete (yyvsp[(1) - (1)].str);
    ;}
    break;

  case 20:

/* Line 1464 of yacc.c  */
#line 111 "src/glupe.y"
    {
        (yyvsp[(1) - (3)].str_list)->push_back(*(yyvsp[(3) - (3)].str));
        delete (yyvsp[(3) - (3)].str);
        (yyval.str_list) = (yyvsp[(1) - (3)].str_list);
    ;}
    break;

  case 21:

/* Line 1464 of yacc.c  */
#line 119 "src/glupe.y"
    { (yyval.str) = (yyvsp[(1) - (1)].str); ;}
    break;

  case 22:

/* Line 1464 of yacc.c  */
#line 120 "src/glupe.y"
    { (yyval.str) = new std::string(*(yyvsp[(1) - (3)].str) + " + " + *(yyvsp[(3) - (3)].str)); delete (yyvsp[(1) - (3)].str); delete (yyvsp[(3) - (3)].str); ;}
    break;

  case 23:

/* Line 1464 of yacc.c  */
#line 121 "src/glupe.y"
    { (yyval.str) = new std::string(*(yyvsp[(1) - (3)].str) + " - " + *(yyvsp[(3) - (3)].str)); delete (yyvsp[(1) - (3)].str); delete (yyvsp[(3) - (3)].str); ;}
    break;

  case 24:

/* Line 1464 of yacc.c  */
#line 122 "src/glupe.y"
    { (yyval.str) = new std::string(*(yyvsp[(1) - (3)].str) + " * " + *(yyvsp[(3) - (3)].str)); delete (yyvsp[(1) - (3)].str); delete (yyvsp[(3) - (3)].str); ;}
    break;

  case 25:

/* Line 1464 of yacc.c  */
#line 123 "src/glupe.y"
    { (yyval.str) = new std::string(*(yyvsp[(1) - (3)].str) + " / " + *(yyvsp[(3) - (3)].str)); delete (yyvsp[(1) - (3)].str); delete (yyvsp[(3) - (3)].str); ;}
    break;

  case 26:

/* Line 1464 of yacc.c  */
#line 124 "src/glupe.y"
    { (yyval.str) = new std::string(*(yyvsp[(1) - (3)].str) + " ^ " + *(yyvsp[(3) - (3)].str)); delete (yyvsp[(1) - (3)].str); delete (yyvsp[(3) - (3)].str); ;}
    break;

  case 27:

/* Line 1464 of yacc.c  */
#line 128 "src/glupe.y"
    { (yyval.str) = new std::string("$" + *(yyvsp[(1) - (1)].str)); delete (yyvsp[(1) - (1)].str); ;}
    break;

  case 28:

/* Line 1464 of yacc.c  */
#line 129 "src/glupe.y"
    { (yyval.str) = new std::string("\"" + *(yyvsp[(1) - (1)].str) + "\""); delete (yyvsp[(1) - (1)].str); ;}
    break;

  case 29:

/* Line 1464 of yacc.c  */
#line 130 "src/glupe.y"
    { (yyval.str) = (yyvsp[(1) - (1)].str); ;}
    break;

  case 30:

/* Line 1464 of yacc.c  */
#line 134 "src/glupe.y"
    { (yyval.str) = (yyvsp[(1) - (1)].str); ;}
    break;

  case 31:

/* Line 1464 of yacc.c  */
#line 135 "src/glupe.y"
    { (yyval.str) = (yyvsp[(1) - (1)].str); ;}
    break;

  case 32:

/* Line 1464 of yacc.c  */
#line 139 "src/glupe.y"
    { (yyval.str) = new std::string(""); ;}
    break;

  case 33:

/* Line 1464 of yacc.c  */
#line 141 "src/glupe.y"
    {
        (yyval.str) = new std::string(*(yyvsp[(1) - (2)].str) + *(yyvsp[(2) - (2)].str));
        delete (yyvsp[(1) - (2)].str); delete (yyvsp[(2) - (2)].str);
    ;}
    break;

  case 34:

/* Line 1464 of yacc.c  */
#line 149 "src/glupe.y"
    {
        auto* cont = new ContainerNode(*(yyvsp[(3) - (8)].str), *(yyvsp[(7) - (8)].str), true, (yyvsp[(2) - (8)].boolean));
        cont->params = *(yyvsp[(4) - (8)].str_list);
        cont->parents = *(yyvsp[(5) - (8)].str_list);
        (yyval.node) = cont;
        delete (yyvsp[(3) - (8)].str); delete (yyvsp[(4) - (8)].str_list); delete (yyvsp[(5) - (8)].str_list); delete (yyvsp[(7) - (8)].str);
    ;}
    break;

  case 35:

/* Line 1464 of yacc.c  */
#line 157 "src/glupe.y"
    {
        auto* cont = new ContainerNode("", *(yyvsp[(4) - (5)].str), true, (yyvsp[(2) - (5)].boolean));
        (yyval.node) = cont;
        delete (yyvsp[(4) - (5)].str);
    ;}
    break;

  case 36:

/* Line 1464 of yacc.c  */
#line 163 "src/glupe.y"
    {
        auto* cont = new ContainerNode(*(yyvsp[(3) - (8)].str), *(yyvsp[(7) - (8)].str), false, (yyvsp[(2) - (8)].boolean));
        cont->params = *(yyvsp[(4) - (8)].str_list);
        cont->parents = *(yyvsp[(5) - (8)].str_list);
        (yyval.node) = cont;
        delete (yyvsp[(3) - (8)].str); delete (yyvsp[(4) - (8)].str_list); delete (yyvsp[(5) - (8)].str_list); delete (yyvsp[(7) - (8)].str);
    ;}
    break;

  case 37:

/* Line 1464 of yacc.c  */
#line 171 "src/glupe.y"
    {
        auto* cont = new ContainerNode("", *(yyvsp[(4) - (5)].str), false, (yyvsp[(2) - (5)].boolean));
        (yyval.node) = cont;
        delete (yyvsp[(4) - (5)].str);
    ;}
    break;



/* Line 1464 of yacc.c  */
#line 1887 "src/glupe.tab.c"
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
      yyerror (root, YY_("syntax error"));
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
	    yyerror (root, yymsg);
	  }
	else
	  {
	    yyerror (root, YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
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
		      yytoken, &yylval, root);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
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
		  yystos[yystate], yyvsp, root);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

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

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (root, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval, root);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, root);
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



/* Line 1684 of yacc.c  */
#line 178 "src/glupe.y"

