/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

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
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         giac_yyparse
#define yylex           giac_yylex
#define yyerror         giac_yyerror
#define yydebug         giac_yydebug
#define yynerrs         giac_yynerrs


/* Copy the first part of user declarations.  */
#line 24 "input_parser.yy" /* yacc.c:339  */

         #define YYPARSE_PARAM scanner
         #define YYLEX_PARAM   scanner
	 
#line 33 "input_parser.yy" /* yacc.c:339  */

#include "giacPCH.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "first.h"
#include <stdexcept>
#include <cstdlib>
#include "giacPCH.h"
#include "index.h"
#include "gen.h"
#define YYSTYPE giac::gen
#define YY_EXTRA_TYPE  const giac::context *
#include "lexer.h"
#include "input_lexer.h"
#include "usual.h"
#include "derive.h"
#include "sym2poly.h"
#include "vecteur.h"
#include "modpoly.h"
#include "alg_ext.h"
#include "prog.h"
#include "rpn.h"
#include "intg.h"
#include "plot.h"
#include "maple.h"
using namespace std;

#ifndef NO_NAMESPACE_GIAC
namespace giac {
#endif // ndef NO_NAMESPACE_GIAC

// It seems there is a bison bug when it reallocates space for the stack
// therefore I redefine YYINITDEPTH to 4000 (max size is YYMAXDEPTH)
// instead of 200
// Feel free to change if you need but then readjust YYMAXDEPTH
#if defined RTOS_THREADX || defined NSPIRE || defined NSPIRE_NEWLIB || defined NUMWORKS
#ifdef RTOS_THREADX
#define YYINITDEPTH 100
#define YYMAXDEPTH 101
#else
#define YYINITDEPTH 200
#define YYMAXDEPTH 201
#endif
#else // RTOS_THREADX
// Note that the compilation by bison with -v option generates a file y.output
// to debug the grammar, compile input_parser.yy with bison
// then add yydebug=1 in input_parser.cc at the beginning of yyparse (
#define YYDEBUG 1
#ifdef GNUWINCE
#define YYINITDEPTH 1000
#else 
#define YYINITDEPTH 4000
#define YYMAXDEPTH 20000
#define YYERROR_VERBOSE 1
#endif // GNUWINCE
#endif // RTOS_THREADX

#if 0
#define YYSTACK_USE_ALLOCA 1
#endif


gen polynome_or_sparse_poly1(const gen & coeff, const gen & index){
  if (index.type==_VECT){
    index_t i;
    const_iterateur it=index._VECTptr->begin(),itend=index._VECTptr->end();
    i.reserve(itend-it);
    for (;it!=itend;++it){
      if (it->type!=_INT_)
         return gentypeerr();
      i.push_back(it->val);
    }
    monomial<gen> m(coeff,i);
    return polynome(m);
  }
  else {
    sparse_poly1 res;
    res.push_back(monome(coeff,index));
    return res;
  }
}

#line 161 "y.tab.c" /* yacc.c:339  */

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
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "y.tab.h".  */
#ifndef YY_GIAC_YY_Y_TAB_H_INCLUDED
# define YY_GIAC_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int giac_yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    T_NUMBER = 258,
    T_SYMBOL = 259,
    T_LITERAL = 260,
    T_DIGITS = 261,
    T_STRING = 262,
    T_END_INPUT = 263,
    T_EXPRESSION = 264,
    T_UNARY_OP = 265,
    T_OF = 266,
    T_NOT = 267,
    T_TYPE_ID = 268,
    T_VIRGULE = 269,
    T_AFFECT = 270,
    T_MAPSTO = 271,
    T_BEGIN_PAR = 272,
    T_END_PAR = 273,
    T_PLUS = 274,
    T_MOINS = 275,
    T_FOIS = 276,
    T_DIV = 277,
    T_MOD = 278,
    T_POW = 279,
    T_QUOTED_BINARY = 280,
    T_QUOTE = 281,
    T_PRIME = 282,
    T_TEST_EQUAL = 283,
    T_EQUAL = 284,
    T_INTERVAL = 285,
    T_UNION = 286,
    T_INTERSECT = 287,
    T_MINUS = 288,
    T_AND_OP = 289,
    T_COMPOSE = 290,
    T_DOLLAR = 291,
    T_DOLLAR_MAPLE = 292,
    T_INDEX_BEGIN = 293,
    T_VECT_BEGIN = 294,
    T_VECT_DISPATCH = 295,
    T_VECT_END = 296,
    T_SET_BEGIN = 297,
    T_SET_END = 298,
    T_SEMI = 299,
    T_DEUXPOINTS = 300,
    T_DOUBLE_DEUX_POINTS = 301,
    T_IF = 302,
    T_RPN_IF = 303,
    T_ELIF = 304,
    T_THEN = 305,
    T_ELSE = 306,
    T_IFTE = 307,
    T_SWITCH = 308,
    T_CASE = 309,
    T_DEFAULT = 310,
    T_ENDCASE = 311,
    T_FOR = 312,
    T_FROM = 313,
    T_TO = 314,
    T_DO = 315,
    T_BY = 316,
    T_WHILE = 317,
    T_MUPMAP_WHILE = 318,
    T_RPN_WHILE = 319,
    T_REPEAT = 320,
    T_UNTIL = 321,
    T_IN = 322,
    T_START = 323,
    T_BREAK = 324,
    T_CONTINUE = 325,
    T_TRY = 326,
    T_CATCH = 327,
    T_TRY_CATCH = 328,
    T_PROC = 329,
    T_BLOC = 330,
    T_BLOC_BEGIN = 331,
    T_BLOC_END = 332,
    T_RETURN = 333,
    T_LOCAL = 334,
    T_LOCALBLOC = 335,
    T_NAME = 336,
    T_PROGRAM = 337,
    T_NULL = 338,
    T_ARGS = 339,
    T_FACTORIAL = 340,
    T_RPN_OP = 341,
    T_RPN_BEGIN = 342,
    T_RPN_END = 343,
    T_STACK = 344,
    T_GROUPE_BEGIN = 345,
    T_GROUPE_END = 346,
    T_LINE_BEGIN = 347,
    T_LINE_END = 348,
    T_VECTOR_BEGIN = 349,
    T_VECTOR_END = 350,
    T_CURVE_BEGIN = 351,
    T_CURVE_END = 352,
    T_ROOTOF_BEGIN = 353,
    T_ROOTOF_END = 354,
    T_SPOLY1_BEGIN = 355,
    T_SPOLY1_END = 356,
    T_POLY1_BEGIN = 357,
    T_POLY1_END = 358,
    T_MATRICE_BEGIN = 359,
    T_MATRICE_END = 360,
    T_ASSUME_BEGIN = 361,
    T_ASSUME_END = 362,
    T_HELP = 363,
    TI_DEUXPOINTS = 364,
    TI_LOCAL = 365,
    TI_LOOP = 366,
    TI_FOR = 367,
    TI_WHILE = 368,
    TI_STO = 369,
    TI_TRY = 370,
    TI_DIALOG = 371,
    T_PIPE = 372,
    TI_DEFINE = 373,
    TI_PRGM = 374,
    TI_SEMI = 375,
    TI_HASH = 376,
    T_ACCENTGRAVE = 377,
    T_MAPLELIB = 378,
    T_INTERROGATION = 379,
    T_UNIT = 380,
    T_BIDON = 381,
    T_LOGO = 382,
    T_SQ = 383,
    T_CASE38 = 384,
    T_IFERR = 385,
    T_MOINS38 = 386,
    T_NEG38 = 387,
    T_UNARY_OP_38 = 388,
    T_FUNCTION = 389,
    T_IMPMULT = 390
  };
#endif
/* Tokens.  */
#define T_NUMBER 258
#define T_SYMBOL 259
#define T_LITERAL 260
#define T_DIGITS 261
#define T_STRING 262
#define T_END_INPUT 263
#define T_EXPRESSION 264
#define T_UNARY_OP 265
#define T_OF 266
#define T_NOT 267
#define T_TYPE_ID 268
#define T_VIRGULE 269
#define T_AFFECT 270
#define T_MAPSTO 271
#define T_BEGIN_PAR 272
#define T_END_PAR 273
#define T_PLUS 274
#define T_MOINS 275
#define T_FOIS 276
#define T_DIV 277
#define T_MOD 278
#define T_POW 279
#define T_QUOTED_BINARY 280
#define T_QUOTE 281
#define T_PRIME 282
#define T_TEST_EQUAL 283
#define T_EQUAL 284
#define T_INTERVAL 285
#define T_UNION 286
#define T_INTERSECT 287
#define T_MINUS 288
#define T_AND_OP 289
#define T_COMPOSE 290
#define T_DOLLAR 291
#define T_DOLLAR_MAPLE 292
#define T_INDEX_BEGIN 293
#define T_VECT_BEGIN 294
#define T_VECT_DISPATCH 295
#define T_VECT_END 296
#define T_SET_BEGIN 297
#define T_SET_END 298
#define T_SEMI 299
#define T_DEUXPOINTS 300
#define T_DOUBLE_DEUX_POINTS 301
#define T_IF 302
#define T_RPN_IF 303
#define T_ELIF 304
#define T_THEN 305
#define T_ELSE 306
#define T_IFTE 307
#define T_SWITCH 308
#define T_CASE 309
#define T_DEFAULT 310
#define T_ENDCASE 311
#define T_FOR 312
#define T_FROM 313
#define T_TO 314
#define T_DO 315
#define T_BY 316
#define T_WHILE 317
#define T_MUPMAP_WHILE 318
#define T_RPN_WHILE 319
#define T_REPEAT 320
#define T_UNTIL 321
#define T_IN 322
#define T_START 323
#define T_BREAK 324
#define T_CONTINUE 325
#define T_TRY 326
#define T_CATCH 327
#define T_TRY_CATCH 328
#define T_PROC 329
#define T_BLOC 330
#define T_BLOC_BEGIN 331
#define T_BLOC_END 332
#define T_RETURN 333
#define T_LOCAL 334
#define T_LOCALBLOC 335
#define T_NAME 336
#define T_PROGRAM 337
#define T_NULL 338
#define T_ARGS 339
#define T_FACTORIAL 340
#define T_RPN_OP 341
#define T_RPN_BEGIN 342
#define T_RPN_END 343
#define T_STACK 344
#define T_GROUPE_BEGIN 345
#define T_GROUPE_END 346
#define T_LINE_BEGIN 347
#define T_LINE_END 348
#define T_VECTOR_BEGIN 349
#define T_VECTOR_END 350
#define T_CURVE_BEGIN 351
#define T_CURVE_END 352
#define T_ROOTOF_BEGIN 353
#define T_ROOTOF_END 354
#define T_SPOLY1_BEGIN 355
#define T_SPOLY1_END 356
#define T_POLY1_BEGIN 357
#define T_POLY1_END 358
#define T_MATRICE_BEGIN 359
#define T_MATRICE_END 360
#define T_ASSUME_BEGIN 361
#define T_ASSUME_END 362
#define T_HELP 363
#define TI_DEUXPOINTS 364
#define TI_LOCAL 365
#define TI_LOOP 366
#define TI_FOR 367
#define TI_WHILE 368
#define TI_STO 369
#define TI_TRY 370
#define TI_DIALOG 371
#define T_PIPE 372
#define TI_DEFINE 373
#define TI_PRGM 374
#define TI_SEMI 375
#define TI_HASH 376
#define T_ACCENTGRAVE 377
#define T_MAPLELIB 378
#define T_INTERROGATION 379
#define T_UNIT 380
#define T_BIDON 381
#define T_LOGO 382
#define T_SQ 383
#define T_CASE38 384
#define T_IFERR 385
#define T_MOINS38 386
#define T_NEG38 387
#define T_UNARY_OP_38 388
#define T_FUNCTION 389
#define T_IMPMULT 390

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int giac_yyparse (void * scanner);

#endif /* !YY_GIAC_YY_Y_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 481 "y.tab.c" /* yacc.c:358  */

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
#define YYFINAL  157
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   14302

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  136
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  30
/* YYNRULES -- Number of rules.  */
#define YYNRULES  261
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  609

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   390

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
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
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   199,   199,   207,   208,   209,   212,   213,   214,   215,
     216,   217,   218,   220,   221,   224,   225,   226,   227,   231,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   254,   255,
     261,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   275,   276,   277,   282,   287,   293,   294,   300,   301,
     302,   303,   304,   305,   306,   320,   325,   329,   336,   339,
     340,   342,   343,   344,   347,   348,   349,   350,   351,   355,
     362,   363,   365,   367,   368,   370,   371,   372,   397,   402,
     415,   428,   432,   436,   441,   446,   452,   456,   460,   461,
     465,   466,   467,   468,   469,   470,   471,   473,   474,   475,
     476,   477,   478,   481,   482,   487,   491,   495,   496,   522,
     531,   537,   538,   539,   540,   545,   549,   550,   559,   560,
     561,   562,   563,   567,   568,   571,   576,   577,   578,   579,
     584,   589,   594,   599,   604,   609,   614,   615,   616,   617,
     618,   619,   620,   621,   625,   628,   632,   636,   637,   638,
     639,   640,   641,   642,   643,   644,   645,   646,   647,   648,
     649,   650,   651,   652,   656,   660,   664,   667,   668,   669,
     670,   671,   675,   676,   695,   707,   708,   709,   710,   713,
     714,   720,   721,   722,   723,   724,   725,   733,   739,   742,
     743,   746,   747,   748,   752,   753,   756,   759,   762,   763,
     770,   771,   772,   773,   774,   775,   776,   777,   785,   795,
     796,   799,   800,   803,   805,   810,   813,   814,   815,   818,
     888,   889,   892,   893,   894,   895,   898,   899,   902,   903,
     904,   908,   911,   918,   919,   923,   926,   931,   932,   935,
     936,   939,   940,   941,   944,   945,   946,   949,   950,   951,
     952,   955
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "T_NUMBER", "T_SYMBOL", "T_LITERAL",
  "T_DIGITS", "T_STRING", "T_END_INPUT", "T_EXPRESSION", "T_UNARY_OP",
  "T_OF", "T_NOT", "T_TYPE_ID", "T_VIRGULE", "T_AFFECT", "T_MAPSTO",
  "T_BEGIN_PAR", "T_END_PAR", "T_PLUS", "T_MOINS", "T_FOIS", "T_DIV",
  "T_MOD", "T_POW", "T_QUOTED_BINARY", "T_QUOTE", "T_PRIME",
  "T_TEST_EQUAL", "T_EQUAL", "T_INTERVAL", "T_UNION", "T_INTERSECT",
  "T_MINUS", "T_AND_OP", "T_COMPOSE", "T_DOLLAR", "T_DOLLAR_MAPLE",
  "T_INDEX_BEGIN", "T_VECT_BEGIN", "T_VECT_DISPATCH", "T_VECT_END",
  "T_SET_BEGIN", "T_SET_END", "T_SEMI", "T_DEUXPOINTS",
  "T_DOUBLE_DEUX_POINTS", "T_IF", "T_RPN_IF", "T_ELIF", "T_THEN", "T_ELSE",
  "T_IFTE", "T_SWITCH", "T_CASE", "T_DEFAULT", "T_ENDCASE", "T_FOR",
  "T_FROM", "T_TO", "T_DO", "T_BY", "T_WHILE", "T_MUPMAP_WHILE",
  "T_RPN_WHILE", "T_REPEAT", "T_UNTIL", "T_IN", "T_START", "T_BREAK",
  "T_CONTINUE", "T_TRY", "T_CATCH", "T_TRY_CATCH", "T_PROC", "T_BLOC",
  "T_BLOC_BEGIN", "T_BLOC_END", "T_RETURN", "T_LOCAL", "T_LOCALBLOC",
  "T_NAME", "T_PROGRAM", "T_NULL", "T_ARGS", "T_FACTORIAL", "T_RPN_OP",
  "T_RPN_BEGIN", "T_RPN_END", "T_STACK", "T_GROUPE_BEGIN", "T_GROUPE_END",
  "T_LINE_BEGIN", "T_LINE_END", "T_VECTOR_BEGIN", "T_VECTOR_END",
  "T_CURVE_BEGIN", "T_CURVE_END", "T_ROOTOF_BEGIN", "T_ROOTOF_END",
  "T_SPOLY1_BEGIN", "T_SPOLY1_END", "T_POLY1_BEGIN", "T_POLY1_END",
  "T_MATRICE_BEGIN", "T_MATRICE_END", "T_ASSUME_BEGIN", "T_ASSUME_END",
  "T_HELP", "TI_DEUXPOINTS", "TI_LOCAL", "TI_LOOP", "TI_FOR", "TI_WHILE",
  "TI_STO", "TI_TRY", "TI_DIALOG", "T_PIPE", "TI_DEFINE", "TI_PRGM",
  "TI_SEMI", "TI_HASH", "T_ACCENTGRAVE", "T_MAPLELIB", "T_INTERROGATION",
  "T_UNIT", "T_BIDON", "T_LOGO", "T_SQ", "T_CASE38", "T_IFERR",
  "T_MOINS38", "T_NEG38", "T_UNARY_OP_38", "T_FUNCTION", "T_IMPMULT",
  "$accept", "input", "correct_input", "exp", "symbol_for", "symbol",
  "symbol_or_literal", "entete", "stack", "local", "nom", "suite_symbol",
  "affectable_symbol", "exp_or_empty", "suite", "prg_suite", "rpn_suite",
  "rpn_token", "step", "from", "loop38_do", "else", "bloc", "elif",
  "ti_bloc_end", "ti_else", "switch", "case", "case38", "semi", YY_NULLPTR
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
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390
};
# endif

#define YYPACT_NINF -523

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-523)))

#define YYTABLE_NINF -259

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-259)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    8983,  -523,   211,   -26,  -523,   135,  -523,  -523,    16,  -523,
    8983,    39,  8983,  8983,  8983,  -523,  9116,  8983,  6589,  -523,
      62,  8983,  6722,    78,  9249,    90,   119,  9382,    22,  9515,
    8983,  8983,  -523,  -523,    63,   153,    88,   181,  1269,   188,
     193,  -523,    19,  -523,   143,   -16,  8983,  8983,  8983,  8983,
    8983,  8983,  8983,  8983,  6855,    81,  8983,   143,   174,  8983,
    1402,    15,  8983,  8983,   207,   225,  -523,   600,   210,  -523,
    -523,  -523,   215,   229,   -17,    24,  8983,  6988,  7121,  8983,
   14174,  -523, 10640, 14174, 14174,    86,  1934, 10707, 13798,  8983,
   14053,  -523,   718, 10766,   195,  -523,  8983, 10345,  8983,  8983,
    9648, 10303,   192,    26,  -523,    39,  7254,  -523,   167,    46,
    8983, 10825, 10884, 13208,  2998,  3131,   170,  8983,  7387,   222,
    8983,  1535, 13208,  8983,  8983,  8983,  8983,  -523,   156,   134,
    8983,  -523, 10943, 13267, 13208, 13208,   234,  3264, 13208,   144,
   11002,  3397,  3264,  -523,   237,   102,   133,  8983, 14147,  7520,
   13798,  8983,  8983,   179,  3530, 14174,  8983,  -523,  -523,   196,
    8983,  8983,  6855,  7387,  8983,  8983,  8983,  8983,  8983,  8983,
    -523,  8983,  8983,   870,  8983,  8983,  8983,  8983,  8983,  8983,
    8983,  9781,  7653,  8983,  8983,  -523,   266,  8983,  8983,  8983,
    8983,  -523,  8983,  7387,  8983,  -523,   105,  -523,  -523,  -523,
    -523,  8983,  -523, 13444,  -523, 11061,  -523, 11120, 11179,   243,
    -523,  1003,  -523, 13680,    70,  -523, 11238,  6855,  8983, 11297,
   11356,    35,   261,  8983,   209,   263, 11415,   227,  8983,  8983,
    8983,  8983,   136, 11474,  8983,  8983,  -523,  8983, 13208,  -523,
    8983,  7786,   197,  3663,   265, 11533,   268,  7387, 11592, 11651,
   11710, 11769, 11828,  -523,   143,  -523, 11887,  -523,  8983,  7387,
    -523,  7919,  -523,  8983,  8983,  8052,  8185,  -523,  7387,  -523,
   11946,  -523, 12005,  3796,  -523,  8983, 12064,  8983, 13680, 13444,
   13857,  -523,   271,   651,   651, 14078, 14095, 14122,   292, 14001,
     718, 14053, 10386, 13956, 14023, 13884,   102,   267,   718,     2,
    6722, 12123,  -523,  -523, 13798, 13926,  -523,  -523,  -523,  -523,
    -523,  -523,  -523,  8983,   168, 13326, 13562, 13739, 14147,   651,
     272, 12182,  -523,   280, 12249,  -523,  -523,  -523,  7387,   192,
     100,   246,    39,    70,  -523,    28,  -523,  1668,  2067,   242,
   13208,  -523,   219,  -523,   236,  3929,  -523,  -523,  -523,  7254,
   12308, 13208, 13208, 13208,  8983,  8983,   160,  1801,  4062,  4195,
   12367, 12426,    70,  -523,  4328,   218,  -523,  8983,  -523,   197,
     281,  -523,  -523,  -523,  -523,  -523,  -523,  -523, 13621,   283,
    -523,  3264,  3264,  3264,  -523,  8052,   286,  -523,  8983,  2200,
    -523, 13926,  -523,  1136,  8983, 10463,  -523, 14147,  7387,  9914,
      -9,   274,   290,  -523,   293,  8983,  8983,  8983,  8983,   296,
      70,  8983,  7387, 12485,    -7,  8983,  -523,  2333,  -523,  -523,
    8983,    63,   104,  8983, 13208,   273,  8983, 12559, 13208,  8983,
    8983,  8983, 12618,  -523,  -523,  -523,  -523,  -523,    27,  -523,
   12677,  4461,  2466,  -523,   -10,  -523,  -523,  -523,  3264,  -523,
     289,  4594,  8983,  -523, 13926,   269,   301,  6722, 12736,  8318,
     197,   319,  -523,  -523, 13208, 13503, 13385, 13503,  -523,  -523,
   10522, 10640,    -7,   282,  -523,  6855, 12795,  8983,  -523,  3264,
    -523,   322,   291,   254,  2599,  8451,  2732,    53, 12854,  4727,
   12913,  -523,  -523,    63,  8983,  4860,   197,  7786,  4993, 10047,
    -523,  8584,   152,  5126,  -523,  -523, 10581,  -523,   200, 13444,
    -523,  7786,  -523,  -523,  8983,  -523, 12972,  -523,  8983, 13031,
    -523,   294,    63,  -523,   261,  -523,   316,  8983,  -523,  -523,
    -523,  8983,  8983,  -523,  8983,  -523,  5259,  -523,  7786,  5392,
    -523,  8717,  2865, 10180,   718,    15,  -523,  -523,   299,  7786,
    5525, 13090,  -523,  2067,  8983,    63,  -523,  6855,  5658,  5791,
    5924,  6057,  -523,  6190,  -523,  8983,  6323,  8983,  -523,  8850,
    3264,  -523,  -523,  6456,  -523,  -523,  -523,  2067,   104, 13149,
    -523,  -523,  -523,  -523,  -523,  -523,   226,  8983,   233,  8983,
    -523,  -523,  -523,  -523,  -523,  8983,   235,  8983,   238,  3264,
    8983,  3264,  8983,  -523,  3264,  -523,  3264,  -523,  -523
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,   127,     6,   189,    31,    32,    13,    14,    68,    58,
       0,    99,     0,     0,     0,   113,     0,     0,     0,   107,
       0,     0,     0,     0,     0,    75,     0,     0,    93,     0,
       0,     0,    85,    86,     0,   159,     0,    81,     0,   133,
      77,   121,    63,   164,   226,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   137,     0,
       0,   257,     0,     0,     0,     0,     2,     0,    30,   128,
     199,   200,     0,     0,     7,     0,     0,     0,     0,     0,
      60,   197,     0,    55,    53,    99,     0,     0,    39,     0,
      49,   105,   101,   222,     0,   195,     0,     0,     0,     0,
       0,   254,     0,   189,   187,     0,     0,   188,     0,   232,
       0,     0,     0,   223,     0,     0,     0,     0,     0,     0,
       0,     0,    82,     0,     0,     0,     0,   229,     0,   226,
       0,   205,     0,     0,   122,   179,    30,     0,   222,     0,
       0,     0,     0,   178,     0,   198,     0,     0,   124,     0,
     129,     0,     0,     0,     0,    54,     0,     1,     3,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      69,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    70,     0,     0,     0,     0,
       0,   126,     0,     0,     0,   196,     0,    10,   192,   191,
     190,     0,   193,    33,    35,     0,    67,     0,     0,   118,
     100,     0,   114,    50,     0,   119,     0,     0,     0,     0,
       0,   189,     0,     0,     0,     0,   220,     0,     0,     0,
       0,     0,   230,     0,     0,     0,   261,     0,   224,   225,
       0,     0,   201,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   136,   226,   227,     0,    57,     0,     0,
     247,     0,   166,     0,     0,     0,     0,   177,     0,   163,
       0,   131,     0,     0,    98,     0,     0,     0,   120,    59,
      79,    78,     0,    40,    41,    43,    44,    46,    45,    37,
      38,    47,   108,   110,   111,    51,   106,   104,   103,    30,
       0,     0,     4,     5,    52,   149,    36,    21,    25,    22,
      23,    24,    26,     0,    20,   112,   172,   123,   125,    42,
       0,     0,     8,     0,     0,    34,    64,    66,     0,   218,
     189,   215,   217,     0,   210,     0,   208,     0,     0,    72,
     167,    74,     0,   161,     0,     0,   162,   186,   148,     0,
       0,   233,   234,   235,     0,     0,     0,     0,     0,     0,
      94,     0,     0,   202,     0,   203,   241,     0,   158,   201,
       0,    80,   132,    76,    61,    62,   228,   204,   120,     0,
     248,     0,     0,     0,   169,     0,     0,   138,     0,     0,
      65,   150,    29,     0,     0,     0,   115,    27,     0,     0,
      28,    11,     0,   194,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   238,     0,   249,     0,    73,   243,
       0,     0,   251,     0,   220,     0,     0,   230,   231,     0,
       0,     0,     0,   153,   155,   156,    95,   207,     0,   242,
       0,     0,     0,    56,    28,   183,   184,   168,     0,   171,
       0,     0,     0,    97,   102,     0,     0,     0,     0,     0,
     201,     0,     9,   117,   211,   212,   213,   216,   214,   209,
       0,     0,   238,     0,   134,     0,     0,     0,   250,     0,
      71,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   154,   206,     0,     0,     0,   201,     0,     0,     0,
     170,     0,   257,     0,   116,    17,     0,    18,   201,    16,
      15,     0,    12,   151,     0,   135,     0,   240,     0,     0,
     244,     0,     0,   160,    58,   256,     0,     0,    87,   236,
     237,     0,     0,    91,     0,   157,     0,   139,     0,     0,
     141,     0,     0,     0,   180,   257,   259,    96,     0,     0,
       0,     0,   239,     0,     0,     0,   252,     0,     0,     0,
       0,     0,   143,     0,   140,     0,     0,     0,   176,     0,
       0,   260,    19,     0,   144,   152,   245,     0,   251,     0,
     146,    88,    89,    90,    92,   142,     0,     0,     0,     0,
     182,   145,   246,   253,   147,     0,     0,     0,     0,     0,
       0,     0,     0,   175,     0,   174,     0,   173,   181
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -523,  -523,   161,     0,  -523,   186,  -523,  -234,  -523,  -523,
    -523,   -14,  -322,  -347,    82,   270,  -126,   303,   -76,  -523,
    -523,  -119,   -31,  -522,  -133,  -369,  -222,  -123,  -487,  -523
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    65,    66,   113,   108,    68,    74,   241,    69,   363,
     242,   335,   336,   227,    94,   114,   128,   129,   356,   232,
     531,   474,   116,   418,   419,   420,   483,   224,   153,   239
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      67,   130,   425,   255,   262,   459,   459,   196,   365,   267,
      80,   409,    82,    83,    84,   546,    87,    88,    90,   193,
      75,    92,    93,   143,    97,   102,   103,   101,   198,   111,
     112,   576,   104,    78,   199,   105,   125,   200,   122,   106,
     225,   410,   410,    81,   416,   475,   132,   133,   134,   135,
     201,   138,   140,   343,    79,   592,   145,   126,   571,   148,
     150,   229,   151,   155,   460,   460,    91,   131,    23,   394,
     152,   492,    75,   329,   330,   230,   203,   205,   207,   208,
     331,    75,    95,   332,   102,     3,   122,   333,   469,   213,
      81,   102,     3,  -185,   105,   411,   216,   529,   219,   220,
      82,   105,   473,   475,   231,   118,   226,    98,   322,   499,
     233,   197,   210,   530,   238,   405,    23,   245,   138,   163,
     248,   135,   323,   249,   250,   251,   252,    23,   376,   406,
     256,   281,   384,   139,    23,   441,    99,   238,   526,   115,
     181,   238,   238,    56,   127,   407,    75,   270,   254,    82,
      76,   272,    77,   127,   238,   107,   276,   202,   481,   482,
     278,   279,   280,   138,   283,   284,   285,   286,   287,   288,
     117,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   301,    67,   304,   305,   398,   339,   315,   316,   317,
     318,    56,   319,   138,   321,   354,   545,   355,   120,   151,
     246,   324,    56,   102,     3,   123,   399,   152,   497,    56,
     124,    87,   147,   105,   109,    70,    71,   118,   340,   429,
     430,    72,   119,   431,   156,   157,   511,   193,   350,   351,
     352,   353,   194,   195,   228,   136,   215,   360,    73,   247,
     361,   144,   244,   238,   253,   282,    23,   138,   445,   446,
     447,   259,   449,   263,   268,   269,   274,    73,   378,   138,
     328,   135,   538,   277,   344,   346,   135,   347,   138,   102,
       3,   349,   306,   238,   549,   320,   307,   391,   240,   105,
     308,   240,   367,   402,   163,   309,   369,   310,   311,   392,
     400,   169,   408,   421,   170,   422,   423,   362,   461,   442,
     395,   444,   178,  -259,   450,   181,   414,   136,   462,   163,
     504,   463,    23,   397,   468,   500,   169,   485,   501,   505,
     137,    56,   512,   141,   142,   521,   433,   178,   138,   370,
     181,   523,   154,   478,   557,   595,   522,   413,   238,   555,
     572,   379,   597,   303,   600,   238,   520,   602,   438,   424,
     386,   487,   185,   515,   427,   428,   593,   432,   238,   238,
     146,   525,     0,     0,   238,     0,   299,   440,     0,     0,
       0,     0,   314,     0,     0,     0,     0,   185,     0,     0,
     312,   238,   238,   238,     0,   243,     0,    56,     0,   238,
     480,   313,     0,   287,   454,   191,     0,     0,   138,   458,
     334,     0,     0,     0,     0,   464,   465,   466,   467,   568,
     404,   470,   471,     0,     0,   476,     0,   135,     0,     0,
     191,     0,   273,     0,     0,     0,     0,     0,   510,   488,
       0,   490,     0,     0,     0,     0,     0,   590,     0,     0,
       0,     0,     0,     0,   517,     0,     0,   136,   238,     0,
       0,   238,   136,     0,     0,     0,     0,   506,     0,   509,
       0,     0,   535,     0,     0,     0,   603,     0,   605,     0,
       0,   607,     0,   608,     0,   516,     0,   519,     0,   238,
     456,     0,     0,     0,   238,   424,   238,   338,     0,   238,
       0,   556,     0,   345,   404,   238,     0,     0,   238,     0,
       0,   544,     0,   238,   358,   359,     0,     0,     0,     0,
       0,   364,     0,     0,   551,     0,     0,     0,     0,   334,
       0,     0,     0,     0,   578,     0,   580,     0,     0,     0,
       0,     0,     0,   381,   382,   383,   238,     0,     0,   238,
       0,   135,   238,     0,     0,   389,     0,     0,   334,     0,
     238,     0,     0,   238,     0,     0,     0,   579,   238,   238,
     238,   238,     0,   238,     0,   138,   135,   138,     0,   135,
     238,     0,     0,   238,     0,     0,     0,   238,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   138,     0,   138,
       0,     0,     0,     0,     0,     0,   334,     0,     0,   238,
       0,   238,     0,   136,   238,     0,   238,     0,   158,     0,
       0,     0,   159,     0,   160,   161,   162,   163,     0,   164,
     165,   166,   167,   168,   169,     0,     0,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,     0,
       0,     0,     0,     0,   182,   183,     0,   586,     0,   588,
       0,     0,     0,     0,     0,   448,     0,     0,   451,     0,
       0,     0,     0,   159,     0,     0,     0,   184,   163,   596,
       0,   598,   166,   167,   168,   169,     0,     0,   170,     0,
       0,     0,     0,     0,     0,   185,   178,   179,     0,   181,
     479,     0,     0,   484,   119,     0,   486,     0,     0,     0,
     489,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   495,   498,     0,   186,     0,     0,   187,     0,     0,
     188,     0,   503,     0,   189,   190,     0,   136,   191,     0,
     159,   192,     0,     0,     0,   163,   185,   164,   165,   166,
     167,   168,   169,     0,     0,   170,   171,   172,   173,   174,
     175,   176,   136,   178,   179,   136,   181,     0,     0,     0,
       0,     0,     0,     0,   536,     0,     0,   539,     0,   542,
       0,     0,     0,     0,     0,     0,   190,     0,     0,   191,
       0,   550,     0,     0,     0,     0,     0,     0,   553,     0,
       0,     0,     0,     0,     0,     0,     0,   558,     0,     0,
       0,   559,   560,   185,   561,     0,     0,     0,   563,     0,
       0,     0,     0,   570,     0,     0,     0,     0,     0,   573,
       0,     0,     0,     0,   577,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   190,     0,     0,   191,     0,     0,   192,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   599,     0,   601,     0,     0,
     604,     1,   606,     2,     3,     4,     5,     6,   -48,     7,
       8,     9,    10,    11,   -48,   -48,   -48,    12,   -48,    13,
      14,   -48,   -48,   -48,   -48,    15,    16,   -48,   -48,   -48,
     -48,   -48,   -48,   -48,   -48,    19,    20,   -48,   -48,     0,
      22,   -48,     0,     0,   -48,   -48,    23,    24,     0,   -48,
     -48,   -48,    25,    26,    27,   -48,   -48,   -48,   -48,   -48,
     -48,   -48,    29,    30,     0,    31,   -48,   -48,     0,    32,
      33,    34,     0,    35,    36,    37,     0,   -48,   -48,     0,
      39,     0,    40,    41,    42,   -48,    43,    44,     0,    45,
       0,     0,     0,     0,     0,     0,     0,     0,    46,   -48,
      47,   -48,     0,     0,     0,     0,     0,     0,    48,   -48,
     -48,    50,    51,    52,   -48,    53,    54,   -48,    55,     0,
     -48,    56,    57,    58,   -48,    59,     0,   -48,   -48,    61,
      62,   -48,    63,    64,     1,     0,     2,     3,     4,     5,
       6,   -84,     7,     8,     9,    10,    85,   -84,   -84,   -84,
      12,   -84,    13,    14,   -84,   -84,   -84,   -84,    15,    16,
     -84,   -84,    17,    18,   -84,   -84,   -84,   -84,    19,    20,
      21,   -84,     0,    22,   -84,     0,     0,   -84,   -84,    23,
      24,     0,   -84,   -84,   -84,    25,    26,    27,   -84,   -84,
      28,   -84,   -84,   -84,   -84,    29,    30,     0,    31,   -84,
     -84,     0,    32,    33,    34,     0,    35,    36,    37,     0,
     -84,    86,     0,    39,     0,    40,    41,    42,   -84,    43,
      44,     0,    45,     0,     0,     0,     0,     0,     0,     0,
       0,    46,   -84,    47,   -84,     0,     0,     0,     0,     0,
       0,    48,    49,   -84,    50,    51,    52,   -84,    53,    54,
     -84,    55,     0,   -84,    56,    57,    58,   -84,    59,     0,
      60,   -84,    61,    62,   -84,    63,    64,     1,     0,  -109,
       3,     4,     5,     6,  -109,     7,     8,     9,    10,    11,
    -109,  -109,  -109,    12,  -109,  -109,  -109,  -109,  -109,  -109,
    -109,    15,    16,  -109,  -109,  -109,  -109,  -109,  -109,  -109,
    -109,    19,    20,  -109,  -109,     0,    22,  -109,     0,     0,
    -109,  -109,    23,    24,     0,  -109,  -109,  -109,    25,    26,
      27,  -109,  -109,  -109,  -109,  -109,  -109,  -109,    29,    30,
       0,    31,  -109,  -109,     0,    32,    33,    34,     0,    35,
      36,    37,     0,  -109,  -109,     0,    39,     0,    40,    41,
      42,  -109,    43,    44,     0,    45,     0,     0,     0,     0,
       0,     0,     0,     0,    46,  -109,    47,  -109,     0,     0,
       0,     0,     0,     0,    48,  -109,  -109,    50,    51,    52,
    -109,    53,    54,  -109,    55,     0,  -109,    56,    57,    58,
    -109,    59,     0,  -109,  -109,    61,    62,  -109,    63,    64,
       1,     0,     2,     3,     4,     5,     6,   -83,     7,     8,
       9,    10,    11,   -83,   -83,   -83,    12,   -83,    13,    14,
     -83,   -83,   -83,   -83,    15,    16,   -83,   -83,    17,    18,
     -83,   -83,   -83,   -83,    19,    20,    21,   -83,     0,    22,
     -83,     0,     0,   -83,   -83,    23,    24,     0,   -83,   -83,
     -83,    25,    26,    27,   -83,   -83,    28,   -83,   -83,   -83,
     -83,    29,    30,     0,    31,   -83,   -83,     0,    32,    33,
      34,     0,    35,    36,    37,     0,   -83,     0,     0,    39,
       0,    40,    41,    42,   -83,    43,    44,     0,    45,     0,
       0,     0,     0,     0,     0,     0,     0,    46,   -83,    47,
     -83,     0,     0,     0,     0,     0,     0,    48,   -83,   -83,
      50,    51,    52,   -83,    53,    54,   -83,    55,     0,   -83,
      56,    57,    58,   -83,    59,     0,    60,   -83,    61,    62,
     -83,    63,    64,     1,     0,     2,     3,     4,     5,     6,
    -130,     7,     8,     9,    10,    11,  -130,  -130,  -130,   149,
    -130,    13,    14,  -130,  -130,  -130,  -130,    15,    16,  -130,
    -130,    17,    18,  -130,  -130,  -130,  -130,    19,    20,    21,
    -130,     0,    22,  -130,     0,     0,  -130,  -130,    23,    24,
       0,  -130,  -130,  -130,    25,    26,    27,  -130,  -130,  -130,
    -130,  -130,  -130,  -130,    29,    30,     0,    31,  -130,  -130,
       0,    32,    33,    34,     0,    35,    36,    37,     0,  -130,
    -130,     0,    39,     0,    40,    41,    42,  -130,    43,    44,
       0,    45,     0,     0,     0,     0,     0,     0,     0,     0,
      46,  -130,    47,  -130,     0,     0,     0,     0,     0,     0,
      48,  -130,  -130,    50,    51,    52,  -130,    53,    54,  -130,
      55,     0,  -130,    56,    57,    58,  -130,    59,     0,     0,
    -130,    61,    62,  -130,    63,    64,     1,     0,     2,     3,
       4,     5,     6,  -165,     7,     8,     9,    10,    11,  -165,
    -165,  -165,    12,  -165,    13,    14,  -165,  -165,  -165,  -165,
      15,    16,  -165,  -165,    17,    18,  -165,  -165,  -165,  -165,
      19,    20,    21,  -165,     0,    22,  -165,     0,     0,  -165,
    -165,    23,    24,     0,  -165,  -165,  -165,    25,    26,    27,
    -165,  -165,    28,  -165,  -165,  -165,  -165,    29,    30,     0,
      31,  -165,  -165,     0,    32,    33,    34,     0,    35,    36,
      37,     0,  -165,    38,     0,    39,     0,    40,    41,    42,
    -165,    43,    44,     0,    45,     0,     0,     0,     0,     0,
       0,     0,     0,    46,  -165,    47,  -165,     0,     0,     0,
       0,     0,     0,    48,     0,  -165,    50,    51,    52,  -165,
      53,    54,  -165,    55,     0,  -165,    56,    57,    58,  -165,
      59,     0,    60,  -165,    61,    62,  -165,    63,    64,     1,
       0,     2,     3,     4,     5,     6,     0,     7,     8,     9,
      10,    11,  -118,  -118,  -118,   412,     0,    13,    14,  -118,
    -118,  -118,  -118,    15,    16,  -118,  -118,    17,    18,  -118,
    -118,  -118,  -118,    19,    20,    21,  -118,     0,    22,     0,
       0,     0,     0,  -118,    23,    24,     0,     0,  -118,     0,
      25,    26,    27,     0,     0,    28,     0,     0,     0,     0,
      29,    30,     0,    31,     0,  -118,     0,    32,    33,    34,
       0,    35,    36,    37,   115,     0,    38,     0,    39,     0,
      40,    41,    42,  -118,    43,    44,     0,    45,     0,     0,
       0,     0,     0,     0,     0,     0,    46,     0,    47,     0,
       0,     0,     0,     0,     0,     0,    48,    49,     0,    50,
      51,    52,  -118,    53,    54,  -118,    55,     0,  -118,    56,
      57,    58,  -118,    59,     0,    60,  -118,    61,    62,  -118,
      63,    64,     1,     0,     2,     3,     4,     5,     6,     0,
       7,     8,     9,    10,    11,  -118,  -118,  -118,   412,     0,
      13,    14,  -118,  -118,  -118,  -118,    15,    16,  -118,  -118,
      17,    18,  -118,  -118,  -118,  -118,    19,    20,    21,  -118,
       0,    22,     0,     0,     0,     0,  -118,    23,    24,     0,
       0,     0,     0,    25,    26,    27,     0,     0,    28,     0,
       0,  -118,     0,    29,    30,     0,    31,     0,  -118,     0,
      32,    33,    34,     0,    35,    36,    37,   115,     0,    38,
       0,    39,     0,    40,    41,    42,  -118,    43,    44,     0,
      45,     0,     0,     0,     0,     0,     0,     0,     0,    46,
       0,    47,     0,     0,     0,     0,     0,     0,     0,    48,
      49,     0,    50,    51,    52,  -118,    53,    54,  -118,    55,
       0,  -118,    56,    57,    58,  -118,    59,     0,    60,  -118,
      61,    62,  -118,    63,    64,     1,     0,     2,     3,     4,
       5,     6,     0,     7,     8,     9,    10,    11,   -83,   -83,
     -83,    12,     0,    13,    14,   -83,   -83,   -83,   -83,    15,
     211,   -83,   -83,    17,    18,   -83,   -83,   -83,   -83,    19,
      20,    21,   -83,     0,    22,     0,     0,     0,     0,   -83,
      23,    24,     0,     0,     0,     0,    25,    26,    27,     0,
       0,    28,     0,     0,     0,     0,    29,    30,     0,    31,
       0,   -83,     0,    32,    33,    34,     0,    35,    36,    37,
       0,     0,    38,     0,    39,     0,    40,    41,    42,   -83,
      43,    44,     0,    45,     0,     0,     0,     0,     0,     0,
       0,     0,    46,     0,    47,     0,     0,     0,     0,     0,
       0,     0,    48,   121,     0,    50,    51,    52,   -83,    53,
      54,   -83,    55,     0,   -83,    56,    57,    58,   -83,    59,
       0,    60,   -83,    61,    62,   -83,    63,    64,     1,     0,
       2,     3,     4,     5,     6,     0,     7,     8,     9,    10,
      11,     0,     0,     0,    12,     0,    13,    14,     0,     0,
       0,     0,    15,    16,     0,     0,    17,    18,     0,     0,
       0,     0,    19,    20,    21,     0,     0,    22,     0,     0,
       0,   236,     0,    23,    24,     0,   415,     0,   416,    25,
      26,    27,     0,     0,    28,     0,     0,     0,     0,    29,
      30,     0,    31,     0,     0,     0,    32,    33,    34,     0,
      35,    36,    37,     0,   260,    38,     0,    39,     0,    40,
      41,    42,     0,    43,    44,     0,    45,     0,     0,     0,
       0,     0,     0,     0,     0,    46,     0,    47,     0,     0,
       0,     0,     0,     0,     0,    48,   417,     0,    50,    51,
      52,     0,    53,    54,     0,    55,     0,     0,    56,    57,
      58,     0,    59,     0,    60,     0,    61,    62,     0,    63,
      64,     1,     0,     2,     3,     4,     5,     6,     0,     7,
       8,     9,    10,    11,     0,     0,     0,    12,     0,    13,
      14,     0,     0,     0,     0,    15,    16,     0,     0,    17,
      18,     0,     0,     0,     0,    19,    20,    21,     0,     0,
      22,     0,     0,     0,   236,     0,    23,    24,     0,     0,
       0,   452,    25,    26,    27,     0,     0,    28,     0,     0,
       0,     0,    29,    30,     0,    31,     0,     0,     0,    32,
      33,    34,     0,    35,    36,    37,     0,   453,    38,     0,
      39,     0,    40,    41,    42,     0,    43,    44,     0,    45,
       0,     0,     0,     0,     0,     0,     0,     0,    46,     0,
      47,     0,     0,     0,     0,     0,     0,     0,    48,    49,
       0,    50,    51,    52,     0,    53,    54,     0,    55,     0,
       0,    56,    57,    58,     0,    59,     0,    60,     0,    61,
      62,     0,    63,    64,     1,     0,     2,     3,     4,     5,
       6,     0,     7,     8,     9,    10,    11,     0,     0,     0,
      12,     0,    13,    14,     0,     0,     0,     0,    15,    16,
       0,     0,    17,    18,     0,     0,     0,     0,    19,    20,
      21,     0,     0,    22,     0,     0,     0,     0,     0,    23,
      24,     0,   477,     0,   478,    25,    26,    27,     0,     0,
      28,     0,     0,     0,     0,    29,    30,     0,    31,     0,
       0,     0,    32,    33,    34,     0,    35,    36,    37,     0,
     380,    38,     0,    39,     0,    40,    41,    42,     0,    43,
      44,     0,    45,     0,     0,     0,     0,     0,     0,     0,
       0,    46,     0,    47,     0,     0,     0,     0,     0,     0,
       0,    48,    49,     0,    50,    51,    52,     0,    53,    54,
       0,    55,     0,     0,    56,    57,    58,     0,    59,     0,
      60,     0,    61,    62,     0,    63,    64,     1,     0,     2,
       3,     4,     5,     6,     0,     7,     8,     9,    10,    11,
       0,     0,     0,    12,     0,    13,    14,     0,     0,     0,
       0,    15,    16,     0,     0,    17,    18,     0,     0,     0,
       0,    19,    20,    21,     0,     0,    22,     0,     0,     0,
       0,     0,    23,    24,     0,     0,     0,     0,    25,    26,
      27,     0,     0,    28,     0,     0,     0,     0,    29,    30,
       0,    31,     0,     0,     0,    32,    33,    34,     0,    35,
      36,    37,   496,     0,    38,  -201,    39,   240,    40,    41,
      42,     0,    43,    44,     0,    45,     0,     0,     0,     0,
       0,     0,     0,     0,    46,     0,    47,     0,     0,     0,
       0,     0,     0,     0,    48,    49,     0,    50,    51,    52,
       0,    53,    54,     0,    55,     0,     0,    56,    57,    58,
       0,    59,     0,    60,     0,    61,    62,     0,    63,    64,
       1,     0,     2,     3,     4,     5,     6,     0,     7,     8,
     524,    10,    11,     0,     0,     0,    12,     0,    13,    14,
       0,     0,     0,     0,    15,    16,     0,     0,    17,    18,
       0,     0,     0,     0,    19,    20,    21,     0,     0,    22,
       0,     0,     0,   236,     0,    23,    24,     0,     0,     0,
       0,    25,    26,    27,   223,  -254,    28,     0,     0,     0,
       0,    29,    30,     0,    31,     0,     0,     0,    32,    33,
      34,     0,    35,    36,    37,     0,     0,    38,     0,    39,
       0,    40,    41,    42,     0,    43,    44,     0,    45,     0,
       0,     0,     0,     0,     0,     0,     0,    46,     0,    47,
       0,     0,     0,     0,     0,     0,     0,    48,    49,     0,
      50,    51,    52,     0,    53,    54,     0,    55,     0,     0,
      56,    57,    58,     0,    59,     0,    60,     0,    61,    62,
       0,    63,    64,     1,     0,     2,     3,     4,     5,     6,
       0,     7,     8,     9,    10,    11,     0,     0,     0,    12,
       0,    13,    14,     0,     0,     0,     0,    15,    16,     0,
       0,    17,    18,     0,     0,     0,     0,    19,    20,    21,
       0,     0,    22,     0,     0,     0,   236,     0,    23,    24,
       0,     0,     0,   527,    25,    26,    27,     0,     0,    28,
       0,     0,     0,     0,    29,    30,     0,    31,     0,     0,
       0,    32,    33,    34,     0,    35,    36,    37,     0,   528,
      38,     0,    39,     0,    40,    41,    42,     0,    43,    44,
       0,    45,     0,     0,     0,     0,     0,     0,     0,     0,
      46,     0,    47,     0,     0,     0,     0,     0,     0,     0,
      48,    49,     0,    50,    51,    52,     0,    53,    54,     0,
      55,     0,     0,    56,    57,    58,     0,    59,     0,    60,
       0,    61,    62,     0,    63,    64,     1,     0,     2,     3,
       4,     5,     6,     0,     7,     8,     9,    10,    11,     0,
       0,     0,    12,     0,    13,    14,     0,     0,     0,     0,
      15,    16,     0,     0,    17,    18,     0,     0,     0,     0,
      19,    20,    21,     0,     0,    22,     0,     0,     0,   236,
       0,    23,    24,     0,     0,     0,     0,    25,    26,    27,
       0,     0,    28,     0,     0,     0,     0,    29,    30,     0,
      31,     0,     0,     0,    32,    33,    34,     0,    35,    36,
      37,     0,   260,    38,     0,    39,     0,    40,    41,    42,
       0,    43,    44,     0,    45,     0,     0,     0,     0,     0,
       0,     0,     0,    46,     0,    47,     0,     0,     0,     0,
       0,     0,     0,    48,   566,   567,    50,    51,    52,     0,
      53,    54,     0,    55,     0,     0,    56,    57,    58,     0,
      59,     0,    60,     0,    61,    62,     0,    63,    64,     1,
       0,     2,     3,     4,     5,     6,     0,     7,     8,     9,
      10,    11,     0,     0,     0,    12,     0,    13,    14,     0,
       0,     0,     0,    15,    16,     0,     0,    17,    18,     0,
       0,     0,     0,    19,    20,    21,     0,     0,    22,     0,
       0,     0,   236,     0,    23,    24,     0,     0,     0,     0,
      25,    26,    27,     0,     0,    28,     0,     0,     0,     0,
      29,    30,     0,    31,   237,     0,     0,    32,    33,    34,
       0,    35,    36,    37,     0,     0,    38,     0,    39,     0,
      40,    41,    42,     0,    43,    44,     0,    45,     0,     0,
       0,     0,     0,     0,     0,     0,    46,     0,    47,     0,
       0,     0,     0,     0,     0,     0,    48,    49,     0,    50,
      51,    52,     0,    53,    54,     0,    55,     0,     0,    56,
      57,    58,     0,    59,     0,    60,     0,    61,    62,     0,
      63,    64,     1,     0,     2,     3,     4,     5,     6,     0,
       7,     8,     9,    10,    11,     0,     0,     0,    12,     0,
      13,    14,     0,     0,     0,     0,    15,    16,     0,     0,
      17,    18,     0,     0,     0,     0,    19,    20,    21,     0,
       0,    22,     0,     0,     0,     0,     0,    23,    24,     0,
       0,     0,     0,    25,    26,    27,     0,     0,    28,     0,
       0,     0,     0,    29,    30,     0,    31,     0,     0,     0,
      32,    33,    34,     0,    35,    36,    37,     0,     0,    38,
    -201,    39,   240,    40,    41,    42,     0,    43,    44,     0,
      45,     0,     0,     0,     0,     0,     0,     0,     0,    46,
       0,    47,     0,     0,     0,     0,     0,     0,     0,    48,
      49,     0,    50,    51,    52,     0,    53,    54,     0,    55,
       0,     0,    56,    57,    58,     0,    59,     0,    60,     0,
      61,    62,     0,    63,    64,     1,     0,     2,     3,     4,
       5,     6,     0,     7,     8,     9,    10,    11,     0,     0,
       0,    12,     0,    13,    14,     0,     0,     0,     0,    15,
      16,     0,     0,    17,    18,     0,     0,     0,     0,    19,
      20,    21,     0,     0,    22,     0,     0,     0,   236,     0,
      23,    24,     0,     0,     0,     0,    25,    26,    27,     0,
       0,    28,     0,     0,     0,     0,    29,    30,     0,    31,
       0,     0,     0,    32,    33,    34,     0,    35,    36,    37,
       0,   260,    38,     0,    39,     0,    40,    41,    42,     0,
      43,    44,     0,    45,     0,     0,     0,     0,     0,     0,
       0,     0,    46,     0,    47,     0,     0,     0,     0,     0,
       0,     0,    48,   261,     0,    50,    51,    52,     0,    53,
      54,     0,    55,     0,     0,    56,    57,    58,     0,    59,
       0,    60,     0,    61,    62,     0,    63,    64,     1,     0,
       2,     3,     4,     5,     6,     0,     7,     8,     9,    10,
      11,     0,     0,     0,    12,     0,    13,    14,     0,     0,
       0,     0,    15,    16,     0,     0,    17,    18,     0,     0,
       0,     0,    19,    20,    21,     0,     0,    22,     0,     0,
       0,   236,     0,    23,    24,     0,     0,     0,   265,    25,
      26,    27,     0,     0,    28,     0,     0,     0,     0,    29,
      30,     0,    31,     0,     0,     0,    32,    33,    34,     0,
      35,    36,    37,     0,     0,    38,     0,    39,     0,    40,
      41,    42,     0,    43,    44,     0,    45,     0,     0,     0,
       0,     0,     0,     0,     0,    46,     0,    47,     0,     0,
       0,     0,     0,     0,     0,    48,   266,     0,    50,    51,
      52,     0,    53,    54,     0,    55,     0,     0,    56,    57,
      58,     0,    59,     0,    60,     0,    61,    62,     0,    63,
      64,     1,     0,     2,     3,     4,     5,     6,     0,     7,
       8,     9,    10,    11,     0,     0,     0,    12,     0,    13,
      14,     0,     0,     0,     0,    15,    16,     0,     0,    17,
      18,     0,     0,     0,     0,    19,    20,    21,     0,     0,
      22,     0,     0,     0,   236,     0,    23,    24,     0,     0,
     275,     0,    25,    26,    27,     0,     0,    28,     0,     0,
       0,     0,    29,    30,     0,    31,     0,     0,     0,    32,
      33,    34,     0,    35,    36,    37,     0,     0,    38,     0,
      39,     0,    40,    41,    42,     0,    43,    44,     0,    45,
       0,     0,     0,     0,     0,     0,     0,     0,    46,     0,
      47,     0,     0,     0,     0,     0,     0,     0,    48,    49,
       0,    50,    51,    52,     0,    53,    54,     0,    55,     0,
       0,    56,    57,    58,     0,    59,     0,    60,     0,    61,
      62,     0,    63,    64,     1,     0,     2,     3,     4,     5,
       6,     0,     7,     8,     9,    10,    11,     0,     0,     0,
      12,     0,    13,    14,     0,     0,     0,     0,    15,    16,
       0,     0,    17,    18,     0,     0,     0,     0,    19,    20,
      21,     0,     0,    22,     0,     0,     0,   236,     0,    23,
      24,     0,     0,     0,     0,    25,    26,    27,     0,     0,
      28,     0,     0,     0,     0,    29,    30,     0,    31,     0,
       0,     0,    32,    33,    34,     0,    35,    36,    37,     0,
     366,    38,     0,    39,     0,    40,    41,    42,     0,    43,
      44,     0,    45,     0,     0,     0,     0,     0,     0,     0,
       0,    46,     0,    47,     0,     0,     0,     0,     0,     0,
       0,    48,    49,     0,    50,    51,    52,     0,    53,    54,
       0,    55,     0,     0,    56,    57,    58,     0,    59,     0,
      60,     0,    61,    62,     0,    63,    64,     1,     0,     2,
       3,     4,     5,     6,     0,     7,     8,     9,    10,    11,
       0,     0,     0,    12,     0,    13,    14,     0,     0,     0,
       0,    15,    16,     0,     0,    17,    18,     0,     0,     0,
       0,    19,    20,    21,     0,     0,    22,     0,     0,     0,
     236,     0,    23,    24,     0,     0,     0,     0,    25,    26,
      27,     0,     0,    28,     0,     0,     0,     0,    29,    30,
       0,    31,     0,     0,     0,    32,    33,    34,     0,    35,
      36,    37,     0,  -258,    38,     0,    39,     0,    40,    41,
      42,     0,    43,    44,     0,    45,     0,     0,     0,     0,
       0,     0,     0,     0,    46,     0,    47,     0,     0,     0,
       0,     0,     0,     0,    48,    49,     0,    50,    51,    52,
       0,    53,    54,     0,    55,     0,     0,    56,    57,    58,
       0,    59,     0,    60,     0,    61,    62,     0,    63,    64,
       1,     0,     2,     3,     4,     5,     6,     0,     7,     8,
       9,    10,    11,     0,     0,     0,    12,     0,    13,    14,
       0,     0,     0,     0,    15,    16,     0,     0,    17,    18,
       0,     0,     0,     0,    19,    20,    21,     0,     0,    22,
       0,     0,     0,   236,     0,    23,    24,     0,     0,     0,
       0,    25,    26,    27,     0,  -255,    28,     0,     0,     0,
       0,    29,    30,     0,    31,     0,     0,     0,    32,    33,
      34,     0,    35,    36,    37,     0,     0,    38,     0,    39,
       0,    40,    41,    42,     0,    43,    44,     0,    45,     0,
       0,     0,     0,     0,     0,     0,     0,    46,     0,    47,
       0,     0,     0,     0,     0,     0,     0,    48,    49,     0,
      50,    51,    52,     0,    53,    54,     0,    55,     0,     0,
      56,    57,    58,     0,    59,     0,    60,     0,    61,    62,
       0,    63,    64,     1,     0,     2,     3,     4,     5,     6,
       0,     7,     8,     9,    10,    11,     0,     0,     0,    12,
       0,    13,    14,     0,     0,     0,     0,    15,    16,     0,
       0,    17,    18,     0,     0,     0,     0,    19,    20,    21,
       0,     0,    22,     0,     0,     0,   236,     0,    23,    24,
       0,     0,     0,     0,    25,    26,    27,     0,     0,    28,
       0,     0,     0,     0,    29,    30,     0,    31,     0,     0,
       0,    32,    33,    34,     0,    35,    36,    37,     0,   434,
      38,     0,    39,     0,    40,    41,    42,     0,    43,    44,
       0,    45,     0,     0,     0,     0,     0,     0,     0,     0,
      46,     0,    47,     0,     0,     0,     0,     0,     0,     0,
      48,    49,     0,    50,    51,    52,     0,    53,    54,     0,
      55,     0,     0,    56,    57,    58,     0,    59,     0,    60,
       0,    61,    62,     0,    63,    64,     1,     0,     2,     3,
       4,     5,     6,     0,     7,     8,     9,    10,    11,     0,
       0,     0,    12,     0,    13,    14,     0,     0,     0,     0,
      15,    16,     0,     0,    17,    18,     0,     0,     0,     0,
      19,    20,    21,     0,     0,    22,     0,     0,     0,   236,
       0,    23,    24,     0,     0,     0,     0,    25,    26,    27,
       0,     0,    28,     0,     0,     0,     0,    29,    30,     0,
      31,     0,     0,     0,    32,    33,    34,     0,    35,    36,
      37,     0,   435,    38,     0,    39,     0,    40,    41,    42,
       0,    43,    44,     0,    45,     0,     0,     0,     0,     0,
       0,     0,     0,    46,     0,    47,     0,     0,     0,     0,
       0,     0,     0,    48,    49,     0,    50,    51,    52,     0,
      53,    54,     0,    55,     0,     0,    56,    57,    58,     0,
      59,     0,    60,     0,    61,    62,     0,    63,    64,     1,
       0,     2,     3,     4,     5,     6,     0,     7,     8,     9,
      10,    11,     0,     0,     0,    12,     0,    13,    14,     0,
       0,     0,     0,    15,    16,     0,     0,    17,    18,     0,
       0,     0,     0,    19,    20,    21,     0,     0,    22,     0,
       0,     0,   236,     0,    23,    24,     0,     0,     0,     0,
      25,    26,    27,     0,     0,    28,     0,     0,     0,     0,
      29,    30,     0,    31,     0,     0,     0,    32,    33,    34,
       0,    35,    36,    37,     0,   439,    38,     0,    39,     0,
      40,    41,    42,     0,    43,    44,     0,    45,     0,     0,
       0,     0,     0,     0,     0,     0,    46,     0,    47,     0,
       0,     0,     0,     0,     0,     0,    48,    49,     0,    50,
      51,    52,     0,    53,    54,     0,    55,     0,     0,    56,
      57,    58,     0,    59,     0,    60,     0,    61,    62,     0,
      63,    64,     1,     0,     2,     3,     4,     5,     6,     0,
       7,     8,     9,    10,    11,     0,     0,     0,    12,     0,
      13,    14,     0,     0,     0,     0,    15,    16,     0,     0,
      17,    18,     0,     0,     0,     0,    19,    20,    21,     0,
       0,    22,     0,     0,     0,     0,     0,    23,    24,     0,
       0,     0,     0,    25,    26,    27,     0,     0,    28,     0,
       0,     0,     0,    29,    30,     0,    31,     0,     0,     0,
      32,    33,    34,     0,    35,    36,    37,   494,     0,    38,
     362,    39,     0,    40,    41,    42,     0,    43,    44,     0,
      45,     0,     0,     0,     0,     0,     0,     0,     0,    46,
       0,    47,     0,     0,     0,     0,     0,     0,     0,    48,
      49,     0,    50,    51,    52,     0,    53,    54,     0,    55,
       0,     0,    56,    57,    58,     0,    59,     0,    60,     0,
      61,    62,     0,    63,    64,     1,     0,     2,     3,     4,
       5,     6,     0,     7,     8,     9,    10,    11,     0,     0,
       0,    12,     0,    13,    14,     0,     0,     0,     0,    15,
      16,     0,     0,    17,    18,     0,     0,     0,     0,    19,
      20,    21,     0,     0,    22,     0,     0,     0,   236,     0,
      23,    24,     0,     0,     0,     0,    25,    26,    27,     0,
       0,    28,     0,     0,     0,     0,    29,    30,     0,    31,
       0,     0,     0,    32,    33,    34,     0,    35,    36,    37,
       0,   502,    38,     0,    39,     0,    40,    41,    42,     0,
      43,    44,     0,    45,     0,     0,     0,     0,     0,     0,
       0,     0,    46,     0,    47,     0,     0,     0,     0,     0,
       0,     0,    48,    49,     0,    50,    51,    52,     0,    53,
      54,     0,    55,     0,     0,    56,    57,    58,     0,    59,
       0,    60,     0,    61,    62,     0,    63,    64,     1,     0,
       2,     3,     4,     5,     6,     0,     7,     8,     9,    10,
      11,     0,     0,     0,    12,     0,    13,    14,     0,     0,
       0,     0,    15,    16,     0,     0,    17,    18,     0,     0,
       0,     0,    19,    20,    21,     0,     0,    22,     0,     0,
       0,   236,     0,    23,    24,     0,     0,     0,     0,    25,
      26,    27,     0,     0,    28,     0,     0,     0,     0,    29,
      30,     0,    31,     0,     0,     0,    32,    33,    34,     0,
      35,    36,    37,     0,   533,    38,     0,    39,     0,    40,
      41,    42,     0,    43,    44,     0,    45,     0,     0,     0,
       0,     0,     0,     0,     0,    46,     0,    47,     0,     0,
       0,     0,     0,     0,     0,    48,    49,     0,    50,    51,
      52,     0,    53,    54,     0,    55,     0,     0,    56,    57,
      58,     0,    59,     0,    60,     0,    61,    62,     0,    63,
      64,     1,     0,     2,     3,     4,     5,     6,     0,     7,
       8,     9,    10,    11,     0,     0,     0,    12,     0,    13,
      14,     0,     0,     0,     0,    15,    16,     0,     0,    17,
      18,     0,     0,     0,     0,    19,    20,    21,     0,     0,
      22,     0,     0,     0,   236,     0,    23,    24,     0,     0,
       0,     0,    25,    26,    27,     0,     0,    28,     0,     0,
       0,     0,    29,    30,     0,    31,     0,     0,     0,    32,
      33,    34,     0,    35,    36,    37,     0,   537,    38,     0,
      39,     0,    40,    41,    42,     0,    43,    44,     0,    45,
       0,     0,     0,     0,     0,     0,     0,     0,    46,     0,
      47,     0,     0,     0,     0,     0,     0,     0,    48,    49,
       0,    50,    51,    52,     0,    53,    54,     0,    55,     0,
       0,    56,    57,    58,     0,    59,     0,    60,     0,    61,
      62,     0,    63,    64,     1,     0,     2,     3,     4,     5,
       6,     0,     7,     8,     9,    10,    11,     0,     0,     0,
      12,     0,    13,    14,     0,     0,     0,     0,    15,    16,
       0,     0,    17,    18,     0,     0,     0,     0,    19,    20,
      21,     0,     0,    22,     0,     0,     0,   236,     0,    23,
      24,     0,     0,     0,     0,    25,    26,    27,     0,     0,
      28,     0,     0,     0,     0,    29,    30,     0,    31,     0,
       0,     0,    32,    33,    34,     0,    35,    36,    37,     0,
     540,    38,     0,    39,     0,    40,    41,    42,     0,    43,
      44,     0,    45,     0,     0,     0,     0,     0,     0,     0,
       0,    46,     0,    47,     0,     0,     0,     0,     0,     0,
       0,    48,    49,     0,    50,    51,    52,     0,    53,    54,
       0,    55,     0,     0,    56,    57,    58,     0,    59,     0,
      60,     0,    61,    62,     0,    63,    64,     1,     0,     2,
       3,     4,     5,     6,     0,     7,     8,     9,    10,    11,
       0,     0,     0,    12,     0,    13,    14,     0,     0,     0,
       0,    15,    16,     0,     0,    17,    18,     0,     0,     0,
       0,    19,    20,    21,     0,     0,    22,     0,     0,     0,
     236,     0,    23,    24,     0,     0,     0,     0,    25,    26,
      27,     0,     0,    28,     0,     0,     0,     0,    29,    30,
       0,    31,     0,     0,     0,    32,    33,    34,     0,    35,
      36,    37,     0,   547,    38,     0,    39,     0,    40,    41,
      42,     0,    43,    44,     0,    45,     0,     0,     0,     0,
       0,     0,     0,     0,    46,     0,    47,     0,     0,     0,
       0,     0,     0,     0,    48,    49,     0,    50,    51,    52,
       0,    53,    54,     0,    55,     0,     0,    56,    57,    58,
       0,    59,     0,    60,     0,    61,    62,     0,    63,    64,
       1,     0,     2,     3,     4,     5,     6,     0,     7,     8,
       9,    10,    11,     0,     0,     0,    12,     0,    13,    14,
       0,     0,     0,     0,    15,    16,     0,     0,    17,    18,
       0,     0,     0,     0,    19,    20,    21,     0,     0,    22,
       0,     0,     0,   236,     0,    23,    24,     0,     0,     0,
       0,    25,    26,    27,     0,     0,    28,     0,     0,     0,
       0,    29,    30,     0,    31,     0,     0,     0,    32,    33,
      34,     0,    35,    36,    37,     0,   562,    38,     0,    39,
       0,    40,    41,    42,     0,    43,    44,     0,    45,     0,
       0,     0,     0,     0,     0,     0,     0,    46,     0,    47,
       0,     0,     0,     0,     0,     0,     0,    48,    49,     0,
      50,    51,    52,     0,    53,    54,     0,    55,     0,     0,
      56,    57,    58,     0,    59,     0,    60,     0,    61,    62,
       0,    63,    64,     1,     0,     2,     3,     4,     5,     6,
       0,     7,     8,     9,    10,    11,     0,     0,     0,    12,
       0,    13,    14,     0,     0,     0,     0,    15,    16,     0,
       0,    17,    18,     0,     0,     0,     0,    19,    20,    21,
       0,     0,    22,     0,     0,     0,   236,     0,    23,    24,
       0,     0,     0,     0,    25,    26,    27,     0,     0,    28,
       0,     0,     0,     0,    29,    30,     0,    31,     0,     0,
       0,    32,    33,    34,     0,    35,    36,    37,     0,   564,
      38,     0,    39,     0,    40,    41,    42,     0,    43,    44,
       0,    45,     0,     0,     0,     0,     0,     0,     0,     0,
      46,     0,    47,     0,     0,     0,     0,     0,     0,     0,
      48,    49,     0,    50,    51,    52,     0,    53,    54,     0,
      55,     0,     0,    56,    57,    58,     0,    59,     0,    60,
       0,    61,    62,     0,    63,    64,     1,     0,     2,     3,
       4,     5,     6,     0,     7,     8,     9,    10,    11,     0,
       0,     0,    12,     0,    13,    14,     0,     0,     0,     0,
      15,    16,     0,     0,    17,    18,     0,     0,     0,     0,
      19,    20,    21,     0,     0,    22,     0,     0,     0,   236,
       0,    23,    24,     0,     0,     0,     0,    25,    26,    27,
       0,     0,    28,     0,     0,     0,     0,    29,    30,     0,
      31,     0,     0,     0,    32,    33,    34,     0,    35,    36,
      37,     0,   574,    38,     0,    39,     0,    40,    41,    42,
       0,    43,    44,     0,    45,     0,     0,     0,     0,     0,
       0,     0,     0,    46,     0,    47,     0,     0,     0,     0,
       0,     0,     0,    48,    49,     0,    50,    51,    52,     0,
      53,    54,     0,    55,     0,     0,    56,    57,    58,     0,
      59,     0,    60,     0,    61,    62,     0,    63,    64,     1,
       0,     2,     3,     4,     5,     6,     0,     7,     8,     9,
      10,    11,     0,     0,     0,    12,     0,    13,    14,     0,
       0,     0,     0,    15,    16,     0,     0,    17,    18,     0,
       0,     0,     0,    19,    20,    21,     0,     0,    22,     0,
       0,     0,   236,     0,    23,    24,     0,     0,     0,     0,
      25,    26,    27,     0,     0,    28,     0,     0,     0,     0,
      29,    30,     0,    31,     0,     0,     0,    32,    33,    34,
       0,    35,    36,    37,     0,   581,    38,     0,    39,     0,
      40,    41,    42,     0,    43,    44,     0,    45,     0,     0,
       0,     0,     0,     0,     0,     0,    46,     0,    47,     0,
       0,     0,     0,     0,     0,     0,    48,    49,     0,    50,
      51,    52,     0,    53,    54,     0,    55,     0,     0,    56,
      57,    58,     0,    59,     0,    60,     0,    61,    62,     0,
      63,    64,     1,     0,     2,     3,     4,     5,     6,     0,
       7,     8,     9,    10,    11,     0,     0,     0,    12,     0,
      13,    14,     0,     0,     0,     0,    15,    16,     0,     0,
      17,    18,     0,     0,     0,     0,    19,    20,    21,     0,
       0,    22,     0,     0,     0,   236,     0,    23,    24,     0,
       0,     0,     0,    25,    26,    27,     0,     0,    28,     0,
       0,     0,     0,    29,    30,     0,    31,     0,     0,     0,
      32,    33,    34,     0,    35,    36,    37,     0,   582,    38,
       0,    39,     0,    40,    41,    42,     0,    43,    44,     0,
      45,     0,     0,     0,     0,     0,     0,     0,     0,    46,
       0,    47,     0,     0,     0,     0,     0,     0,     0,    48,
      49,     0,    50,    51,    52,     0,    53,    54,     0,    55,
       0,     0,    56,    57,    58,     0,    59,     0,    60,     0,
      61,    62,     0,    63,    64,     1,     0,     2,     3,     4,
       5,     6,     0,     7,     8,     9,    10,    11,     0,     0,
       0,    12,     0,    13,    14,     0,     0,     0,     0,    15,
      16,     0,     0,    17,    18,     0,     0,     0,     0,    19,
      20,    21,     0,     0,    22,     0,     0,     0,   236,     0,
      23,    24,     0,     0,     0,     0,    25,    26,    27,     0,
       0,    28,     0,     0,     0,     0,    29,    30,     0,    31,
       0,     0,     0,    32,    33,    34,     0,    35,    36,    37,
       0,   583,    38,     0,    39,     0,    40,    41,    42,     0,
      43,    44,     0,    45,     0,     0,     0,     0,     0,     0,
       0,     0,    46,     0,    47,     0,     0,     0,     0,     0,
       0,     0,    48,    49,     0,    50,    51,    52,     0,    53,
      54,     0,    55,     0,     0,    56,    57,    58,     0,    59,
       0,    60,     0,    61,    62,     0,    63,    64,     1,     0,
       2,     3,     4,     5,     6,     0,     7,     8,     9,    10,
      11,     0,     0,     0,    12,     0,    13,    14,     0,     0,
       0,     0,    15,    16,     0,     0,    17,    18,     0,     0,
       0,     0,    19,    20,    21,     0,     0,    22,     0,     0,
       0,   236,     0,    23,    24,     0,     0,     0,     0,    25,
      26,    27,     0,     0,    28,     0,     0,     0,     0,    29,
      30,     0,    31,     0,     0,     0,    32,    33,    34,     0,
      35,    36,    37,     0,   584,    38,     0,    39,     0,    40,
      41,    42,     0,    43,    44,     0,    45,     0,     0,     0,
       0,     0,     0,     0,     0,    46,     0,    47,     0,     0,
       0,     0,     0,     0,     0,    48,    49,     0,    50,    51,
      52,     0,    53,    54,     0,    55,     0,     0,    56,    57,
      58,     0,    59,     0,    60,     0,    61,    62,     0,    63,
      64,     1,     0,     2,     3,     4,     5,     6,     0,     7,
       8,     9,    10,    11,     0,     0,     0,    12,     0,    13,
      14,     0,     0,     0,     0,    15,    16,     0,     0,    17,
      18,     0,     0,     0,     0,    19,    20,    21,     0,     0,
      22,     0,     0,     0,   236,     0,    23,    24,     0,     0,
       0,     0,    25,    26,    27,     0,     0,    28,     0,     0,
       0,     0,    29,    30,     0,    31,     0,     0,     0,    32,
      33,    34,     0,    35,    36,    37,     0,   585,    38,     0,
      39,     0,    40,    41,    42,     0,    43,    44,     0,    45,
       0,     0,     0,     0,     0,     0,     0,     0,    46,     0,
      47,     0,     0,     0,     0,     0,     0,     0,    48,    49,
       0,    50,    51,    52,     0,    53,    54,     0,    55,     0,
       0,    56,    57,    58,     0,    59,     0,    60,     0,    61,
      62,     0,    63,    64,     1,     0,     2,     3,     4,     5,
       6,     0,     7,     8,     9,    10,    11,     0,     0,     0,
      12,     0,    13,    14,     0,     0,     0,     0,    15,    16,
       0,     0,    17,    18,     0,     0,     0,     0,    19,    20,
      21,     0,     0,    22,     0,     0,     0,     0,     0,    23,
      24,     0,     0,     0,     0,    25,    26,    27,     0,     0,
      28,     0,     0,     0,     0,    29,    30,     0,    31,     0,
       0,     0,    32,    33,    34,     0,    35,    36,    37,     0,
     380,    38,     0,    39,     0,    40,    41,    42,     0,    43,
      44,     0,    45,     0,     0,     0,     0,     0,     0,     0,
       0,    46,     0,    47,     0,     0,     0,     0,     0,     0,
       0,    48,    49,   587,    50,    51,    52,     0,    53,    54,
       0,    55,     0,     0,    56,    57,    58,     0,    59,     0,
      60,     0,    61,    62,     0,    63,    64,     1,     0,     2,
       3,     4,     5,     6,     0,     7,     8,     9,    10,    11,
       0,     0,     0,    12,     0,    13,    14,     0,     0,     0,
       0,    15,    16,     0,     0,    17,    18,     0,     0,     0,
       0,    19,    20,    21,     0,     0,    22,     0,     0,     0,
     236,     0,    23,    24,     0,     0,     0,     0,    25,    26,
      27,     0,     0,    28,     0,     0,     0,     0,    29,    30,
       0,    31,     0,     0,     0,    32,    33,    34,     0,    35,
      36,    37,     0,   591,    38,     0,    39,     0,    40,    41,
      42,     0,    43,    44,     0,    45,     0,     0,     0,     0,
       0,     0,     0,     0,    46,     0,    47,     0,     0,     0,
       0,     0,     0,     0,    48,    49,     0,    50,    51,    52,
       0,    53,    54,     0,    55,     0,     0,    56,    57,    58,
       0,    59,     0,    60,     0,    61,    62,     0,    63,    64,
       1,     0,     2,     3,     4,     5,     6,     0,     7,     8,
       9,    10,    11,    89,     0,     0,    12,     0,    13,    14,
       0,     0,     0,     0,    15,    16,     0,     0,    17,    18,
       0,     0,     0,     0,    19,    20,    21,     0,     0,    22,
       0,     0,     0,     0,     0,    23,    24,     0,     0,     0,
       0,    25,    26,    27,     0,     0,    28,     0,     0,     0,
       0,    29,    30,     0,    31,     0,     0,     0,    32,    33,
      34,     0,    35,    36,    37,     0,     0,    38,     0,    39,
       0,    40,    41,    42,     0,    43,    44,     0,    45,     0,
       0,     0,     0,     0,     0,     0,     0,    46,     0,    47,
       0,     0,     0,     0,     0,     0,     0,    48,    49,     0,
      50,    51,    52,     0,    53,    54,     0,    55,     0,     0,
      56,    57,    58,     0,    59,     0,    60,     0,    61,    62,
       0,    63,    64,     1,     0,     2,     3,     4,     5,     6,
       0,     7,     8,     9,    10,    11,     0,     0,     0,    12,
       0,    13,    14,     0,     0,     0,     0,    15,    16,     0,
       0,    17,    18,     0,     0,     0,     0,    19,    20,    21,
       0,     0,    22,  -221,     0,     0,     0,     0,    23,    24,
       0,     0,     0,     0,    25,    26,    27,     0,     0,    28,
       0,     0,     0,     0,    29,    30,     0,    31,     0,     0,
       0,    32,    33,    34,     0,    35,    36,    37,     0,     0,
      38,     0,    39,     0,    40,    41,    42,     0,    43,    44,
       0,    45,     0,     0,     0,     0,     0,     0,     0,     0,
      46,     0,    47,     0,     0,     0,     0,     0,     0,     0,
      48,    49,     0,    50,    51,    52,     0,    53,    54,     0,
      55,     0,     0,    56,    57,    58,     0,    59,     0,    60,
       0,    61,    62,     0,    63,    64,     1,     0,     2,     3,
       4,     5,     6,     0,     7,     8,     9,    10,    11,     0,
       0,     0,    12,     0,    13,    14,     0,     0,     0,     0,
      15,    16,     0,     0,    17,    18,     0,     0,     0,     0,
      19,    20,    21,     0,     0,    22,     0,     0,     0,     0,
       0,    23,    24,     0,     0,     0,     0,    25,    26,    27,
       0,     0,    28,     0,     0,     0,     0,    29,    30,     0,
      31,     0,     0,     0,    32,    33,    34,     0,    35,    36,
      37,   115,     0,    38,     0,    39,     0,    40,    41,    42,
       0,    43,    44,     0,    45,     0,     0,     0,     0,     0,
       0,     0,     0,    46,     0,    47,     0,     0,     0,     0,
       0,     0,     0,    48,    49,     0,    50,    51,    52,     0,
      53,    54,     0,    55,     0,     0,    56,    57,    58,     0,
      59,     0,    60,     0,    61,    62,     0,    63,    64,     1,
       0,     2,     3,     4,     5,     6,     0,     7,     8,     9,
      10,    11,     0,     0,     0,    12,   204,    13,    14,     0,
       0,     0,     0,    15,    16,     0,     0,    17,    18,     0,
       0,     0,     0,    19,    20,    21,     0,     0,    22,     0,
       0,     0,     0,     0,    23,    24,     0,     0,     0,     0,
      25,    26,    27,     0,     0,    28,     0,     0,     0,     0,
      29,    30,     0,    31,     0,     0,     0,    32,    33,    34,
       0,    35,    36,    37,     0,     0,    38,     0,    39,     0,
      40,    41,    42,     0,    43,    44,     0,    45,     0,     0,
       0,     0,     0,     0,     0,     0,    46,     0,    47,     0,
       0,     0,     0,     0,     0,     0,    48,    49,     0,    50,
      51,    52,     0,    53,    54,     0,    55,     0,     0,    56,
      57,    58,     0,    59,     0,    60,     0,    61,    62,     0,
      63,    64,     1,     0,     2,     3,     4,     5,     6,     0,
       7,     8,     9,    10,    11,     0,     0,     0,    12,   206,
      13,    14,     0,     0,     0,     0,    15,    16,     0,     0,
      17,    18,     0,     0,     0,     0,    19,    20,    21,     0,
       0,    22,     0,     0,     0,     0,     0,    23,    24,     0,
       0,     0,     0,    25,    26,    27,     0,     0,    28,     0,
       0,     0,     0,    29,    30,     0,    31,     0,     0,     0,
      32,    33,    34,     0,    35,    36,    37,     0,     0,    38,
       0,    39,     0,    40,    41,    42,     0,    43,    44,     0,
      45,     0,     0,     0,     0,     0,     0,     0,     0,    46,
       0,    47,     0,     0,     0,     0,     0,     0,     0,    48,
      49,     0,    50,    51,    52,     0,    53,    54,     0,    55,
       0,     0,    56,    57,    58,     0,    59,     0,    60,     0,
      61,    62,     0,    63,    64,     1,     0,     2,     3,     4,
       5,     6,     0,     7,     8,     9,    10,    11,     0,     0,
       0,    12,     0,    13,    14,     0,     0,     0,     0,    15,
      16,     0,     0,    17,    18,     0,     0,     0,     0,    19,
      20,    21,     0,     0,    22,     0,     0,     0,  -219,     0,
      23,    24,     0,     0,     0,     0,    25,    26,    27,     0,
       0,    28,     0,     0,     0,     0,    29,    30,     0,    31,
       0,     0,     0,    32,    33,    34,     0,    35,    36,    37,
       0,     0,    38,     0,    39,     0,    40,    41,    42,     0,
      43,    44,     0,    45,     0,     0,     0,     0,     0,     0,
       0,     0,    46,     0,    47,     0,     0,     0,     0,     0,
       0,     0,    48,    49,     0,    50,    51,    52,     0,    53,
      54,     0,    55,     0,     0,    56,    57,    58,     0,    59,
       0,    60,     0,    61,    62,     0,    63,    64,     1,     0,
       2,     3,     4,     5,     6,     0,     7,     8,     9,    10,
      11,     0,     0,     0,    12,  -221,    13,    14,     0,     0,
       0,     0,    15,    16,     0,     0,    17,    18,     0,     0,
       0,     0,    19,    20,    21,     0,     0,    22,     0,     0,
       0,     0,     0,    23,    24,     0,     0,     0,     0,    25,
      26,    27,     0,     0,    28,     0,     0,     0,     0,    29,
      30,     0,    31,     0,     0,     0,    32,    33,    34,     0,
      35,    36,    37,     0,     0,    38,     0,    39,     0,    40,
      41,    42,     0,    43,    44,     0,    45,     0,     0,     0,
       0,     0,     0,     0,     0,    46,     0,    47,     0,     0,
       0,     0,     0,     0,     0,    48,    49,     0,    50,    51,
      52,     0,    53,    54,     0,    55,     0,     0,    56,    57,
      58,     0,    59,     0,    60,     0,    61,    62,     0,    63,
      64,     1,     0,     2,     3,     4,     5,     6,     0,     7,
       8,     9,    10,    11,     0,     0,     0,    12,   271,    13,
      14,     0,     0,     0,     0,    15,    16,     0,     0,    17,
      18,     0,     0,     0,     0,    19,    20,    21,     0,     0,
      22,     0,     0,     0,     0,     0,    23,    24,     0,     0,
       0,     0,    25,    26,    27,     0,     0,    28,     0,     0,
       0,     0,    29,    30,     0,    31,     0,     0,     0,    32,
      33,    34,     0,    35,    36,    37,     0,     0,    38,     0,
      39,     0,    40,    41,    42,     0,    43,    44,     0,    45,
       0,     0,     0,     0,     0,     0,     0,     0,    46,     0,
      47,     0,     0,     0,     0,     0,     0,     0,    48,    49,
       0,    50,    51,    52,     0,    53,    54,     0,    55,     0,
       0,    56,    57,    58,     0,    59,     0,    60,     0,    61,
      62,     0,    63,    64,     1,     0,     2,     3,     4,     5,
       6,   302,     7,     8,     9,    10,    11,     0,     0,     0,
      12,     0,    13,    14,     0,     0,     0,     0,    15,    16,
       0,     0,    17,    18,     0,     0,     0,     0,    19,    20,
      21,     0,     0,    22,     0,     0,     0,     0,     0,    23,
      24,     0,     0,     0,     0,    25,    26,    27,     0,     0,
      28,     0,     0,     0,     0,    29,    30,     0,    31,     0,
       0,     0,    32,    33,    34,     0,    35,    36,    37,     0,
       0,    38,     0,    39,     0,    40,    41,    42,     0,    43,
      44,     0,    45,     0,     0,     0,     0,     0,     0,     0,
       0,    46,     0,    47,     0,     0,     0,     0,     0,     0,
       0,    48,    49,     0,    50,    51,    52,     0,    53,    54,
       0,    55,     0,     0,    56,    57,    58,     0,    59,     0,
      60,     0,    61,    62,     0,    63,    64,     1,     0,     2,
       3,     4,     5,     6,     0,     7,     8,     9,    10,    11,
       0,     0,     0,    12,     0,    13,    14,     0,     0,     0,
       0,    15,    16,     0,     0,    17,    18,     0,     0,     0,
       0,    19,    20,    21,     0,     0,    22,     0,     0,     0,
       0,     0,    23,    24,     0,     0,     0,     0,    25,    26,
      27,     0,     0,    28,     0,     0,     0,     0,    29,    30,
       0,    31,     0,     0,     0,    32,    33,    34,     0,    35,
      36,    37,     0,     0,    38,   362,    39,     0,    40,    41,
      42,     0,    43,    44,     0,    45,     0,     0,     0,     0,
       0,     0,     0,     0,    46,     0,    47,     0,     0,     0,
       0,     0,     0,     0,    48,    49,     0,    50,    51,    52,
       0,    53,    54,     0,    55,     0,     0,    56,    57,    58,
       0,    59,     0,    60,     0,    61,    62,     0,    63,    64,
       1,     0,     2,     3,     4,     5,     6,     0,     7,     8,
       9,    10,    11,     0,     0,     0,    12,     0,    13,    14,
       0,     0,     0,     0,    15,    16,     0,     0,    17,    18,
       0,     0,     0,     0,    19,    20,    21,     0,     0,    22,
       0,     0,     0,     0,     0,    23,    24,     0,     0,     0,
       0,    25,    26,    27,     0,     0,    28,     0,     0,     0,
       0,    29,    30,     0,    31,     0,     0,     0,    32,    33,
      34,     0,    35,    36,    37,     0,   380,    38,     0,    39,
       0,    40,    41,    42,     0,    43,    44,     0,    45,     0,
       0,     0,     0,     0,     0,     0,     0,    46,     0,    47,
       0,     0,     0,     0,     0,     0,     0,    48,    49,     0,
      50,    51,    52,     0,    53,    54,     0,    55,     0,     0,
      56,    57,    58,     0,    59,     0,    60,     0,    61,    62,
       0,    63,    64,     1,     0,     2,     3,     4,     5,     6,
       0,     7,     8,     9,    10,    11,     0,     0,     0,    12,
       0,    13,    14,     0,     0,     0,     0,    15,    16,     0,
       0,    17,    18,     0,     0,     0,     0,    19,    20,    21,
       0,     0,    22,     0,     0,     0,     0,     0,    23,    24,
       0,     0,     0,     0,    25,    26,    27,     0,     0,    28,
       0,     0,     0,     0,    29,    30,     0,    31,     0,     0,
       0,    32,    33,    34,     0,    35,    36,    37,     0,   260,
      38,     0,    39,     0,    40,    41,    42,     0,    43,    44,
       0,    45,     0,     0,     0,     0,     0,     0,     0,     0,
      46,     0,    47,     0,     0,     0,     0,     0,     0,     0,
      48,   261,     0,    50,    51,    52,     0,    53,    54,     0,
      55,     0,     0,    56,    57,    58,     0,    59,     0,    60,
       0,    61,    62,     0,    63,    64,     1,     0,     2,     3,
       4,     5,     6,     0,     7,     8,     9,    10,    11,     0,
       0,     0,    12,     0,    13,    14,     0,     0,     0,     0,
      15,    16,     0,     0,    17,    18,     0,     0,     0,     0,
      19,    20,    21,     0,     0,    22,     0,     0,     0,     0,
       0,    23,    24,     0,     0,     0,   385,    25,    26,    27,
       0,     0,    28,     0,     0,     0,     0,    29,    30,     0,
      31,     0,     0,     0,    32,    33,    34,     0,    35,    36,
      37,     0,     0,    38,     0,    39,     0,    40,    41,    42,
       0,    43,    44,     0,    45,     0,     0,     0,     0,     0,
       0,     0,     0,    46,     0,    47,     0,     0,     0,     0,
       0,     0,     0,    48,    49,     0,    50,    51,    52,     0,
      53,    54,     0,    55,     0,     0,    56,    57,    58,     0,
      59,     0,    60,     0,    61,    62,     0,    63,    64,     1,
       0,     2,     3,     4,     5,     6,     0,     7,     8,     9,
      10,    11,     0,     0,     0,    12,     0,    13,    14,     0,
       0,     0,     0,    15,    16,     0,     0,    17,    18,     0,
       0,     0,     0,    19,    20,    21,     0,     0,    22,     0,
       0,     0,     0,     0,    23,    24,     0,     0,     0,     0,
      25,    26,    27,     0,     0,    28,     0,     0,     0,     0,
      29,    30,     0,    31,     0,     0,     0,    32,    33,    34,
       0,    35,   508,    37,   115,     0,    38,     0,    39,     0,
      40,    41,    42,     0,    43,    44,     0,    45,     0,     0,
       0,     0,     0,     0,     0,     0,    46,     0,    47,     0,
       0,     0,     0,     0,     0,     0,    48,    49,     0,    50,
      51,    52,     0,    53,    54,     0,    55,     0,     0,    56,
      57,    58,     0,    59,     0,    60,     0,    61,    62,     0,
      63,    64,     1,     0,     2,     3,     4,     5,     6,     0,
       7,     8,     9,    10,    11,     0,     0,     0,    12,  -219,
      13,    14,     0,     0,     0,     0,    15,    16,     0,     0,
      17,    18,     0,     0,     0,     0,    19,    20,    21,     0,
       0,    22,     0,     0,     0,     0,     0,    23,    24,     0,
       0,     0,     0,    25,    26,    27,     0,     0,    28,     0,
       0,     0,     0,    29,    30,     0,    31,     0,     0,     0,
      32,    33,    34,     0,    35,    36,    37,     0,     0,    38,
       0,    39,     0,    40,    41,    42,     0,    43,    44,     0,
      45,     0,     0,     0,     0,     0,     0,     0,     0,    46,
       0,    47,     0,     0,     0,     0,     0,     0,     0,    48,
      49,     0,    50,    51,    52,     0,    53,    54,     0,    55,
       0,     0,    56,    57,    58,     0,    59,     0,    60,     0,
      61,    62,     0,    63,    64,     1,     0,     2,     3,     4,
       5,     6,     0,     7,     8,     9,    10,    11,     0,     0,
       0,    12,     0,    13,    14,     0,     0,     0,     0,    15,
      16,     0,     0,    17,    18,     0,     0,     0,     0,    19,
      20,    21,     0,     0,    22,     0,     0,     0,     0,     0,
      23,    24,     0,     0,     0,     0,    25,    26,    27,     0,
       0,    28,     0,     0,     0,     0,    29,    30,     0,    31,
       0,     0,     0,    32,    33,    34,     0,    35,    36,    37,
       0,     0,    38,     0,    39,     0,    40,    41,    42,     0,
      43,    44,     0,    45,     0,     0,     0,     0,     0,     0,
       0,     0,    46,     0,    47,     0,     0,     0,     0,     0,
       0,     0,    48,    49,     0,    50,    51,    52,     0,    53,
      54,     0,    55,   543,     0,    56,    57,    58,     0,    59,
       0,    60,     0,    61,    62,     0,    63,    64,     1,     0,
       2,     3,     4,     5,     6,     0,     7,     8,     9,    10,
      11,     0,     0,     0,    12,     0,    13,    14,     0,     0,
       0,     0,    15,    16,     0,     0,    17,    18,     0,     0,
       0,     0,    19,    20,    21,     0,     0,    22,     0,     0,
       0,     0,     0,    23,    24,     0,     0,     0,     0,    25,
      26,    27,     0,     0,    28,     0,     0,     0,     0,    29,
      30,     0,    31,     0,     0,     0,    32,    33,    34,     0,
      35,    36,    37,     0,     0,    38,     0,    39,     0,    40,
      41,    42,     0,    43,    44,     0,    45,     0,     0,     0,
       0,     0,     0,     0,     0,    46,     0,    47,     0,     0,
       0,     0,     0,     0,     0,    48,    49,   565,    50,    51,
      52,     0,    53,    54,     0,    55,     0,     0,    56,    57,
      58,     0,    59,     0,    60,     0,    61,    62,     0,    63,
      64,     1,     0,     2,     3,     4,     5,     6,     0,     7,
       8,     9,    10,    11,     0,     0,     0,    12,     0,    13,
      14,     0,     0,     0,     0,    15,    16,     0,     0,    17,
      18,     0,     0,     0,     0,    19,    20,    21,     0,     0,
      22,     0,     0,     0,     0,     0,    23,    24,     0,     0,
       0,     0,    25,    26,    27,     0,     0,    28,     0,     0,
       0,     0,    29,    30,     0,    31,     0,     0,     0,    32,
      33,    34,     0,    35,    36,    37,     0,     0,    38,     0,
      39,     0,    40,    41,    42,     0,    43,    44,     0,    45,
       0,     0,     0,     0,     0,     0,     0,     0,    46,     0,
      47,     0,     0,     0,     0,     0,     0,     0,    48,    49,
     589,    50,    51,    52,     0,    53,    54,     0,    55,     0,
       0,    56,    57,    58,     0,    59,     0,    60,     0,    61,
      62,     0,    63,    64,     1,     0,     2,     3,     4,     5,
       6,     0,     7,     8,     9,    10,    11,     0,     0,     0,
      12,     0,    13,    14,     0,     0,     0,     0,    15,    16,
       0,     0,    17,    18,     0,     0,     0,     0,    19,    20,
      21,     0,     0,    22,     0,     0,     0,     0,     0,    23,
      24,     0,     0,     0,     0,    25,    26,    27,     0,     0,
      28,     0,     0,     0,     0,    29,    30,     0,    31,     0,
       0,     0,    32,    33,    34,     0,    35,    36,    37,     0,
       0,    38,     0,    39,     0,    40,    41,    42,     0,    43,
      44,     0,    45,     0,     0,     0,     0,     0,     0,     0,
       0,    46,     0,    47,     0,     0,     0,     0,     0,     0,
       0,    48,    49,     0,    50,    51,    52,     0,    53,    54,
       0,    55,     0,     0,    56,    57,    58,     0,    59,     0,
      60,     0,    61,    62,     0,    63,    64,     1,     0,     2,
       3,     4,     5,     6,     0,     7,     8,     9,    10,    85,
       0,     0,     0,    12,     0,    13,    14,     0,     0,     0,
       0,    15,    16,     0,     0,    17,    18,     0,     0,     0,
       0,    19,    20,    21,     0,     0,    22,     0,     0,     0,
       0,     0,    23,    24,     0,     0,     0,     0,    25,    26,
      27,     0,     0,    28,     0,     0,     0,     0,    29,    30,
       0,    31,     0,     0,     0,    32,    33,    34,     0,    35,
      36,    37,     0,     0,    86,     0,    39,     0,    40,    41,
      42,     0,    43,    44,     0,    45,     0,     0,     0,     0,
       0,     0,     0,     0,    46,     0,    47,     0,     0,     0,
       0,     0,     0,     0,    48,    49,     0,    50,    51,    52,
       0,    53,    54,     0,    55,     0,     0,    56,    57,    58,
       0,    59,     0,    60,     0,    61,    62,     0,    63,    64,
       1,     0,     2,     3,     4,     5,     6,     0,     7,     8,
       9,    10,    11,     0,     0,     0,    96,     0,    13,    14,
       0,     0,     0,     0,    15,    16,     0,     0,    17,    18,
       0,     0,     0,     0,    19,    20,    21,     0,     0,    22,
       0,     0,     0,     0,     0,    23,    24,     0,     0,     0,
       0,    25,    26,    27,     0,     0,    28,     0,     0,     0,
       0,    29,    30,     0,    31,     0,     0,     0,    32,    33,
      34,     0,    35,    36,    37,     0,     0,    38,     0,    39,
       0,    40,    41,    42,     0,    43,    44,     0,    45,     0,
       0,     0,     0,     0,     0,     0,     0,    46,     0,    47,
       0,     0,     0,     0,     0,     0,     0,    48,    49,     0,
      50,    51,    52,     0,    53,    54,     0,    55,     0,     0,
      56,    57,    58,     0,    59,     0,    60,     0,    61,    62,
       0,    63,    64,     1,     0,     2,     3,     4,     5,     6,
       0,     7,     8,     9,    10,    11,     0,     0,     0,   100,
       0,    13,    14,     0,     0,     0,     0,    15,    16,     0,
       0,    17,    18,     0,     0,     0,     0,    19,    20,    21,
       0,     0,    22,     0,     0,     0,     0,     0,    23,    24,
       0,     0,     0,     0,    25,    26,    27,     0,     0,    28,
       0,     0,     0,     0,    29,    30,     0,    31,     0,     0,
       0,    32,    33,    34,     0,    35,    36,    37,     0,     0,
      38,     0,    39,     0,    40,    41,    42,     0,    43,    44,
       0,    45,     0,     0,     0,     0,     0,     0,     0,     0,
      46,     0,    47,     0,     0,     0,     0,     0,     0,     0,
      48,    49,     0,    50,    51,    52,     0,    53,    54,     0,
      55,     0,     0,    56,    57,    58,     0,    59,     0,    60,
       0,    61,    62,     0,    63,    64,     1,     0,     2,     3,
       4,     5,     6,     0,     7,     8,     9,    10,    11,     0,
       0,     0,   110,     0,    13,    14,     0,     0,     0,     0,
      15,    16,     0,     0,    17,    18,     0,     0,     0,     0,
      19,    20,    21,     0,     0,    22,     0,     0,     0,     0,
       0,    23,    24,     0,     0,     0,     0,    25,    26,    27,
       0,     0,    28,     0,     0,     0,     0,    29,    30,     0,
      31,     0,     0,     0,    32,    33,    34,     0,    35,    36,
      37,     0,     0,    38,     0,    39,     0,    40,    41,    42,
       0,    43,    44,     0,    45,     0,     0,     0,     0,     0,
       0,     0,     0,    46,     0,    47,     0,     0,     0,     0,
       0,     0,     0,    48,    49,     0,    50,    51,    52,     0,
      53,    54,     0,    55,     0,     0,    56,    57,    58,     0,
      59,     0,    60,     0,    61,    62,     0,    63,    64,     1,
       0,     2,   221,     4,     5,     6,     0,     7,     8,     9,
      10,    11,     0,     0,     0,    12,     0,    13,    14,     0,
       0,     0,     0,    15,    16,     0,     0,    17,    18,     0,
       0,     0,     0,    19,    20,    21,     0,     0,    22,     0,
       0,     0,     0,     0,    23,    24,     0,     0,     0,     0,
      25,    26,    27,     0,     0,    28,     0,     0,     0,     0,
      29,    30,     0,    31,     0,     0,     0,    32,    33,    34,
       0,    35,    36,    37,     0,     0,    38,     0,    39,     0,
      40,    41,    42,     0,    43,    44,     0,    45,     0,     0,
       0,     0,     0,     0,     0,     0,    46,     0,    47,     0,
       0,     0,     0,     0,     0,     0,    48,    49,     0,    50,
      51,    52,     0,    53,    54,     0,    55,     0,     0,    56,
      57,    58,     0,    59,     0,    60,     0,    61,    62,     0,
      63,    64,     1,     0,     2,     3,     4,     5,     6,     0,
       7,     8,     9,    10,    11,     0,     0,     0,    12,     0,
      13,    14,     0,     0,     0,     0,    15,    16,     0,     0,
      17,    18,     0,     0,     0,     0,    19,    20,    21,     0,
       0,   300,     0,     0,     0,     0,     0,    23,    24,     0,
       0,     0,     0,    25,    26,    27,     0,     0,    28,     0,
       0,     0,     0,    29,    30,     0,    31,     0,     0,     0,
      32,    33,    34,     0,    35,    36,    37,     0,     0,    38,
       0,    39,     0,    40,    41,    42,     0,    43,    44,     0,
      45,     0,     0,     0,     0,     0,     0,     0,     0,    46,
       0,    47,     0,     0,     0,     0,     0,     0,     0,    48,
      49,     0,    50,    51,    52,     0,    53,    54,     0,    55,
       0,     0,    56,    57,    58,     0,    59,     0,    60,     0,
      61,    62,     0,    63,    64,     1,     0,     2,     3,     4,
       5,     6,     0,     7,     8,     9,    10,    11,     0,     0,
       0,    12,     0,    13,    14,     0,     0,     0,     0,    15,
      16,     0,     0,    17,    18,     0,     0,     0,     0,    19,
      20,    21,     0,     0,   457,     0,     0,     0,     0,     0,
      23,    24,     0,     0,     0,     0,    25,    26,    27,     0,
       0,    28,     0,     0,     0,     0,    29,    30,     0,    31,
       0,     0,     0,    32,    33,    34,     0,    35,    36,    37,
       0,     0,    38,     0,    39,     0,    40,    41,    42,     0,
      43,    44,     0,    45,     0,     0,     0,     0,     0,     0,
       0,     0,    46,     0,    47,     0,     0,     0,     0,     0,
       0,     0,    48,    49,     0,    50,    51,    52,     0,    53,
      54,     0,    55,     0,     0,    56,    57,    58,     0,    59,
       0,    60,     0,    61,    62,     0,    63,    64,     1,     0,
       2,     3,     4,     5,     6,     0,     7,     8,     9,    10,
      11,     0,     0,     0,    12,     0,    13,    14,     0,     0,
       0,     0,    15,    16,     0,     0,    17,    18,     0,     0,
       0,     0,    19,    20,    21,     0,     0,    22,     0,     0,
       0,     0,     0,    23,    24,     0,     0,     0,     0,    25,
      26,    27,     0,     0,    28,     0,     0,     0,     0,    29,
      30,     0,    31,     0,     0,     0,    32,    33,    34,     0,
      35,    36,    37,     0,     0,    38,     0,    39,     0,    40,
      41,    42,     0,    43,    44,     0,    45,     0,     0,     0,
       0,     0,     0,     0,     0,    46,     0,    47,     0,     0,
       0,     0,     0,     0,     0,    48,   541,     0,    50,    51,
      52,     0,    53,    54,     0,    55,     0,     0,    56,    57,
      58,     0,    59,     0,    60,     0,    61,    62,     0,    63,
      64,     1,     0,     2,     3,     4,     5,     6,     0,     7,
       8,     9,    10,    11,     0,     0,     0,    12,     0,    13,
      14,     0,     0,     0,     0,    15,    16,     0,     0,    17,
      18,     0,     0,     0,     0,    19,    20,    21,     0,     0,
      22,     0,     0,     0,     0,     0,    23,    24,     0,     0,
       0,     0,    25,    26,    27,     0,     0,    28,     0,     0,
       0,     0,    29,    30,     0,    31,     0,     0,     0,    32,
      33,    34,     0,    35,    36,    37,     0,     0,    38,     0,
      39,     0,    40,    41,    42,     0,    43,    44,     0,    45,
       0,     0,     0,     0,     0,     0,     0,     0,    46,     0,
      47,     0,     0,     0,     0,     0,     0,     0,    48,   569,
       0,    50,    51,    52,     0,    53,    54,     0,    55,     0,
       0,    56,    57,    58,     0,    59,     0,    60,     0,    61,
      62,     0,    63,    64,   222,   159,     0,   160,   161,   162,
     163,     0,   164,   165,   166,   167,   168,   169,     0,     0,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,     0,     0,     0,     0,     0,     0,   183,     0,
       0,     0,     0,     0,     0,     0,     0,   159,   223,   160,
     161,   162,   163,     0,   164,   165,   166,   167,   168,   169,
     184,     0,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,     0,     0,     0,     0,   185,     0,
     183,     0,     0,     0,     0,   217,     0,     0,   159,     0,
       0,     0,     0,   163,     0,   164,   165,   166,   167,   393,
     169,     0,   184,   170,     0,     0,   173,   186,   175,   176,
     187,   178,   179,   188,   181,     0,     0,   189,   190,     0,
     185,   191,     0,     0,   192,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   218,     0,     0,     0,     0,   186,
       0,     0,   187,     0,     0,   188,     0,     0,     0,   189,
     190,   185,     0,   191,     0,   159,   192,   160,   161,   162,
     163,     0,   164,   165,   166,   167,   168,   169,     0,     0,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,     0,     0,   455,     0,     0,     0,   183,     0,
       0,   190,     0,     0,   191,     0,     0,   192,     0,     0,
     214,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     184,     0,     0,     0,   159,     0,   160,   161,   162,   163,
       0,   164,   165,   166,   167,   168,   169,     0,   185,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,     0,     0,   513,     0,     0,     0,   183,     0,   514,
       0,     0,     0,     0,     0,     0,     0,   186,     0,     0,
     187,     0,     0,   188,     0,     0,     0,   189,   190,   184,
       0,   191,     0,   159,   192,   160,   161,   162,   163,     0,
     164,   165,   166,   167,   168,   169,     0,   185,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
       0,     0,   548,     0,     0,     0,   183,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   186,     0,   214,   187,
       0,     0,   188,     0,     0,     0,   189,   190,   184,     0,
     191,     0,   159,   192,   160,   161,   162,   163,   209,   164,
     165,   166,   167,   168,   169,     0,   185,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,     0,
       0,     0,     0,     0,     0,   183,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   186,     0,     0,   187,     0,
       0,   188,     0,     0,     0,   189,   190,   184,     0,   191,
       0,     0,   192,     0,     0,     0,     0,     0,     0,   159,
       0,   160,   161,   162,   163,   185,   164,   165,   166,   167,
     168,   169,     0,   212,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,     0,     0,     0,     0,
       0,     0,   183,     0,   186,     0,     0,   187,     0,     0,
     188,     0,     0,     0,   189,   190,     0,     0,   191,     0,
       0,   192,     0,     0,   184,     0,     0,     0,   159,     0,
     160,   161,   162,   163,     0,   164,   165,   166,   167,   168,
     169,     0,   185,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,     0,     0,     0,     0,     0,
       0,   183,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   186,     0,   214,   187,     0,     0,   188,     0,     0,
       0,   189,   190,   184,     0,   191,     0,   159,   192,   160,
     161,   162,   163,     0,   164,   165,   166,   167,   168,   169,
       0,   185,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,     0,     0,     0,     0,     0,     0,
     183,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     186,     0,     0,   187,     0,   234,   188,     0,     0,     0,
     189,   190,   184,     0,   191,     0,   159,   192,   160,   161,
     162,   163,     0,   164,   165,   166,   167,   168,   169,     0,
     185,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,     0,     0,     0,     0,     0,     0,   183,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   186,
       0,     0,   187,     0,   235,   188,     0,     0,     0,   189,
     190,   184,     0,   191,     0,   159,   192,   160,   161,   162,
     163,     0,   164,   165,   166,   167,   168,   169,     0,   185,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,     0,     0,     0,     0,     0,     0,   183,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   186,     0,
       0,   187,     0,     0,   188,     0,     0,     0,   189,   190,
     184,     0,   191,     0,   159,   192,   160,   161,   162,   163,
       0,   164,   165,   166,   167,   168,   169,     0,   185,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,     0,   257,     0,     0,     0,     0,   183,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   186,     0,     0,
     187,     0,     0,   188,     0,     0,     0,   189,   190,   184,
       0,   191,     0,   159,   192,   160,   161,   162,   163,   325,
     164,   165,   166,   167,   168,   169,     0,   185,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
       0,     0,     0,     0,     0,     0,   183,     0,     0,     0,
       0,   264,     0,     0,     0,     0,   186,     0,     0,   187,
       0,     0,   188,     0,     0,     0,   189,   190,   184,     0,
     191,     0,   159,   192,   160,   161,   162,   163,   326,   164,
     165,   166,   167,   168,   169,     0,   185,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,     0,
       0,     0,     0,     0,     0,   183,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   186,     0,     0,   187,     0,
       0,   188,     0,     0,     0,   189,   190,   184,     0,   191,
       0,   159,   192,   160,   161,   162,   163,     0,   164,   165,
     166,   167,   168,   169,     0,   185,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,     0,     0,
     327,     0,     0,     0,   183,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   186,     0,     0,   187,     0,     0,
     188,     0,     0,     0,   189,   190,   184,     0,   191,     0,
     159,   192,   160,   161,   162,   163,   337,   164,   165,   166,
     167,   168,   169,     0,   185,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,     0,     0,     0,
       0,     0,     0,   183,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   186,     0,     0,   187,     0,     0,   188,
       0,     0,     0,   189,   190,   184,     0,   191,     0,   159,
     192,   160,   161,   162,   163,   341,   164,   165,   166,   167,
     168,   169,     0,   185,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,     0,     0,     0,     0,
       0,     0,   183,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   186,     0,     0,   187,     0,     0,   188,     0,
       0,     0,   189,   190,   184,     0,   191,     0,   159,   192,
     160,   161,   162,   163,   342,   164,   165,   166,   167,   168,
     169,     0,   185,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,     0,     0,     0,     0,     0,
       0,   183,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   186,     0,     0,   187,     0,     0,   188,     0,     0,
       0,   189,   190,   184,     0,   191,     0,   159,   192,   160,
     161,   162,   163,   348,   164,   165,   166,   167,   168,   169,
       0,   185,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,     0,     0,     0,     0,     0,     0,
     183,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     186,     0,     0,   187,     0,     0,   188,     0,     0,     0,
     189,   190,   184,     0,   191,     0,   159,   192,   160,   161,
     162,   163,   357,   164,   165,   166,   167,   168,   169,     0,
     185,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,     0,     0,     0,     0,     0,     0,   183,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   186,
       0,     0,   187,     0,     0,   188,     0,     0,     0,   189,
     190,   184,     0,   191,     0,   159,   192,   160,   161,   162,
     163,   368,   164,   165,   166,   167,   168,   169,     0,   185,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,     0,     0,     0,     0,     0,     0,   183,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   186,     0,
       0,   187,     0,     0,   188,     0,     0,     0,   189,   190,
     184,     0,   191,     0,   159,   192,   160,   161,   162,   163,
     371,   164,   165,   166,   167,   168,   169,     0,   185,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,     0,     0,     0,     0,     0,     0,   183,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   186,     0,     0,
     187,     0,     0,   188,     0,     0,     0,   189,   190,   184,
       0,   191,     0,   159,   192,   160,   161,   162,   163,   372,
     164,   165,   166,   167,   168,   169,     0,   185,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
       0,     0,     0,     0,     0,     0,   183,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   186,     0,     0,   187,
       0,     0,   188,     0,     0,     0,   189,   190,   184,     0,
     191,     0,   159,   192,   160,   161,   162,   163,   373,   164,
     165,   166,   167,   168,   169,     0,   185,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,     0,
       0,     0,     0,     0,     0,   183,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   186,     0,     0,   187,     0,
       0,   188,     0,     0,     0,   189,   190,   184,     0,   191,
       0,   159,   192,   160,   161,   162,   163,   374,   164,   165,
     166,   167,   168,   169,     0,   185,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,     0,     0,
       0,     0,     0,     0,   183,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   186,     0,     0,   187,     0,     0,
     188,     0,     0,     0,   189,   190,   184,     0,   191,     0,
     159,   192,   160,   161,   162,   163,     0,   164,   165,   166,
     167,   168,   169,     0,   185,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,     0,     0,   375,
       0,     0,     0,   183,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   186,     0,     0,   187,     0,     0,   188,
       0,     0,     0,   189,   190,   184,     0,   191,     0,   159,
     192,   160,   161,   162,   163,   377,   164,   165,   166,   167,
     168,   169,     0,   185,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,     0,     0,     0,     0,
       0,     0,   183,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   186,     0,     0,   187,     0,     0,   188,     0,
       0,     0,   189,   190,   184,     0,   191,     0,   159,   192,
     160,   161,   162,   163,     0,   164,   165,   166,   167,   168,
     169,     0,   185,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,     0,     0,   387,     0,     0,
       0,   183,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   186,     0,     0,   187,     0,     0,   188,     0,     0,
       0,   189,   190,   184,     0,   191,     0,   159,   192,   160,
     161,   162,   163,     0,   164,   165,   166,   167,   168,   169,
       0,   185,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,     0,     0,     0,     0,     0,     0,
     183,     0,     0,     0,     0,   388,     0,     0,     0,     0,
     186,     0,     0,   187,     0,     0,   188,     0,     0,     0,
     189,   190,   184,     0,   191,     0,   159,   192,   160,   161,
     162,   163,   390,   164,   165,   166,   167,   168,   169,     0,
     185,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,     0,     0,     0,     0,     0,     0,   183,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   186,
       0,     0,   187,     0,     0,   188,     0,     0,     0,   189,
     190,   184,     0,   191,     0,   159,   192,   160,   161,   162,
     163,     0,   164,   165,   166,   167,   168,   169,     0,   185,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,     0,     0,   396,     0,     0,     0,   183,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   186,     0,
       0,   187,     0,     0,   188,     0,     0,     0,   189,   190,
     184,     0,   191,     0,   159,   192,   160,   161,   162,   163,
     401,   164,   165,   166,   167,   168,   169,     0,   185,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,     0,     0,     0,     0,     0,     0,   183,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   186,     0,     0,
     187,     0,     0,   188,     0,     0,     0,   189,   190,   184,
       0,   191,     0,     0,   192,     0,     0,     0,     0,     0,
       0,   159,     0,   160,   161,   162,   163,   185,   164,   165,
     166,   167,   168,   169,     0,   403,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,     0,     0,
       0,     0,     0,     0,   183,     0,   186,     0,     0,   187,
       0,     0,   188,     0,     0,     0,   189,   190,     0,     0,
     191,     0,     0,   192,     0,     0,   184,     0,     0,     0,
     159,     0,   160,   161,   162,   163,     0,   164,   165,   166,
     167,   168,   169,     0,   185,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,     0,     0,     0,
       0,     0,     0,   183,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   186,     0,     0,   187,     0,   426,   188,
       0,     0,     0,   189,   190,   184,     0,   191,     0,   159,
     192,   160,   161,   162,   163,     0,   164,   165,   166,   167,
     168,   169,     0,   185,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,     0,     0,     0,     0,
       0,     0,   183,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   186,     0,     0,   187,     0,     0,   188,     0,
       0,     0,   189,   190,   184,     0,   191,     0,   159,   192,
     160,   161,   162,   163,   436,   164,   165,   166,   167,   168,
     169,     0,   185,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,     0,     0,     0,     0,     0,
     437,   183,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   186,     0,     0,   187,     0,     0,   188,     0,     0,
       0,   189,   190,   184,     0,   191,     0,   159,   192,   160,
     161,   162,   163,     0,   164,   165,   166,   167,   168,   169,
       0,   185,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,     0,     0,     0,     0,     0,   472,
     183,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     186,     0,     0,   187,     0,     0,   188,     0,     0,     0,
     189,   190,   184,     0,   191,     0,     0,   192,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     185,   159,     0,   160,   161,   162,   163,     0,   164,   165,
     166,   167,   168,   169,     0,     0,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,     0,   186,
       0,     0,   187,     0,   183,   188,     0,     0,     0,   189,
     190,     0,     0,   191,     0,     0,   192,     0,     0,     0,
     355,     0,     0,     0,     0,     0,   184,     0,     0,     0,
     159,     0,   160,   161,   162,   163,     0,   164,   165,   166,
     167,   168,   169,     0,   185,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,     0,     0,     0,
       0,     0,   491,   183,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   186,     0,     0,   187,     0,     0,   188,
       0,     0,     0,   189,   190,   184,     0,   191,     0,   159,
     192,   160,   161,   162,   163,   493,   164,   165,   166,   167,
     168,   169,     0,   185,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,     0,     0,     0,     0,
       0,     0,   183,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   186,     0,     0,   187,     0,     0,   188,     0,
       0,     0,   189,   190,   184,     0,   191,     0,   159,   192,
     160,   161,   162,   163,     0,   164,   165,   166,   167,   168,
     169,     0,   185,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,     0,     0,   507,     0,     0,
       0,   183,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   186,     0,     0,   187,     0,     0,   188,     0,     0,
       0,   189,   190,   184,     0,   191,     0,   159,   192,   160,
     161,   162,   163,     0,   164,   165,   166,   167,   168,   169,
       0,   185,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,     0,     0,     0,     0,     0,     0,
     183,     0,     0,     0,     0,   518,     0,     0,     0,     0,
     186,     0,     0,   187,     0,     0,   188,     0,     0,     0,
     189,   190,   184,     0,   191,     0,   159,   192,   160,   161,
     162,   163,     0,   164,   165,   166,   167,   168,   169,     0,
     185,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,     0,     0,     0,     0,     0,     0,   183,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   186,
       0,     0,   187,     0,   532,   188,     0,     0,     0,   189,
     190,   184,     0,   191,     0,   159,   192,   160,   161,   162,
     163,     0,   164,   165,   166,   167,   168,   169,     0,   185,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,     0,     0,     0,     0,     0,     0,   183,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   186,     0,
       0,   187,     0,   534,   188,     0,     0,     0,   189,   190,
     184,     0,   191,     0,   159,   192,   160,   161,   162,   163,
       0,   164,   165,   166,   167,   168,   169,     0,   185,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,     0,     0,     0,     0,     0,   552,   183,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   186,     0,     0,
     187,     0,     0,   188,     0,     0,     0,   189,   190,   184,
       0,   191,     0,   159,   192,   160,   161,   162,   163,     0,
     164,   165,   166,   167,   168,   169,     0,   185,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
       0,     0,     0,     0,     0,     0,   183,     0,     0,     0,
       0,   554,     0,     0,     0,     0,   186,     0,     0,   187,
       0,     0,   188,     0,     0,     0,   189,   190,   184,     0,
     191,     0,   159,   192,   160,   161,   162,   163,     0,   164,
     165,   166,   167,   168,   169,     0,   185,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,     0,
       0,   575,     0,     0,     0,   183,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   186,     0,     0,   187,     0,
       0,   188,     0,     0,     0,   189,   190,   184,     0,   191,
       0,   159,   192,   160,   161,   162,   163,     0,   164,   165,
     166,   167,   168,   169,     0,   185,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,     0,     0,
       0,     0,     0,   594,   183,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   186,     0,     0,   187,     0,     0,
     188,     0,     0,     0,   189,   190,   184,     0,   191,     0,
     159,   192,   160,   161,   162,   163,     0,   164,   165,   166,
     167,   168,   169,     0,   185,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,     0,     0,     0,
       0,     0,     0,   183,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   186,     0,     0,   187,     0,     0,   188,
       0,     0,     0,   189,   190,   184,     0,   191,     0,   159,
     192,   258,   161,   162,   163,     0,   164,   165,   166,   167,
     168,   169,     0,   185,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,     0,     0,     0,     0,
       0,     0,   183,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   186,     0,     0,   187,     0,     0,   188,     0,
       0,     0,   189,   190,   184,     0,   191,     0,   159,   192,
     160,   161,   162,   163,     0,   164,   165,   166,   167,   168,
     169,     0,   185,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,     0,     0,     0,     0,     0,
       0,   183,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   186,     0,     0,   187,     0,     0,   188,     0,     0,
       0,   189,   190,   184,     0,   191,     0,   159,   192,     0,
     161,   162,   163,     0,   164,   165,   166,   167,   168,   169,
       0,   185,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,     0,     0,     0,     0,     0,     0,
     183,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  -259,     0,     0,   188,     0,     0,     0,
     189,   190,   184,     0,   191,     0,   159,   192,   160,   161,
     162,   163,     0,   164,   165,   166,   167,   168,   169,     0,
     185,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,     0,     0,     0,     0,     0,     0,   183,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   186,
       0,     0,   187,     0,     0,   188,     0,     0,     0,   189,
     190,   184,     0,   191,     0,   159,   192,     0,   161,   162,
     163,     0,   164,   165,   166,   167,   168,   169,     0,   185,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,     0,     0,     0,     0,     0,     0,   183,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   188,     0,     0,     0,   189,   190,
       0,     0,   191,     0,   159,   192,   160,     0,   162,   163,
       0,   164,   165,   166,   167,   168,   169,     0,   185,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,     0,     0,     0,     0,     0,     0,   183,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   186,     0,     0,
     187,     0,     0,   188,     0,     0,     0,   189,   190,   184,
       0,   191,     0,   159,   192,     0,     0,   162,   163,     0,
     164,   165,   166,   167,   168,   169,     0,   185,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
       0,     0,     0,     0,     0,     0,   183,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   189,   190,   184,     0,
     191,     0,   159,   192,     0,     0,   162,   163,     0,   164,
     165,   166,   167,   168,   169,     0,   185,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,     0,
       0,     0,   443,     0,     0,   183,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   189,   190,   184,     0,   191,
       0,   159,   192,     0,     0,   162,   163,     0,   164,   165,
     166,   167,   168,   169,     0,   185,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,     0,     0,
       0,     0,     0,     0,   183,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   189,   190,   184,     0,   191,     0,
     159,   192,     0,     0,   162,   163,     0,   164,   165,   166,
     167,   168,   169,     0,   185,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,     0,     0,     0,
       0,     0,     0,   183,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  -259,   190,   184,     0,   191,     0,   159,
     192,     0,     0,  -259,   163,     0,   164,   165,   166,   167,
     168,   169,     0,   185,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   159,     0,     0,     0,
       0,   163,     0,   164,   165,   166,   167,   168,   169,     0,
       0,   170,   171,   172,   173,   174,   175,   176,     0,   178,
     179,   180,   181,   190,   184,     0,   191,     0,     0,   192,
       0,     0,     0,     0,     0,     0,     0,     0,   159,     0,
       0,     0,   185,   163,     0,   164,   165,   166,   167,   168,
     169,   184,     0,   170,   171,   172,   173,   174,   175,   176,
       0,   178,   179,   180,   181,     0,     0,     0,   159,   185,
       0,     0,     0,   163,     0,   164,   165,   166,   167,   168,
     169,     0,   190,   170,     0,   191,   173,     0,   192,     0,
       0,   178,   179,  -259,   181,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   190,
       0,   185,   191,   159,     0,   192,     0,     0,   163,     0,
     164,   165,   166,   167,   168,   169,     0,     0,   170,     0,
       0,   173,   174,   175,   176,   159,   178,   179,     0,   181,
     163,   185,   164,   165,   166,   167,   168,   169,     0,     0,
     170,   190,     0,   173,   191,   175,  -259,   192,   178,   179,
       0,   181,     0,     0,     0,   159,     0,     0,     0,     0,
     163,     0,   164,   165,   166,   167,   168,   169,     0,     0,
     170,   190,     0,     0,   191,     0,   185,   192,   178,   179,
     159,   181,     0,     0,     0,   163,     0,     0,     0,     0,
     167,   168,   169,     0,     0,   170,     0,   159,   185,     0,
       0,     0,   163,   178,   179,     0,   181,     0,   168,   169,
       0,     0,   170,     0,     0,     0,   190,     0,     0,   191,
     178,   179,   192,   181,   159,     0,     0,     0,   185,   163,
       0,     0,     0,     0,     0,  -259,   169,     0,   190,   170,
       0,   191,     0,     0,   192,     0,     0,   178,   179,   159,
     181,     0,     0,   185,   163,     0,     0,     0,     0,     0,
       0,   169,     0,     0,   170,     0,     0,     0,   190,     0,
     185,   191,   178,   179,   192,   181,  -259,     0,     0,     0,
       0,   163,     0,     0,     0,     0,     0,     0,   169,     0,
       0,   170,     0,   190,     0,     0,   191,   185,     0,   178,
     179,     0,   181,     0,     0,     0,     0,     0,     0,     0,
     190,     0,     0,   191,     0,     0,     0,     0,     0,     0,
       0,     0,   185,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   190,     0,     0,
     191,     0,     0,     0,     0,     0,     0,     0,     0,   185,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  -259,     0,     0,   191,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   191
};

static const yytype_int16 yycheck[] =
{
       0,    17,   349,   129,   137,    15,    15,    24,   242,   142,
      10,   333,    12,    13,    14,   502,    16,    17,    18,    17,
      46,    21,    22,    54,    24,     3,     4,    27,     4,    29,
      30,   553,    10,    17,    10,    13,    17,    13,    38,    17,
      14,    14,    14,     4,    51,   414,    46,    47,    48,    49,
      26,    51,    52,    18,    38,   577,    56,    38,   545,    59,
      60,    15,    47,    63,    74,    74,     4,    83,    46,    67,
      55,    44,    46,     3,     4,    29,    76,    77,    78,    79,
      10,    46,     4,    13,     3,     4,    86,    17,   410,    89,
       4,     3,     4,    67,    13,    67,    96,    44,    98,    99,
     100,    13,   109,   472,    58,    17,   106,    17,     3,   119,
     110,   128,    26,    60,   114,    15,    46,   117,   118,    17,
     120,   121,    17,   123,   124,   125,   126,    46,   254,    29,
     130,   162,   265,    51,    46,   369,    17,   137,   485,    76,
      38,   141,   142,   121,    10,    45,    46,   147,    14,   149,
      15,   151,    17,    10,   154,   133,   156,   133,    54,    55,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
      17,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   184,    17,   217,   187,   188,   189,
     190,   121,   192,   193,   194,    59,    44,    61,    17,    47,
     118,   201,   121,     3,     4,    17,    38,    55,   442,   121,
      17,   211,    38,    13,    28,     4,     5,    17,   218,    59,
      60,    10,    36,    63,    17,     0,   460,    17,   228,   229,
     230,   231,    17,     4,    67,    49,    41,   237,    46,    17,
     240,    55,    72,   243,    88,   163,    46,   247,   381,   382,
     383,    17,   385,   109,    17,   122,    77,    46,   258,   259,
      17,   261,   496,    67,     3,    56,   266,     4,   268,     3,
       4,    44,     6,   273,   508,   193,    10,   277,    81,    13,
      14,    81,    17,     3,    17,    19,    18,    21,    22,    18,
      18,    24,    46,    51,    27,    76,    60,    79,    24,    18,
     300,    18,    35,    36,    18,    38,   337,   121,    18,    17,
      41,    18,    46,   313,    18,   448,    24,    44,    29,    18,
      50,   121,     3,    53,    54,     3,   357,    35,   328,   247,
      38,    77,    62,    51,    18,   109,    45,   337,   338,    45,
      41,   259,   109,   182,   109,   345,   479,   109,   362,   349,
     268,   427,    85,   472,   354,   355,   578,   357,   358,   359,
      57,   484,    -1,    -1,   364,    -1,   180,   367,    -1,    -1,
      -1,    -1,   186,    -1,    -1,    -1,    -1,    85,    -1,    -1,
     114,   381,   382,   383,    -1,   115,    -1,   121,    -1,   389,
     421,   125,    -1,   393,   394,   128,    -1,    -1,   398,   399,
     214,    -1,    -1,    -1,    -1,   405,   406,   407,   408,   542,
     328,   411,   412,    -1,    -1,   415,    -1,   417,    -1,    -1,
     128,    -1,   152,    -1,    -1,    -1,    -1,    -1,   459,   429,
      -1,   431,    -1,    -1,    -1,    -1,    -1,   570,    -1,    -1,
      -1,    -1,    -1,    -1,   475,    -1,    -1,   261,   448,    -1,
      -1,   451,   266,    -1,    -1,    -1,    -1,   457,    -1,   459,
      -1,    -1,   493,    -1,    -1,    -1,   599,    -1,   601,    -1,
      -1,   604,    -1,   606,    -1,   475,    -1,   477,    -1,   479,
     398,    -1,    -1,    -1,   484,   485,   486,   217,    -1,   489,
      -1,   522,    -1,   223,   412,   495,    -1,    -1,   498,    -1,
      -1,   501,    -1,   503,   234,   235,    -1,    -1,    -1,    -1,
      -1,   241,    -1,    -1,   514,    -1,    -1,    -1,    -1,   333,
      -1,    -1,    -1,    -1,   555,    -1,   557,    -1,    -1,    -1,
      -1,    -1,    -1,   263,   264,   265,   536,    -1,    -1,   539,
      -1,   541,   542,    -1,    -1,   275,    -1,    -1,   362,    -1,
     550,    -1,    -1,   553,    -1,    -1,    -1,   557,   558,   559,
     560,   561,    -1,   563,    -1,   565,   566,   567,    -1,   569,
     570,    -1,    -1,   573,    -1,    -1,    -1,   577,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   587,    -1,   589,
      -1,    -1,    -1,    -1,    -1,    -1,   410,    -1,    -1,   599,
      -1,   601,    -1,   417,   604,    -1,   606,    -1,     8,    -1,
      -1,    -1,    12,    -1,    14,    15,    16,    17,    -1,    19,
      20,    21,    22,    23,    24,    -1,    -1,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    -1,
      -1,    -1,    -1,    -1,    44,    45,    -1,   565,    -1,   567,
      -1,    -1,    -1,    -1,    -1,   385,    -1,    -1,   388,    -1,
      -1,    -1,    -1,    12,    -1,    -1,    -1,    67,    17,   587,
      -1,   589,    21,    22,    23,    24,    -1,    -1,    27,    -1,
      -1,    -1,    -1,    -1,    -1,    85,    35,    36,    -1,    38,
     420,    -1,    -1,   423,   508,    -1,   426,    -1,    -1,    -1,
     430,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   441,   442,    -1,   114,    -1,    -1,   117,    -1,    -1,
     120,    -1,   452,    -1,   124,   125,    -1,   541,   128,    -1,
      12,   131,    -1,    -1,    -1,    17,    85,    19,    20,    21,
      22,    23,    24,    -1,    -1,    27,    28,    29,    30,    31,
      32,    33,   566,    35,    36,   569,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   494,    -1,    -1,   497,    -1,   499,
      -1,    -1,    -1,    -1,    -1,    -1,   125,    -1,    -1,   128,
      -1,   511,    -1,    -1,    -1,    -1,    -1,    -1,   518,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   527,    -1,    -1,
      -1,   531,   532,    85,   534,    -1,    -1,    -1,   538,    -1,
      -1,    -1,    -1,   543,    -1,    -1,    -1,    -1,    -1,   549,
      -1,    -1,    -1,    -1,   554,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   125,    -1,    -1,   128,    -1,    -1,   131,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   595,    -1,   597,    -1,    -1,
     600,     1,   602,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    -1,
      40,    41,    -1,    -1,    44,    45,    46,    47,    -1,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    -1,    65,    66,    67,    -1,    69,
      70,    71,    -1,    73,    74,    75,    -1,    77,    78,    -1,
      80,    -1,    82,    83,    84,    85,    86,    87,    -1,    89,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    99,
     100,   101,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,    -1,
     120,   121,   122,   123,   124,   125,    -1,   127,   128,   129,
     130,   131,   132,   133,     1,    -1,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    -1,    40,    41,    -1,    -1,    44,    45,    46,
      47,    -1,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    -1,    65,    66,
      67,    -1,    69,    70,    71,    -1,    73,    74,    75,    -1,
      77,    78,    -1,    80,    -1,    82,    83,    84,    85,    86,
      87,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    99,   100,   101,    -1,    -1,    -1,    -1,    -1,
      -1,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,    -1,   120,   121,   122,   123,   124,   125,    -1,
     127,   128,   129,   130,   131,   132,   133,     1,    -1,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    -1,    40,    41,    -1,    -1,
      44,    45,    46,    47,    -1,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      -1,    65,    66,    67,    -1,    69,    70,    71,    -1,    73,
      74,    75,    -1,    77,    78,    -1,    80,    -1,    82,    83,
      84,    85,    86,    87,    -1,    89,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    99,   100,   101,    -1,    -1,
      -1,    -1,    -1,    -1,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,    -1,   120,   121,   122,   123,
     124,   125,    -1,   127,   128,   129,   130,   131,   132,   133,
       1,    -1,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    -1,    40,
      41,    -1,    -1,    44,    45,    46,    47,    -1,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    -1,    65,    66,    67,    -1,    69,    70,
      71,    -1,    73,    74,    75,    -1,    77,    -1,    -1,    80,
      -1,    82,    83,    84,    85,    86,    87,    -1,    89,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    99,   100,
     101,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,    -1,   120,
     121,   122,   123,   124,   125,    -1,   127,   128,   129,   130,
     131,   132,   133,     1,    -1,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    -1,    40,    41,    -1,    -1,    44,    45,    46,    47,
      -1,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    -1,    65,    66,    67,
      -1,    69,    70,    71,    -1,    73,    74,    75,    -1,    77,
      78,    -1,    80,    -1,    82,    83,    84,    85,    86,    87,
      -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    99,   100,   101,    -1,    -1,    -1,    -1,    -1,    -1,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,    -1,   120,   121,   122,   123,   124,   125,    -1,    -1,
     128,   129,   130,   131,   132,   133,     1,    -1,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    -1,    40,    41,    -1,    -1,    44,
      45,    46,    47,    -1,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    -1,
      65,    66,    67,    -1,    69,    70,    71,    -1,    73,    74,
      75,    -1,    77,    78,    -1,    80,    -1,    82,    83,    84,
      85,    86,    87,    -1,    89,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    99,   100,   101,    -1,    -1,    -1,
      -1,    -1,    -1,   108,    -1,   110,   111,   112,   113,   114,
     115,   116,   117,   118,    -1,   120,   121,   122,   123,   124,
     125,    -1,   127,   128,   129,   130,   131,   132,   133,     1,
      -1,     3,     4,     5,     6,     7,    -1,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    -1,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    -1,    40,    -1,
      -1,    -1,    -1,    45,    46,    47,    -1,    -1,    50,    -1,
      52,    53,    54,    -1,    -1,    57,    -1,    -1,    -1,    -1,
      62,    63,    -1,    65,    -1,    67,    -1,    69,    70,    71,
      -1,    73,    74,    75,    76,    -1,    78,    -1,    80,    -1,
      82,    83,    84,    85,    86,    87,    -1,    89,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,   100,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   108,   109,    -1,   111,
     112,   113,   114,   115,   116,   117,   118,    -1,   120,   121,
     122,   123,   124,   125,    -1,   127,   128,   129,   130,   131,
     132,   133,     1,    -1,     3,     4,     5,     6,     7,    -1,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    -1,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      -1,    40,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,
      -1,    -1,    -1,    52,    53,    54,    -1,    -1,    57,    -1,
      -1,    60,    -1,    62,    63,    -1,    65,    -1,    67,    -1,
      69,    70,    71,    -1,    73,    74,    75,    76,    -1,    78,
      -1,    80,    -1,    82,    83,    84,    85,    86,    87,    -1,
      89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,
     109,    -1,   111,   112,   113,   114,   115,   116,   117,   118,
      -1,   120,   121,   122,   123,   124,   125,    -1,   127,   128,
     129,   130,   131,   132,   133,     1,    -1,     3,     4,     5,
       6,     7,    -1,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    -1,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    -1,    40,    -1,    -1,    -1,    -1,    45,
      46,    47,    -1,    -1,    -1,    -1,    52,    53,    54,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    62,    63,    -1,    65,
      -1,    67,    -1,    69,    70,    71,    -1,    73,    74,    75,
      -1,    -1,    78,    -1,    80,    -1,    82,    83,    84,    85,
      86,    87,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    98,    -1,   100,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   108,   109,    -1,   111,   112,   113,   114,   115,
     116,   117,   118,    -1,   120,   121,   122,   123,   124,   125,
      -1,   127,   128,   129,   130,   131,   132,   133,     1,    -1,
       3,     4,     5,     6,     7,    -1,     9,    10,    11,    12,
      13,    -1,    -1,    -1,    17,    -1,    19,    20,    -1,    -1,
      -1,    -1,    25,    26,    -1,    -1,    29,    30,    -1,    -1,
      -1,    -1,    35,    36,    37,    -1,    -1,    40,    -1,    -1,
      -1,    44,    -1,    46,    47,    -1,    49,    -1,    51,    52,
      53,    54,    -1,    -1,    57,    -1,    -1,    -1,    -1,    62,
      63,    -1,    65,    -1,    -1,    -1,    69,    70,    71,    -1,
      73,    74,    75,    -1,    77,    78,    -1,    80,    -1,    82,
      83,    84,    -1,    86,    87,    -1,    89,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,   100,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   108,   109,    -1,   111,   112,
     113,    -1,   115,   116,    -1,   118,    -1,    -1,   121,   122,
     123,    -1,   125,    -1,   127,    -1,   129,   130,    -1,   132,
     133,     1,    -1,     3,     4,     5,     6,     7,    -1,     9,
      10,    11,    12,    13,    -1,    -1,    -1,    17,    -1,    19,
      20,    -1,    -1,    -1,    -1,    25,    26,    -1,    -1,    29,
      30,    -1,    -1,    -1,    -1,    35,    36,    37,    -1,    -1,
      40,    -1,    -1,    -1,    44,    -1,    46,    47,    -1,    -1,
      -1,    51,    52,    53,    54,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    62,    63,    -1,    65,    -1,    -1,    -1,    69,
      70,    71,    -1,    73,    74,    75,    -1,    77,    78,    -1,
      80,    -1,    82,    83,    84,    -1,    86,    87,    -1,    89,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
     100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,
      -1,   111,   112,   113,    -1,   115,   116,    -1,   118,    -1,
      -1,   121,   122,   123,    -1,   125,    -1,   127,    -1,   129,
     130,    -1,   132,   133,     1,    -1,     3,     4,     5,     6,
       7,    -1,     9,    10,    11,    12,    13,    -1,    -1,    -1,
      17,    -1,    19,    20,    -1,    -1,    -1,    -1,    25,    26,
      -1,    -1,    29,    30,    -1,    -1,    -1,    -1,    35,    36,
      37,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    46,
      47,    -1,    49,    -1,    51,    52,    53,    54,    -1,    -1,
      57,    -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,
      -1,    -1,    69,    70,    71,    -1,    73,    74,    75,    -1,
      77,    78,    -1,    80,    -1,    82,    83,    84,    -1,    86,
      87,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   108,   109,    -1,   111,   112,   113,    -1,   115,   116,
      -1,   118,    -1,    -1,   121,   122,   123,    -1,   125,    -1,
     127,    -1,   129,   130,    -1,   132,   133,     1,    -1,     3,
       4,     5,     6,     7,    -1,     9,    10,    11,    12,    13,
      -1,    -1,    -1,    17,    -1,    19,    20,    -1,    -1,    -1,
      -1,    25,    26,    -1,    -1,    29,    30,    -1,    -1,    -1,
      -1,    35,    36,    37,    -1,    -1,    40,    -1,    -1,    -1,
      -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,    53,
      54,    -1,    -1,    57,    -1,    -1,    -1,    -1,    62,    63,
      -1,    65,    -1,    -1,    -1,    69,    70,    71,    -1,    73,
      74,    75,    76,    -1,    78,    79,    80,    81,    82,    83,
      84,    -1,    86,    87,    -1,    89,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,   100,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   108,   109,    -1,   111,   112,   113,
      -1,   115,   116,    -1,   118,    -1,    -1,   121,   122,   123,
      -1,   125,    -1,   127,    -1,   129,   130,    -1,   132,   133,
       1,    -1,     3,     4,     5,     6,     7,    -1,     9,    10,
      11,    12,    13,    -1,    -1,    -1,    17,    -1,    19,    20,
      -1,    -1,    -1,    -1,    25,    26,    -1,    -1,    29,    30,
      -1,    -1,    -1,    -1,    35,    36,    37,    -1,    -1,    40,
      -1,    -1,    -1,    44,    -1,    46,    47,    -1,    -1,    -1,
      -1,    52,    53,    54,    55,    56,    57,    -1,    -1,    -1,
      -1,    62,    63,    -1,    65,    -1,    -1,    -1,    69,    70,
      71,    -1,    73,    74,    75,    -1,    -1,    78,    -1,    80,
      -1,    82,    83,    84,    -1,    86,    87,    -1,    89,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,   100,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,    -1,
     111,   112,   113,    -1,   115,   116,    -1,   118,    -1,    -1,
     121,   122,   123,    -1,   125,    -1,   127,    -1,   129,   130,
      -1,   132,   133,     1,    -1,     3,     4,     5,     6,     7,
      -1,     9,    10,    11,    12,    13,    -1,    -1,    -1,    17,
      -1,    19,    20,    -1,    -1,    -1,    -1,    25,    26,    -1,
      -1,    29,    30,    -1,    -1,    -1,    -1,    35,    36,    37,
      -1,    -1,    40,    -1,    -1,    -1,    44,    -1,    46,    47,
      -1,    -1,    -1,    51,    52,    53,    54,    -1,    -1,    57,
      -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,    -1,
      -1,    69,    70,    71,    -1,    73,    74,    75,    -1,    77,
      78,    -1,    80,    -1,    82,    83,    84,    -1,    86,    87,
      -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     108,   109,    -1,   111,   112,   113,    -1,   115,   116,    -1,
     118,    -1,    -1,   121,   122,   123,    -1,   125,    -1,   127,
      -1,   129,   130,    -1,   132,   133,     1,    -1,     3,     4,
       5,     6,     7,    -1,     9,    10,    11,    12,    13,    -1,
      -1,    -1,    17,    -1,    19,    20,    -1,    -1,    -1,    -1,
      25,    26,    -1,    -1,    29,    30,    -1,    -1,    -1,    -1,
      35,    36,    37,    -1,    -1,    40,    -1,    -1,    -1,    44,
      -1,    46,    47,    -1,    -1,    -1,    -1,    52,    53,    54,
      -1,    -1,    57,    -1,    -1,    -1,    -1,    62,    63,    -1,
      65,    -1,    -1,    -1,    69,    70,    71,    -1,    73,    74,
      75,    -1,    77,    78,    -1,    80,    -1,    82,    83,    84,
      -1,    86,    87,    -1,    89,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    -1,   100,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   108,   109,   110,   111,   112,   113,    -1,
     115,   116,    -1,   118,    -1,    -1,   121,   122,   123,    -1,
     125,    -1,   127,    -1,   129,   130,    -1,   132,   133,     1,
      -1,     3,     4,     5,     6,     7,    -1,     9,    10,    11,
      12,    13,    -1,    -1,    -1,    17,    -1,    19,    20,    -1,
      -1,    -1,    -1,    25,    26,    -1,    -1,    29,    30,    -1,
      -1,    -1,    -1,    35,    36,    37,    -1,    -1,    40,    -1,
      -1,    -1,    44,    -1,    46,    47,    -1,    -1,    -1,    -1,
      52,    53,    54,    -1,    -1,    57,    -1,    -1,    -1,    -1,
      62,    63,    -1,    65,    66,    -1,    -1,    69,    70,    71,
      -1,    73,    74,    75,    -1,    -1,    78,    -1,    80,    -1,
      82,    83,    84,    -1,    86,    87,    -1,    89,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,   100,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   108,   109,    -1,   111,
     112,   113,    -1,   115,   116,    -1,   118,    -1,    -1,   121,
     122,   123,    -1,   125,    -1,   127,    -1,   129,   130,    -1,
     132,   133,     1,    -1,     3,     4,     5,     6,     7,    -1,
       9,    10,    11,    12,    13,    -1,    -1,    -1,    17,    -1,
      19,    20,    -1,    -1,    -1,    -1,    25,    26,    -1,    -1,
      29,    30,    -1,    -1,    -1,    -1,    35,    36,    37,    -1,
      -1,    40,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,
      -1,    -1,    -1,    52,    53,    54,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    62,    63,    -1,    65,    -1,    -1,    -1,
      69,    70,    71,    -1,    73,    74,    75,    -1,    -1,    78,
      79,    80,    81,    82,    83,    84,    -1,    86,    87,    -1,
      89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,
     109,    -1,   111,   112,   113,    -1,   115,   116,    -1,   118,
      -1,    -1,   121,   122,   123,    -1,   125,    -1,   127,    -1,
     129,   130,    -1,   132,   133,     1,    -1,     3,     4,     5,
       6,     7,    -1,     9,    10,    11,    12,    13,    -1,    -1,
      -1,    17,    -1,    19,    20,    -1,    -1,    -1,    -1,    25,
      26,    -1,    -1,    29,    30,    -1,    -1,    -1,    -1,    35,
      36,    37,    -1,    -1,    40,    -1,    -1,    -1,    44,    -1,
      46,    47,    -1,    -1,    -1,    -1,    52,    53,    54,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    62,    63,    -1,    65,
      -1,    -1,    -1,    69,    70,    71,    -1,    73,    74,    75,
      -1,    77,    78,    -1,    80,    -1,    82,    83,    84,    -1,
      86,    87,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    98,    -1,   100,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   108,   109,    -1,   111,   112,   113,    -1,   115,
     116,    -1,   118,    -1,    -1,   121,   122,   123,    -1,   125,
      -1,   127,    -1,   129,   130,    -1,   132,   133,     1,    -1,
       3,     4,     5,     6,     7,    -1,     9,    10,    11,    12,
      13,    -1,    -1,    -1,    17,    -1,    19,    20,    -1,    -1,
      -1,    -1,    25,    26,    -1,    -1,    29,    30,    -1,    -1,
      -1,    -1,    35,    36,    37,    -1,    -1,    40,    -1,    -1,
      -1,    44,    -1,    46,    47,    -1,    -1,    -1,    51,    52,
      53,    54,    -1,    -1,    57,    -1,    -1,    -1,    -1,    62,
      63,    -1,    65,    -1,    -1,    -1,    69,    70,    71,    -1,
      73,    74,    75,    -1,    -1,    78,    -1,    80,    -1,    82,
      83,    84,    -1,    86,    87,    -1,    89,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,   100,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   108,   109,    -1,   111,   112,
     113,    -1,   115,   116,    -1,   118,    -1,    -1,   121,   122,
     123,    -1,   125,    -1,   127,    -1,   129,   130,    -1,   132,
     133,     1,    -1,     3,     4,     5,     6,     7,    -1,     9,
      10,    11,    12,    13,    -1,    -1,    -1,    17,    -1,    19,
      20,    -1,    -1,    -1,    -1,    25,    26,    -1,    -1,    29,
      30,    -1,    -1,    -1,    -1,    35,    36,    37,    -1,    -1,
      40,    -1,    -1,    -1,    44,    -1,    46,    47,    -1,    -1,
      50,    -1,    52,    53,    54,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    62,    63,    -1,    65,    -1,    -1,    -1,    69,
      70,    71,    -1,    73,    74,    75,    -1,    -1,    78,    -1,
      80,    -1,    82,    83,    84,    -1,    86,    87,    -1,    89,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
     100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,
      -1,   111,   112,   113,    -1,   115,   116,    -1,   118,    -1,
      -1,   121,   122,   123,    -1,   125,    -1,   127,    -1,   129,
     130,    -1,   132,   133,     1,    -1,     3,     4,     5,     6,
       7,    -1,     9,    10,    11,    12,    13,    -1,    -1,    -1,
      17,    -1,    19,    20,    -1,    -1,    -1,    -1,    25,    26,
      -1,    -1,    29,    30,    -1,    -1,    -1,    -1,    35,    36,
      37,    -1,    -1,    40,    -1,    -1,    -1,    44,    -1,    46,
      47,    -1,    -1,    -1,    -1,    52,    53,    54,    -1,    -1,
      57,    -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,
      -1,    -1,    69,    70,    71,    -1,    73,    74,    75,    -1,
      77,    78,    -1,    80,    -1,    82,    83,    84,    -1,    86,
      87,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   108,   109,    -1,   111,   112,   113,    -1,   115,   116,
      -1,   118,    -1,    -1,   121,   122,   123,    -1,   125,    -1,
     127,    -1,   129,   130,    -1,   132,   133,     1,    -1,     3,
       4,     5,     6,     7,    -1,     9,    10,    11,    12,    13,
      -1,    -1,    -1,    17,    -1,    19,    20,    -1,    -1,    -1,
      -1,    25,    26,    -1,    -1,    29,    30,    -1,    -1,    -1,
      -1,    35,    36,    37,    -1,    -1,    40,    -1,    -1,    -1,
      44,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,    53,
      54,    -1,    -1,    57,    -1,    -1,    -1,    -1,    62,    63,
      -1,    65,    -1,    -1,    -1,    69,    70,    71,    -1,    73,
      74,    75,    -1,    77,    78,    -1,    80,    -1,    82,    83,
      84,    -1,    86,    87,    -1,    89,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,   100,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   108,   109,    -1,   111,   112,   113,
      -1,   115,   116,    -1,   118,    -1,    -1,   121,   122,   123,
      -1,   125,    -1,   127,    -1,   129,   130,    -1,   132,   133,
       1,    -1,     3,     4,     5,     6,     7,    -1,     9,    10,
      11,    12,    13,    -1,    -1,    -1,    17,    -1,    19,    20,
      -1,    -1,    -1,    -1,    25,    26,    -1,    -1,    29,    30,
      -1,    -1,    -1,    -1,    35,    36,    37,    -1,    -1,    40,
      -1,    -1,    -1,    44,    -1,    46,    47,    -1,    -1,    -1,
      -1,    52,    53,    54,    -1,    56,    57,    -1,    -1,    -1,
      -1,    62,    63,    -1,    65,    -1,    -1,    -1,    69,    70,
      71,    -1,    73,    74,    75,    -1,    -1,    78,    -1,    80,
      -1,    82,    83,    84,    -1,    86,    87,    -1,    89,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,   100,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,    -1,
     111,   112,   113,    -1,   115,   116,    -1,   118,    -1,    -1,
     121,   122,   123,    -1,   125,    -1,   127,    -1,   129,   130,
      -1,   132,   133,     1,    -1,     3,     4,     5,     6,     7,
      -1,     9,    10,    11,    12,    13,    -1,    -1,    -1,    17,
      -1,    19,    20,    -1,    -1,    -1,    -1,    25,    26,    -1,
      -1,    29,    30,    -1,    -1,    -1,    -1,    35,    36,    37,
      -1,    -1,    40,    -1,    -1,    -1,    44,    -1,    46,    47,
      -1,    -1,    -1,    -1,    52,    53,    54,    -1,    -1,    57,
      -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,    -1,
      -1,    69,    70,    71,    -1,    73,    74,    75,    -1,    77,
      78,    -1,    80,    -1,    82,    83,    84,    -1,    86,    87,
      -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     108,   109,    -1,   111,   112,   113,    -1,   115,   116,    -1,
     118,    -1,    -1,   121,   122,   123,    -1,   125,    -1,   127,
      -1,   129,   130,    -1,   132,   133,     1,    -1,     3,     4,
       5,     6,     7,    -1,     9,    10,    11,    12,    13,    -1,
      -1,    -1,    17,    -1,    19,    20,    -1,    -1,    -1,    -1,
      25,    26,    -1,    -1,    29,    30,    -1,    -1,    -1,    -1,
      35,    36,    37,    -1,    -1,    40,    -1,    -1,    -1,    44,
      -1,    46,    47,    -1,    -1,    -1,    -1,    52,    53,    54,
      -1,    -1,    57,    -1,    -1,    -1,    -1,    62,    63,    -1,
      65,    -1,    -1,    -1,    69,    70,    71,    -1,    73,    74,
      75,    -1,    77,    78,    -1,    80,    -1,    82,    83,    84,
      -1,    86,    87,    -1,    89,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    -1,   100,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   108,   109,    -1,   111,   112,   113,    -1,
     115,   116,    -1,   118,    -1,    -1,   121,   122,   123,    -1,
     125,    -1,   127,    -1,   129,   130,    -1,   132,   133,     1,
      -1,     3,     4,     5,     6,     7,    -1,     9,    10,    11,
      12,    13,    -1,    -1,    -1,    17,    -1,    19,    20,    -1,
      -1,    -1,    -1,    25,    26,    -1,    -1,    29,    30,    -1,
      -1,    -1,    -1,    35,    36,    37,    -1,    -1,    40,    -1,
      -1,    -1,    44,    -1,    46,    47,    -1,    -1,    -1,    -1,
      52,    53,    54,    -1,    -1,    57,    -1,    -1,    -1,    -1,
      62,    63,    -1,    65,    -1,    -1,    -1,    69,    70,    71,
      -1,    73,    74,    75,    -1,    77,    78,    -1,    80,    -1,
      82,    83,    84,    -1,    86,    87,    -1,    89,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,   100,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   108,   109,    -1,   111,
     112,   113,    -1,   115,   116,    -1,   118,    -1,    -1,   121,
     122,   123,    -1,   125,    -1,   127,    -1,   129,   130,    -1,
     132,   133,     1,    -1,     3,     4,     5,     6,     7,    -1,
       9,    10,    11,    12,    13,    -1,    -1,    -1,    17,    -1,
      19,    20,    -1,    -1,    -1,    -1,    25,    26,    -1,    -1,
      29,    30,    -1,    -1,    -1,    -1,    35,    36,    37,    -1,
      -1,    40,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,
      -1,    -1,    -1,    52,    53,    54,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    62,    63,    -1,    65,    -1,    -1,    -1,
      69,    70,    71,    -1,    73,    74,    75,    76,    -1,    78,
      79,    80,    -1,    82,    83,    84,    -1,    86,    87,    -1,
      89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,
     109,    -1,   111,   112,   113,    -1,   115,   116,    -1,   118,
      -1,    -1,   121,   122,   123,    -1,   125,    -1,   127,    -1,
     129,   130,    -1,   132,   133,     1,    -1,     3,     4,     5,
       6,     7,    -1,     9,    10,    11,    12,    13,    -1,    -1,
      -1,    17,    -1,    19,    20,    -1,    -1,    -1,    -1,    25,
      26,    -1,    -1,    29,    30,    -1,    -1,    -1,    -1,    35,
      36,    37,    -1,    -1,    40,    -1,    -1,    -1,    44,    -1,
      46,    47,    -1,    -1,    -1,    -1,    52,    53,    54,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    62,    63,    -1,    65,
      -1,    -1,    -1,    69,    70,    71,    -1,    73,    74,    75,
      -1,    77,    78,    -1,    80,    -1,    82,    83,    84,    -1,
      86,    87,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    98,    -1,   100,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   108,   109,    -1,   111,   112,   113,    -1,   115,
     116,    -1,   118,    -1,    -1,   121,   122,   123,    -1,   125,
      -1,   127,    -1,   129,   130,    -1,   132,   133,     1,    -1,
       3,     4,     5,     6,     7,    -1,     9,    10,    11,    12,
      13,    -1,    -1,    -1,    17,    -1,    19,    20,    -1,    -1,
      -1,    -1,    25,    26,    -1,    -1,    29,    30,    -1,    -1,
      -1,    -1,    35,    36,    37,    -1,    -1,    40,    -1,    -1,
      -1,    44,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,
      53,    54,    -1,    -1,    57,    -1,    -1,    -1,    -1,    62,
      63,    -1,    65,    -1,    -1,    -1,    69,    70,    71,    -1,
      73,    74,    75,    -1,    77,    78,    -1,    80,    -1,    82,
      83,    84,    -1,    86,    87,    -1,    89,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,   100,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   108,   109,    -1,   111,   112,
     113,    -1,   115,   116,    -1,   118,    -1,    -1,   121,   122,
     123,    -1,   125,    -1,   127,    -1,   129,   130,    -1,   132,
     133,     1,    -1,     3,     4,     5,     6,     7,    -1,     9,
      10,    11,    12,    13,    -1,    -1,    -1,    17,    -1,    19,
      20,    -1,    -1,    -1,    -1,    25,    26,    -1,    -1,    29,
      30,    -1,    -1,    -1,    -1,    35,    36,    37,    -1,    -1,
      40,    -1,    -1,    -1,    44,    -1,    46,    47,    -1,    -1,
      -1,    -1,    52,    53,    54,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    62,    63,    -1,    65,    -1,    -1,    -1,    69,
      70,    71,    -1,    73,    74,    75,    -1,    77,    78,    -1,
      80,    -1,    82,    83,    84,    -1,    86,    87,    -1,    89,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
     100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,
      -1,   111,   112,   113,    -1,   115,   116,    -1,   118,    -1,
      -1,   121,   122,   123,    -1,   125,    -1,   127,    -1,   129,
     130,    -1,   132,   133,     1,    -1,     3,     4,     5,     6,
       7,    -1,     9,    10,    11,    12,    13,    -1,    -1,    -1,
      17,    -1,    19,    20,    -1,    -1,    -1,    -1,    25,    26,
      -1,    -1,    29,    30,    -1,    -1,    -1,    -1,    35,    36,
      37,    -1,    -1,    40,    -1,    -1,    -1,    44,    -1,    46,
      47,    -1,    -1,    -1,    -1,    52,    53,    54,    -1,    -1,
      57,    -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,
      -1,    -1,    69,    70,    71,    -1,    73,    74,    75,    -1,
      77,    78,    -1,    80,    -1,    82,    83,    84,    -1,    86,
      87,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   108,   109,    -1,   111,   112,   113,    -1,   115,   116,
      -1,   118,    -1,    -1,   121,   122,   123,    -1,   125,    -1,
     127,    -1,   129,   130,    -1,   132,   133,     1,    -1,     3,
       4,     5,     6,     7,    -1,     9,    10,    11,    12,    13,
      -1,    -1,    -1,    17,    -1,    19,    20,    -1,    -1,    -1,
      -1,    25,    26,    -1,    -1,    29,    30,    -1,    -1,    -1,
      -1,    35,    36,    37,    -1,    -1,    40,    -1,    -1,    -1,
      44,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,    53,
      54,    -1,    -1,    57,    -1,    -1,    -1,    -1,    62,    63,
      -1,    65,    -1,    -1,    -1,    69,    70,    71,    -1,    73,
      74,    75,    -1,    77,    78,    -1,    80,    -1,    82,    83,
      84,    -1,    86,    87,    -1,    89,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,   100,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   108,   109,    -1,   111,   112,   113,
      -1,   115,   116,    -1,   118,    -1,    -1,   121,   122,   123,
      -1,   125,    -1,   127,    -1,   129,   130,    -1,   132,   133,
       1,    -1,     3,     4,     5,     6,     7,    -1,     9,    10,
      11,    12,    13,    -1,    -1,    -1,    17,    -1,    19,    20,
      -1,    -1,    -1,    -1,    25,    26,    -1,    -1,    29,    30,
      -1,    -1,    -1,    -1,    35,    36,    37,    -1,    -1,    40,
      -1,    -1,    -1,    44,    -1,    46,    47,    -1,    -1,    -1,
      -1,    52,    53,    54,    -1,    -1,    57,    -1,    -1,    -1,
      -1,    62,    63,    -1,    65,    -1,    -1,    -1,    69,    70,
      71,    -1,    73,    74,    75,    -1,    77,    78,    -1,    80,
      -1,    82,    83,    84,    -1,    86,    87,    -1,    89,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,   100,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,    -1,
     111,   112,   113,    -1,   115,   116,    -1,   118,    -1,    -1,
     121,   122,   123,    -1,   125,    -1,   127,    -1,   129,   130,
      -1,   132,   133,     1,    -1,     3,     4,     5,     6,     7,
      -1,     9,    10,    11,    12,    13,    -1,    -1,    -1,    17,
      -1,    19,    20,    -1,    -1,    -1,    -1,    25,    26,    -1,
      -1,    29,    30,    -1,    -1,    -1,    -1,    35,    36,    37,
      -1,    -1,    40,    -1,    -1,    -1,    44,    -1,    46,    47,
      -1,    -1,    -1,    -1,    52,    53,    54,    -1,    -1,    57,
      -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,    -1,
      -1,    69,    70,    71,    -1,    73,    74,    75,    -1,    77,
      78,    -1,    80,    -1,    82,    83,    84,    -1,    86,    87,
      -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     108,   109,    -1,   111,   112,   113,    -1,   115,   116,    -1,
     118,    -1,    -1,   121,   122,   123,    -1,   125,    -1,   127,
      -1,   129,   130,    -1,   132,   133,     1,    -1,     3,     4,
       5,     6,     7,    -1,     9,    10,    11,    12,    13,    -1,
      -1,    -1,    17,    -1,    19,    20,    -1,    -1,    -1,    -1,
      25,    26,    -1,    -1,    29,    30,    -1,    -1,    -1,    -1,
      35,    36,    37,    -1,    -1,    40,    -1,    -1,    -1,    44,
      -1,    46,    47,    -1,    -1,    -1,    -1,    52,    53,    54,
      -1,    -1,    57,    -1,    -1,    -1,    -1,    62,    63,    -1,
      65,    -1,    -1,    -1,    69,    70,    71,    -1,    73,    74,
      75,    -1,    77,    78,    -1,    80,    -1,    82,    83,    84,
      -1,    86,    87,    -1,    89,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    -1,   100,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   108,   109,    -1,   111,   112,   113,    -1,
     115,   116,    -1,   118,    -1,    -1,   121,   122,   123,    -1,
     125,    -1,   127,    -1,   129,   130,    -1,   132,   133,     1,
      -1,     3,     4,     5,     6,     7,    -1,     9,    10,    11,
      12,    13,    -1,    -1,    -1,    17,    -1,    19,    20,    -1,
      -1,    -1,    -1,    25,    26,    -1,    -1,    29,    30,    -1,
      -1,    -1,    -1,    35,    36,    37,    -1,    -1,    40,    -1,
      -1,    -1,    44,    -1,    46,    47,    -1,    -1,    -1,    -1,
      52,    53,    54,    -1,    -1,    57,    -1,    -1,    -1,    -1,
      62,    63,    -1,    65,    -1,    -1,    -1,    69,    70,    71,
      -1,    73,    74,    75,    -1,    77,    78,    -1,    80,    -1,
      82,    83,    84,    -1,    86,    87,    -1,    89,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,   100,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   108,   109,    -1,   111,
     112,   113,    -1,   115,   116,    -1,   118,    -1,    -1,   121,
     122,   123,    -1,   125,    -1,   127,    -1,   129,   130,    -1,
     132,   133,     1,    -1,     3,     4,     5,     6,     7,    -1,
       9,    10,    11,    12,    13,    -1,    -1,    -1,    17,    -1,
      19,    20,    -1,    -1,    -1,    -1,    25,    26,    -1,    -1,
      29,    30,    -1,    -1,    -1,    -1,    35,    36,    37,    -1,
      -1,    40,    -1,    -1,    -1,    44,    -1,    46,    47,    -1,
      -1,    -1,    -1,    52,    53,    54,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    62,    63,    -1,    65,    -1,    -1,    -1,
      69,    70,    71,    -1,    73,    74,    75,    -1,    77,    78,
      -1,    80,    -1,    82,    83,    84,    -1,    86,    87,    -1,
      89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,
     109,    -1,   111,   112,   113,    -1,   115,   116,    -1,   118,
      -1,    -1,   121,   122,   123,    -1,   125,    -1,   127,    -1,
     129,   130,    -1,   132,   133,     1,    -1,     3,     4,     5,
       6,     7,    -1,     9,    10,    11,    12,    13,    -1,    -1,
      -1,    17,    -1,    19,    20,    -1,    -1,    -1,    -1,    25,
      26,    -1,    -1,    29,    30,    -1,    -1,    -1,    -1,    35,
      36,    37,    -1,    -1,    40,    -1,    -1,    -1,    44,    -1,
      46,    47,    -1,    -1,    -1,    -1,    52,    53,    54,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    62,    63,    -1,    65,
      -1,    -1,    -1,    69,    70,    71,    -1,    73,    74,    75,
      -1,    77,    78,    -1,    80,    -1,    82,    83,    84,    -1,
      86,    87,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    98,    -1,   100,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   108,   109,    -1,   111,   112,   113,    -1,   115,
     116,    -1,   118,    -1,    -1,   121,   122,   123,    -1,   125,
      -1,   127,    -1,   129,   130,    -1,   132,   133,     1,    -1,
       3,     4,     5,     6,     7,    -1,     9,    10,    11,    12,
      13,    -1,    -1,    -1,    17,    -1,    19,    20,    -1,    -1,
      -1,    -1,    25,    26,    -1,    -1,    29,    30,    -1,    -1,
      -1,    -1,    35,    36,    37,    -1,    -1,    40,    -1,    -1,
      -1,    44,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,
      53,    54,    -1,    -1,    57,    -1,    -1,    -1,    -1,    62,
      63,    -1,    65,    -1,    -1,    -1,    69,    70,    71,    -1,
      73,    74,    75,    -1,    77,    78,    -1,    80,    -1,    82,
      83,    84,    -1,    86,    87,    -1,    89,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,   100,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   108,   109,    -1,   111,   112,
     113,    -1,   115,   116,    -1,   118,    -1,    -1,   121,   122,
     123,    -1,   125,    -1,   127,    -1,   129,   130,    -1,   132,
     133,     1,    -1,     3,     4,     5,     6,     7,    -1,     9,
      10,    11,    12,    13,    -1,    -1,    -1,    17,    -1,    19,
      20,    -1,    -1,    -1,    -1,    25,    26,    -1,    -1,    29,
      30,    -1,    -1,    -1,    -1,    35,    36,    37,    -1,    -1,
      40,    -1,    -1,    -1,    44,    -1,    46,    47,    -1,    -1,
      -1,    -1,    52,    53,    54,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    62,    63,    -1,    65,    -1,    -1,    -1,    69,
      70,    71,    -1,    73,    74,    75,    -1,    77,    78,    -1,
      80,    -1,    82,    83,    84,    -1,    86,    87,    -1,    89,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
     100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,
      -1,   111,   112,   113,    -1,   115,   116,    -1,   118,    -1,
      -1,   121,   122,   123,    -1,   125,    -1,   127,    -1,   129,
     130,    -1,   132,   133,     1,    -1,     3,     4,     5,     6,
       7,    -1,     9,    10,    11,    12,    13,    -1,    -1,    -1,
      17,    -1,    19,    20,    -1,    -1,    -1,    -1,    25,    26,
      -1,    -1,    29,    30,    -1,    -1,    -1,    -1,    35,    36,
      37,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    46,
      47,    -1,    -1,    -1,    -1,    52,    53,    54,    -1,    -1,
      57,    -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,
      -1,    -1,    69,    70,    71,    -1,    73,    74,    75,    -1,
      77,    78,    -1,    80,    -1,    82,    83,    84,    -1,    86,
      87,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   108,   109,   110,   111,   112,   113,    -1,   115,   116,
      -1,   118,    -1,    -1,   121,   122,   123,    -1,   125,    -1,
     127,    -1,   129,   130,    -1,   132,   133,     1,    -1,     3,
       4,     5,     6,     7,    -1,     9,    10,    11,    12,    13,
      -1,    -1,    -1,    17,    -1,    19,    20,    -1,    -1,    -1,
      -1,    25,    26,    -1,    -1,    29,    30,    -1,    -1,    -1,
      -1,    35,    36,    37,    -1,    -1,    40,    -1,    -1,    -1,
      44,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,    53,
      54,    -1,    -1,    57,    -1,    -1,    -1,    -1,    62,    63,
      -1,    65,    -1,    -1,    -1,    69,    70,    71,    -1,    73,
      74,    75,    -1,    77,    78,    -1,    80,    -1,    82,    83,
      84,    -1,    86,    87,    -1,    89,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,   100,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   108,   109,    -1,   111,   112,   113,
      -1,   115,   116,    -1,   118,    -1,    -1,   121,   122,   123,
      -1,   125,    -1,   127,    -1,   129,   130,    -1,   132,   133,
       1,    -1,     3,     4,     5,     6,     7,    -1,     9,    10,
      11,    12,    13,    14,    -1,    -1,    17,    -1,    19,    20,
      -1,    -1,    -1,    -1,    25,    26,    -1,    -1,    29,    30,
      -1,    -1,    -1,    -1,    35,    36,    37,    -1,    -1,    40,
      -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,
      -1,    52,    53,    54,    -1,    -1,    57,    -1,    -1,    -1,
      -1,    62,    63,    -1,    65,    -1,    -1,    -1,    69,    70,
      71,    -1,    73,    74,    75,    -1,    -1,    78,    -1,    80,
      -1,    82,    83,    84,    -1,    86,    87,    -1,    89,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,   100,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,    -1,
     111,   112,   113,    -1,   115,   116,    -1,   118,    -1,    -1,
     121,   122,   123,    -1,   125,    -1,   127,    -1,   129,   130,
      -1,   132,   133,     1,    -1,     3,     4,     5,     6,     7,
      -1,     9,    10,    11,    12,    13,    -1,    -1,    -1,    17,
      -1,    19,    20,    -1,    -1,    -1,    -1,    25,    26,    -1,
      -1,    29,    30,    -1,    -1,    -1,    -1,    35,    36,    37,
      -1,    -1,    40,    41,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    -1,    -1,    52,    53,    54,    -1,    -1,    57,
      -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,    -1,
      -1,    69,    70,    71,    -1,    73,    74,    75,    -1,    -1,
      78,    -1,    80,    -1,    82,    83,    84,    -1,    86,    87,
      -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     108,   109,    -1,   111,   112,   113,    -1,   115,   116,    -1,
     118,    -1,    -1,   121,   122,   123,    -1,   125,    -1,   127,
      -1,   129,   130,    -1,   132,   133,     1,    -1,     3,     4,
       5,     6,     7,    -1,     9,    10,    11,    12,    13,    -1,
      -1,    -1,    17,    -1,    19,    20,    -1,    -1,    -1,    -1,
      25,    26,    -1,    -1,    29,    30,    -1,    -1,    -1,    -1,
      35,    36,    37,    -1,    -1,    40,    -1,    -1,    -1,    -1,
      -1,    46,    47,    -1,    -1,    -1,    -1,    52,    53,    54,
      -1,    -1,    57,    -1,    -1,    -1,    -1,    62,    63,    -1,
      65,    -1,    -1,    -1,    69,    70,    71,    -1,    73,    74,
      75,    76,    -1,    78,    -1,    80,    -1,    82,    83,    84,
      -1,    86,    87,    -1,    89,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    -1,   100,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   108,   109,    -1,   111,   112,   113,    -1,
     115,   116,    -1,   118,    -1,    -1,   121,   122,   123,    -1,
     125,    -1,   127,    -1,   129,   130,    -1,   132,   133,     1,
      -1,     3,     4,     5,     6,     7,    -1,     9,    10,    11,
      12,    13,    -1,    -1,    -1,    17,    18,    19,    20,    -1,
      -1,    -1,    -1,    25,    26,    -1,    -1,    29,    30,    -1,
      -1,    -1,    -1,    35,    36,    37,    -1,    -1,    40,    -1,
      -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,
      52,    53,    54,    -1,    -1,    57,    -1,    -1,    -1,    -1,
      62,    63,    -1,    65,    -1,    -1,    -1,    69,    70,    71,
      -1,    73,    74,    75,    -1,    -1,    78,    -1,    80,    -1,
      82,    83,    84,    -1,    86,    87,    -1,    89,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,   100,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   108,   109,    -1,   111,
     112,   113,    -1,   115,   116,    -1,   118,    -1,    -1,   121,
     122,   123,    -1,   125,    -1,   127,    -1,   129,   130,    -1,
     132,   133,     1,    -1,     3,     4,     5,     6,     7,    -1,
       9,    10,    11,    12,    13,    -1,    -1,    -1,    17,    18,
      19,    20,    -1,    -1,    -1,    -1,    25,    26,    -1,    -1,
      29,    30,    -1,    -1,    -1,    -1,    35,    36,    37,    -1,
      -1,    40,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,
      -1,    -1,    -1,    52,    53,    54,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    62,    63,    -1,    65,    -1,    -1,    -1,
      69,    70,    71,    -1,    73,    74,    75,    -1,    -1,    78,
      -1,    80,    -1,    82,    83,    84,    -1,    86,    87,    -1,
      89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,
     109,    -1,   111,   112,   113,    -1,   115,   116,    -1,   118,
      -1,    -1,   121,   122,   123,    -1,   125,    -1,   127,    -1,
     129,   130,    -1,   132,   133,     1,    -1,     3,     4,     5,
       6,     7,    -1,     9,    10,    11,    12,    13,    -1,    -1,
      -1,    17,    -1,    19,    20,    -1,    -1,    -1,    -1,    25,
      26,    -1,    -1,    29,    30,    -1,    -1,    -1,    -1,    35,
      36,    37,    -1,    -1,    40,    -1,    -1,    -1,    44,    -1,
      46,    47,    -1,    -1,    -1,    -1,    52,    53,    54,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    62,    63,    -1,    65,
      -1,    -1,    -1,    69,    70,    71,    -1,    73,    74,    75,
      -1,    -1,    78,    -1,    80,    -1,    82,    83,    84,    -1,
      86,    87,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    98,    -1,   100,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   108,   109,    -1,   111,   112,   113,    -1,   115,
     116,    -1,   118,    -1,    -1,   121,   122,   123,    -1,   125,
      -1,   127,    -1,   129,   130,    -1,   132,   133,     1,    -1,
       3,     4,     5,     6,     7,    -1,     9,    10,    11,    12,
      13,    -1,    -1,    -1,    17,    18,    19,    20,    -1,    -1,
      -1,    -1,    25,    26,    -1,    -1,    29,    30,    -1,    -1,
      -1,    -1,    35,    36,    37,    -1,    -1,    40,    -1,    -1,
      -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,
      53,    54,    -1,    -1,    57,    -1,    -1,    -1,    -1,    62,
      63,    -1,    65,    -1,    -1,    -1,    69,    70,    71,    -1,
      73,    74,    75,    -1,    -1,    78,    -1,    80,    -1,    82,
      83,    84,    -1,    86,    87,    -1,    89,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,   100,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   108,   109,    -1,   111,   112,
     113,    -1,   115,   116,    -1,   118,    -1,    -1,   121,   122,
     123,    -1,   125,    -1,   127,    -1,   129,   130,    -1,   132,
     133,     1,    -1,     3,     4,     5,     6,     7,    -1,     9,
      10,    11,    12,    13,    -1,    -1,    -1,    17,    18,    19,
      20,    -1,    -1,    -1,    -1,    25,    26,    -1,    -1,    29,
      30,    -1,    -1,    -1,    -1,    35,    36,    37,    -1,    -1,
      40,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,
      -1,    -1,    52,    53,    54,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    62,    63,    -1,    65,    -1,    -1,    -1,    69,
      70,    71,    -1,    73,    74,    75,    -1,    -1,    78,    -1,
      80,    -1,    82,    83,    84,    -1,    86,    87,    -1,    89,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
     100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,
      -1,   111,   112,   113,    -1,   115,   116,    -1,   118,    -1,
      -1,   121,   122,   123,    -1,   125,    -1,   127,    -1,   129,
     130,    -1,   132,   133,     1,    -1,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    -1,    -1,    -1,
      17,    -1,    19,    20,    -1,    -1,    -1,    -1,    25,    26,
      -1,    -1,    29,    30,    -1,    -1,    -1,    -1,    35,    36,
      37,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    46,
      47,    -1,    -1,    -1,    -1,    52,    53,    54,    -1,    -1,
      57,    -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,
      -1,    -1,    69,    70,    71,    -1,    73,    74,    75,    -1,
      -1,    78,    -1,    80,    -1,    82,    83,    84,    -1,    86,
      87,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   108,   109,    -1,   111,   112,   113,    -1,   115,   116,
      -1,   118,    -1,    -1,   121,   122,   123,    -1,   125,    -1,
     127,    -1,   129,   130,    -1,   132,   133,     1,    -1,     3,
       4,     5,     6,     7,    -1,     9,    10,    11,    12,    13,
      -1,    -1,    -1,    17,    -1,    19,    20,    -1,    -1,    -1,
      -1,    25,    26,    -1,    -1,    29,    30,    -1,    -1,    -1,
      -1,    35,    36,    37,    -1,    -1,    40,    -1,    -1,    -1,
      -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,    53,
      54,    -1,    -1,    57,    -1,    -1,    -1,    -1,    62,    63,
      -1,    65,    -1,    -1,    -1,    69,    70,    71,    -1,    73,
      74,    75,    -1,    -1,    78,    79,    80,    -1,    82,    83,
      84,    -1,    86,    87,    -1,    89,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,   100,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   108,   109,    -1,   111,   112,   113,
      -1,   115,   116,    -1,   118,    -1,    -1,   121,   122,   123,
      -1,   125,    -1,   127,    -1,   129,   130,    -1,   132,   133,
       1,    -1,     3,     4,     5,     6,     7,    -1,     9,    10,
      11,    12,    13,    -1,    -1,    -1,    17,    -1,    19,    20,
      -1,    -1,    -1,    -1,    25,    26,    -1,    -1,    29,    30,
      -1,    -1,    -1,    -1,    35,    36,    37,    -1,    -1,    40,
      -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,
      -1,    52,    53,    54,    -1,    -1,    57,    -1,    -1,    -1,
      -1,    62,    63,    -1,    65,    -1,    -1,    -1,    69,    70,
      71,    -1,    73,    74,    75,    -1,    77,    78,    -1,    80,
      -1,    82,    83,    84,    -1,    86,    87,    -1,    89,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,   100,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,    -1,
     111,   112,   113,    -1,   115,   116,    -1,   118,    -1,    -1,
     121,   122,   123,    -1,   125,    -1,   127,    -1,   129,   130,
      -1,   132,   133,     1,    -1,     3,     4,     5,     6,     7,
      -1,     9,    10,    11,    12,    13,    -1,    -1,    -1,    17,
      -1,    19,    20,    -1,    -1,    -1,    -1,    25,    26,    -1,
      -1,    29,    30,    -1,    -1,    -1,    -1,    35,    36,    37,
      -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    -1,    -1,    52,    53,    54,    -1,    -1,    57,
      -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,    -1,
      -1,    69,    70,    71,    -1,    73,    74,    75,    -1,    77,
      78,    -1,    80,    -1,    82,    83,    84,    -1,    86,    87,
      -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     108,   109,    -1,   111,   112,   113,    -1,   115,   116,    -1,
     118,    -1,    -1,   121,   122,   123,    -1,   125,    -1,   127,
      -1,   129,   130,    -1,   132,   133,     1,    -1,     3,     4,
       5,     6,     7,    -1,     9,    10,    11,    12,    13,    -1,
      -1,    -1,    17,    -1,    19,    20,    -1,    -1,    -1,    -1,
      25,    26,    -1,    -1,    29,    30,    -1,    -1,    -1,    -1,
      35,    36,    37,    -1,    -1,    40,    -1,    -1,    -1,    -1,
      -1,    46,    47,    -1,    -1,    -1,    51,    52,    53,    54,
      -1,    -1,    57,    -1,    -1,    -1,    -1,    62,    63,    -1,
      65,    -1,    -1,    -1,    69,    70,    71,    -1,    73,    74,
      75,    -1,    -1,    78,    -1,    80,    -1,    82,    83,    84,
      -1,    86,    87,    -1,    89,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    -1,   100,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   108,   109,    -1,   111,   112,   113,    -1,
     115,   116,    -1,   118,    -1,    -1,   121,   122,   123,    -1,
     125,    -1,   127,    -1,   129,   130,    -1,   132,   133,     1,
      -1,     3,     4,     5,     6,     7,    -1,     9,    10,    11,
      12,    13,    -1,    -1,    -1,    17,    -1,    19,    20,    -1,
      -1,    -1,    -1,    25,    26,    -1,    -1,    29,    30,    -1,
      -1,    -1,    -1,    35,    36,    37,    -1,    -1,    40,    -1,
      -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,
      52,    53,    54,    -1,    -1,    57,    -1,    -1,    -1,    -1,
      62,    63,    -1,    65,    -1,    -1,    -1,    69,    70,    71,
      -1,    73,    74,    75,    76,    -1,    78,    -1,    80,    -1,
      82,    83,    84,    -1,    86,    87,    -1,    89,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,   100,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   108,   109,    -1,   111,
     112,   113,    -1,   115,   116,    -1,   118,    -1,    -1,   121,
     122,   123,    -1,   125,    -1,   127,    -1,   129,   130,    -1,
     132,   133,     1,    -1,     3,     4,     5,     6,     7,    -1,
       9,    10,    11,    12,    13,    -1,    -1,    -1,    17,    18,
      19,    20,    -1,    -1,    -1,    -1,    25,    26,    -1,    -1,
      29,    30,    -1,    -1,    -1,    -1,    35,    36,    37,    -1,
      -1,    40,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,
      -1,    -1,    -1,    52,    53,    54,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    62,    63,    -1,    65,    -1,    -1,    -1,
      69,    70,    71,    -1,    73,    74,    75,    -1,    -1,    78,
      -1,    80,    -1,    82,    83,    84,    -1,    86,    87,    -1,
      89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,
     109,    -1,   111,   112,   113,    -1,   115,   116,    -1,   118,
      -1,    -1,   121,   122,   123,    -1,   125,    -1,   127,    -1,
     129,   130,    -1,   132,   133,     1,    -1,     3,     4,     5,
       6,     7,    -1,     9,    10,    11,    12,    13,    -1,    -1,
      -1,    17,    -1,    19,    20,    -1,    -1,    -1,    -1,    25,
      26,    -1,    -1,    29,    30,    -1,    -1,    -1,    -1,    35,
      36,    37,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,
      46,    47,    -1,    -1,    -1,    -1,    52,    53,    54,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    62,    63,    -1,    65,
      -1,    -1,    -1,    69,    70,    71,    -1,    73,    74,    75,
      -1,    -1,    78,    -1,    80,    -1,    82,    83,    84,    -1,
      86,    87,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    98,    -1,   100,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   108,   109,    -1,   111,   112,   113,    -1,   115,
     116,    -1,   118,   119,    -1,   121,   122,   123,    -1,   125,
      -1,   127,    -1,   129,   130,    -1,   132,   133,     1,    -1,
       3,     4,     5,     6,     7,    -1,     9,    10,    11,    12,
      13,    -1,    -1,    -1,    17,    -1,    19,    20,    -1,    -1,
      -1,    -1,    25,    26,    -1,    -1,    29,    30,    -1,    -1,
      -1,    -1,    35,    36,    37,    -1,    -1,    40,    -1,    -1,
      -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,
      53,    54,    -1,    -1,    57,    -1,    -1,    -1,    -1,    62,
      63,    -1,    65,    -1,    -1,    -1,    69,    70,    71,    -1,
      73,    74,    75,    -1,    -1,    78,    -1,    80,    -1,    82,
      83,    84,    -1,    86,    87,    -1,    89,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,   100,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   108,   109,   110,   111,   112,
     113,    -1,   115,   116,    -1,   118,    -1,    -1,   121,   122,
     123,    -1,   125,    -1,   127,    -1,   129,   130,    -1,   132,
     133,     1,    -1,     3,     4,     5,     6,     7,    -1,     9,
      10,    11,    12,    13,    -1,    -1,    -1,    17,    -1,    19,
      20,    -1,    -1,    -1,    -1,    25,    26,    -1,    -1,    29,
      30,    -1,    -1,    -1,    -1,    35,    36,    37,    -1,    -1,
      40,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,
      -1,    -1,    52,    53,    54,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    62,    63,    -1,    65,    -1,    -1,    -1,    69,
      70,    71,    -1,    73,    74,    75,    -1,    -1,    78,    -1,
      80,    -1,    82,    83,    84,    -1,    86,    87,    -1,    89,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
     100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,
     110,   111,   112,   113,    -1,   115,   116,    -1,   118,    -1,
      -1,   121,   122,   123,    -1,   125,    -1,   127,    -1,   129,
     130,    -1,   132,   133,     1,    -1,     3,     4,     5,     6,
       7,    -1,     9,    10,    11,    12,    13,    -1,    -1,    -1,
      17,    -1,    19,    20,    -1,    -1,    -1,    -1,    25,    26,
      -1,    -1,    29,    30,    -1,    -1,    -1,    -1,    35,    36,
      37,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    46,
      47,    -1,    -1,    -1,    -1,    52,    53,    54,    -1,    -1,
      57,    -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,
      -1,    -1,    69,    70,    71,    -1,    73,    74,    75,    -1,
      -1,    78,    -1,    80,    -1,    82,    83,    84,    -1,    86,
      87,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   108,   109,    -1,   111,   112,   113,    -1,   115,   116,
      -1,   118,    -1,    -1,   121,   122,   123,    -1,   125,    -1,
     127,    -1,   129,   130,    -1,   132,   133,     1,    -1,     3,
       4,     5,     6,     7,    -1,     9,    10,    11,    12,    13,
      -1,    -1,    -1,    17,    -1,    19,    20,    -1,    -1,    -1,
      -1,    25,    26,    -1,    -1,    29,    30,    -1,    -1,    -1,
      -1,    35,    36,    37,    -1,    -1,    40,    -1,    -1,    -1,
      -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,    53,
      54,    -1,    -1,    57,    -1,    -1,    -1,    -1,    62,    63,
      -1,    65,    -1,    -1,    -1,    69,    70,    71,    -1,    73,
      74,    75,    -1,    -1,    78,    -1,    80,    -1,    82,    83,
      84,    -1,    86,    87,    -1,    89,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,   100,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   108,   109,    -1,   111,   112,   113,
      -1,   115,   116,    -1,   118,    -1,    -1,   121,   122,   123,
      -1,   125,    -1,   127,    -1,   129,   130,    -1,   132,   133,
       1,    -1,     3,     4,     5,     6,     7,    -1,     9,    10,
      11,    12,    13,    -1,    -1,    -1,    17,    -1,    19,    20,
      -1,    -1,    -1,    -1,    25,    26,    -1,    -1,    29,    30,
      -1,    -1,    -1,    -1,    35,    36,    37,    -1,    -1,    40,
      -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,
      -1,    52,    53,    54,    -1,    -1,    57,    -1,    -1,    -1,
      -1,    62,    63,    -1,    65,    -1,    -1,    -1,    69,    70,
      71,    -1,    73,    74,    75,    -1,    -1,    78,    -1,    80,
      -1,    82,    83,    84,    -1,    86,    87,    -1,    89,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,   100,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,    -1,
     111,   112,   113,    -1,   115,   116,    -1,   118,    -1,    -1,
     121,   122,   123,    -1,   125,    -1,   127,    -1,   129,   130,
      -1,   132,   133,     1,    -1,     3,     4,     5,     6,     7,
      -1,     9,    10,    11,    12,    13,    -1,    -1,    -1,    17,
      -1,    19,    20,    -1,    -1,    -1,    -1,    25,    26,    -1,
      -1,    29,    30,    -1,    -1,    -1,    -1,    35,    36,    37,
      -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    -1,    -1,    52,    53,    54,    -1,    -1,    57,
      -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,    -1,
      -1,    69,    70,    71,    -1,    73,    74,    75,    -1,    -1,
      78,    -1,    80,    -1,    82,    83,    84,    -1,    86,    87,
      -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     108,   109,    -1,   111,   112,   113,    -1,   115,   116,    -1,
     118,    -1,    -1,   121,   122,   123,    -1,   125,    -1,   127,
      -1,   129,   130,    -1,   132,   133,     1,    -1,     3,     4,
       5,     6,     7,    -1,     9,    10,    11,    12,    13,    -1,
      -1,    -1,    17,    -1,    19,    20,    -1,    -1,    -1,    -1,
      25,    26,    -1,    -1,    29,    30,    -1,    -1,    -1,    -1,
      35,    36,    37,    -1,    -1,    40,    -1,    -1,    -1,    -1,
      -1,    46,    47,    -1,    -1,    -1,    -1,    52,    53,    54,
      -1,    -1,    57,    -1,    -1,    -1,    -1,    62,    63,    -1,
      65,    -1,    -1,    -1,    69,    70,    71,    -1,    73,    74,
      75,    -1,    -1,    78,    -1,    80,    -1,    82,    83,    84,
      -1,    86,    87,    -1,    89,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    -1,   100,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   108,   109,    -1,   111,   112,   113,    -1,
     115,   116,    -1,   118,    -1,    -1,   121,   122,   123,    -1,
     125,    -1,   127,    -1,   129,   130,    -1,   132,   133,     1,
      -1,     3,     4,     5,     6,     7,    -1,     9,    10,    11,
      12,    13,    -1,    -1,    -1,    17,    -1,    19,    20,    -1,
      -1,    -1,    -1,    25,    26,    -1,    -1,    29,    30,    -1,
      -1,    -1,    -1,    35,    36,    37,    -1,    -1,    40,    -1,
      -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,
      52,    53,    54,    -1,    -1,    57,    -1,    -1,    -1,    -1,
      62,    63,    -1,    65,    -1,    -1,    -1,    69,    70,    71,
      -1,    73,    74,    75,    -1,    -1,    78,    -1,    80,    -1,
      82,    83,    84,    -1,    86,    87,    -1,    89,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,   100,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   108,   109,    -1,   111,
     112,   113,    -1,   115,   116,    -1,   118,    -1,    -1,   121,
     122,   123,    -1,   125,    -1,   127,    -1,   129,   130,    -1,
     132,   133,     1,    -1,     3,     4,     5,     6,     7,    -1,
       9,    10,    11,    12,    13,    -1,    -1,    -1,    17,    -1,
      19,    20,    -1,    -1,    -1,    -1,    25,    26,    -1,    -1,
      29,    30,    -1,    -1,    -1,    -1,    35,    36,    37,    -1,
      -1,    40,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,
      -1,    -1,    -1,    52,    53,    54,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    62,    63,    -1,    65,    -1,    -1,    -1,
      69,    70,    71,    -1,    73,    74,    75,    -1,    -1,    78,
      -1,    80,    -1,    82,    83,    84,    -1,    86,    87,    -1,
      89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,
     109,    -1,   111,   112,   113,    -1,   115,   116,    -1,   118,
      -1,    -1,   121,   122,   123,    -1,   125,    -1,   127,    -1,
     129,   130,    -1,   132,   133,     1,    -1,     3,     4,     5,
       6,     7,    -1,     9,    10,    11,    12,    13,    -1,    -1,
      -1,    17,    -1,    19,    20,    -1,    -1,    -1,    -1,    25,
      26,    -1,    -1,    29,    30,    -1,    -1,    -1,    -1,    35,
      36,    37,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,
      46,    47,    -1,    -1,    -1,    -1,    52,    53,    54,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    62,    63,    -1,    65,
      -1,    -1,    -1,    69,    70,    71,    -1,    73,    74,    75,
      -1,    -1,    78,    -1,    80,    -1,    82,    83,    84,    -1,
      86,    87,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    98,    -1,   100,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   108,   109,    -1,   111,   112,   113,    -1,   115,
     116,    -1,   118,    -1,    -1,   121,   122,   123,    -1,   125,
      -1,   127,    -1,   129,   130,    -1,   132,   133,     1,    -1,
       3,     4,     5,     6,     7,    -1,     9,    10,    11,    12,
      13,    -1,    -1,    -1,    17,    -1,    19,    20,    -1,    -1,
      -1,    -1,    25,    26,    -1,    -1,    29,    30,    -1,    -1,
      -1,    -1,    35,    36,    37,    -1,    -1,    40,    -1,    -1,
      -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,
      53,    54,    -1,    -1,    57,    -1,    -1,    -1,    -1,    62,
      63,    -1,    65,    -1,    -1,    -1,    69,    70,    71,    -1,
      73,    74,    75,    -1,    -1,    78,    -1,    80,    -1,    82,
      83,    84,    -1,    86,    87,    -1,    89,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,   100,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   108,   109,    -1,   111,   112,
     113,    -1,   115,   116,    -1,   118,    -1,    -1,   121,   122,
     123,    -1,   125,    -1,   127,    -1,   129,   130,    -1,   132,
     133,     1,    -1,     3,     4,     5,     6,     7,    -1,     9,
      10,    11,    12,    13,    -1,    -1,    -1,    17,    -1,    19,
      20,    -1,    -1,    -1,    -1,    25,    26,    -1,    -1,    29,
      30,    -1,    -1,    -1,    -1,    35,    36,    37,    -1,    -1,
      40,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,
      -1,    -1,    52,    53,    54,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    62,    63,    -1,    65,    -1,    -1,    -1,    69,
      70,    71,    -1,    73,    74,    75,    -1,    -1,    78,    -1,
      80,    -1,    82,    83,    84,    -1,    86,    87,    -1,    89,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
     100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,
      -1,   111,   112,   113,    -1,   115,   116,    -1,   118,    -1,
      -1,   121,   122,   123,    -1,   125,    -1,   127,    -1,   129,
     130,    -1,   132,   133,    11,    12,    -1,    14,    15,    16,
      17,    -1,    19,    20,    21,    22,    23,    24,    -1,    -1,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    12,    55,    14,
      15,    16,    17,    -1,    19,    20,    21,    22,    23,    24,
      67,    -1,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    -1,    -1,    -1,    -1,    85,    -1,
      45,    -1,    -1,    -1,    -1,    50,    -1,    -1,    12,    -1,
      -1,    -1,    -1,    17,    -1,    19,    20,    21,    22,    23,
      24,    -1,    67,    27,    -1,    -1,    30,   114,    32,    33,
     117,    35,    36,   120,    38,    -1,    -1,   124,   125,    -1,
      85,   128,    -1,    -1,   131,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   109,    -1,    -1,    -1,    -1,   114,
      -1,    -1,   117,    -1,    -1,   120,    -1,    -1,    -1,   124,
     125,    85,    -1,   128,    -1,    12,   131,    14,    15,    16,
      17,    -1,    19,    20,    21,    22,    23,    24,    -1,    -1,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    -1,    -1,    41,    -1,    -1,    -1,    45,    -1,
      -1,   125,    -1,    -1,   128,    -1,    -1,   131,    -1,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    -1,    -1,    -1,    12,    -1,    14,    15,    16,    17,
      -1,    19,    20,    21,    22,    23,    24,    -1,    85,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    -1,    -1,    41,    -1,    -1,    -1,    45,    -1,    47,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,    -1,    -1,
     117,    -1,    -1,   120,    -1,    -1,    -1,   124,   125,    67,
      -1,   128,    -1,    12,   131,    14,    15,    16,    17,    -1,
      19,    20,    21,    22,    23,    24,    -1,    85,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      -1,    -1,    41,    -1,    -1,    -1,    45,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   114,    -1,    57,   117,
      -1,    -1,   120,    -1,    -1,    -1,   124,   125,    67,    -1,
     128,    -1,    12,   131,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    -1,    85,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   114,    -1,    -1,   117,    -1,
      -1,   120,    -1,    -1,    -1,   124,   125,    67,    -1,   128,
      -1,    -1,   131,    -1,    -1,    -1,    -1,    -1,    -1,    12,
      -1,    14,    15,    16,    17,    85,    19,    20,    21,    22,
      23,    24,    -1,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    -1,   114,    -1,    -1,   117,    -1,    -1,
     120,    -1,    -1,    -1,   124,   125,    -1,    -1,   128,    -1,
      -1,   131,    -1,    -1,    67,    -1,    -1,    -1,    12,    -1,
      14,    15,    16,    17,    -1,    19,    20,    21,    22,    23,
      24,    -1,    85,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   114,    -1,    57,   117,    -1,    -1,   120,    -1,    -1,
      -1,   124,   125,    67,    -1,   128,    -1,    12,   131,    14,
      15,    16,    17,    -1,    19,    20,    21,    22,    23,    24,
      -1,    85,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     114,    -1,    -1,   117,    -1,    60,   120,    -1,    -1,    -1,
     124,   125,    67,    -1,   128,    -1,    12,   131,    14,    15,
      16,    17,    -1,    19,    20,    21,    22,    23,    24,    -1,
      85,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,
      -1,    -1,   117,    -1,    60,   120,    -1,    -1,    -1,   124,
     125,    67,    -1,   128,    -1,    12,   131,    14,    15,    16,
      17,    -1,    19,    20,    21,    22,    23,    24,    -1,    85,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,    -1,
      -1,   117,    -1,    -1,   120,    -1,    -1,    -1,   124,   125,
      67,    -1,   128,    -1,    12,   131,    14,    15,    16,    17,
      -1,    19,    20,    21,    22,    23,    24,    -1,    85,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    -1,    99,    -1,    -1,    -1,    -1,    45,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,    -1,    -1,
     117,    -1,    -1,   120,    -1,    -1,    -1,   124,   125,    67,
      -1,   128,    -1,    12,   131,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    -1,    85,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    -1,
      -1,   109,    -1,    -1,    -1,    -1,   114,    -1,    -1,   117,
      -1,    -1,   120,    -1,    -1,    -1,   124,   125,    67,    -1,
     128,    -1,    12,   131,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    -1,    85,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   114,    -1,    -1,   117,    -1,
      -1,   120,    -1,    -1,    -1,   124,   125,    67,    -1,   128,
      -1,    12,   131,    14,    15,    16,    17,    -1,    19,    20,
      21,    22,    23,    24,    -1,    85,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    -1,    -1,
      41,    -1,    -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   114,    -1,    -1,   117,    -1,    -1,
     120,    -1,    -1,    -1,   124,   125,    67,    -1,   128,    -1,
      12,   131,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    -1,    85,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   114,    -1,    -1,   117,    -1,    -1,   120,
      -1,    -1,    -1,   124,   125,    67,    -1,   128,    -1,    12,
     131,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    -1,    85,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   114,    -1,    -1,   117,    -1,    -1,   120,    -1,
      -1,    -1,   124,   125,    67,    -1,   128,    -1,    12,   131,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    -1,    85,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   114,    -1,    -1,   117,    -1,    -1,   120,    -1,    -1,
      -1,   124,   125,    67,    -1,   128,    -1,    12,   131,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      -1,    85,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     114,    -1,    -1,   117,    -1,    -1,   120,    -1,    -1,    -1,
     124,   125,    67,    -1,   128,    -1,    12,   131,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    -1,
      85,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,
      -1,    -1,   117,    -1,    -1,   120,    -1,    -1,    -1,   124,
     125,    67,    -1,   128,    -1,    12,   131,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    -1,    85,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,    -1,
      -1,   117,    -1,    -1,   120,    -1,    -1,    -1,   124,   125,
      67,    -1,   128,    -1,    12,   131,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    -1,    85,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,    -1,    -1,
     117,    -1,    -1,   120,    -1,    -1,    -1,   124,   125,    67,
      -1,   128,    -1,    12,   131,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    -1,    85,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   114,    -1,    -1,   117,
      -1,    -1,   120,    -1,    -1,    -1,   124,   125,    67,    -1,
     128,    -1,    12,   131,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    -1,    85,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   114,    -1,    -1,   117,    -1,
      -1,   120,    -1,    -1,    -1,   124,   125,    67,    -1,   128,
      -1,    12,   131,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    -1,    85,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   114,    -1,    -1,   117,    -1,    -1,
     120,    -1,    -1,    -1,   124,   125,    67,    -1,   128,    -1,
      12,   131,    14,    15,    16,    17,    -1,    19,    20,    21,
      22,    23,    24,    -1,    85,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    -1,    -1,    41,
      -1,    -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   114,    -1,    -1,   117,    -1,    -1,   120,
      -1,    -1,    -1,   124,   125,    67,    -1,   128,    -1,    12,
     131,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    -1,    85,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   114,    -1,    -1,   117,    -1,    -1,   120,    -1,
      -1,    -1,   124,   125,    67,    -1,   128,    -1,    12,   131,
      14,    15,    16,    17,    -1,    19,    20,    21,    22,    23,
      24,    -1,    85,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    -1,    -1,    41,    -1,    -1,
      -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   114,    -1,    -1,   117,    -1,    -1,   120,    -1,    -1,
      -1,   124,   125,    67,    -1,   128,    -1,    12,   131,    14,
      15,    16,    17,    -1,    19,    20,    21,    22,    23,    24,
      -1,    85,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    -1,    -1,    -1,    -1,    50,    -1,    -1,    -1,    -1,
     114,    -1,    -1,   117,    -1,    -1,   120,    -1,    -1,    -1,
     124,   125,    67,    -1,   128,    -1,    12,   131,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    -1,
      85,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,
      -1,    -1,   117,    -1,    -1,   120,    -1,    -1,    -1,   124,
     125,    67,    -1,   128,    -1,    12,   131,    14,    15,    16,
      17,    -1,    19,    20,    21,    22,    23,    24,    -1,    85,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    -1,    -1,    41,    -1,    -1,    -1,    45,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,    -1,
      -1,   117,    -1,    -1,   120,    -1,    -1,    -1,   124,   125,
      67,    -1,   128,    -1,    12,   131,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    -1,    85,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,    -1,    -1,
     117,    -1,    -1,   120,    -1,    -1,    -1,   124,   125,    67,
      -1,   128,    -1,    -1,   131,    -1,    -1,    -1,    -1,    -1,
      -1,    12,    -1,    14,    15,    16,    17,    85,    19,    20,
      21,    22,    23,    24,    -1,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    -1,   114,    -1,    -1,   117,
      -1,    -1,   120,    -1,    -1,    -1,   124,   125,    -1,    -1,
     128,    -1,    -1,   131,    -1,    -1,    67,    -1,    -1,    -1,
      12,    -1,    14,    15,    16,    17,    -1,    19,    20,    21,
      22,    23,    24,    -1,    85,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   114,    -1,    -1,   117,    -1,    60,   120,
      -1,    -1,    -1,   124,   125,    67,    -1,   128,    -1,    12,
     131,    14,    15,    16,    17,    -1,    19,    20,    21,    22,
      23,    24,    -1,    85,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   114,    -1,    -1,   117,    -1,    -1,   120,    -1,
      -1,    -1,   124,   125,    67,    -1,   128,    -1,    12,   131,
      14,    15,    16,    17,    77,    19,    20,    21,    22,    23,
      24,    -1,    85,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    -1,    -1,    -1,    -1,    -1,
      44,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   114,    -1,    -1,   117,    -1,    -1,   120,    -1,    -1,
      -1,   124,   125,    67,    -1,   128,    -1,    12,   131,    14,
      15,    16,    17,    -1,    19,    20,    21,    22,    23,    24,
      -1,    85,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    -1,    -1,    -1,    -1,    -1,    44,
      45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     114,    -1,    -1,   117,    -1,    -1,   120,    -1,    -1,    -1,
     124,   125,    67,    -1,   128,    -1,    -1,   131,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      85,    12,    -1,    14,    15,    16,    17,    -1,    19,    20,
      21,    22,    23,    24,    -1,    -1,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    -1,   114,
      -1,    -1,   117,    -1,    45,   120,    -1,    -1,    -1,   124,
     125,    -1,    -1,   128,    -1,    -1,   131,    -1,    -1,    -1,
      61,    -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,
      12,    -1,    14,    15,    16,    17,    -1,    19,    20,    21,
      22,    23,    24,    -1,    85,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    -1,    -1,    -1,
      -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   114,    -1,    -1,   117,    -1,    -1,   120,
      -1,    -1,    -1,   124,   125,    67,    -1,   128,    -1,    12,
     131,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    -1,    85,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   114,    -1,    -1,   117,    -1,    -1,   120,    -1,
      -1,    -1,   124,   125,    67,    -1,   128,    -1,    12,   131,
      14,    15,    16,    17,    -1,    19,    20,    21,    22,    23,
      24,    -1,    85,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    -1,    -1,    41,    -1,    -1,
      -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   114,    -1,    -1,   117,    -1,    -1,   120,    -1,    -1,
      -1,   124,   125,    67,    -1,   128,    -1,    12,   131,    14,
      15,    16,    17,    -1,    19,    20,    21,    22,    23,    24,
      -1,    85,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    -1,    -1,    -1,    -1,    50,    -1,    -1,    -1,    -1,
     114,    -1,    -1,   117,    -1,    -1,   120,    -1,    -1,    -1,
     124,   125,    67,    -1,   128,    -1,    12,   131,    14,    15,
      16,    17,    -1,    19,    20,    21,    22,    23,    24,    -1,
      85,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,
      -1,    -1,   117,    -1,    60,   120,    -1,    -1,    -1,   124,
     125,    67,    -1,   128,    -1,    12,   131,    14,    15,    16,
      17,    -1,    19,    20,    21,    22,    23,    24,    -1,    85,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,    -1,
      -1,   117,    -1,    60,   120,    -1,    -1,    -1,   124,   125,
      67,    -1,   128,    -1,    12,   131,    14,    15,    16,    17,
      -1,    19,    20,    21,    22,    23,    24,    -1,    85,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    -1,    -1,    -1,    -1,    -1,    44,    45,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,    -1,    -1,
     117,    -1,    -1,   120,    -1,    -1,    -1,   124,   125,    67,
      -1,   128,    -1,    12,   131,    14,    15,    16,    17,    -1,
      19,    20,    21,    22,    23,    24,    -1,    85,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    -1,
      -1,    50,    -1,    -1,    -1,    -1,   114,    -1,    -1,   117,
      -1,    -1,   120,    -1,    -1,    -1,   124,   125,    67,    -1,
     128,    -1,    12,   131,    14,    15,    16,    17,    -1,    19,
      20,    21,    22,    23,    24,    -1,    85,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    -1,
      -1,    41,    -1,    -1,    -1,    45,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   114,    -1,    -1,   117,    -1,
      -1,   120,    -1,    -1,    -1,   124,   125,    67,    -1,   128,
      -1,    12,   131,    14,    15,    16,    17,    -1,    19,    20,
      21,    22,    23,    24,    -1,    85,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    -1,    -1,
      -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   114,    -1,    -1,   117,    -1,    -1,
     120,    -1,    -1,    -1,   124,   125,    67,    -1,   128,    -1,
      12,   131,    14,    15,    16,    17,    -1,    19,    20,    21,
      22,    23,    24,    -1,    85,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   114,    -1,    -1,   117,    -1,    -1,   120,
      -1,    -1,    -1,   124,   125,    67,    -1,   128,    -1,    12,
     131,    14,    15,    16,    17,    -1,    19,    20,    21,    22,
      23,    24,    -1,    85,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   114,    -1,    -1,   117,    -1,    -1,   120,    -1,
      -1,    -1,   124,   125,    67,    -1,   128,    -1,    12,   131,
      14,    15,    16,    17,    -1,    19,    20,    21,    22,    23,
      24,    -1,    85,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   114,    -1,    -1,   117,    -1,    -1,   120,    -1,    -1,
      -1,   124,   125,    67,    -1,   128,    -1,    12,   131,    -1,
      15,    16,    17,    -1,    19,    20,    21,    22,    23,    24,
      -1,    85,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   117,    -1,    -1,   120,    -1,    -1,    -1,
     124,   125,    67,    -1,   128,    -1,    12,   131,    14,    15,
      16,    17,    -1,    19,    20,    21,    22,    23,    24,    -1,
      85,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,
      -1,    -1,   117,    -1,    -1,   120,    -1,    -1,    -1,   124,
     125,    67,    -1,   128,    -1,    12,   131,    -1,    15,    16,
      17,    -1,    19,    20,    21,    22,    23,    24,    -1,    85,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   120,    -1,    -1,    -1,   124,   125,
      -1,    -1,   128,    -1,    12,   131,    14,    -1,    16,    17,
      -1,    19,    20,    21,    22,    23,    24,    -1,    85,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,    -1,    -1,
     117,    -1,    -1,   120,    -1,    -1,    -1,   124,   125,    67,
      -1,   128,    -1,    12,   131,    -1,    -1,    16,    17,    -1,
      19,    20,    21,    22,    23,    24,    -1,    85,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   124,   125,    67,    -1,
     128,    -1,    12,   131,    -1,    -1,    16,    17,    -1,    19,
      20,    21,    22,    23,    24,    -1,    85,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    -1,
      -1,    -1,   101,    -1,    -1,    45,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   124,   125,    67,    -1,   128,
      -1,    12,   131,    -1,    -1,    16,    17,    -1,    19,    20,
      21,    22,    23,    24,    -1,    85,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,   125,    67,    -1,   128,    -1,
      12,   131,    -1,    -1,    16,    17,    -1,    19,    20,    21,
      22,    23,    24,    -1,    85,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   124,   125,    67,    -1,   128,    -1,    12,
     131,    -1,    -1,    16,    17,    -1,    19,    20,    21,    22,
      23,    24,    -1,    85,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    12,    -1,    -1,    -1,
      -1,    17,    -1,    19,    20,    21,    22,    23,    24,    -1,
      -1,    27,    28,    29,    30,    31,    32,    33,    -1,    35,
      36,    37,    38,   125,    67,    -1,   128,    -1,    -1,   131,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    12,    -1,
      -1,    -1,    85,    17,    -1,    19,    20,    21,    22,    23,
      24,    67,    -1,    27,    28,    29,    30,    31,    32,    33,
      -1,    35,    36,    37,    38,    -1,    -1,    -1,    12,    85,
      -1,    -1,    -1,    17,    -1,    19,    20,    21,    22,    23,
      24,    -1,   125,    27,    -1,   128,    30,    -1,   131,    -1,
      -1,    35,    36,    67,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,
      -1,    85,   128,    12,    -1,   131,    -1,    -1,    17,    -1,
      19,    20,    21,    22,    23,    24,    -1,    -1,    27,    -1,
      -1,    30,    31,    32,    33,    12,    35,    36,    -1,    38,
      17,    85,    19,    20,    21,    22,    23,    24,    -1,    -1,
      27,   125,    -1,    30,   128,    32,    33,   131,    35,    36,
      -1,    38,    -1,    -1,    -1,    12,    -1,    -1,    -1,    -1,
      17,    -1,    19,    20,    21,    22,    23,    24,    -1,    -1,
      27,   125,    -1,    -1,   128,    -1,    85,   131,    35,    36,
      12,    38,    -1,    -1,    -1,    17,    -1,    -1,    -1,    -1,
      22,    23,    24,    -1,    -1,    27,    -1,    12,    85,    -1,
      -1,    -1,    17,    35,    36,    -1,    38,    -1,    23,    24,
      -1,    -1,    27,    -1,    -1,    -1,   125,    -1,    -1,   128,
      35,    36,   131,    38,    12,    -1,    -1,    -1,    85,    17,
      -1,    -1,    -1,    -1,    -1,    23,    24,    -1,   125,    27,
      -1,   128,    -1,    -1,   131,    -1,    -1,    35,    36,    12,
      38,    -1,    -1,    85,    17,    -1,    -1,    -1,    -1,    -1,
      -1,    24,    -1,    -1,    27,    -1,    -1,    -1,   125,    -1,
      85,   128,    35,    36,   131,    38,    12,    -1,    -1,    -1,
      -1,    17,    -1,    -1,    -1,    -1,    -1,    -1,    24,    -1,
      -1,    27,    -1,   125,    -1,    -1,   128,    85,    -1,    35,
      36,    -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     125,    -1,    -1,   128,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    85,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,    -1,    -1,
     128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    85,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   125,    -1,    -1,   128,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   128
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     3,     4,     5,     6,     7,     9,    10,    11,
      12,    13,    17,    19,    20,    25,    26,    29,    30,    35,
      36,    37,    40,    46,    47,    52,    53,    54,    57,    62,
      63,    65,    69,    70,    71,    73,    74,    75,    78,    80,
      82,    83,    84,    86,    87,    89,    98,   100,   108,   109,
     111,   112,   113,   115,   116,   118,   121,   122,   123,   125,
     127,   129,   130,   132,   133,   137,   138,   139,   141,   144,
       4,     5,    10,    46,   142,    46,    15,    17,    17,    38,
     139,     4,   139,   139,   139,    13,    78,   139,   139,    14,
     139,     4,   139,   139,   150,     4,    17,   139,    17,    17,
      17,   139,     3,     4,    10,    13,    17,   133,   140,   141,
      17,   139,   139,   139,   151,    76,   158,    17,    17,   141,
      17,   109,   139,    17,    17,    17,    38,    10,   152,   153,
      17,    83,   139,   139,   139,   139,   141,   151,   139,   150,
     139,   151,   151,   158,   141,   139,   153,    38,   139,    17,
     139,    47,    55,   164,   151,   139,    17,     0,     8,    12,
      14,    15,    16,    17,    19,    20,    21,    22,    23,    24,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    44,    45,    67,    85,   114,   117,   120,   124,
     125,   128,   131,    17,    17,     4,    24,   128,     4,    10,
      13,    26,   133,   139,    18,   139,    18,   139,   139,    18,
      26,    26,    26,   139,    57,    41,   139,    50,   109,   139,
     139,     4,    11,    55,   163,    14,   139,   149,    67,    15,
      29,    58,   155,   139,    60,    60,    44,    66,   139,   165,
      81,   143,   146,   151,    72,   139,   150,    17,   139,   139,
     139,   139,   139,    88,    14,   152,   139,    99,    14,    17,
      77,   109,   160,   109,   109,    51,   109,   160,    17,   122,
     139,    18,   139,   151,    77,    50,   139,    67,   139,   139,
     139,   158,   150,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   141,
      40,   139,     8,   138,   139,   139,     6,    10,    14,    19,
      21,    22,   114,   125,   141,   139,   139,   139,   139,   139,
     150,   139,     3,    17,   139,    18,    18,    41,    17,     3,
       4,    10,    13,    17,   141,   147,   148,    18,   151,   158,
     139,    18,    18,    18,     3,   151,    56,     4,    18,    44,
     139,   139,   139,   139,    59,    61,   154,    18,   151,   151,
     139,   139,    79,   145,   151,   143,    77,    17,    18,    18,
     150,    18,    18,    18,    18,    41,   152,    18,   139,   150,
      77,   151,   151,   151,   160,    51,   150,    41,    50,   151,
      18,   139,    18,    23,    67,   139,    41,   139,    17,    38,
      18,    18,     3,    26,   150,    15,    29,    45,    46,   148,
      14,    67,    17,   139,   158,    49,    51,   109,   159,   160,
     161,    51,    76,    60,   139,   149,    60,   139,   139,    59,
      60,    63,   139,   158,    77,    77,    77,    44,   147,    77,
     139,   143,    18,   101,    18,   160,   160,   160,   151,   160,
      18,   151,    51,    77,   139,    41,   150,    40,   139,    15,
      74,    24,    18,    18,   139,   139,   139,   139,    18,   148,
     139,   139,    44,   109,   157,   161,   139,    49,    51,   151,
     158,    54,    55,   162,   151,    44,   151,   154,   139,   151,
     139,    44,    44,    18,    76,   151,    76,   143,   151,   119,
     160,    29,    77,   151,    41,    18,   139,    41,    74,   139,
     158,   143,     3,    41,    47,   157,   139,   158,    50,   139,
     160,     3,    45,    77,    11,   163,   149,    51,    77,    44,
      60,   156,    60,    77,    60,   158,   151,    77,   143,   151,
      77,   109,   151,   119,   139,    44,   164,    77,    41,   143,
     151,   139,    44,   151,    50,    45,   158,    18,   151,   151,
     151,   151,    77,   151,    77,   110,   109,   110,   160,   109,
     151,   164,    41,   151,    77,    41,   159,   151,   158,   139,
     158,    77,    77,    77,    77,    77,   150,   110,   150,   110,
     160,    77,   159,   162,    44,   109,   150,   109,   150,   151,
     109,   151,   109,   160,   151,   160,   151,   160,   160
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   136,   137,   138,   138,   138,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   140,   140,   140,   140,   141,
     141,   141,   141,   141,   141,   141,   141,   141,   141,   142,
     142,   143,   143,   143,   144,   144,   145,   146,   147,   147,
     148,   148,   148,   148,   148,   148,   148,   148,   148,   149,
     149,   150,   150,   151,   151,   151,   152,   152,   152,   153,
     154,   154,   155,   155,   155,   155,   156,   156,   157,   157,
     157,   158,   158,   159,   159,   159,   159,   160,   160,   161,
     161,   162,   162,   162,   163,   163,   163,   164,   164,   164,
     164,   165
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     3,     3,     1,     2,     4,     6,
       3,     5,     7,     1,     1,     6,     6,     6,     6,     8,
       3,     3,     3,     3,     3,     3,     3,     4,     4,     4,
       1,     1,     1,     3,     4,     3,     3,     3,     3,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     2,
       3,     3,     3,     2,     2,     2,     5,     3,     1,     3,
       2,     4,     4,     1,     4,     4,     4,     3,     1,     2,
       2,     6,     4,     5,     4,     1,     4,     1,     3,     3,
       4,     1,     2,     1,     3,     1,     1,     7,     9,     9,
       9,     7,     9,     1,     4,     5,     7,     5,     3,     1,
       3,     2,     5,     3,     3,     2,     3,     1,     3,     4,
       3,     3,     3,     1,     3,     4,     6,     6,     3,     3,
       3,     1,     2,     3,     2,     3,     2,     1,     1,     2,
       1,     3,     4,     1,     6,     7,     3,     1,     4,     7,
       8,     7,     9,     8,     8,     9,     9,    10,     4,     3,
       4,     7,     9,     5,     6,     5,     5,     7,     4,     1,
       7,     4,     4,     3,     1,     2,     3,     4,     5,     4,
       6,     5,     3,    13,    12,    12,     8,     3,     2,     2,
       7,    13,     9,     5,     5,     1,     3,     1,     1,     1,
       3,     3,     3,     3,     5,     2,     3,     2,     2,     1,
       1,     0,     2,     2,     4,     2,     3,     3,     1,     3,
       1,     3,     3,     3,     3,     1,     3,     1,     1,     0,
       1,     0,     1,     1,     2,     2,     0,     2,     3,     1,
       0,     2,     0,     2,     2,     2,     1,     1,     0,     3,
       2,     3,     4,     1,     3,     5,     6,     1,     2,     1,
       2,     0,     3,     5,     0,     2,     5,     0,     2,     6,
       7,     1
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
      yyerror (scanner, YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



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

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, scanner); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, void * scanner)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (scanner);
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, void * scanner)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, scanner);
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule, void * scanner)
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
                                              , scanner);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, scanner); \
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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, void * scanner)
{
  YYUSE (yyvaluep);
  YYUSE (scanner);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void * scanner)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

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

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
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

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
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
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex (&yylval,YYLEX_PARAM);
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


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 199 "input_parser.yy" /* yacc.c:1646  */
    {   const giac::context * contextptr = giac_yyget_extra(scanner);
			    if ((yyvsp[0])._VECTptr->size()==1)
			     parsed_gen((yyvsp[0])._VECTptr->front(),contextptr);
                          else
			     parsed_gen(gen(*(yyvsp[0])._VECTptr,_SEQ__VECT),contextptr);
			 }
#line 4735 "y.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 207 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=vecteur(1,(yyvsp[-1])); }
#line 4741 "y.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 208 "input_parser.yy" /* yacc.c:1646  */
    { if ((yyvsp[-1]).val==1) (yyval)=vecteur(1,symbolic(at_nodisp,(yyvsp[-2]))); else (yyval)=vecteur(1,(yyvsp[-2])); }
#line 4747 "y.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 209 "input_parser.yy" /* yacc.c:1646  */
    { if ((yyvsp[-1]).val==1) (yyval)=mergevecteur(makevecteur(symbolic(at_nodisp,(yyvsp[-2]))),*(yyvsp[0])._VECTptr); else (yyval)=mergevecteur(makevecteur((yyvsp[-2])),*(yyvsp[0])._VECTptr); }
#line 4753 "y.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 212 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = (yyvsp[0]);}
#line 4759 "y.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 213 "input_parser.yy" /* yacc.c:1646  */
    {if (is_one((yyvsp[-1]))) (yyval)=(yyvsp[0]); else (yyval)=symbolic(at_prod,gen(makevecteur((yyvsp[-1]),(yyvsp[0])),_SEQ__VECT));}
#line 4765 "y.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 214 "input_parser.yy" /* yacc.c:1646  */
    {if (is_one((yyvsp[-3]))) (yyval)=symb_pow((yyvsp[-2]),(yyvsp[0])); else (yyval)=symbolic(at_prod,gen(makevecteur((yyvsp[-3]),symb_pow((yyvsp[-2]),(yyvsp[0]))),_SEQ__VECT));}
#line 4771 "y.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 215 "input_parser.yy" /* yacc.c:1646  */
    {if (is_one((yyvsp[-5]))) (yyval)=symb_pow((yyvsp[-4]),(yyvsp[-1])); else (yyval)=symbolic(at_prod,gen(makevecteur((yyvsp[-5]),symb_pow((yyvsp[-4]),(yyvsp[-1]))),_SEQ__VECT));}
#line 4777 "y.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 216 "input_parser.yy" /* yacc.c:1646  */
    {(yyval)=symbolic(at_prod,gen(makevecteur((yyvsp[-2]),symb_pow((yyvsp[-1]),(yyvsp[0]))) ,_SEQ__VECT));}
#line 4783 "y.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 217 "input_parser.yy" /* yacc.c:1646  */
    { (yyval) =(yyvsp[-4])*symbolic(*(yyvsp[-3])._FUNCptr,python_compat(giac_yyget_extra(scanner))?denest_sto(os_nary_workaround((yyvsp[-1]))):os_nary_workaround((yyvsp[-1]))); }
#line 4789 "y.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 218 "input_parser.yy" /* yacc.c:1646  */
    { (yyval) =(yyvsp[-6])*symb_pow(symbolic(*(yyvsp[-5])._FUNCptr,python_compat(giac_yyget_extra(scanner))?denest_sto(os_nary_workaround((yyvsp[-3]))):os_nary_workaround((yyvsp[-3]))),(yyvsp[0])); }
#line 4795 "y.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 220 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 4801 "y.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 221 "input_parser.yy" /* yacc.c:1646  */
    { if ((yyvsp[0]).type==_FUNC) (yyval)=symbolic(*(yyvsp[0])._FUNCptr,gen(vecteur(0),_SEQ__VECT)); else (yyval)=(yyvsp[0]); }
#line 4807 "y.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 224 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symb_program_sto((yyvsp[-3]),(yyvsp[-3])*gen_zero,(yyvsp[0]),(yyvsp[-5]),false,giac_yyget_extra(scanner));}
#line 4813 "y.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 225 "input_parser.yy" /* yacc.c:1646  */
    {if (is_array_index((yyvsp[-5]),(yyvsp[-3]),giac_yyget_extra(scanner)) || (abs_calc_mode(giac_yyget_extra(scanner))==38 && (yyvsp[-5]).type==_IDNT && strlen((yyvsp[-5])._IDNTptr->id_name)==2 && check_vect_38((yyvsp[-5])._IDNTptr->id_name))) (yyval)=symbolic(at_sto,gen(makevecteur((yyvsp[0]),symbolic(at_of,gen(makevecteur((yyvsp[-5]),(yyvsp[-3])) ,_SEQ__VECT))) ,_SEQ__VECT)); else { (yyval) = symb_program_sto((yyvsp[-3]),(yyvsp[-3])*gen_zero,(yyvsp[0]),(yyvsp[-5]),true,giac_yyget_extra(scanner)); (yyval)._SYMBptr->feuille.subtype=_SORTED__VECT;  } }
#line 4819 "y.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 226 "input_parser.yy" /* yacc.c:1646  */
    {if (is_array_index((yyvsp[-3]),(yyvsp[-1]),giac_yyget_extra(scanner)) || (abs_calc_mode(giac_yyget_extra(scanner))==38 && (yyvsp[-3]).type==_IDNT && check_vect_38((yyvsp[-3])._IDNTptr->id_name))) (yyval)=symbolic(at_sto,gen(makevecteur((yyvsp[-5]),symbolic(at_of,gen(makevecteur((yyvsp[-3]),(yyvsp[-1])) ,_SEQ__VECT))) ,_SEQ__VECT)); else (yyval) = symb_program_sto((yyvsp[-1]),(yyvsp[-1])*gen_zero,(yyvsp[-5]),(yyvsp[-3]),false,giac_yyget_extra(scanner));}
#line 4825 "y.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 227 "input_parser.yy" /* yacc.c:1646  */
    { 
         const giac::context * contextptr = giac_yyget_extra(scanner);
         gen g=symb_at((yyvsp[-3]),(yyvsp[-1]),contextptr); (yyval)=parser_symb_sto((yyvsp[-5]),g); 
        }
#line 4834 "y.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 231 "input_parser.yy" /* yacc.c:1646  */
    { 
         const giac::context * contextptr = giac_yyget_extra(scanner);
         gen g=symbolic(at_of,gen(makevecteur((yyvsp[-5]),(yyvsp[-2])) ,_SEQ__VECT)); (yyval)=parser_symb_sto((yyvsp[-7]),g); 
        }
#line 4843 "y.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 235 "input_parser.yy" /* yacc.c:1646  */
    { if ((yyvsp[0]).type==_IDNT) { string s=(yyvsp[0]).print(context0); const char * ch=s.c_str(); if (ch[0]=='_' && unit_conversion_map().find(ch+1) != unit_conversion_map().end()) (yyval)=symbolic(at_convert,gen(makevecteur((yyvsp[-2]),symbolic(at_unit,makevecteur(1,(yyvsp[0])))) ,_SEQ__VECT)); else (yyval)=parser_symb_sto((yyvsp[-2]),(yyvsp[0])); } else (yyval)=parser_symb_sto((yyvsp[-2]),(yyvsp[0])); }
#line 4849 "y.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 236 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symbolic(at_convert,gen(makevecteur((yyvsp[-2]),(yyvsp[0])) ,_SEQ__VECT)); }
#line 4855 "y.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 237 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symbolic(at_convert,gen(makevecteur((yyvsp[-2]),(yyvsp[0])) ,_SEQ__VECT)); }
#line 4861 "y.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 238 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symbolic(at_convert,gen(makevecteur((yyvsp[-2]),(yyvsp[0])) ,_SEQ__VECT)); }
#line 4867 "y.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 239 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symbolic(at_convert,gen(makevecteur((yyvsp[-2]),(yyvsp[0])) ,_SEQ__VECT)); }
#line 4873 "y.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 240 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symbolic(at_time,(yyvsp[-2]));}
#line 4879 "y.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 241 "input_parser.yy" /* yacc.c:1646  */
    { if ((yyvsp[-2])==16 || (yyvsp[-2])==10 || (yyvsp[-2])==8 || (yyvsp[-2])==2) (yyval)=symbolic(at_integer_format,(yyvsp[-2])); else (yyval)=symbolic(at_solve,symb_equal((yyvsp[-2]),0));}
#line 4885 "y.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 242 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symbolic(at_convert,gen(makevecteur((yyvsp[-3]),symb_unit(gen(1),(yyvsp[0]),giac_yyget_extra(scanner))),_SEQ__VECT)); opened_quote(giac_yyget_extra(scanner)) &= 0x7ffffffd;}
#line 4891 "y.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 243 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = check_symb_of((yyvsp[-3]),python_compat(giac_yyget_extra(scanner))?denest_sto(os_nary_workaround((yyvsp[-1]))):os_nary_workaround((yyvsp[-1])),giac_yyget_extra(scanner));}
#line 4897 "y.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 244 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = check_symb_of((yyvsp[-3]),python_compat(giac_yyget_extra(scanner))?denest_sto(os_nary_workaround((yyvsp[-1]))):os_nary_workaround((yyvsp[-1])),giac_yyget_extra(scanner));}
#line 4903 "y.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 245 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = (yyvsp[0]);}
#line 4909 "y.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 246 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = (yyvsp[0]);}
#line 4915 "y.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 247 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = (yyvsp[0]);}
#line 4921 "y.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 248 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(*(yyvsp[-2])._FUNCptr,(yyvsp[0]));}
#line 4927 "y.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 249 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(*(yyvsp[-3])._FUNCptr,(yyvsp[-1]));}
#line 4933 "y.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 250 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(*(yyvsp[-2])._FUNCptr,gen(vecteur(0),_SEQ__VECT));}
#line 4939 "y.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 251 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(*(yyvsp[0])._FUNCptr,(yyvsp[-2]));}
#line 4945 "y.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 252 "input_parser.yy" /* yacc.c:1646  */
    {(yyval)=symb_test_equal((yyvsp[-2]),(yyvsp[-1]),(yyvsp[0]));}
#line 4951 "y.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 254 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(*(yyvsp[-1])._FUNCptr,makesequence((yyvsp[-2]),(yyvsp[0]))); }
#line 4957 "y.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 255 "input_parser.yy" /* yacc.c:1646  */
    { 
	if ((yyvsp[0]).type==_SYMB) (yyval)=(yyvsp[0]); else (yyval)=symbolic(at_nop,(yyvsp[0])); 
	(yyval).change_subtype(_SPREAD__SYMB); 
        const giac::context * contextptr = giac_yyget_extra(scanner);
       spread_formula(false,contextptr); 
	}
#line 4968 "y.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 261 "input_parser.yy" /* yacc.c:1646  */
    { if ((yyvsp[-2]).is_symb_of_sommet(at_plus) && (yyvsp[-2])._SYMBptr->feuille.type==_VECT){ (yyvsp[-2])._SYMBptr->feuille._VECTptr->push_back((yyvsp[0])); (yyval)=(yyvsp[-2]); } else
  (yyval) =symbolic(*(yyvsp[-1])._FUNCptr,gen(makevecteur((yyvsp[-2]),(yyvsp[0])),_SEQ__VECT));}
#line 4975 "y.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 263 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symb_plus((yyvsp[-2]),(yyvsp[0]).type<_IDNT?-(yyvsp[0]):symbolic(at_neg,(yyvsp[0])));}
#line 4981 "y.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 264 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symb_plus((yyvsp[-2]),(yyvsp[0]).type<_IDNT?-(yyvsp[0]):symbolic(at_neg,(yyvsp[0])));}
#line 4987 "y.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 265 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) =symbolic(*(yyvsp[-1])._FUNCptr,gen(makevecteur((yyvsp[-2]),(yyvsp[0])),_SEQ__VECT));}
#line 4993 "y.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 266 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) =symbolic(*(yyvsp[-1])._FUNCptr,gen(makevecteur((yyvsp[-2]),(yyvsp[0])),_SEQ__VECT));}
#line 4999 "y.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 267 "input_parser.yy" /* yacc.c:1646  */
    {if ((yyvsp[-2])==symbolic(at_exp,1) && (yyvsp[-1])==at_pow) (yyval)=symbolic(at_exp,(yyvsp[0])); else (yyval) =symbolic(*(yyvsp[-1])._FUNCptr,gen(makevecteur((yyvsp[-2]),(yyvsp[0])),_SEQ__VECT));}
#line 5005 "y.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 268 "input_parser.yy" /* yacc.c:1646  */
    {if ((yyvsp[-1]).type==_FUNC) (yyval)=symbolic(*(yyvsp[-1])._FUNCptr,gen(makevecteur((yyvsp[-2]),(yyvsp[0])),_SEQ__VECT)); else (yyval) = symbolic(at_normalmod,gen(makevecteur((yyvsp[-2]),(yyvsp[0])),_SEQ__VECT));}
#line 5011 "y.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 269 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(*(yyvsp[-1])._FUNCptr,gen(makevecteur((yyvsp[-2]),(yyvsp[0])) ,_SEQ__VECT)); }
#line 5017 "y.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 270 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(*(yyvsp[0])._FUNCptr,gen(makevecteur((yyvsp[-1]),RAND_MAX) ,_SEQ__VECT)); }
#line 5023 "y.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 271 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(*(yyvsp[-1])._FUNCptr,gen(makevecteur(0,(yyvsp[0])) ,_SEQ__VECT)); }
#line 5029 "y.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 272 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = makesequence(symbolic(*(yyvsp[-2])._FUNCptr,gen(makevecteur(0,RAND_MAX) ,_SEQ__VECT)),(yyvsp[0])); }
#line 5035 "y.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 275 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(*(yyvsp[-1])._FUNCptr,gen(makevecteur((yyvsp[-2]),(yyvsp[0])),_SEQ__VECT));}
#line 5041 "y.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 276 "input_parser.yy" /* yacc.c:1646  */
    {(yyval)= symbolic(at_deuxpoints,gen(makevecteur((yyvsp[-2]),(yyvsp[0])) ,_SEQ__VECT));}
#line 5047 "y.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 277 "input_parser.yy" /* yacc.c:1646  */
    { 
					if ((yyvsp[0])==unsigned_inf)
						(yyval) = minus_inf;
					else { if ((yyvsp[0]).type==_INT_) (yyval)=(-(yyvsp[0]).val); else { if ((yyvsp[0]).type==_DOUBLE_) (yyval)=(-(yyvsp[0])._DOUBLE_val); else (yyval)=symbolic(at_neg,(yyvsp[0])); } }
				}
#line 5057 "y.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 282 "input_parser.yy" /* yacc.c:1646  */
    { 
					if ((yyvsp[0])==unsigned_inf)
						(yyval) = minus_inf;
					else { if ((yyvsp[0]).type==_INT_ || (yyvsp[0]).type==_DOUBLE_ || (yyvsp[0]).type==_FLOAT_) (yyval)=-(yyvsp[0]); else (yyval)=symbolic(at_neg,(yyvsp[0])); }
				}
#line 5067 "y.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 287 "input_parser.yy" /* yacc.c:1646  */
    {
					if ((yyvsp[0])==unsigned_inf)
						(yyval) = plus_inf;
					else
						(yyval) = (yyvsp[0]);
				}
#line 5078 "y.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 293 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = polynome_or_sparse_poly1(eval((yyvsp[-3]),1, giac_yyget_extra(scanner)),(yyvsp[-1]));}
#line 5084 "y.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 294 "input_parser.yy" /* yacc.c:1646  */
    { 
           if ( ((yyvsp[-1]).type==_SYMB) && ((yyvsp[-1])._SYMBptr->sommet==at_deuxpoints) )
             (yyval) = algebraic_EXTension((yyvsp[-1])._SYMBptr->feuille._VECTptr->front(),(yyvsp[-1])._SYMBptr->feuille._VECTptr->back());
           else (yyval)=(yyvsp[-1]);
        }
#line 5094 "y.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 300 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=gen(at_of,2); }
#line 5100 "y.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 301 "input_parser.yy" /* yacc.c:1646  */
    {if ((yyvsp[-2]).type==_FUNC) *logptr(giac_yyget_extra(scanner))<< ("Warning: "+(yyvsp[-2]).print(context0)+" is a reserved word")<<'\n'; if ((yyvsp[-2]).type==_INT_) (yyval)=symb_equal((yyvsp[-2]),(yyvsp[0])); else {(yyval) = parser_symb_sto((yyvsp[0]),(yyvsp[-2]),(yyvsp[-1])==at_array_sto); if ((yyvsp[0]).is_symb_of_sommet(at_program)) *logptr(giac_yyget_extra(scanner))<<"// End defining "<<(yyvsp[-2])<<'\n';}}
#line 5106 "y.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 302 "input_parser.yy" /* yacc.c:1646  */
    { (yyval) = symbolic(*(yyvsp[-1])._FUNCptr,(yyvsp[0]));}
#line 5112 "y.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 303 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symb_args((yyvsp[-1]));}
#line 5118 "y.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 304 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symb_args((yyvsp[-1]));}
#line 5124 "y.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 305 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_args(vecteur(0)); }
#line 5130 "y.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 306 "input_parser.yy" /* yacc.c:1646  */
    {
	gen tmp=python_compat(giac_yyget_extra(scanner))?denest_sto(os_nary_workaround((yyvsp[-1]))):os_nary_workaround((yyvsp[-1]));
	// CERR << python_compat(giac_yyget_extra(scanner)) << tmp << '\n';
	(yyval) = symbolic(*(yyvsp[-3])._FUNCptr,tmp);
        const giac::context * contextptr = giac_yyget_extra(scanner);
	if ((yyvsp[-1]).type==_INT_ && (*(yyvsp[-3])._FUNCptr==at_maple_mode ||*(yyvsp[-3])._FUNCptr==at_xcas_mode )){
          xcas_mode(contextptr)=(yyvsp[-1]).val;
        }
        if ((yyvsp[-1]).type==_INT_ && *(yyvsp[-3])._FUNCptr==at_python_compat)
          python_compat(contextptr)=(yyvsp[-1]).val;
	if (*(yyvsp[-3])._FUNCptr==at_user_operator){
          user_operator((yyvsp[-1]),contextptr);
        }
	}
#line 5149 "y.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 320 "input_parser.yy" /* yacc.c:1646  */
    {
	if ((yyvsp[-1]).type==_VECT && (yyvsp[-1])._VECTptr->empty())
          giac_yyerror(scanner,"void argument");
	(yyval) = symbolic(*(yyvsp[-3])._FUNCptr,python_compat(giac_yyget_extra(scanner))?denest_sto(os_nary_workaround((yyvsp[-1]))):os_nary_workaround((yyvsp[-1])));	
	}
#line 5159 "y.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 325 "input_parser.yy" /* yacc.c:1646  */
    { 
          const giac::context * contextptr = giac_yyget_extra(scanner);
          (yyval)=symb_at((yyvsp[-3]),(yyvsp[-1]),contextptr);
        }
#line 5168 "y.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 329 "input_parser.yy" /* yacc.c:1646  */
    {
	(yyval) = symbolic(*(yyvsp[-2])._FUNCptr,gen(vecteur(0),_SEQ__VECT));
	if (*(yyvsp[-2])._FUNCptr==at_rpn)
          rpn_mode(giac_yyget_extra(scanner))=1;
	if (*(yyvsp[-2])._FUNCptr==at_alg)
          rpn_mode(giac_yyget_extra(scanner))=0;
	}
#line 5180 "y.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 336 "input_parser.yy" /* yacc.c:1646  */
    {
	(yyval) = (yyvsp[0]);
	}
#line 5188 "y.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 339 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(at_derive,(yyvsp[-1]));}
#line 5194 "y.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 340 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symbolic(*(yyvsp[0])._FUNCptr,(yyvsp[-1])); }
#line 5200 "y.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 342 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(*(yyvsp[-5])._FUNCptr,makevecteur(equaltosame((yyvsp[-4])),symb_bloc((yyvsp[-2])),symb_bloc((yyvsp[0]))));}
#line 5206 "y.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 343 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(*(yyvsp[-3])._FUNCptr,makevecteur(equaltosame((yyvsp[-2])),(yyvsp[0]),0));}
#line 5212 "y.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 344 "input_parser.yy" /* yacc.c:1646  */
    {
	(yyval) = symbolic(*(yyvsp[-4])._FUNCptr,makevecteur(equaltosame((yyvsp[-3])),symb_bloc((yyvsp[-1])),(yyvsp[0])));
	}
#line 5220 "y.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 347 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(*(yyvsp[-3])._FUNCptr,(yyvsp[-1]));}
#line 5226 "y.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 348 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = (yyvsp[0]);}
#line 5232 "y.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 349 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symb_program((yyvsp[-1]));}
#line 5238 "y.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 350 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = gen(at_program,3);}
#line 5244 "y.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 351 "input_parser.yy" /* yacc.c:1646  */
    {
          const giac::context * contextptr = giac_yyget_extra(scanner);
         (yyval) = symb_program((yyvsp[-2]),gen_zero*(yyvsp[-2]),(yyvsp[0]),contextptr);
        }
#line 5253 "y.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 355 "input_parser.yy" /* yacc.c:1646  */
    {
          const giac::context * contextptr = giac_yyget_extra(scanner);
             if ((yyvsp[0]).type==_VECT) 
                (yyval) = symb_program((yyvsp[-2]),gen_zero*(yyvsp[-2]),symb_bloc(makevecteur(at_nop,(yyvsp[0]))),contextptr); 
             else 
                (yyval) = symb_program((yyvsp[-2]),gen_zero*(yyvsp[-2]),(yyvsp[0]),contextptr);
		}
#line 5265 "y.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 362 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symb_bloc((yyvsp[-1]));}
#line 5271 "y.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 363 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = at_bloc;}
#line 5277 "y.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 365 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symbolic(*(yyvsp[-1])._FUNCptr,(yyvsp[0])); }
#line 5283 "y.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 367 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = gen(*(yyvsp[0])._FUNCptr,0);}
#line 5289 "y.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 368 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[-1]);}
#line 5295 "y.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 370 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(at_break,gen_zero);}
#line 5301 "y.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 371 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(at_continue,gen_zero);}
#line 5307 "y.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 372 "input_parser.yy" /* yacc.c:1646  */
    { 
	/*
	  gen kk(identificateur("index"));
	  vecteur v(*$6._VECTptr);
          const giac::context * contextptr = giac_yyget_extra(scanner);
	  v.insert(v.begin(),symb_sto(symb_at($4,kk,contextptr),$2));
	  $$=symbolic(*$1._FUNCptr,makevecteur(symb_sto(xcas_mode(contextptr)!=0,kk),symb_inferieur_strict(kk,symb_size($4)+(xcas_mode(contextptr)!=0)),symb_sto(symb_plus(kk,gen(1)),kk),symb_bloc(v))); 
          */
          if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=2 && (yyvsp[0]).val!=9)
	    giac_yyerror(scanner,"missing loop end delimiter");
 	  bool rg=(yyvsp[-3]).is_symb_of_sommet(at_range);
          gen f=(yyvsp[-3]).type==_SYMB?(yyvsp[-3])._SYMBptr->feuille:0,inc=1;
          if (rg){
            if (f.type!=_VECT) f=makesequence(0,f);
            vecteur v=*f._VECTptr;
            if (v.size()==3) inc=v[2];
            if (v.size()>=2) f=makesequence(v.front(),v[1]-inc);
          }
          if (inc.type==_INT_  && inc.val!=0 && f.type==_VECT && f._VECTptr->size()==2 && (rg || ((yyvsp[-3]).is_symb_of_sommet(at_interval) 
	  // && f._VECTptr->front().type==_INT_ && f._VECTptr->back().type==_INT_ 
	  )))
            (yyval)=symbolic(*(yyvsp[-6])._FUNCptr,makevecteur(symb_sto(f._VECTptr->front(),(yyvsp[-5])),inc.val>0?symb_inferieur_egal((yyvsp[-5]),f._VECTptr->back()):symb_superieur_egal((yyvsp[-5]),f._VECTptr->back()),symb_sto(symb_plus((yyvsp[-5]),inc),(yyvsp[-5])),symb_bloc((yyvsp[-1]))));
          else 
            (yyval)=symbolic(*(yyvsp[-6])._FUNCptr,makevecteur(1,symbolic(*(yyvsp[-6])._FUNCptr,makevecteur((yyvsp[-5]),(yyvsp[-3]))),1,symb_bloc((yyvsp[-1]))));
	  }
#line 5337 "y.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 397 "input_parser.yy" /* yacc.c:1646  */
    { 
          if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=2 && (yyvsp[0]).val!=9)
	    giac_yyerror(scanner,"missing loop end delimiter");
	  (yyval)=symbolic(*(yyvsp[-8])._FUNCptr,makevecteur(1,symbolic(*(yyvsp[-8])._FUNCptr,makevecteur((yyvsp[-7]),(yyvsp[-5]),symb_bloc((yyvsp[-1])))),1,symb_bloc((yyvsp[-3]))));
	  }
#line 5347 "y.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 402 "input_parser.yy" /* yacc.c:1646  */
    { 
          if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=2 && (yyvsp[0]).val!=9) giac_yyerror(scanner,"missing loop end delimiter");
          gen tmp,st=(yyvsp[-3]);  
       if (st==1 && (yyvsp[-5])!=1) st=(yyvsp[-5]);
          const giac::context * contextptr = giac_yyget_extra(scanner);
	  if (!lidnt(st).empty())
            *logptr(contextptr) << "Warning, step is not numeric " << st << '\n';
          bool b=has_evalf(st,tmp,1,context0);
          if (!b || is_positive(tmp,context0)) 
             (yyval)=symbolic(*(yyvsp[-8])._FUNCptr,makevecteur(symb_sto((yyvsp[-6]),(yyvsp[-7])),symb_inferieur_egal((yyvsp[-7]),(yyvsp[-4])),symb_sto(symb_plus((yyvsp[-7]),b?abs(st,context0):symb_abs(st)),(yyvsp[-7])),symb_bloc((yyvsp[-1])))); 
          else 
            (yyval)=symbolic(*(yyvsp[-8])._FUNCptr,makevecteur(symb_sto((yyvsp[-6]),(yyvsp[-7])),symb_superieur_egal((yyvsp[-7]),(yyvsp[-4])),symb_sto(symb_plus((yyvsp[-7]),st),(yyvsp[-7])),symb_bloc((yyvsp[-1])))); 
        }
#line 5365 "y.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 415 "input_parser.yy" /* yacc.c:1646  */
    { 
          if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=2 && (yyvsp[0]).val!=9) giac_yyerror(scanner,"missing loop end delimiter");
         gen tmp,st=(yyvsp[-5]); 
        if (st==1 && (yyvsp[-4])!=1) st=(yyvsp[-4]);
         const giac::context * contextptr = giac_yyget_extra(scanner);
	 if (!lidnt(st).empty())
            *logptr(contextptr) << "Warning, step is not numeric " << st << '\n';
         bool b=has_evalf(st,tmp,1,context0);
         if (!b || is_positive(tmp,context0)) 
           (yyval)=symbolic(*(yyvsp[-8])._FUNCptr,makevecteur(symb_sto((yyvsp[-6]),(yyvsp[-7])),symb_inferieur_egal((yyvsp[-7]),(yyvsp[-3])),symb_sto(symb_plus((yyvsp[-7]),b?abs(st,context0):symb_abs(st)),(yyvsp[-7])),symb_bloc((yyvsp[-1])))); 
         else 
           (yyval)=symbolic(*(yyvsp[-8])._FUNCptr,makevecteur(symb_sto((yyvsp[-6]),(yyvsp[-7])),symb_superieur_egal((yyvsp[-7]),(yyvsp[-3])),symb_sto(symb_plus((yyvsp[-7]),st),(yyvsp[-7])),symb_bloc((yyvsp[-1])))); 
        }
#line 5383 "y.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 428 "input_parser.yy" /* yacc.c:1646  */
    { 
          if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=2 && (yyvsp[0]).val!=9) giac_yyerror(scanner,"missing loop end delimiter");
          (yyval)=symbolic(*(yyvsp[-6])._FUNCptr,makevecteur(symb_sto((yyvsp[-4]),(yyvsp[-5])),gen(1),symb_sto(symb_plus((yyvsp[-5]),(yyvsp[-3])),(yyvsp[-5])),symb_bloc((yyvsp[-1])))); 
        }
#line 5392 "y.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 432 "input_parser.yy" /* yacc.c:1646  */
    { 
          if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=2 && (yyvsp[0]).val!=9 && (yyvsp[0]).val!=8) giac_yyerror(scanner,"missing loop end delimiter");
          (yyval)=symbolic(*(yyvsp[-8])._FUNCptr,makevecteur(symb_sto((yyvsp[-6]),(yyvsp[-7])),(yyvsp[-3]),symb_sto(symb_plus((yyvsp[-7]),(yyvsp[-5])),(yyvsp[-7])),symb_bloc((yyvsp[-1])))); 
        }
#line 5401 "y.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 436 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = gen(*(yyvsp[0])._FUNCptr,4);}
#line 5407 "y.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 441 "input_parser.yy" /* yacc.c:1646  */
    { 
        vecteur v=gen2vecteur((yyvsp[-2]));
        v.push_back(symb_ifte(equaltosame((yyvsp[0])),symbolic(at_break,gen_zero),0));
	(yyval)=symbolic(*(yyvsp[-3])._FUNCptr,makevecteur(gen_zero,1,gen_zero,symb_bloc(v))); 
	}
#line 5417 "y.tab.c" /* yacc.c:1646  */
    break;

  case 95:
#line 446 "input_parser.yy" /* yacc.c:1646  */
    { 
        if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=2 && (yyvsp[0]).val!=9) giac_yyerror(scanner,"missing loop end delimiter");
        vecteur v=gen2vecteur((yyvsp[-3]));
        v.push_back(symb_ifte(equaltosame((yyvsp[-1])),symbolic(at_break,gen_zero),0));
	(yyval)=symbolic(*(yyvsp[-4])._FUNCptr,makevecteur(gen_zero,1,gen_zero,symb_bloc(v))); 
	}
#line 5428 "y.tab.c" /* yacc.c:1646  */
    break;

  case 96:
#line 452 "input_parser.yy" /* yacc.c:1646  */
    {
          if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=4) giac_yyerror(scanner,"missing iferr end delimiter");
           (yyval)=symbolic(at_try_catch,makevecteur(symb_bloc((yyvsp[-5])),0,symb_bloc((yyvsp[-3])),symb_bloc((yyvsp[-1]))));
        }
#line 5437 "y.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 456 "input_parser.yy" /* yacc.c:1646  */
    {
          if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=4) giac_yyerror(scanner,"missing iferr end delimiter");
           (yyval)=symbolic(at_try_catch,makevecteur(symb_bloc((yyvsp[-3])),0,symb_bloc((yyvsp[-1])),symb_bloc(0)));
        }
#line 5446 "y.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 460 "input_parser.yy" /* yacc.c:1646  */
    {(yyval)=symbolic(at_piecewise,(yyvsp[-1])); }
#line 5452 "y.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 461 "input_parser.yy" /* yacc.c:1646  */
    { 
	(yyval)=(yyvsp[0]); 
	// $$.subtype=1; 
	}
#line 5461 "y.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 465 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[-1]); /* $$.subtype=1; */ }
#line 5467 "y.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 466 "input_parser.yy" /* yacc.c:1646  */
    { (yyval) = symb_dollar((yyvsp[0])); }
#line 5473 "y.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 467 "input_parser.yy" /* yacc.c:1646  */
    {(yyval)=symb_dollar(gen(makevecteur((yyvsp[-4]),(yyvsp[-2]),(yyvsp[0])) ,_SEQ__VECT));}
#line 5479 "y.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 468 "input_parser.yy" /* yacc.c:1646  */
    { (yyval) = symb_dollar(gen(makevecteur((yyvsp[-2]),(yyvsp[0])) ,_SEQ__VECT)); }
#line 5485 "y.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 469 "input_parser.yy" /* yacc.c:1646  */
    { (yyval) = symb_dollar(gen(makevecteur((yyvsp[-2]),(yyvsp[0])) ,_SEQ__VECT)); }
#line 5491 "y.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 470 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_dollar((yyvsp[0])); }
#line 5497 "y.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 471 "input_parser.yy" /* yacc.c:1646  */
    {  //CERR << $1 << " compose " << $2 << $3 << '\n';
(yyval) = symbolic(*(yyvsp[-1])._FUNCptr,gen(makevecteur((yyvsp[-2]),python_compat(giac_yyget_extra(scanner))?denest_sto((yyvsp[0])):(yyvsp[0])) ,_SEQ__VECT)); }
#line 5504 "y.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 473 "input_parser.yy" /* yacc.c:1646  */
    {(yyval)=symbolic(at_ans,-1);}
#line 5510 "y.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 474 "input_parser.yy" /* yacc.c:1646  */
    { (yyval) = symbolic(((yyvsp[-1]).type==_FUNC?*(yyvsp[-1])._FUNCptr:*at_union),gen(makevecteur((yyvsp[-2]),(yyvsp[0])) ,_SEQ__VECT)); }
#line 5516 "y.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 475 "input_parser.yy" /* yacc.c:1646  */
    { (yyval) = symbolic(((yyvsp[-2]).type==_FUNC?*(yyvsp[-2])._FUNCptr:*at_union),gen(makevecteur((yyvsp[-3]),(yyvsp[-3])*(yyvsp[-1])/100) ,_SEQ__VECT)); }
#line 5522 "y.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 476 "input_parser.yy" /* yacc.c:1646  */
    { (yyval) = symb_intersect(gen(makevecteur((yyvsp[-2]),(yyvsp[0])) ,_SEQ__VECT)); }
#line 5528 "y.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 477 "input_parser.yy" /* yacc.c:1646  */
    { (yyval) = symb_minus(gen(makevecteur((yyvsp[-2]),(yyvsp[0])) ,_SEQ__VECT)); }
#line 5534 "y.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 478 "input_parser.yy" /* yacc.c:1646  */
    { 
	(yyval)=symbolic(*(yyvsp[-1])._FUNCptr,gen(makevecteur((yyvsp[-2]),(yyvsp[0])) ,_SEQ__VECT)); 
	}
#line 5542 "y.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 481 "input_parser.yy" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 5548 "y.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 482 "input_parser.yy" /* yacc.c:1646  */
    {if ((yyvsp[-1]).type==_FUNC) (yyval)=(yyvsp[-1]); else { 
          // const giac::context * contextptr = giac_yyget_extra(scanner);
          (yyval)=symb_quote((yyvsp[-1]));
          } 
        }
#line 5558 "y.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 487 "input_parser.yy" /* yacc.c:1646  */
    {
          const giac::context * contextptr = giac_yyget_extra(scanner);
	  (yyval) = symb_at((yyvsp[-3]),(yyvsp[-1]),contextptr);
        }
#line 5567 "y.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 491 "input_parser.yy" /* yacc.c:1646  */
    {
          const giac::context * contextptr = giac_yyget_extra(scanner);
	  (yyval) = symbolic(at_of,gen(makevecteur((yyvsp[-5]),(yyvsp[-2])) ,_SEQ__VECT));
        }
#line 5576 "y.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 495 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = check_symb_of((yyvsp[-4]),(yyvsp[-1]),giac_yyget_extra(scanner));}
#line 5582 "y.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 496 "input_parser.yy" /* yacc.c:1646  */
    {
	if ( ((yyvsp[-2])==_LIST__VECT && python_compat(giac_yyget_extra(scanner))) ||
              python_compat(giac_yyget_extra(scanner))==2){
           if (python_compat(giac_yyget_extra(scanner))==2)
             (yyval)=change_subtype((yyvsp[-1]),_TUPLE__VECT);
           else
             (yyval)=symbolic(at_python_list,(yyvsp[-1]));
        }
        else {
 	 if (abs_calc_mode(giac_yyget_extra(scanner))==38 && (yyvsp[-1]).type==_VECT && (yyvsp[-1]).subtype==_SEQ__VECT && (yyvsp[-1])._VECTptr->size()==2 && ((yyvsp[-1])._VECTptr->front().type<=_DOUBLE_ || (yyvsp[-1])._VECTptr->front().type==_FLOAT_) && ((yyvsp[-1])._VECTptr->back().type<=_DOUBLE_ || (yyvsp[-1])._VECTptr->back().type==_FLOAT_)){ 
           const giac::context * contextptr = giac_yyget_extra(scanner);
	   gen a=evalf((yyvsp[-1])._VECTptr->front(),1,contextptr),
	       b=evalf((yyvsp[-1])._VECTptr->back(),1,contextptr);
	   if ( (a.type==_DOUBLE_ || a.type==_FLOAT_) &&
                (b.type==_DOUBLE_ || b.type==_FLOAT_))
             (yyval)= a+b*cst_i; 
           else (yyval)=(yyvsp[-1]);
  	 } else {
              if (calc_mode(giac_yyget_extra(scanner))==1 && (yyvsp[-1]).type==_VECT && (yyvsp[-2])!=_LIST__VECT &&
	      (yyvsp[-1]).subtype==_SEQ__VECT && ((yyvsp[-1])._VECTptr->size()==2 || (yyvsp[-1])._VECTptr->size()==3) )
                (yyval) = gen(*(yyvsp[-1])._VECTptr,_GGB__VECT);
              else
                (yyval)=(yyvsp[-1]);
          }
	 }
        }
#line 5613 "y.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 522 "input_parser.yy" /* yacc.c:1646  */
    { 
        //cerr << $1 << " " << $2 << '\n';
        (yyval) = gen(*((yyvsp[-1])._VECTptr),(yyvsp[-2]).val);
	if ((yyvsp[-1])._VECTptr->size()==1 && (yyvsp[-1])._VECTptr->front().is_symb_of_sommet(at_ti_semi) ) {
	  (yyval)=(yyvsp[-1])._VECTptr->front();
        }
        // cerr << $$ << '\n';

        }
#line 5627 "y.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 531 "input_parser.yy" /* yacc.c:1646  */
    { 
         if ((yyvsp[-2]).type==_VECT && (yyvsp[-2]).subtype==_SEQ__VECT && !((yyvsp[0]).type==_VECT && (yyvsp[-1]).subtype==_SEQ__VECT)){ (yyval)=(yyvsp[-2]); (yyval)._VECTptr->push_back((yyvsp[0])); }
	 else
           (yyval) = makesuite((yyvsp[-2]),(yyvsp[0])); 

        }
#line 5638 "y.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 537 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=gen(vecteur(0),_SEQ__VECT); }
#line 5644 "y.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 538 "input_parser.yy" /* yacc.c:1646  */
    {(yyval)=symb_findhelp((yyvsp[0]));}
#line 5650 "y.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 539 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_interrogation((yyvsp[-2]),(yyvsp[0])); }
#line 5656 "y.tab.c" /* yacc.c:1646  */
    break;

  case 124:
#line 540 "input_parser.yy" /* yacc.c:1646  */
    {
          const giac::context * contextptr = giac_yyget_extra(scanner);
          (yyval)=symb_unit(gen(1),(yyvsp[0]),contextptr); 
          opened_quote(giac_yyget_extra(scanner)) &= 0x7ffffffd;	
        }
#line 5666 "y.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 545 "input_parser.yy" /* yacc.c:1646  */
    {
          const giac::context * contextptr = giac_yyget_extra(scanner);
          (yyval)=symb_unit((yyvsp[-2]),(yyvsp[0]),contextptr); 
          opened_quote(giac_yyget_extra(scanner)) &= 0x7ffffffd;        }
#line 5675 "y.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 549 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_pow((yyvsp[-1]),(yyvsp[0])); }
#line 5681 "y.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 550 "input_parser.yy" /* yacc.c:1646  */
    { 
        const giac::context * contextptr = giac_yyget_extra(scanner);
#ifdef HAVE_SIGNAL_H_OLD
	messages_to_print += parser_filename(contextptr) + parser_error(contextptr); 
	/* *logptr(giac_yyget_extra(scanner)) << messages_to_print; */
#endif
	(yyval)=undef;
        spread_formula(false,contextptr); 
	}
#line 5695 "y.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 559 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 5701 "y.tab.c" /* yacc.c:1646  */
    break;

  case 129:
#line 560 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symbolic(*(yyvsp[-1])._FUNCptr,(yyvsp[0])); }
#line 5707 "y.tab.c" /* yacc.c:1646  */
    break;

  case 130:
#line 561 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(*(yyvsp[0])._FUNCptr,gen(vecteur(0),_SEQ__VECT));}
#line 5713 "y.tab.c" /* yacc.c:1646  */
    break;

  case 131:
#line 562 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(*(yyvsp[-2])._FUNCptr,gen(vecteur(0),_SEQ__VECT));}
#line 5719 "y.tab.c" /* yacc.c:1646  */
    break;

  case 132:
#line 563 "input_parser.yy" /* yacc.c:1646  */
    {
          const giac::context * contextptr = giac_yyget_extra(scanner);
          (yyval) = symb_local((yyvsp[-1]),contextptr);
        }
#line 5728 "y.tab.c" /* yacc.c:1646  */
    break;

  case 133:
#line 567 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = gen(at_local,2);}
#line 5734 "y.tab.c" /* yacc.c:1646  */
    break;

  case 134:
#line 568 "input_parser.yy" /* yacc.c:1646  */
    {
	(yyval) = symbolic(*(yyvsp[-5])._FUNCptr,makevecteur(equaltosame((yyvsp[-3])),symb_bloc((yyvsp[-1])),(yyvsp[0])));
	}
#line 5742 "y.tab.c" /* yacc.c:1646  */
    break;

  case 135:
#line 571 "input_parser.yy" /* yacc.c:1646  */
    {
        vecteur v=makevecteur(equaltosame((yyvsp[-4])),(yyvsp[-2]),(yyvsp[0]));
	// *logptr(giac_yyget_extra(scanner)) << v << '\n';
	(yyval) = symbolic(*(yyvsp[-6])._FUNCptr,v);
	}
#line 5752 "y.tab.c" /* yacc.c:1646  */
    break;

  case 136:
#line 576 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_rpn_prog((yyvsp[-1])); }
#line 5758 "y.tab.c" /* yacc.c:1646  */
    break;

  case 137:
#line 577 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 5764 "y.tab.c" /* yacc.c:1646  */
    break;

  case 138:
#line 578 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symbolic(at_maple_lib,makevecteur((yyvsp[-3]),(yyvsp[-1]))); }
#line 5770 "y.tab.c" /* yacc.c:1646  */
    break;

  case 139:
#line 579 "input_parser.yy" /* yacc.c:1646  */
    { 
          if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=3) giac_yyerror(scanner,"missing func/prog/proc end delimiter");
          const giac::context * contextptr = giac_yyget_extra(scanner);
           (yyval)=symb_program((yyvsp[-4]),gen_zero*(yyvsp[-4]),symb_local((yyvsp[-2]),(yyvsp[-1]),contextptr),contextptr); 
        }
#line 5780 "y.tab.c" /* yacc.c:1646  */
    break;

  case 140:
#line 584 "input_parser.yy" /* yacc.c:1646  */
    { 
          if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=3) giac_yyerror(scanner,"missing func/prog/proc end delimiter");
          const giac::context * contextptr = giac_yyget_extra(scanner);
           (yyval)=symb_program_sto((yyvsp[-4]),gen_zero*(yyvsp[-4]),symb_local((yyvsp[-2]),(yyvsp[-1]),contextptr),(yyvsp[-6]),false,contextptr); 
        }
#line 5790 "y.tab.c" /* yacc.c:1646  */
    break;

  case 141:
#line 589 "input_parser.yy" /* yacc.c:1646  */
    { 
          if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=3) giac_yyerror(scanner,"missing func/prog/proc end delimiter");
          const giac::context * contextptr = giac_yyget_extra(scanner);
           (yyval)=symb_program_sto((yyvsp[-3]),gen_zero*(yyvsp[-3]),symb_bloc((yyvsp[-1])),(yyvsp[-5]),false,contextptr); 
        }
#line 5800 "y.tab.c" /* yacc.c:1646  */
    break;

  case 142:
#line 594 "input_parser.yy" /* yacc.c:1646  */
    { 
          if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=3) giac_yyerror(scanner,"missing func/prog/proc end delimiter");
          const giac::context * contextptr = giac_yyget_extra(scanner);
           (yyval)=symb_program_sto((yyvsp[-5]),gen_zero*(yyvsp[-5]),symb_local((yyvsp[-2]),(yyvsp[-1]),contextptr),(yyvsp[-7]),false,contextptr); 
        }
#line 5810 "y.tab.c" /* yacc.c:1646  */
    break;

  case 143:
#line 599 "input_parser.yy" /* yacc.c:1646  */
    { 
          if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=3) giac_yyerror(scanner,"missing func/prog/proc end delimiter");
          const giac::context * contextptr = giac_yyget_extra(scanner);
         (yyval)=symb_program((yyvsp[-5]),gen_zero*(yyvsp[-5]),symb_local((yyvsp[-3]),(yyvsp[-1]),contextptr),contextptr); 
        }
#line 5820 "y.tab.c" /* yacc.c:1646  */
    break;

  case 144:
#line 604 "input_parser.yy" /* yacc.c:1646  */
    { 
          if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=3) giac_yyerror(scanner,"missing func/prog/proc end delimiter");
          const giac::context * contextptr = giac_yyget_extra(scanner);
           (yyval)=symb_program_sto((yyvsp[-5]),gen_zero*(yyvsp[-5]),symb_local((yyvsp[-2]),(yyvsp[-1]),contextptr),(yyvsp[-7]),false,contextptr); 
        }
#line 5830 "y.tab.c" /* yacc.c:1646  */
    break;

  case 145:
#line 609 "input_parser.yy" /* yacc.c:1646  */
    { 
          if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=3) giac_yyerror(scanner,"missing func/prog/proc end delimiter");
          const giac::context * contextptr = giac_yyget_extra(scanner);
           (yyval)=symb_program_sto((yyvsp[-6]),gen_zero*(yyvsp[-6]),symb_local((yyvsp[-2]),(yyvsp[-1]),contextptr),(yyvsp[-8]),false,contextptr); 
        }
#line 5840 "y.tab.c" /* yacc.c:1646  */
    break;

  case 146:
#line 614 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(*(yyvsp[-8])._FUNCptr,makevecteur((yyvsp[-6]),equaltosame((yyvsp[-4])),(yyvsp[-2]),symb_bloc((yyvsp[0]))));}
#line 5846 "y.tab.c" /* yacc.c:1646  */
    break;

  case 147:
#line 615 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(*(yyvsp[-9])._FUNCptr,makevecteur((yyvsp[-7]),equaltosame((yyvsp[-5])),(yyvsp[-3]),(yyvsp[-1])));}
#line 5852 "y.tab.c" /* yacc.c:1646  */
    break;

  case 148:
#line 616 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(*(yyvsp[-3])._FUNCptr,gen2vecteur((yyvsp[-1])));}
#line 5858 "y.tab.c" /* yacc.c:1646  */
    break;

  case 149:
#line 617 "input_parser.yy" /* yacc.c:1646  */
    {(yyval)=symbolic(at_member,makesequence((yyvsp[-2]),(yyvsp[0]))); if ((yyvsp[-1])==at_not) (yyval)=symbolic(at_not,(yyval));}
#line 5864 "y.tab.c" /* yacc.c:1646  */
    break;

  case 150:
#line 618 "input_parser.yy" /* yacc.c:1646  */
    {(yyval)=symbolic(at_not,symbolic(at_member,makesequence((yyvsp[-3]),(yyvsp[0]))));}
#line 5870 "y.tab.c" /* yacc.c:1646  */
    break;

  case 151:
#line 619 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symbolic(at_apply,makesequence(symbolic(at_program,makesequence((yyvsp[-3]),0*(yyvsp[-3]),vecteur(1,(yyvsp[-5])))),(yyvsp[-1]))); if ((yyvsp[-6])==_TABLE__VECT) (yyval)=symbolic(at_table,(yyval));}
#line 5876 "y.tab.c" /* yacc.c:1646  */
    break;

  case 152:
#line 620 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symbolic(at_apply,symbolic(at_program,makesequence((yyvsp[-5]),0*(yyvsp[-5]),vecteur(1,(yyvsp[-7])))),symbolic(at_select,makesequence(symbolic(at_program,makesequence((yyvsp[-5]),0*(yyvsp[-5]),(yyvsp[-1]))),(yyvsp[-3])))); if ((yyvsp[-8])==_TABLE__VECT) (yyval)=symbolic(at_table,(yyval));}
#line 5882 "y.tab.c" /* yacc.c:1646  */
    break;

  case 153:
#line 621 "input_parser.yy" /* yacc.c:1646  */
    { 
	vecteur v=makevecteur(gen_zero,equaltosame((yyvsp[-2])),gen_zero,symb_bloc((yyvsp[0])));
	(yyval)=symbolic(*(yyvsp[-4])._FUNCptr,v); 
	}
#line 5891 "y.tab.c" /* yacc.c:1646  */
    break;

  case 154:
#line 625 "input_parser.yy" /* yacc.c:1646  */
    { 
	(yyval)=symbolic(*(yyvsp[-5])._FUNCptr,makevecteur(gen_zero,equaltosame((yyvsp[-3])),gen_zero,(yyvsp[-1]))); 
	}
#line 5899 "y.tab.c" /* yacc.c:1646  */
    break;

  case 155:
#line 628 "input_parser.yy" /* yacc.c:1646  */
    { 
          if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=9 && (yyvsp[0]).val!=8) giac_yyerror(scanner,"missing loop end delimiter");
	  (yyval)=symbolic(*(yyvsp[-4])._FUNCptr,makevecteur(gen_zero,equaltosame((yyvsp[-3])),gen_zero,symb_bloc((yyvsp[-1])))); 
        }
#line 5908 "y.tab.c" /* yacc.c:1646  */
    break;

  case 156:
#line 632 "input_parser.yy" /* yacc.c:1646  */
    { 
          if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=9 && (yyvsp[0]).val!=8) giac_yyerror(scanner,"missing loop end delimiter");
          (yyval)=symbolic(*(yyvsp[-4])._FUNCptr,makevecteur(gen_zero,equaltosame((yyvsp[-3])),gen_zero,symb_bloc((yyvsp[-1])))); 
        }
#line 5917 "y.tab.c" /* yacc.c:1646  */
    break;

  case 157:
#line 636 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_try_catch(makevecteur(symb_bloc((yyvsp[-5])),(yyvsp[-2]),symb_bloc((yyvsp[0]))));}
#line 5923 "y.tab.c" /* yacc.c:1646  */
    break;

  case 158:
#line 637 "input_parser.yy" /* yacc.c:1646  */
    {(yyval)=symb_try_catch(gen2vecteur((yyvsp[-1])));}
#line 5929 "y.tab.c" /* yacc.c:1646  */
    break;

  case 159:
#line 638 "input_parser.yy" /* yacc.c:1646  */
    {(yyval)=gen(at_try_catch,3);}
#line 5935 "y.tab.c" /* yacc.c:1646  */
    break;

  case 160:
#line 639 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_case((yyvsp[-4]),(yyvsp[-1])); }
#line 5941 "y.tab.c" /* yacc.c:1646  */
    break;

  case 161:
#line 640 "input_parser.yy" /* yacc.c:1646  */
    { (yyval) = symb_case((yyvsp[-1])); }
#line 5947 "y.tab.c" /* yacc.c:1646  */
    break;

  case 162:
#line 641 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_case((yyvsp[-2]),(yyvsp[-1])); }
#line 5953 "y.tab.c" /* yacc.c:1646  */
    break;

  case 163:
#line 642 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[-1]); }
#line 5959 "y.tab.c" /* yacc.c:1646  */
    break;

  case 164:
#line 643 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 5965 "y.tab.c" /* yacc.c:1646  */
    break;

  case 165:
#line 644 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = gen(*(yyvsp[-1])._FUNCptr,0);}
#line 5971 "y.tab.c" /* yacc.c:1646  */
    break;

  case 166:
#line 645 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symbolic(*(yyvsp[-2])._FUNCptr,makevecteur(gen_zero,gen(1),gen_zero,symb_bloc((yyvsp[-1])))); }
#line 5977 "y.tab.c" /* yacc.c:1646  */
    break;

  case 167:
#line 646 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(*(yyvsp[-3])._FUNCptr,makevecteur(equaltosame((yyvsp[-2])),(yyvsp[0]),0));}
#line 5983 "y.tab.c" /* yacc.c:1646  */
    break;

  case 168:
#line 647 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_try_catch(makevecteur(symb_bloc((yyvsp[-3])),at_break,symb_bloc((yyvsp[-1])))); }
#line 5989 "y.tab.c" /* yacc.c:1646  */
    break;

  case 169:
#line 648 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_try_catch(makevecteur(symb_bloc((yyvsp[-2])),at_break,0)); }
#line 5995 "y.tab.c" /* yacc.c:1646  */
    break;

  case 170:
#line 649 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_try_catch(makevecteur(symb_bloc((yyvsp[-4])),at_break,symb_bloc((yyvsp[-1])))); }
#line 6001 "y.tab.c" /* yacc.c:1646  */
    break;

  case 171:
#line 650 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_try_catch(makevecteur(symb_bloc((yyvsp[-3])),at_break,0)); }
#line 6007 "y.tab.c" /* yacc.c:1646  */
    break;

  case 172:
#line 651 "input_parser.yy" /* yacc.c:1646  */
    { vecteur v1(gen2vecteur((yyvsp[-2]))),v3(gen2vecteur((yyvsp[0]))); (yyval)=symbolic(at_ti_semi,makevecteur(v1,v3)); }
#line 6013 "y.tab.c" /* yacc.c:1646  */
    break;

  case 173:
#line 652 "input_parser.yy" /* yacc.c:1646  */
    { 
          const giac::context * contextptr = giac_yyget_extra(scanner);
          (yyval)=symb_program_sto((yyvsp[-9]),(yyvsp[-9])*gen_zero,symb_local((yyvsp[-3]),mergevecteur(*(yyvsp[-6])._VECTptr,*(yyvsp[-1])._VECTptr),contextptr),(yyvsp[-11]),false,contextptr); 
	}
#line 6022 "y.tab.c" /* yacc.c:1646  */
    break;

  case 174:
#line 656 "input_parser.yy" /* yacc.c:1646  */
    { 
          const giac::context * contextptr = giac_yyget_extra(scanner);
	(yyval)=symb_program_sto((yyvsp[-8]),(yyvsp[-8])*gen_zero,symb_local((yyvsp[-3]),mergevecteur(*(yyvsp[-5])._VECTptr,*(yyvsp[-1])._VECTptr),contextptr),(yyvsp[-10]),false,contextptr); 
	}
#line 6031 "y.tab.c" /* yacc.c:1646  */
    break;

  case 175:
#line 660 "input_parser.yy" /* yacc.c:1646  */
    { 
          const giac::context * contextptr = giac_yyget_extra(scanner);
	(yyval)=symb_program_sto((yyvsp[-8]),(yyvsp[-8])*gen_zero,symb_local((yyvsp[-3]),(yyvsp[-1]),contextptr),(yyvsp[-10]),false,contextptr); 
	}
#line 6040 "y.tab.c" /* yacc.c:1646  */
    break;

  case 176:
#line 664 "input_parser.yy" /* yacc.c:1646  */
    { 
	(yyval)=symb_program_sto((yyvsp[-4]),(yyvsp[-4])*gen_zero,symb_bloc((yyvsp[-1])),(yyvsp[-6]),false,giac_yyget_extra(scanner)); 
	}
#line 6048 "y.tab.c" /* yacc.c:1646  */
    break;

  case 177:
#line 667 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symbolic(*(yyvsp[-2])._FUNCptr,(yyvsp[-1])); }
#line 6054 "y.tab.c" /* yacc.c:1646  */
    break;

  case 178:
#line 668 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symbolic(*(yyvsp[-1])._FUNCptr,(yyvsp[0])); }
#line 6060 "y.tab.c" /* yacc.c:1646  */
    break;

  case 179:
#line 669 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 6066 "y.tab.c" /* yacc.c:1646  */
    break;

  case 180:
#line 670 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_program_sto((yyvsp[-3]),(yyvsp[-3])*gen_zero,(yyvsp[0]),(yyvsp[-5]),false,giac_yyget_extra(scanner));}
#line 6072 "y.tab.c" /* yacc.c:1646  */
    break;

  case 181:
#line 671 "input_parser.yy" /* yacc.c:1646  */
    { 
          const giac::context * contextptr = giac_yyget_extra(scanner);
          (yyval)=symb_program_sto((yyvsp[-9]),(yyvsp[-9])*gen_zero,symb_local((yyvsp[-3]),(yyvsp[-1]),contextptr),(yyvsp[-11]),false,contextptr);
        }
#line 6081 "y.tab.c" /* yacc.c:1646  */
    break;

  case 182:
#line 675 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_program_sto((yyvsp[-5]),(yyvsp[-5])*gen_zero,symb_bloc((yyvsp[-1])),(yyvsp[-7]),false,giac_yyget_extra(scanner)); }
#line 6087 "y.tab.c" /* yacc.c:1646  */
    break;

  case 183:
#line 676 "input_parser.yy" /* yacc.c:1646  */
    {
           vecteur & v=*(yyvsp[-3])._VECTptr;
           if ( (v.size()<3) || v[0].type!=_IDNT){
             *logptr(giac_yyget_extra(scanner)) << "Syntax For name,begin,end[,step]" << '\n';
             (yyval)=undef;
           }
           else {
             gen pas(gen(1));
             if (v.size()==4)
               pas=v[3];
             gen condition;
             if (is_positive(-pas,0))
               condition=symb_superieur_egal(v[0],v[2]);
            else
               condition=symb_inferieur_egal(v[0],v[2]);
            vecteur w=makevecteur(symb_sto(v[1],v[0]),condition,symb_sto(symb_plus(v[0],pas),v[0]),symb_bloc((yyvsp[-1])));
             (yyval)=symbolic(*(yyvsp[-4])._FUNCptr,w);
           }
	}
#line 6111 "y.tab.c" /* yacc.c:1646  */
    break;

  case 184:
#line 695 "input_parser.yy" /* yacc.c:1646  */
    { 
	vecteur v=makevecteur(gen_zero,equaltosame((yyvsp[-3])),gen_zero,symb_bloc((yyvsp[-1])));
	(yyval)=symbolic(*(yyvsp[-4])._FUNCptr,v); 
	}
#line 6120 "y.tab.c" /* yacc.c:1646  */
    break;

  case 185:
#line 707 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 6126 "y.tab.c" /* yacc.c:1646  */
    break;

  case 186:
#line 708 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=makesequence((yyvsp[-2]),(yyvsp[0]));}
#line 6132 "y.tab.c" /* yacc.c:1646  */
    break;

  case 187:
#line 709 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 6138 "y.tab.c" /* yacc.c:1646  */
    break;

  case 188:
#line 710 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 6144 "y.tab.c" /* yacc.c:1646  */
    break;

  case 189:
#line 713 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 6150 "y.tab.c" /* yacc.c:1646  */
    break;

  case 190:
#line 714 "input_parser.yy" /* yacc.c:1646  */
    { 
	       gen tmp((yyvsp[0])); 
	       // tmp.subtype=1; 
	       //$$=symb_check_type(makevecteur(tmp,$1),context0); 
               (yyval)=symbolic(at_deuxpoints,makesequence((yyvsp[-2]),(yyvsp[0])));
          }
#line 6161 "y.tab.c" /* yacc.c:1646  */
    break;

  case 191:
#line 720 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_double_deux_points(makevecteur((yyvsp[-2]),(yyvsp[0]))); }
#line 6167 "y.tab.c" /* yacc.c:1646  */
    break;

  case 192:
#line 721 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_double_deux_points(makevecteur((yyvsp[-2]),(yyvsp[0]))); }
#line 6173 "y.tab.c" /* yacc.c:1646  */
    break;

  case 193:
#line 722 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_double_deux_points(makevecteur((yyvsp[-2]),(yyvsp[0]))); }
#line 6179 "y.tab.c" /* yacc.c:1646  */
    break;

  case 194:
#line 723 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_double_deux_points(makevecteur((yyvsp[-4]),(yyvsp[-1]))); }
#line 6185 "y.tab.c" /* yacc.c:1646  */
    break;

  case 195:
#line 724 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_double_deux_points(makevecteur(0,(yyvsp[0]))); }
#line 6191 "y.tab.c" /* yacc.c:1646  */
    break;

  case 196:
#line 725 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_double_deux_points(makevecteur((yyvsp[-2]),(yyvsp[0]))); }
#line 6197 "y.tab.c" /* yacc.c:1646  */
    break;

  case 197:
#line 733 "input_parser.yy" /* yacc.c:1646  */
    { 
	  gen tmp((yyvsp[-1])); 
	  // tmp.subtype=1; 
	  // $$=symb_check_type(makevecteur(tmp,$2),context0); 
          (yyval)=symbolic(at_deuxpoints,makesequence((yyvsp[0]),(yyvsp[-1])));
	  }
#line 6208 "y.tab.c" /* yacc.c:1646  */
    break;

  case 198:
#line 739 "input_parser.yy" /* yacc.c:1646  */
    {(yyval)=symbolic(*(yyvsp[-1])._FUNCptr,(yyvsp[0])); }
#line 6214 "y.tab.c" /* yacc.c:1646  */
    break;

  case 199:
#line 742 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 6220 "y.tab.c" /* yacc.c:1646  */
    break;

  case 200:
#line 743 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 6226 "y.tab.c" /* yacc.c:1646  */
    break;

  case 201:
#line 746 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=makevecteur(vecteur(0),vecteur(0)); }
#line 6232 "y.tab.c" /* yacc.c:1646  */
    break;

  case 202:
#line 747 "input_parser.yy" /* yacc.c:1646  */
    { vecteur v1 =gen2vecteur((yyvsp[-1])); vecteur v2=gen2vecteur((yyvsp[0])); (yyval)=makevecteur(mergevecteur(gen2vecteur(v1[0]),gen2vecteur(v2[0])),mergevecteur(gen2vecteur(v1[1]),gen2vecteur(v2[1]))); }
#line 6238 "y.tab.c" /* yacc.c:1646  */
    break;

  case 203:
#line 748 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 6244 "y.tab.c" /* yacc.c:1646  */
    break;

  case 204:
#line 752 "input_parser.yy" /* yacc.c:1646  */
    { if ((yyvsp[-1]).type==_VECT) (yyval)=gen(*(yyvsp[-1])._VECTptr,_RPN_STACK__VECT); else (yyval)=gen(vecteur(1,(yyvsp[-1])),_RPN_STACK__VECT); }
#line 6250 "y.tab.c" /* yacc.c:1646  */
    break;

  case 205:
#line 753 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=gen(vecteur(0),_RPN_STACK__VECT); }
#line 6256 "y.tab.c" /* yacc.c:1646  */
    break;

  case 206:
#line 756 "input_parser.yy" /* yacc.c:1646  */
    { if (!(yyvsp[-2]).val) (yyval)=makevecteur((yyvsp[-1]),vecteur(0)); else (yyval)=makevecteur(vecteur(0),(yyvsp[-1]));}
#line 6262 "y.tab.c" /* yacc.c:1646  */
    break;

  case 207:
#line 759 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[-1]); }
#line 6268 "y.tab.c" /* yacc.c:1646  */
    break;

  case 208:
#line 762 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=gen(vecteur(1,(yyvsp[0])),_SEQ__VECT); }
#line 6274 "y.tab.c" /* yacc.c:1646  */
    break;

  case 209:
#line 763 "input_parser.yy" /* yacc.c:1646  */
    { 
	       vecteur v=*(yyvsp[-2])._VECTptr;
	       v.push_back((yyvsp[0]));
	       (yyval)=gen(v,_SEQ__VECT);
	     }
#line 6284 "y.tab.c" /* yacc.c:1646  */
    break;

  case 210:
#line 770 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 6290 "y.tab.c" /* yacc.c:1646  */
    break;

  case 211:
#line 771 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=parser_symb_sto((yyvsp[0]),(yyvsp[-2]),(yyvsp[-1])==at_array_sto); }
#line 6296 "y.tab.c" /* yacc.c:1646  */
    break;

  case 212:
#line 772 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_equal((yyvsp[-2]),(yyvsp[0])); }
#line 6302 "y.tab.c" /* yacc.c:1646  */
    break;

  case 213:
#line 773 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symbolic(at_deuxpoints,makesequence((yyvsp[-2]),(yyvsp[0])));  }
#line 6308 "y.tab.c" /* yacc.c:1646  */
    break;

  case 214:
#line 774 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[-1]); }
#line 6314 "y.tab.c" /* yacc.c:1646  */
    break;

  case 215:
#line 775 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); *logptr(giac_yyget_extra(scanner)) << "Error: reserved word "<< (yyvsp[0]) <<'\n';}
#line 6320 "y.tab.c" /* yacc.c:1646  */
    break;

  case 216:
#line 776 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_double_deux_points(makevecteur((yyvsp[-2]),(yyvsp[0]))); *logptr(giac_yyget_extra(scanner)) << "Error: reserved word "<< (yyvsp[-2]) <<'\n'; }
#line 6326 "y.tab.c" /* yacc.c:1646  */
    break;

  case 217:
#line 777 "input_parser.yy" /* yacc.c:1646  */
    { 
  const giac::context * contextptr = giac_yyget_extra(scanner);
  (yyval)=string2gen("_"+(yyvsp[0]).print(contextptr),false); 
  if (!giac::first_error_line(contextptr)){
    giac::first_error_line(giac::lexer_line_number(contextptr),contextptr);
    giac:: error_token_name((yyvsp[0]).print(contextptr)+ " (reserved word)",contextptr);
  }
}
#line 6339 "y.tab.c" /* yacc.c:1646  */
    break;

  case 218:
#line 785 "input_parser.yy" /* yacc.c:1646  */
    { 
  const giac::context * contextptr = giac_yyget_extra(scanner);
  (yyval)=string2gen("_"+(yyvsp[0]).print(contextptr),false);
  if (!giac::first_error_line(contextptr)){
    giac::first_error_line(giac::lexer_line_number(contextptr),contextptr);
    giac:: error_token_name((yyvsp[0]).print(contextptr)+ " reserved word",contextptr);
  }
}
#line 6352 "y.tab.c" /* yacc.c:1646  */
    break;

  case 219:
#line 795 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=gen(1);}
#line 6358 "y.tab.c" /* yacc.c:1646  */
    break;

  case 220:
#line 796 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 6364 "y.tab.c" /* yacc.c:1646  */
    break;

  case 221:
#line 799 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=gen(vecteur(0),_SEQ__VECT); }
#line 6370 "y.tab.c" /* yacc.c:1646  */
    break;

  case 222:
#line 800 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=makesuite((yyvsp[0])); }
#line 6376 "y.tab.c" /* yacc.c:1646  */
    break;

  case 223:
#line 803 "input_parser.yy" /* yacc.c:1646  */
    { (yyval) = gen(makevecteur((yyvsp[0])),_PRG__VECT); }
#line 6382 "y.tab.c" /* yacc.c:1646  */
    break;

  case 224:
#line 805 "input_parser.yy" /* yacc.c:1646  */
    { vecteur v(1,(yyvsp[-1])); 
			  if ((yyvsp[-1]).type==_VECT) v=*((yyvsp[-1])._VECTptr); 
			  v.push_back((yyvsp[0])); 
			  (yyval) = gen(v,_PRG__VECT);
			}
#line 6392 "y.tab.c" /* yacc.c:1646  */
    break;

  case 225:
#line 810 "input_parser.yy" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 6398 "y.tab.c" /* yacc.c:1646  */
    break;

  case 226:
#line 813 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=vecteur(0); }
#line 6404 "y.tab.c" /* yacc.c:1646  */
    break;

  case 227:
#line 814 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=mergevecteur(vecteur(1,(yyvsp[-1])),*((yyvsp[0])._VECTptr));}
#line 6410 "y.tab.c" /* yacc.c:1646  */
    break;

  case 228:
#line 815 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=mergevecteur(vecteur(1,(yyvsp[-2])),*((yyvsp[0])._VECTptr));}
#line 6416 "y.tab.c" /* yacc.c:1646  */
    break;

  case 229:
#line 818 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 6422 "y.tab.c" /* yacc.c:1646  */
    break;

  case 230:
#line 888 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=gen(1); }
#line 6428 "y.tab.c" /* yacc.c:1646  */
    break;

  case 231:
#line 889 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 6434 "y.tab.c" /* yacc.c:1646  */
    break;

  case 232:
#line 892 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=gen(1); }
#line 6440 "y.tab.c" /* yacc.c:1646  */
    break;

  case 233:
#line 893 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 6446 "y.tab.c" /* yacc.c:1646  */
    break;

  case 234:
#line 894 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 6452 "y.tab.c" /* yacc.c:1646  */
    break;

  case 235:
#line 895 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 6458 "y.tab.c" /* yacc.c:1646  */
    break;

  case 236:
#line 898 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=gen(1); }
#line 6464 "y.tab.c" /* yacc.c:1646  */
    break;

  case 237:
#line 899 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 6470 "y.tab.c" /* yacc.c:1646  */
    break;

  case 238:
#line 902 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=0; }
#line 6476 "y.tab.c" /* yacc.c:1646  */
    break;

  case 239:
#line 903 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[-1]); }
#line 6482 "y.tab.c" /* yacc.c:1646  */
    break;

  case 240:
#line 904 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_bloc((yyvsp[0])); }
#line 6488 "y.tab.c" /* yacc.c:1646  */
    break;

  case 241:
#line 908 "input_parser.yy" /* yacc.c:1646  */
    { 
	(yyval) = (yyvsp[-1]);
	}
#line 6496 "y.tab.c" /* yacc.c:1646  */
    break;

  case 242:
#line 911 "input_parser.yy" /* yacc.c:1646  */
    {
          const giac::context * contextptr = giac_yyget_extra(scanner);
          (yyval) = symb_local((yyvsp[-2]),(yyvsp[-1]),contextptr);
         }
#line 6505 "y.tab.c" /* yacc.c:1646  */
    break;

  case 243:
#line 918 "input_parser.yy" /* yacc.c:1646  */
    { if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=4) giac_yyerror(scanner,"missing test end delimiter"); (yyval)=0; }
#line 6511 "y.tab.c" /* yacc.c:1646  */
    break;

  case 244:
#line 919 "input_parser.yy" /* yacc.c:1646  */
    {
          if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=4) giac_yyerror(scanner,"missing test end delimiter");
	(yyval)=symb_bloc((yyvsp[-1])); 
	}
#line 6520 "y.tab.c" /* yacc.c:1646  */
    break;

  case 245:
#line 923 "input_parser.yy" /* yacc.c:1646  */
    { 
	  (yyval)=symb_ifte(equaltosame((yyvsp[-3])),symb_bloc((yyvsp[-1])),(yyvsp[0]));
	  }
#line 6528 "y.tab.c" /* yacc.c:1646  */
    break;

  case 246:
#line 926 "input_parser.yy" /* yacc.c:1646  */
    { 
	  (yyval)=symb_ifte(equaltosame((yyvsp[-3])),symb_bloc((yyvsp[-1])),(yyvsp[0]));
	  }
#line 6536 "y.tab.c" /* yacc.c:1646  */
    break;

  case 247:
#line 931 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 6542 "y.tab.c" /* yacc.c:1646  */
    break;

  case 248:
#line 932 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 6548 "y.tab.c" /* yacc.c:1646  */
    break;

  case 249:
#line 935 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=0; }
#line 6554 "y.tab.c" /* yacc.c:1646  */
    break;

  case 250:
#line 936 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=0; }
#line 6560 "y.tab.c" /* yacc.c:1646  */
    break;

  case 251:
#line 939 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=vecteur(0); }
#line 6566 "y.tab.c" /* yacc.c:1646  */
    break;

  case 252:
#line 940 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=makevecteur(symb_bloc((yyvsp[0])));}
#line 6572 "y.tab.c" /* yacc.c:1646  */
    break;

  case 253:
#line 941 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=mergevecteur(makevecteur((yyvsp[-3]),symb_bloc((yyvsp[-1]))),*((yyvsp[0])._VECTptr));}
#line 6578 "y.tab.c" /* yacc.c:1646  */
    break;

  case 254:
#line 944 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=vecteur(0); }
#line 6584 "y.tab.c" /* yacc.c:1646  */
    break;

  case 255:
#line 945 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=vecteur(1,symb_bloc((yyvsp[0]))); }
#line 6590 "y.tab.c" /* yacc.c:1646  */
    break;

  case 256:
#line 946 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=mergevecteur(makevecteur((yyvsp[-3]),symb_bloc((yyvsp[-1]))),*((yyvsp[0])._VECTptr));}
#line 6596 "y.tab.c" /* yacc.c:1646  */
    break;

  case 257:
#line 949 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=vecteur(0); }
#line 6602 "y.tab.c" /* yacc.c:1646  */
    break;

  case 258:
#line 950 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=vecteur(1,symb_bloc((yyvsp[0]))); }
#line 6608 "y.tab.c" /* yacc.c:1646  */
    break;

  case 259:
#line 951 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=mergevecteur(makevecteur((yyvsp[-4]),symb_bloc((yyvsp[-2]))),gen2vecteur((yyvsp[0])));}
#line 6614 "y.tab.c" /* yacc.c:1646  */
    break;

  case 260:
#line 952 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=mergevecteur(makevecteur((yyvsp[-5]),symb_bloc((yyvsp[-3]))),gen2vecteur((yyvsp[0])));}
#line 6620 "y.tab.c" /* yacc.c:1646  */
    break;

  case 261:
#line 955 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 6626 "y.tab.c" /* yacc.c:1646  */
    break;


#line 6630 "y.tab.c" /* yacc.c:1646  */
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
      yyerror (scanner, YY_("syntax error"));
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
        yyerror (scanner, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
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
                      yytoken, &yylval, scanner);
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


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, scanner);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


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
  yyerror (scanner, YY_("memory exhausted"));
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
                  yytoken, &yylval, scanner);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, scanner);
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
#line 962 "input_parser.yy" /* yacc.c:1906  */


#ifndef NO_NAMESPACE_GIAC
} // namespace giac


#endif // ndef NO_NAMESPACE_GIAC
int giac_yyget_column  (yyscan_t yyscanner);

// Error print routine (store error string in parser_error)
#if 1
int giac_yyerror(yyscan_t scanner,const char *s) {
 const giac::context * contextptr = giac_yyget_extra(scanner);
 int col = giac_yyget_column(scanner);
 int line = giac::lexer_line_number(contextptr);
 const char * scanb=giac::currently_scanned(contextptr);
 std::string curline;
 if (scanb){
  for (int i=1;i<line;++i){
   for (;*scanb;++scanb){
     if (*scanb=='\n'){
       ++scanb;
       break;
     }
   }
  }
  const char * scane=scanb;
  for (;*scane;++scane){
    if (*scane=='\n') break;
  }
  curline=std::string (scanb,scane);
 }
 std::string token_name=string(giac_yyget_text(scanner));
 bool is_at_end = (token_name.size()==2 && (token_name[0]==char(0xC3)) && (token_name[1]==char(0xBF)));
 std::string suffix = " (reserved word)";
 if (token_name.size()>suffix.size() && token_name.compare(token_name.size()-suffix.size(),suffix.size(),suffix)) {
  if (col>=token_name.size()-suffix.size()) {
   col -= token_name.size()-suffix.size();
  }
 } else if (col>=token_name.size()) {
   col -= token_name.size();
 }
 giac::lexer_column_number(contextptr)=col;
 string sy("syntax error ");
 if (0 && strlen(s)){
   sy += ": ";
   sy += s;
   sy +=", ";
 }
 if (is_at_end) {
  parser_error(":" + giac::print_INT_(line) + ": " +sy + " at end of input\n",contextptr); // string(s) replaced with syntax error
  giac::parsed_gen(giac::undef,contextptr);
 } else {
 parser_error( ":" + giac::print_INT_(line) + ": " + sy + " line " + giac::print_INT_(line) + " col " + giac::print_INT_(col) + " at " + token_name +" in "+curline+" \n",contextptr); // string(s) replaced with syntax error
 giac::parsed_gen(giac::string2gen(token_name,false),contextptr);
 }
 if (!giac::first_error_line(contextptr)) {
  giac::first_error_line(line,contextptr);
  if (is_at_end) {
   token_name="end of input";
  }
  giac:: error_token_name(token_name,contextptr);
 }
 return line;
}

#else

int giac_yyerror(yyscan_t scanner,const char *s)
{
  const giac::context * contextptr = giac_yyget_extra(scanner);
  int col= giac_yyget_column(scanner);
  giac::lexer_column_number(contextptr)=col;
  if ( (*giac_yyget_text( scanner )) && (giac_yyget_text( scanner )[0]!=-61) && (giac_yyget_text( scanner )[1]!=-65)){
    std::string txt=giac_yyget_text( scanner );
    parser_error( ":" + giac::print_INT_(giac::lexer_line_number(contextptr)) + ": " + string(s) + " line " + giac::print_INT_(giac::lexer_line_number(contextptr)) + " col " + giac::print_INT_(col) + " at " + txt +"\n",contextptr);
     giac::parsed_gen(giac::string2gen(txt,false),contextptr);
  }
  else {
    parser_error(":" + giac::print_INT_(giac::lexer_line_number(contextptr)) + ": " +string(s) + " at end of input\n",contextptr);
    giac::parsed_gen(giac::undef,contextptr);
  }
  if (!giac::first_error_line(contextptr)){
    giac::first_error_line(giac::lexer_line_number(contextptr),contextptr);
    std::string s=string(giac_yyget_text( scanner ));
    if (s.size()==2 && s[0]==-61 && s[1]==-65)
      s="end of input";
    giac:: error_token_name(s,contextptr);
  }
  return giac::lexer_line_number(contextptr);
}
#endif
