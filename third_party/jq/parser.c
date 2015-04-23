/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

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
#define YYBISON_VERSION "3.0.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 1 "parser.y" /* yacc.c:339  */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "compile.h"
#include "jv_alloc.h"
#define YYMALLOC jv_mem_alloc
#define YYFREE jv_mem_free

#line 76 "parser.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* In a future release of Bison, this section will be replaced
   by #include "y.tab.h".  */
#ifndef YY_YY_PARSER_H_INCLUDED
# define YY_YY_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 10 "parser.y" /* yacc.c:355  */

#include "locfile.h"
struct lexer_param;

#define YYLTYPE location
#define YYLLOC_DEFAULT(Loc, Rhs, N)             \
  do {                                          \
    if (N) {                                    \
      (Loc).start = YYRHSLOC(Rhs, 1).start;     \
      (Loc).end = YYRHSLOC(Rhs, N).end;         \
    } else {                                    \
      (Loc).start = YYRHSLOC(Rhs, 0).end;       \
      (Loc).end = YYRHSLOC(Rhs, 0).end;         \
    }                                           \
  } while (0)
 

#line 124 "parser.c" /* yacc.c:355  */

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    INVALID_CHARACTER = 258,
    IDENT = 259,
    FIELD = 260,
    LITERAL = 261,
    FORMAT = 262,
    Q = 263,
    REC = 264,
    SETMOD = 265,
    EQ = 266,
    NEQ = 267,
    DEFINEDOR = 268,
    AS = 269,
    DEF = 270,
    IF = 271,
    THEN = 272,
    ELSE = 273,
    ELSE_IF = 274,
    REDUCE = 275,
    END = 276,
    AND = 277,
    OR = 278,
    SETPIPE = 279,
    SETPLUS = 280,
    SETMINUS = 281,
    SETMULT = 282,
    SETDIV = 283,
    SETDEFINEDOR = 284,
    LESSEQ = 285,
    GREATEREQ = 286,
    QQSTRING_START = 287,
    QQSTRING_TEXT = 288,
    QQSTRING_INTERP_START = 289,
    QQSTRING_INTERP_END = 290,
    QQSTRING_END = 291
  };
#endif
/* Tokens.  */
#define INVALID_CHARACTER 258
#define IDENT 259
#define FIELD 260
#define LITERAL 261
#define FORMAT 262
#define Q 263
#define REC 264
#define SETMOD 265
#define EQ 266
#define NEQ 267
#define DEFINEDOR 268
#define AS 269
#define DEF 270
#define IF 271
#define THEN 272
#define ELSE 273
#define ELSE_IF 274
#define REDUCE 275
#define END 276
#define AND 277
#define OR 278
#define SETPIPE 279
#define SETPLUS 280
#define SETMINUS 281
#define SETMULT 282
#define SETDIV 283
#define SETDEFINEDOR 284
#define LESSEQ 285
#define GREATEREQ 286
#define QQSTRING_START 287
#define QQSTRING_TEXT 288
#define QQSTRING_INTERP_START 289
#define QQSTRING_INTERP_END 290
#define QQSTRING_END 291

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 30 "parser.y" /* yacc.c:355  */

  jv literal;
  block blk;

#line 213 "parser.c" /* yacc.c:355  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



int yyparse (block* answer, int* errors, struct locfile* locations, struct lexer_param* lexer_param_ptr);

#endif /* !YY_YY_PARSER_H_INCLUDED  */

/* Copy the second part of user declarations.  */
#line 98 "parser.y" /* yacc.c:358  */

#include "lexer.h"
struct lexer_param {
  yyscan_t lexer;
};
#define FAIL(loc, msg)                                             \
  do {                                                             \
    location l = loc;                                              \
    yyerror(&l, answer, errors, locations, lexer_param_ptr, msg);  \
    /*YYERROR*/;                                                   \
  } while (0)

void yyerror(YYLTYPE* loc, block* answer, int* errors, 
             struct locfile* locations, struct lexer_param* lexer_param_ptr, const char *s){
  (*errors)++;
  locfile_locate(locations, *loc, "error: %s", s);
}

int yylex(YYSTYPE* yylval, YYLTYPE* yylloc, block* answer, int* errors, 
          struct locfile* locations, struct lexer_param* lexer_param_ptr) {
  yyscan_t lexer = lexer_param_ptr->lexer;
  int tok = jq_yylex(yylval, yylloc, lexer);
  if ((tok == LITERAL || tok == QQSTRING_TEXT) && !jv_is_valid(yylval->literal)) {
    jv msg = jv_invalid_get_msg(jv_copy(yylval->literal));
    if (jv_get_kind(msg) == JV_KIND_STRING) {
      FAIL(*yylloc, jv_string_value(msg));
    } else {
      FAIL(*yylloc, "Invalid literal");
    }
    jv_free(msg);
    jv_free(yylval->literal);
    yylval->literal = jv_null();
  }
  return tok;
}

static block gen_dictpair(block k, block v) {
  return BLOCK(gen_subexp(k), gen_subexp(v), gen_op_simple(INSERT));
}

static block gen_index(block obj, block key) {
  return BLOCK(gen_subexp(key), obj, gen_op_simple(INDEX));
}

static block gen_index_opt(block obj, block key) {
  return BLOCK(gen_subexp(key), obj, gen_op_simple(INDEX_OPT));
}

static block gen_slice_index(block obj, block start, block end, opcode idx_op) {
  block key = BLOCK(gen_subexp(gen_const(jv_object())),
                    gen_subexp(gen_const(jv_string("start"))),
                    gen_subexp(start),
                    gen_op_simple(INSERT),
                    gen_subexp(gen_const(jv_string("end"))),
                    gen_subexp(end),
                    gen_op_simple(INSERT));
  return BLOCK(key, obj, gen_op_simple(idx_op));
}

static block gen_binop(block a, block b, int op) {
  const char* funcname = 0;
  switch (op) {
  case '+': funcname = "_plus"; break;
  case '-': funcname = "_minus"; break;
  case '*': funcname = "_multiply"; break;
  case '/': funcname = "_divide"; break;
  case '%': funcname = "_mod"; break;
  case EQ: funcname = "_equal"; break;
  case NEQ: funcname = "_notequal"; break;
  case '<': funcname = "_less"; break;
  case '>': funcname = "_greater"; break;
  case LESSEQ: funcname = "_lesseq"; break;
  case GREATEREQ: funcname = "_greatereq"; break;
  }
  assert(funcname);

  return gen_call(funcname, BLOCK(gen_lambda(a), gen_lambda(b)));
}

static block gen_format(block a, jv fmt) {
  return BLOCK(a, gen_call("format", BLOCK(gen_lambda(gen_const(fmt)))));
}

static block gen_definedor_assign(block object, block val) {
  block tmp = gen_op_var_fresh(STOREV, "tmp");
  return BLOCK(gen_op_simple(DUP),
               val, tmp,
               gen_call("_modify", BLOCK(gen_lambda(object),
                                         gen_lambda(gen_definedor(gen_noop(), 
                                                                  gen_op_bound(LOADV, tmp))))));
}
 
static block gen_update(block object, block val, int optype) {
  block tmp = gen_op_var_fresh(STOREV, "tmp");
  return BLOCK(gen_op_simple(DUP),
               val,
               tmp,
               gen_call("_modify", BLOCK(gen_lambda(object), 
                                         gen_lambda(gen_binop(gen_noop(),
                                                              gen_op_bound(LOADV, tmp),
                                                              optype)))));
}


#line 345 "parser.c" /* yacc.c:358  */

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
#else
typedef signed char yytype_int8;
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
# elif ! defined YYSIZE_T
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
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  47
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1702

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  58
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  14
/* YYNRULES -- Number of rules.  */
#define YYNRULES  106
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  241

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   291

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,    48,    47,     2,     2,
      49,    50,    45,    43,    39,    44,    52,    46,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    51,    37,
      41,    40,    42,    53,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    54,     2,    55,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    56,    38,    57,     2,     2,     2,     2,
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
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   205,   205,   208,   213,   216,   221,   225,   230,   235,
     238,   243,   247,   251,   255,   259,   263,   267,   271,   275,
     279,   283,   287,   291,   295,   299,   303,   307,   311,   315,
     319,   323,   327,   331,   335,   339,   343,   348,   353,   361,
     371,   383,   397,   413,   433,   433,   437,   437,   444,   447,
     450,   456,   459,   464,   467,   470,   476,   479,   482,   485,
     488,   491,   494,   497,   500,   503,   506,   510,   516,   519,
     522,   525,   528,   531,   534,   537,   540,   543,   546,   549,
     552,   555,   558,   561,   564,   567,   571,   575,   580,   585,
     590,   595,   600,   605,   606,   607,   608,   611,   614,   615,
     616,   619,   622,   625,   629,   633,   636
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "INVALID_CHARACTER", "IDENT", "FIELD",
  "LITERAL", "FORMAT", "\"?\"", "\"..\"", "\"%=\"", "\"==\"", "\"!=\"",
  "\"//\"", "\"as\"", "\"def\"", "\"if\"", "\"then\"", "\"else\"",
  "\"elif\"", "\"reduce\"", "\"end\"", "\"and\"", "\"or\"", "\"|=\"",
  "\"+=\"", "\"-=\"", "\"*=\"", "\"/=\"", "\"//=\"", "\"<=\"", "\">=\"",
  "QQSTRING_START", "QQSTRING_TEXT", "QQSTRING_INTERP_START",
  "QQSTRING_INTERP_END", "QQSTRING_END", "';'", "'|'", "','", "'='", "'<'",
  "'>'", "'+'", "'-'", "'*'", "'/'", "'%'", "'$'", "'('", "')'", "':'",
  "'.'", "'?'", "'['", "']'", "'{'", "'}'", "$accept", "TopLevel",
  "FuncDefs", "Exp", "FuncDef", "String", "@1", "@2", "QQString",
  "ElseBody", "ExpD", "Term", "MkDict", "MkDictPair", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,    59,   124,    44,
      61,    60,    62,    43,    45,    42,    47,    37,    36,    40,
      41,    58,    46,    63,    91,    93,   123,   125
};
# endif

#define YYPACT_NINF -74

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-74)))

#define YYTABLE_NINF -98

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-98)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     449,   -41,   -40,   -74,    -2,   -74,    19,   449,   396,   -74,
     449,    24,   141,   231,   304,    79,    46,   -74,  1537,   449,
     -74,    22,   449,   -74,   -74,    -9,  1081,   449,    70,   -74,
     -12,   -74,    28,   879,   -74,    78,    -2,    36,    37,   -74,
     560,   -36,    44,   360,    45,    40,    62,   -74,   449,   449,
     449,   449,   449,   449,   449,   449,   449,   449,   449,   449,
     449,   449,   449,   449,   449,   449,   449,   449,   449,   449,
     449,   449,   -74,  1537,    50,    56,    -1,   286,   674,   -74,
     101,   449,   378,    58,     5,   -74,   -74,   -74,   -74,   -74,
     -74,    84,   -74,   467,    57,   920,   467,   -74,    84,  1601,
    1648,  1648,  1575,  1655,  1627,  1601,  1601,  1601,  1601,  1601,
    1601,  1648,  1648,  1537,  1575,  1601,  1648,  1648,   -12,   -12,
     -74,   -74,   -74,   -74,   104,    59,    54,   449,    60,   514,
     449,   -74,    11,   -35,  1119,   -74,  1043,   106,   -74,   449,
     -74,    75,   -74,   467,    77,    97,    66,    67,    77,   -74,
      81,   -74,   -74,   598,   -74,   431,    68,   715,   -74,   116,
      72,   -74,   449,   449,   -74,    76,  1157,   -74,   467,   467,
     467,   449,    73,    82,   636,   -74,   449,   -74,   -33,   449,
    1195,  1233,   449,   -74,    77,    77,    77,  1537,   -74,   -74,
      85,   756,   125,    80,  1271,   -74,   449,  1309,   -74,   449,
     -74,   -21,   449,   -74,  1043,   449,   797,   128,    83,  1347,
     -74,   961,   449,   -74,   -18,   449,   -74,   -74,   838,   133,
      92,  1385,   449,   -74,   -13,   449,   -74,  1002,   140,   102,
    1423,   -74,   108,   449,   -74,   103,  1461,   449,   -74,  1499,
     -74
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       4,    86,    63,    78,    80,    57,     0,     0,     0,    44,
       0,     0,     0,     0,     0,     0,     0,     3,     2,     4,
      79,    36,     0,    59,    46,     0,     0,     0,     0,    48,
      21,    85,     0,     0,    66,     0,     0,    65,     0,    83,
       0,     0,   104,     0,   103,     0,    98,     1,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     5,     6,    62,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,    93,    81,    67,    61,    94,
      82,     0,    96,     0,     0,     0,     0,    84,     0,    29,
      30,    31,    14,    13,    12,    16,    20,    23,    25,    28,
      15,    34,    35,    17,    18,    11,    32,    33,    19,    22,
      24,    26,    27,    58,     0,    64,     0,     0,    71,     0,
       0,    87,     0,     0,     0,    10,     0,     0,    49,     0,
      45,     0,   100,     0,   101,    55,     0,     0,   102,    99,
       0,    60,    95,     0,    70,     0,    69,     0,    47,     0,
       0,    37,     0,     0,     9,     0,     0,    54,     0,     0,
       0,     0,    77,    76,     0,    68,     0,    88,     0,     0,
       0,     0,     0,    50,    53,   106,   105,     7,    74,    73,
      75,     0,     0,     0,     0,    52,     0,     0,    72,     0,
      89,     0,     0,    38,     0,     0,     0,     0,     0,     0,
      51,     0,     0,    90,     0,     0,    39,     8,     0,     0,
       0,     0,     0,    91,     0,     0,    40,     0,     0,     0,
       0,    92,     0,     0,    41,     0,     0,     0,    42,     0,
      43
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -74,   -74,   145,     0,     1,    -4,   -74,   -74,    89,   -52,
     -70,    -3,   -73,   -74
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    16,    17,    73,    27,    20,    29,    79,    84,   164,
     144,    21,    45,    46
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      18,    19,   159,    91,   192,    28,    36,    26,    22,    37,
      30,    44,    33,    23,    40,   160,   207,   193,   142,   219,
      19,    92,    78,    25,   228,   149,   148,    74,    31,   208,
      24,     9,   220,    69,    70,    71,    75,   229,   138,   139,
      80,   140,    81,    95,   138,   139,    47,   158,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   125,   167,    76,    74,    77,   129,    85,    87,
      41,   134,   136,    42,    83,   141,    36,    44,    42,    88,
     145,    36,    89,   145,    44,    93,    96,    97,   184,   185,
     186,    98,    74,   123,   124,   133,   137,   146,   150,   152,
     165,     9,   151,   154,    91,   168,     9,   169,   170,   171,
     178,   175,    76,   179,    77,   182,   188,   153,    43,   201,
     157,   202,   214,    43,   215,   189,   -97,   224,   198,   166,
     145,   -97,    32,   225,   232,     1,     2,     3,     4,    76,
       5,    77,   210,   233,   237,   174,     6,     7,   235,     0,
       0,     8,   180,   181,    72,   145,   145,   145,   132,     0,
       0,   187,     0,     9,     0,     0,   191,     0,     0,   194,
       0,     0,   197,     0,     0,    10,     0,     0,     0,    11,
      12,     0,     0,    13,     0,    14,   204,    15,     0,   206,
       0,     0,   209,     0,     0,   211,     0,     0,     0,     0,
       0,     0,   218,     0,     0,   221,     0,     0,     0,     0,
       0,     0,   227,     0,     0,   230,     0,     0,     0,     0,
       0,   -56,    34,   236,     0,    35,   -56,   239,    36,     0,
       0,   -56,   -56,   -56,   -56,   -56,     0,     0,   -56,   -56,
     -56,     0,   -56,   -56,   -56,   -56,   -56,   -56,   -56,   -56,
     -56,   -56,   -56,     9,     0,     0,   -56,     0,   -56,   -56,
     -56,   -56,   -56,   -56,   -56,   -56,   -56,   -56,   -56,     0,
       0,   -56,   -56,   -56,     0,   -56,   -56,   126,   -56,     0,
       1,     2,     3,     4,     0,     5,     0,     0,     0,     0,
       0,     6,     7,     0,     0,    38,     8,     0,     1,     2,
       3,     4,     0,     5,     0,     0,     0,     0,     9,     6,
       7,     0,     0,     0,     8,     0,     0,     0,     0,     0,
      10,     0,     0,     0,    11,    12,     9,   127,    13,     0,
      14,   128,    15,     0,     0,     0,     0,     0,    10,     0,
       0,     0,    11,    12,     0,     0,    13,     0,    14,    39,
      15,    94,     0,     0,     1,     2,     3,     4,     0,     5,
       0,     0,     0,     0,     0,     6,     7,     0,     0,   135,
       8,     0,     1,     2,     3,     4,     0,     5,     0,     0,
       0,     0,     9,     6,     7,     0,     0,     0,     8,     0,
       1,     2,     3,     4,    10,     5,     0,     0,    11,    12,
       9,     0,    13,     0,    14,     0,    15,     0,     0,     0,
       0,     0,    10,     0,     0,     0,    11,    12,     9,     0,
      13,     0,    14,     0,    15,     1,     2,     3,     4,     0,
       5,     0,     0,     0,    11,    12,     6,     7,    13,     0,
      14,     8,    15,     1,     2,     3,     4,     0,     5,     0,
       0,     0,     0,     9,     6,     7,     0,     0,     0,     8,
       0,     1,     2,     3,     4,    10,     5,     0,     0,    11,
      12,     9,     0,    13,     0,    14,   173,    15,     0,     0,
       0,     0,     0,    10,     0,     0,     0,    11,    12,     9,
       0,    13,     0,    14,     0,    15,     0,     0,     0,     0,
       0,   143,     0,     0,     0,    11,    12,     0,     0,    13,
       0,    14,     0,    15,    48,    49,    50,    51,     0,     0,
       0,     0,     0,     0,     0,     0,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,     0,     0,     0,     0,
       0,     0,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,     0,     0,     0,   155,     0,     0,     0,   156,
      48,    49,    50,    51,     0,     0,     0,     0,     0,     0,
       0,     0,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,     0,     0,     0,     0,     0,     0,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    48,    49,
      50,    51,     0,     0,     0,    90,     0,     0,     0,     0,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
       0,     0,     0,     0,     0,     0,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    48,    49,    50,    51,
       0,     0,     0,   172,     0,     0,     0,     0,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,     0,     0,
       0,     0,     0,     0,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    48,    49,    50,    51,     0,     0,
       0,   190,     0,     0,     0,     0,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,     0,     0,     0,     0,
       0,   130,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,     0,     0,   131,    48,    49,    50,    51,     0,
       0,     0,     0,     0,     0,     0,     0,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,     0,     0,     0,
       0,     0,   176,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,     0,     0,   177,    48,    49,    50,    51,
       0,     0,     0,     0,     0,     0,     0,     0,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,     0,     0,
       0,     0,     0,   199,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,     0,     0,   200,    48,    49,    50,
      51,     0,     0,     0,     0,     0,     0,     0,     0,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,     0,
       0,     0,     0,     0,   212,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,     0,     0,   213,    48,    49,
      50,    51,     0,     0,     0,     0,     0,     0,     0,     0,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
       0,     0,     0,     0,     0,   222,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,     0,     0,   223,    48,
      49,    50,    51,     0,     0,     0,     0,     0,     0,     0,
       0,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,     0,     0,     0,     0,     0,     0,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,     0,     0,    86,
      48,    49,    50,    51,     0,     0,     0,     0,     0,     0,
       0,     0,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,     0,     0,     0,     0,     0,     0,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,     0,     0,
     147,    48,    49,    50,    51,     0,     0,     0,     0,     0,
       0,     0,     0,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,     0,     0,     0,     0,     0,     0,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,     0,
       0,   217,    48,    49,    50,    51,     0,     0,     0,     0,
       0,     0,     0,     0,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,     0,     0,     0,     0,     0,     0,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
       0,     0,   231,    48,    49,    50,    51,     0,     0,     0,
       0,   162,   163,     0,     0,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,     0,     0,     0,     0,     0,
       0,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    48,    49,    50,    51,     0,     0,     0,    82,     0,
       0,     0,     0,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,     0,     0,     0,     0,     0,     0,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    48,
      49,    50,    51,     0,     0,     0,     0,     0,     0,     0,
       0,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,     0,     0,     0,     0,     0,   161,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    48,    49,    50,
      51,     0,     0,     0,     0,     0,     0,     0,     0,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,     0,
       0,     0,   183,     0,     0,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    48,    49,    50,    51,     0,
       0,     0,     0,     0,     0,     0,   195,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,     0,     0,     0,
       0,     0,     0,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    48,    49,    50,    51,     0,     0,     0,
     196,     0,     0,     0,     0,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,     0,     0,     0,     0,     0,
       0,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    48,    49,    50,    51,     0,     0,     0,     0,     0,
       0,     0,     0,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,     0,     0,     0,     0,     0,   203,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    48,
      49,    50,    51,     0,     0,     0,     0,     0,     0,     0,
       0,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,     0,     0,     0,     0,     0,   205,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    48,    49,    50,
      51,     0,     0,     0,     0,     0,     0,     0,     0,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,     0,
       0,     0,     0,     0,   216,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    48,    49,    50,    51,     0,
       0,     0,     0,     0,     0,     0,     0,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,     0,     0,     0,
       0,     0,   226,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    48,    49,    50,    51,     0,     0,     0,
       0,     0,     0,     0,     0,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,     0,     0,     0,     0,     0,
     234,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    48,    49,    50,    51,     0,     0,     0,     0,     0,
       0,     0,     0,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,     0,     0,     0,     0,     0,   238,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    48,
      49,    50,    51,     0,     0,     0,     0,     0,     0,     0,
       0,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,     0,     0,     0,     0,     0,   240,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    48,    49,    50,
      51,     0,     0,     0,     0,     0,     0,     0,     0,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,     0,
       0,     0,     0,     0,     0,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    48,    49,    50,    51,     0,
       0,     0,     0,     0,     0,     0,     0,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,     0,     0,     0,
       0,   -98,    49,    50,     0,    64,    65,    66,    67,    68,
      69,    70,    71,    52,    53,   -98,   -98,   -98,   -98,   -98,
     -98,    60,    61,     0,     0,     0,     0,     0,    49,    50,
       0,   -98,    65,    66,    67,    68,    69,    70,    71,    52,
       0,     0,     0,     0,     0,     0,     0,    60,    61,   -98,
     -98,     0,     0,     0,     0,     0,    49,    50,    65,    66,
      67,    68,    69,    70,    71,     0,     0,     0,   -98,   -98,
       0,     0,     0,     0,     0,    60,    61,     0,     0,   -98,
     -98,    67,    68,    69,    70,    71,    65,    66,    67,    68,
      69,    70,    71
};

static const yytype_int16 yycheck[] =
{
       0,     0,    37,    39,    37,     8,     7,     7,    49,    13,
      10,    15,    12,    53,    14,    50,    37,    50,    91,    37,
      19,    57,    22,     4,    37,    98,    96,     5,     4,    50,
      32,    32,    50,    45,    46,    47,    14,    50,    33,    34,
      49,    36,    51,    43,    33,    34,     0,    36,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    76,   143,    52,     5,    54,    77,    50,     1,
       1,    81,    82,     4,    14,     1,     7,    91,     4,    53,
      93,     7,    55,    96,    98,    51,    51,    57,   168,   169,
     170,    39,     5,    53,    48,     4,    48,    50,     4,    55,
       4,    32,    53,    53,    39,    38,    32,    51,    51,    38,
       4,    53,    52,    51,    54,    49,    53,   127,    49,     4,
     130,    51,     4,    49,    51,    53,    57,     4,    53,   139,
     143,    57,     1,    51,     4,     4,     5,     6,     7,    52,
       9,    54,   204,    51,    51,   155,    15,    16,    50,    -1,
      -1,    20,   162,   163,    19,   168,   169,   170,    79,    -1,
      -1,   171,    -1,    32,    -1,    -1,   176,    -1,    -1,   179,
      -1,    -1,   182,    -1,    -1,    44,    -1,    -1,    -1,    48,
      49,    -1,    -1,    52,    -1,    54,   196,    56,    -1,   199,
      -1,    -1,   202,    -1,    -1,   205,    -1,    -1,    -1,    -1,
      -1,    -1,   212,    -1,    -1,   215,    -1,    -1,    -1,    -1,
      -1,    -1,   222,    -1,    -1,   225,    -1,    -1,    -1,    -1,
      -1,     0,     1,   233,    -1,     4,     5,   237,     7,    -1,
      -1,    10,    11,    12,    13,    14,    -1,    -1,    17,    18,
      19,    -1,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    -1,    -1,    35,    -1,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      -1,    50,    51,    52,    -1,    54,    55,     1,    57,    -1,
       4,     5,     6,     7,    -1,     9,    -1,    -1,    -1,    -1,
      -1,    15,    16,    -1,    -1,     1,    20,    -1,     4,     5,
       6,     7,    -1,     9,    -1,    -1,    -1,    -1,    32,    15,
      16,    -1,    -1,    -1,    20,    -1,    -1,    -1,    -1,    -1,
      44,    -1,    -1,    -1,    48,    49,    32,    51,    52,    -1,
      54,    55,    56,    -1,    -1,    -1,    -1,    -1,    44,    -1,
      -1,    -1,    48,    49,    -1,    -1,    52,    -1,    54,    55,
      56,     1,    -1,    -1,     4,     5,     6,     7,    -1,     9,
      -1,    -1,    -1,    -1,    -1,    15,    16,    -1,    -1,     1,
      20,    -1,     4,     5,     6,     7,    -1,     9,    -1,    -1,
      -1,    -1,    32,    15,    16,    -1,    -1,    -1,    20,    -1,
       4,     5,     6,     7,    44,     9,    -1,    -1,    48,    49,
      32,    -1,    52,    -1,    54,    -1,    56,    -1,    -1,    -1,
      -1,    -1,    44,    -1,    -1,    -1,    48,    49,    32,    -1,
      52,    -1,    54,    -1,    56,     4,     5,     6,     7,    -1,
       9,    -1,    -1,    -1,    48,    49,    15,    16,    52,    -1,
      54,    20,    56,     4,     5,     6,     7,    -1,     9,    -1,
      -1,    -1,    -1,    32,    15,    16,    -1,    -1,    -1,    20,
      -1,     4,     5,     6,     7,    44,     9,    -1,    -1,    48,
      49,    32,    -1,    52,    -1,    54,    55,    56,    -1,    -1,
      -1,    -1,    -1,    44,    -1,    -1,    -1,    48,    49,    32,
      -1,    52,    -1,    54,    -1,    56,    -1,    -1,    -1,    -1,
      -1,    44,    -1,    -1,    -1,    48,    49,    -1,    -1,    52,
      -1,    54,    -1,    56,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    -1,    -1,    -1,    -1,
      -1,    -1,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    -1,    -1,    51,    -1,    -1,    -1,    55,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    -1,    -1,    -1,    -1,    -1,    -1,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    10,    11,
      12,    13,    -1,    -1,    -1,    55,    -1,    -1,    -1,    -1,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    10,    11,    12,    13,
      -1,    -1,    -1,    55,    -1,    -1,    -1,    -1,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    -1,    -1,
      -1,    -1,    -1,    -1,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    10,    11,    12,    13,    -1,    -1,
      -1,    55,    -1,    -1,    -1,    -1,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    -1,    -1,    -1,    -1,
      -1,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    -1,    50,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    -1,    -1,    -1,
      -1,    -1,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    -1,    50,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    -1,    -1,
      -1,    -1,    -1,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    -1,    50,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    -1,
      -1,    -1,    -1,    -1,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    -1,    50,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      -1,    -1,    -1,    -1,    -1,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    -1,    50,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    -1,    -1,    -1,    -1,    -1,    -1,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    -1,    50,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    -1,    -1,    -1,    -1,    -1,    -1,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    -1,
      50,    10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    -1,    -1,    -1,    -1,    -1,    -1,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      -1,    50,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    -1,    -1,    -1,    -1,    -1,    -1,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    -1,    50,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,    18,    19,    -1,    -1,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    -1,    -1,    -1,    -1,    -1,
      -1,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    10,    11,    12,    13,    -1,    -1,    -1,    17,    -1,
      -1,    -1,    -1,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    -1,    -1,    -1,    -1,    -1,    -1,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    -1,    -1,    -1,    -1,    -1,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    -1,
      -1,    -1,    35,    -1,    -1,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    -1,    -1,    -1,
      -1,    -1,    -1,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    10,    11,    12,    13,    -1,    -1,    -1,
      17,    -1,    -1,    -1,    -1,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    -1,    -1,    -1,    -1,    -1,
      -1,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    -1,    -1,    -1,    -1,    -1,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    -1,    -1,    -1,    -1,    -1,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    -1,
      -1,    -1,    -1,    -1,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    -1,    -1,    -1,
      -1,    -1,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    10,    11,    12,    13,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    -1,    -1,    -1,    -1,    -1,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    -1,    -1,    -1,    -1,    -1,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    10,
      11,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    -1,    -1,    -1,    -1,    -1,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    -1,
      -1,    -1,    -1,    -1,    -1,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    10,    11,    12,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    -1,    -1,    -1,
      -1,    10,    11,    12,    -1,    40,    41,    42,    43,    44,
      45,    46,    47,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    -1,    -1,    -1,    -1,    -1,    11,    12,
      -1,    40,    41,    42,    43,    44,    45,    46,    47,    22,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    31,    11,
      12,    -1,    -1,    -1,    -1,    -1,    11,    12,    41,    42,
      43,    44,    45,    46,    47,    -1,    -1,    -1,    30,    31,
      -1,    -1,    -1,    -1,    -1,    30,    31,    -1,    -1,    41,
      42,    43,    44,    45,    46,    47,    41,    42,    43,    44,
      45,    46,    47
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     4,     5,     6,     7,     9,    15,    16,    20,    32,
      44,    48,    49,    52,    54,    56,    59,    60,    61,    62,
      63,    69,    49,    53,    32,     4,    61,    62,    69,    64,
      61,     4,     1,    61,     1,     4,     7,    63,     1,    55,
      61,     1,     4,    49,    63,    70,    71,     0,    10,    11,
      12,    13,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    60,    61,     5,    14,    52,    54,    61,    65,
      49,    51,    17,    14,    66,    50,    50,     1,    53,    55,
      55,    39,    57,    51,     1,    61,    51,    57,    39,    61,
      61,    61,    61,    61,    61,    61,    61,    61,    61,    61,
      61,    61,    61,    61,    61,    61,    61,    61,    61,    61,
      61,    61,    61,    53,    48,    63,     1,    51,    55,    61,
      37,    50,    66,     4,    61,     1,    61,    48,    33,    34,
      36,     1,    70,    44,    68,    69,    50,    50,    68,    70,
       4,    53,    55,    61,    53,    51,    55,    61,    36,    37,
      50,    37,    18,    19,    67,     4,    61,    68,    38,    51,
      51,    38,    55,    55,    61,    53,    37,    50,     4,    51,
      61,    61,    49,    35,    68,    68,    68,    61,    53,    53,
      55,    61,    37,    50,    61,    21,    17,    61,    53,    37,
      50,     4,    51,    37,    61,    37,    61,    37,    50,    61,
      67,    61,    37,    50,     4,    51,    37,    50,    61,    37,
      50,    61,    37,    50,     4,    51,    37,    61,    37,    50,
      61,    50,     4,    51,    37,    50,    61,    51,    37,    61,
      37
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    58,    59,    59,    60,    60,    61,    61,    61,    61,
      61,    61,    61,    61,    61,    61,    61,    61,    61,    61,
      61,    61,    61,    61,    61,    61,    61,    61,    61,    61,
      61,    61,    61,    61,    61,    61,    61,    62,    62,    62,
      62,    62,    62,    62,    64,    63,    65,    63,    66,    66,
      66,    67,    67,    68,    68,    68,    69,    69,    69,    69,
      69,    69,    69,    69,    69,    69,    69,    69,    69,    69,
      69,    69,    69,    69,    69,    69,    69,    69,    69,    69,
      69,    69,    69,    69,    69,    69,    69,    69,    69,    69,
      69,    69,    69,    69,    69,    69,    69,    70,    70,    70,
      70,    71,    71,    71,    71,    71,    71
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     0,     2,     2,     6,    10,     5,
       4,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     1,     5,     8,    10,
      12,    14,    16,    18,     0,     4,     0,     5,     0,     2,
       4,     5,     3,     3,     2,     1,     1,     1,     3,     2,
       4,     3,     2,     1,     3,     2,     2,     3,     5,     4,
       4,     3,     7,     6,     6,     6,     5,     5,     1,     1,
       1,     3,     3,     2,     3,     2,     1,     4,     6,     8,
      10,    12,    14,     3,     3,     4,     3,     0,     1,     3,
       3,     3,     3,     1,     1,     5,     5
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (&yylloc, answer, errors, locations, lexer_param_ptr, YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static unsigned
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  unsigned res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
 }

#  define YY_LOCATION_PRINT(File, Loc)          \
  yy_location_print_ (File, &(Loc))

# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, Location, answer, errors, locations, lexer_param_ptr); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, block* answer, int* errors, struct locfile* locations, struct lexer_param* lexer_param_ptr)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (yylocationp);
  YYUSE (answer);
  YYUSE (errors);
  YYUSE (locations);
  YYUSE (lexer_param_ptr);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, block* answer, int* errors, struct locfile* locations, struct lexer_param* lexer_param_ptr)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, answer, errors, locations, lexer_param_ptr);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, block* answer, int* errors, struct locfile* locations, struct lexer_param* lexer_param_ptr)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                       , &(yylsp[(yyi + 1) - (yynrhs)])                       , answer, errors, locations, lexer_param_ptr);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule, answer, errors, locations, lexer_param_ptr); \
} while (0)

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
#ifndef YYINITDEPTH
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
static YYSIZE_T
yystrlen (const char *yystr)
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
static char *
yystpcpy (char *yydest, const char *yysrc)
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

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, block* answer, int* errors, struct locfile* locations, struct lexer_param* lexer_param_ptr)
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  YYUSE (answer);
  YYUSE (errors);
  YYUSE (locations);
  YYUSE (lexer_param_ptr);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yytype)
    {
          case 4: /* IDENT  */
#line 35 "parser.y" /* yacc.c:1257  */
      { jv_free(((*yyvaluep).literal)); }
#line 1698 "parser.c" /* yacc.c:1257  */
        break;

    case 5: /* FIELD  */
#line 35 "parser.y" /* yacc.c:1257  */
      { jv_free(((*yyvaluep).literal)); }
#line 1704 "parser.c" /* yacc.c:1257  */
        break;

    case 6: /* LITERAL  */
#line 35 "parser.y" /* yacc.c:1257  */
      { jv_free(((*yyvaluep).literal)); }
#line 1710 "parser.c" /* yacc.c:1257  */
        break;

    case 7: /* FORMAT  */
#line 35 "parser.y" /* yacc.c:1257  */
      { jv_free(((*yyvaluep).literal)); }
#line 1716 "parser.c" /* yacc.c:1257  */
        break;

    case 33: /* QQSTRING_TEXT  */
#line 35 "parser.y" /* yacc.c:1257  */
      { jv_free(((*yyvaluep).literal)); }
#line 1722 "parser.c" /* yacc.c:1257  */
        break;

    case 60: /* FuncDefs  */
#line 36 "parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 1728 "parser.c" /* yacc.c:1257  */
        break;

    case 61: /* Exp  */
#line 36 "parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 1734 "parser.c" /* yacc.c:1257  */
        break;

    case 62: /* FuncDef  */
#line 36 "parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 1740 "parser.c" /* yacc.c:1257  */
        break;

    case 63: /* String  */
#line 36 "parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 1746 "parser.c" /* yacc.c:1257  */
        break;

    case 66: /* QQString  */
#line 36 "parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 1752 "parser.c" /* yacc.c:1257  */
        break;

    case 67: /* ElseBody  */
#line 36 "parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 1758 "parser.c" /* yacc.c:1257  */
        break;

    case 68: /* ExpD  */
#line 36 "parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 1764 "parser.c" /* yacc.c:1257  */
        break;

    case 69: /* Term  */
#line 36 "parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 1770 "parser.c" /* yacc.c:1257  */
        break;

    case 70: /* MkDict  */
#line 36 "parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 1776 "parser.c" /* yacc.c:1257  */
        break;

    case 71: /* MkDictPair  */
#line 36 "parser.y" /* yacc.c:1257  */
      { block_free(((*yyvaluep).blk)); }
#line 1782 "parser.c" /* yacc.c:1257  */
        break;


      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (block* answer, int* errors, struct locfile* locations, struct lexer_param* lexer_param_ptr)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

/* Location data for the lookahead symbol.  */
static YYLTYPE yyloc_default
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
YYLTYPE yylloc = yyloc_default;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.
       'yyls': related to locations.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[3];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yylsp = yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  yylsp[0] = yylloc;
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
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yyls1, yysize * sizeof (*yylsp),
                    &yystacksize);

        yyls = yyls1;
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
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

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
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex (&yylval, &yylloc, answer, errors, locations, lexer_param_ptr);
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
      if (yytable_value_is_error (yyn))
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
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;
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
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 205 "parser.y" /* yacc.c:1646  */
    {
  *answer = (yyvsp[0].blk);
}
#line 2078 "parser.c" /* yacc.c:1646  */
    break;

  case 3:
#line 208 "parser.y" /* yacc.c:1646  */
    {
  *answer = (yyvsp[0].blk);
}
#line 2086 "parser.c" /* yacc.c:1646  */
    break;

  case 4:
#line 213 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_noop();
}
#line 2094 "parser.c" /* yacc.c:1646  */
    break;

  case 5:
#line 216 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = block_join((yyvsp[-1].blk), (yyvsp[0].blk));
}
#line 2102 "parser.c" /* yacc.c:1646  */
    break;

  case 6:
#line 221 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = block_bind((yyvsp[-1].blk), (yyvsp[0].blk), OP_IS_CALL_PSEUDO);
}
#line 2110 "parser.c" /* yacc.c:1646  */
    break;

  case 7:
#line 225 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_var_binding((yyvsp[-5].blk), jv_string_value((yyvsp[-2].literal)), (yyvsp[0].blk));
  jv_free((yyvsp[-2].literal));
}
#line 2119 "parser.c" /* yacc.c:1646  */
    break;

  case 8:
#line 230 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_reduce(jv_string_value((yyvsp[-5].literal)), (yyvsp[-8].blk), (yyvsp[-3].blk), (yyvsp[-1].blk));
  jv_free((yyvsp[-5].literal));
}
#line 2128 "parser.c" /* yacc.c:1646  */
    break;

  case 9:
#line 235 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_cond((yyvsp[-3].blk), (yyvsp[-1].blk), (yyvsp[0].blk));
}
#line 2136 "parser.c" /* yacc.c:1646  */
    break;

  case 10:
#line 238 "parser.y" /* yacc.c:1646  */
    {
  FAIL((yyloc), "Possibly unterminated 'if' statement");
  (yyval.blk) = (yyvsp[-2].blk);
}
#line 2145 "parser.c" /* yacc.c:1646  */
    break;

  case 11:
#line 243 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_call("_assign", BLOCK(gen_lambda((yyvsp[-2].blk)), gen_lambda((yyvsp[0].blk))));
}
#line 2153 "parser.c" /* yacc.c:1646  */
    break;

  case 12:
#line 247 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_or((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2161 "parser.c" /* yacc.c:1646  */
    break;

  case 13:
#line 251 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_and((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2169 "parser.c" /* yacc.c:1646  */
    break;

  case 14:
#line 255 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_definedor((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2177 "parser.c" /* yacc.c:1646  */
    break;

  case 15:
#line 259 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_definedor_assign((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2185 "parser.c" /* yacc.c:1646  */
    break;

  case 16:
#line 263 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_call("_modify", BLOCK(gen_lambda((yyvsp[-2].blk)), gen_lambda((yyvsp[0].blk))));
}
#line 2193 "parser.c" /* yacc.c:1646  */
    break;

  case 17:
#line 267 "parser.y" /* yacc.c:1646  */
    { 
  (yyval.blk) = block_join((yyvsp[-2].blk), (yyvsp[0].blk)); 
}
#line 2201 "parser.c" /* yacc.c:1646  */
    break;

  case 18:
#line 271 "parser.y" /* yacc.c:1646  */
    { 
  (yyval.blk) = gen_both((yyvsp[-2].blk), (yyvsp[0].blk)); 
}
#line 2209 "parser.c" /* yacc.c:1646  */
    break;

  case 19:
#line 275 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '+');
}
#line 2217 "parser.c" /* yacc.c:1646  */
    break;

  case 20:
#line 279 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_update((yyvsp[-2].blk), (yyvsp[0].blk), '+');
}
#line 2225 "parser.c" /* yacc.c:1646  */
    break;

  case 21:
#line 283 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = BLOCK((yyvsp[0].blk), gen_call("_negate", gen_noop()));
}
#line 2233 "parser.c" /* yacc.c:1646  */
    break;

  case 22:
#line 287 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '-');
}
#line 2241 "parser.c" /* yacc.c:1646  */
    break;

  case 23:
#line 291 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_update((yyvsp[-2].blk), (yyvsp[0].blk), '-');
}
#line 2249 "parser.c" /* yacc.c:1646  */
    break;

  case 24:
#line 295 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '*');
}
#line 2257 "parser.c" /* yacc.c:1646  */
    break;

  case 25:
#line 299 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_update((yyvsp[-2].blk), (yyvsp[0].blk), '*');
}
#line 2265 "parser.c" /* yacc.c:1646  */
    break;

  case 26:
#line 303 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '/');
}
#line 2273 "parser.c" /* yacc.c:1646  */
    break;

  case 27:
#line 307 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '%');
}
#line 2281 "parser.c" /* yacc.c:1646  */
    break;

  case 28:
#line 311 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_update((yyvsp[-2].blk), (yyvsp[0].blk), '/');
}
#line 2289 "parser.c" /* yacc.c:1646  */
    break;

  case 29:
#line 315 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_update((yyvsp[-2].blk), (yyvsp[0].blk), '%');
}
#line 2297 "parser.c" /* yacc.c:1646  */
    break;

  case 30:
#line 319 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), EQ);
}
#line 2305 "parser.c" /* yacc.c:1646  */
    break;

  case 31:
#line 323 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), NEQ);
}
#line 2313 "parser.c" /* yacc.c:1646  */
    break;

  case 32:
#line 327 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '<');
}
#line 2321 "parser.c" /* yacc.c:1646  */
    break;

  case 33:
#line 331 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), '>');
}
#line 2329 "parser.c" /* yacc.c:1646  */
    break;

  case 34:
#line 335 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), LESSEQ);
}
#line 2337 "parser.c" /* yacc.c:1646  */
    break;

  case 35:
#line 339 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-2].blk), (yyvsp[0].blk), GREATEREQ);
}
#line 2345 "parser.c" /* yacc.c:1646  */
    break;

  case 36:
#line 343 "parser.y" /* yacc.c:1646  */
    { 
  (yyval.blk) = (yyvsp[0].blk); 
}
#line 2353 "parser.c" /* yacc.c:1646  */
    break;

  case 37:
#line 348 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_function(jv_string_value((yyvsp[-3].literal)), gen_noop(), (yyvsp[-1].blk));
  jv_free((yyvsp[-3].literal));
}
#line 2362 "parser.c" /* yacc.c:1646  */
    break;

  case 38:
#line 353 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_function(jv_string_value((yyvsp[-6].literal)), 
                    gen_param(jv_string_value((yyvsp[-4].literal))), 
                    (yyvsp[-1].blk));
  jv_free((yyvsp[-6].literal));
  jv_free((yyvsp[-4].literal));
}
#line 2374 "parser.c" /* yacc.c:1646  */
    break;

  case 39:
#line 361 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_function(jv_string_value((yyvsp[-8].literal)), 
                    BLOCK(gen_param(jv_string_value((yyvsp[-6].literal))), 
                          gen_param(jv_string_value((yyvsp[-4].literal)))),
                    (yyvsp[-1].blk));
  jv_free((yyvsp[-8].literal));
  jv_free((yyvsp[-6].literal));
  jv_free((yyvsp[-4].literal));
}
#line 2388 "parser.c" /* yacc.c:1646  */
    break;

  case 40:
#line 371 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_function(jv_string_value((yyvsp[-10].literal)), 
                    BLOCK(gen_param(jv_string_value((yyvsp[-8].literal))), 
                          gen_param(jv_string_value((yyvsp[-6].literal))),
                          gen_param(jv_string_value((yyvsp[-4].literal)))),
                    (yyvsp[-1].blk));
  jv_free((yyvsp[-10].literal));
  jv_free((yyvsp[-8].literal));
  jv_free((yyvsp[-6].literal));
  jv_free((yyvsp[-4].literal));
}
#line 2404 "parser.c" /* yacc.c:1646  */
    break;

  case 41:
#line 383 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_function(jv_string_value((yyvsp[-12].literal)), 
                    BLOCK(gen_param(jv_string_value((yyvsp[-10].literal))), 
                          gen_param(jv_string_value((yyvsp[-8].literal))),
                          gen_param(jv_string_value((yyvsp[-6].literal))),
                          gen_param(jv_string_value((yyvsp[-4].literal)))),
                    (yyvsp[-1].blk));
  jv_free((yyvsp[-12].literal));
  jv_free((yyvsp[-10].literal));
  jv_free((yyvsp[-8].literal));
  jv_free((yyvsp[-6].literal));
  jv_free((yyvsp[-4].literal));
}
#line 2422 "parser.c" /* yacc.c:1646  */
    break;

  case 42:
#line 397 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_function(jv_string_value((yyvsp[-14].literal)), 
                    BLOCK(gen_param(jv_string_value((yyvsp[-12].literal))), 
                          gen_param(jv_string_value((yyvsp[-10].literal))),
                          gen_param(jv_string_value((yyvsp[-8].literal))),
                          gen_param(jv_string_value((yyvsp[-6].literal))),
                          gen_param(jv_string_value((yyvsp[-4].literal)))),
                    (yyvsp[-1].blk));
  jv_free((yyvsp[-14].literal));
  jv_free((yyvsp[-12].literal));
  jv_free((yyvsp[-10].literal));
  jv_free((yyvsp[-8].literal));
  jv_free((yyvsp[-6].literal));
  jv_free((yyvsp[-4].literal));
}
#line 2442 "parser.c" /* yacc.c:1646  */
    break;

  case 43:
#line 413 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_function(jv_string_value((yyvsp[-16].literal)), 
                    BLOCK(gen_param(jv_string_value((yyvsp[-14].literal))), 
                          gen_param(jv_string_value((yyvsp[-12].literal))),
                          gen_param(jv_string_value((yyvsp[-10].literal))),
                          gen_param(jv_string_value((yyvsp[-8].literal))),
                          gen_param(jv_string_value((yyvsp[-6].literal))),
                          gen_param(jv_string_value((yyvsp[-4].literal)))),
                    (yyvsp[-1].blk));
  jv_free((yyvsp[-16].literal));
  jv_free((yyvsp[-14].literal));
  jv_free((yyvsp[-12].literal));
  jv_free((yyvsp[-10].literal));
  jv_free((yyvsp[-8].literal));
  jv_free((yyvsp[-6].literal));
  jv_free((yyvsp[-4].literal));
}
#line 2464 "parser.c" /* yacc.c:1646  */
    break;

  case 44:
#line 433 "parser.y" /* yacc.c:1646  */
    { (yyval.literal) = jv_string("text"); }
#line 2470 "parser.c" /* yacc.c:1646  */
    break;

  case 45:
#line 433 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = (yyvsp[-1].blk);
  jv_free((yyvsp[-2].literal));
}
#line 2479 "parser.c" /* yacc.c:1646  */
    break;

  case 46:
#line 437 "parser.y" /* yacc.c:1646  */
    { (yyval.literal) = (yyvsp[-1].literal); }
#line 2485 "parser.c" /* yacc.c:1646  */
    break;

  case 47:
#line 437 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = (yyvsp[-1].blk);
  jv_free((yyvsp[-2].literal));
}
#line 2494 "parser.c" /* yacc.c:1646  */
    break;

  case 48:
#line 444 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_const(jv_string(""));
}
#line 2502 "parser.c" /* yacc.c:1646  */
    break;

  case 49:
#line 447 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-1].blk), gen_const((yyvsp[0].literal)), '+');
}
#line 2510 "parser.c" /* yacc.c:1646  */
    break;

  case 50:
#line 450 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_binop((yyvsp[-3].blk), gen_format((yyvsp[-1].blk), jv_copy((yyvsp[-4].literal))), '+');
}
#line 2518 "parser.c" /* yacc.c:1646  */
    break;

  case 51:
#line 456 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_cond((yyvsp[-3].blk), (yyvsp[-1].blk), (yyvsp[0].blk));
}
#line 2526 "parser.c" /* yacc.c:1646  */
    break;

  case 52:
#line 459 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = (yyvsp[-1].blk);
}
#line 2534 "parser.c" /* yacc.c:1646  */
    break;

  case 53:
#line 464 "parser.y" /* yacc.c:1646  */
    { 
  (yyval.blk) = block_join((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2542 "parser.c" /* yacc.c:1646  */
    break;

  case 54:
#line 467 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = BLOCK((yyvsp[0].blk), gen_call("_negate", gen_noop()));
}
#line 2550 "parser.c" /* yacc.c:1646  */
    break;

  case 55:
#line 470 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = (yyvsp[0].blk);
}
#line 2558 "parser.c" /* yacc.c:1646  */
    break;

  case 56:
#line 476 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_noop(); 
}
#line 2566 "parser.c" /* yacc.c:1646  */
    break;

  case 57:
#line 479 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_call("recurse_down", gen_noop());
}
#line 2574 "parser.c" /* yacc.c:1646  */
    break;

  case 58:
#line 482 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_index_opt((yyvsp[-2].blk), gen_const((yyvsp[-1].literal)));
}
#line 2582 "parser.c" /* yacc.c:1646  */
    break;

  case 59:
#line 485 "parser.y" /* yacc.c:1646  */
    { 
  (yyval.blk) = gen_index_opt(gen_noop(), gen_const((yyvsp[-1].literal))); 
}
#line 2590 "parser.c" /* yacc.c:1646  */
    break;

  case 60:
#line 488 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_index_opt((yyvsp[-3].blk), (yyvsp[-1].blk));
}
#line 2598 "parser.c" /* yacc.c:1646  */
    break;

  case 61:
#line 491 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_index_opt(gen_noop(), (yyvsp[-1].blk));
}
#line 2606 "parser.c" /* yacc.c:1646  */
    break;

  case 62:
#line 494 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_index((yyvsp[-1].blk), gen_const((yyvsp[0].literal)));
}
#line 2614 "parser.c" /* yacc.c:1646  */
    break;

  case 63:
#line 497 "parser.y" /* yacc.c:1646  */
    { 
  (yyval.blk) = gen_index(gen_noop(), gen_const((yyvsp[0].literal))); 
}
#line 2622 "parser.c" /* yacc.c:1646  */
    break;

  case 64:
#line 500 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_index((yyvsp[-2].blk), (yyvsp[0].blk));
}
#line 2630 "parser.c" /* yacc.c:1646  */
    break;

  case 65:
#line 503 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_index(gen_noop(), (yyvsp[0].blk));
}
#line 2638 "parser.c" /* yacc.c:1646  */
    break;

  case 66:
#line 506 "parser.y" /* yacc.c:1646  */
    {
  FAIL((yyloc), "try .[\"field\"] instead of .field for unusually named fields");
  (yyval.blk) = gen_noop();
}
#line 2647 "parser.c" /* yacc.c:1646  */
    break;

  case 67:
#line 510 "parser.y" /* yacc.c:1646  */
    {
  jv_free((yyvsp[-1].literal));
  FAIL((yyloc), "try .[\"field\"] instead of .field for unusually named fields");
  (yyval.blk) = gen_noop();
}
#line 2657 "parser.c" /* yacc.c:1646  */
    break;

  case 68:
#line 516 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_index_opt((yyvsp[-4].blk), (yyvsp[-2].blk)); 
}
#line 2665 "parser.c" /* yacc.c:1646  */
    break;

  case 69:
#line 519 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_index((yyvsp[-3].blk), (yyvsp[-1].blk)); 
}
#line 2673 "parser.c" /* yacc.c:1646  */
    break;

  case 70:
#line 522 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = block_join((yyvsp[-3].blk), gen_op_simple(EACH_OPT)); 
}
#line 2681 "parser.c" /* yacc.c:1646  */
    break;

  case 71:
#line 525 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = block_join((yyvsp[-2].blk), gen_op_simple(EACH)); 
}
#line 2689 "parser.c" /* yacc.c:1646  */
    break;

  case 72:
#line 528 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_slice_index((yyvsp[-6].blk), (yyvsp[-4].blk), (yyvsp[-2].blk), INDEX_OPT);
}
#line 2697 "parser.c" /* yacc.c:1646  */
    break;

  case 73:
#line 531 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_slice_index((yyvsp[-5].blk), (yyvsp[-3].blk), gen_const(jv_null()), INDEX_OPT);
}
#line 2705 "parser.c" /* yacc.c:1646  */
    break;

  case 74:
#line 534 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_slice_index((yyvsp[-5].blk), gen_const(jv_null()), (yyvsp[-2].blk), INDEX_OPT);
}
#line 2713 "parser.c" /* yacc.c:1646  */
    break;

  case 75:
#line 537 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_slice_index((yyvsp[-5].blk), (yyvsp[-3].blk), (yyvsp[-1].blk), INDEX);
}
#line 2721 "parser.c" /* yacc.c:1646  */
    break;

  case 76:
#line 540 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_slice_index((yyvsp[-4].blk), (yyvsp[-2].blk), gen_const(jv_null()), INDEX);
}
#line 2729 "parser.c" /* yacc.c:1646  */
    break;

  case 77:
#line 543 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_slice_index((yyvsp[-4].blk), gen_const(jv_null()), (yyvsp[-1].blk), INDEX);
}
#line 2737 "parser.c" /* yacc.c:1646  */
    break;

  case 78:
#line 546 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_const((yyvsp[0].literal)); 
}
#line 2745 "parser.c" /* yacc.c:1646  */
    break;

  case 79:
#line 549 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = (yyvsp[0].blk);
}
#line 2753 "parser.c" /* yacc.c:1646  */
    break;

  case 80:
#line 552 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_format(gen_noop(), (yyvsp[0].literal));
}
#line 2761 "parser.c" /* yacc.c:1646  */
    break;

  case 81:
#line 555 "parser.y" /* yacc.c:1646  */
    { 
  (yyval.blk) = (yyvsp[-1].blk); 
}
#line 2769 "parser.c" /* yacc.c:1646  */
    break;

  case 82:
#line 558 "parser.y" /* yacc.c:1646  */
    { 
  (yyval.blk) = gen_collect((yyvsp[-1].blk)); 
}
#line 2777 "parser.c" /* yacc.c:1646  */
    break;

  case 83:
#line 561 "parser.y" /* yacc.c:1646  */
    { 
  (yyval.blk) = gen_const(jv_array()); 
}
#line 2785 "parser.c" /* yacc.c:1646  */
    break;

  case 84:
#line 564 "parser.y" /* yacc.c:1646  */
    { 
  (yyval.blk) = BLOCK(gen_subexp(gen_const(jv_object())), (yyvsp[-1].blk), gen_op_simple(POP));
}
#line 2793 "parser.c" /* yacc.c:1646  */
    break;

  case 85:
#line 567 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_location((yyloc), gen_op_unbound(LOADV, jv_string_value((yyvsp[0].literal))));
  jv_free((yyvsp[0].literal));
}
#line 2802 "parser.c" /* yacc.c:1646  */
    break;

  case 86:
#line 571 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_location((yyloc), gen_call(jv_string_value((yyvsp[0].literal)), gen_noop()));
  jv_free((yyvsp[0].literal));
}
#line 2811 "parser.c" /* yacc.c:1646  */
    break;

  case 87:
#line 575 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_call(jv_string_value((yyvsp[-3].literal)), gen_lambda((yyvsp[-1].blk)));
  (yyval.blk) = gen_location((yylsp[-3]), (yyval.blk));
  jv_free((yyvsp[-3].literal));
}
#line 2821 "parser.c" /* yacc.c:1646  */
    break;

  case 88:
#line 580 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_call(jv_string_value((yyvsp[-5].literal)), BLOCK(gen_lambda((yyvsp[-3].blk)), gen_lambda((yyvsp[-1].blk))));
  (yyval.blk) = gen_location((yylsp[-5]), (yyval.blk));
  jv_free((yyvsp[-5].literal));
}
#line 2831 "parser.c" /* yacc.c:1646  */
    break;

  case 89:
#line 585 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_call(jv_string_value((yyvsp[-7].literal)), BLOCK(gen_lambda((yyvsp[-5].blk)), gen_lambda((yyvsp[-3].blk)), gen_lambda((yyvsp[-1].blk))));
  (yyval.blk) = gen_location((yylsp[-7]), (yyval.blk));
  jv_free((yyvsp[-7].literal));
}
#line 2841 "parser.c" /* yacc.c:1646  */
    break;

  case 90:
#line 590 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_call(jv_string_value((yyvsp[-9].literal)), BLOCK(gen_lambda((yyvsp[-7].blk)), gen_lambda((yyvsp[-5].blk)), gen_lambda((yyvsp[-3].blk)), gen_lambda((yyvsp[-1].blk))));
  (yyval.blk) = gen_location((yylsp[-9]), (yyval.blk));
  jv_free((yyvsp[-9].literal));
}
#line 2851 "parser.c" /* yacc.c:1646  */
    break;

  case 91:
#line 595 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_call(jv_string_value((yyvsp[-11].literal)), BLOCK(gen_lambda((yyvsp[-9].blk)), gen_lambda((yyvsp[-7].blk)), gen_lambda((yyvsp[-5].blk)), gen_lambda((yyvsp[-3].blk)), gen_lambda((yyvsp[-1].blk))));
  (yyval.blk) = gen_location((yylsp[-11]), (yyval.blk));
  jv_free((yyvsp[-11].literal));
}
#line 2861 "parser.c" /* yacc.c:1646  */
    break;

  case 92:
#line 600 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_call(jv_string_value((yyvsp[-13].literal)), BLOCK(gen_lambda((yyvsp[-11].blk)), gen_lambda((yyvsp[-9].blk)), gen_lambda((yyvsp[-7].blk)), gen_lambda((yyvsp[-5].blk)), gen_lambda((yyvsp[-3].blk)), gen_lambda((yyvsp[-1].blk))));
  (yyval.blk) = gen_location((yylsp[-13]), (yyval.blk));
  jv_free((yyvsp[-13].literal));
}
#line 2871 "parser.c" /* yacc.c:1646  */
    break;

  case 93:
#line 605 "parser.y" /* yacc.c:1646  */
    { (yyval.blk) = gen_noop(); }
#line 2877 "parser.c" /* yacc.c:1646  */
    break;

  case 94:
#line 606 "parser.y" /* yacc.c:1646  */
    { (yyval.blk) = gen_noop(); }
#line 2883 "parser.c" /* yacc.c:1646  */
    break;

  case 95:
#line 607 "parser.y" /* yacc.c:1646  */
    { (yyval.blk) = (yyvsp[-3].blk); }
#line 2889 "parser.c" /* yacc.c:1646  */
    break;

  case 96:
#line 608 "parser.y" /* yacc.c:1646  */
    { (yyval.blk) = gen_noop(); }
#line 2895 "parser.c" /* yacc.c:1646  */
    break;

  case 97:
#line 611 "parser.y" /* yacc.c:1646  */
    { 
  (yyval.blk)=gen_noop(); 
}
#line 2903 "parser.c" /* yacc.c:1646  */
    break;

  case 98:
#line 614 "parser.y" /* yacc.c:1646  */
    { (yyval.blk) = (yyvsp[0].blk); }
#line 2909 "parser.c" /* yacc.c:1646  */
    break;

  case 99:
#line 615 "parser.y" /* yacc.c:1646  */
    { (yyval.blk)=block_join((yyvsp[-2].blk), (yyvsp[0].blk)); }
#line 2915 "parser.c" /* yacc.c:1646  */
    break;

  case 100:
#line 616 "parser.y" /* yacc.c:1646  */
    { (yyval.blk) = (yyvsp[0].blk); }
#line 2921 "parser.c" /* yacc.c:1646  */
    break;

  case 101:
#line 619 "parser.y" /* yacc.c:1646  */
    { 
  (yyval.blk) = gen_dictpair(gen_const((yyvsp[-2].literal)), (yyvsp[0].blk));
 }
#line 2929 "parser.c" /* yacc.c:1646  */
    break;

  case 102:
#line 622 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_dictpair((yyvsp[-2].blk), (yyvsp[0].blk));
  }
#line 2937 "parser.c" /* yacc.c:1646  */
    break;

  case 103:
#line 625 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_dictpair((yyvsp[0].blk), BLOCK(gen_op_simple(POP), gen_op_simple(DUP2),
                              gen_op_simple(DUP2), gen_op_simple(INDEX)));
  }
#line 2946 "parser.c" /* yacc.c:1646  */
    break;

  case 104:
#line 629 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_dictpair(gen_const(jv_copy((yyvsp[0].literal))),
                    gen_index(gen_noop(), gen_const((yyvsp[0].literal))));
  }
#line 2955 "parser.c" /* yacc.c:1646  */
    break;

  case 105:
#line 633 "parser.y" /* yacc.c:1646  */
    {
  (yyval.blk) = gen_dictpair((yyvsp[-3].blk), (yyvsp[0].blk));
  }
#line 2963 "parser.c" /* yacc.c:1646  */
    break;

  case 106:
#line 636 "parser.y" /* yacc.c:1646  */
    { (yyval.blk) = (yyvsp[0].blk); }
#line 2969 "parser.c" /* yacc.c:1646  */
    break;


#line 2973 "parser.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (&yylloc, answer, errors, locations, lexer_param_ptr, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (&yylloc, answer, errors, locations, lexer_param_ptr, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }

  yyerror_range[1] = yylloc;

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
                      yytoken, &yylval, &yylloc, answer, errors, locations, lexer_param_ptr);
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

  yyerror_range[1] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule whose action triggered
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
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
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

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, yylsp, answer, errors, locations, lexer_param_ptr);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
  *++yylsp = yyloc;

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

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (&yylloc, answer, errors, locations, lexer_param_ptr, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc, answer, errors, locations, lexer_param_ptr);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, yylsp, answer, errors, locations, lexer_param_ptr);
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
  return yyresult;
}
#line 637 "parser.y" /* yacc.c:1906  */


int jq_parse(struct locfile* locations, block* answer) {
  struct lexer_param scanner;
  YY_BUFFER_STATE buf;
  jq_yylex_init_extra(0, &scanner.lexer);
  buf = jq_yy_scan_bytes(locations->data, locations->length, scanner.lexer);
  int errors = 0;
  *answer = gen_noop();
  yyparse(answer, &errors, locations, &scanner);
  jq_yy_delete_buffer(buf, scanner.lexer);
  jq_yylex_destroy(scanner.lexer);
  if (errors > 0) {
    block_free(*answer);
    *answer = gen_noop();
  }
  return errors;
}

int jq_parse_library(struct locfile* locations, block* answer) {
  int errs = jq_parse(locations, answer);
  if (errs) return errs;
  if (!block_has_only_binders(*answer, OP_IS_CALL_PSEUDO)) {
    locfile_locate(locations, UNKNOWN_LOCATION, "error: library should only have function definitions, not a main expression");
    return 1;
  }
  return 0;
}
