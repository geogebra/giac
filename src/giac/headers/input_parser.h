/* A Bison parser, made by GNU Bison 3.7.5.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_GIAC_YY_Y_TAB_H_INCLUDED
# define YY_GIAC_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int giac_yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    T_NUMBER = 258,                /* T_NUMBER  */
    T_SYMBOL = 259,                /* T_SYMBOL  */
    T_LITERAL = 260,               /* T_LITERAL  */
    T_DIGITS = 261,                /* T_DIGITS  */
    T_STRING = 262,                /* T_STRING  */
    T_END_INPUT = 263,             /* T_END_INPUT  */
    T_EXPRESSION = 264,            /* T_EXPRESSION  */
    T_UNARY_OP = 265,              /* T_UNARY_OP  */
    T_OF = 266,                    /* T_OF  */
    T_NOT = 267,                   /* T_NOT  */
    T_TYPE_ID = 268,               /* T_TYPE_ID  */
    T_VIRGULE = 269,               /* T_VIRGULE  */
    T_AFFECT = 270,                /* T_AFFECT  */
    T_MAPSTO = 271,                /* T_MAPSTO  */
    T_BEGIN_PAR = 272,             /* T_BEGIN_PAR  */
    T_END_PAR = 273,               /* T_END_PAR  */
    T_PLUS = 274,                  /* T_PLUS  */
    T_MOINS = 275,                 /* T_MOINS  */
    T_FOIS = 276,                  /* T_FOIS  */
    T_DIV = 277,                   /* T_DIV  */
    T_MOD = 278,                   /* T_MOD  */
    T_POW = 279,                   /* T_POW  */
    T_QUOTED_BINARY = 280,         /* T_QUOTED_BINARY  */
    T_QUOTE = 281,                 /* T_QUOTE  */
    T_PRIME = 282,                 /* T_PRIME  */
    T_TEST_EQUAL = 283,            /* T_TEST_EQUAL  */
    T_EQUAL = 284,                 /* T_EQUAL  */
    T_INTERVAL = 285,              /* T_INTERVAL  */
    T_UNION = 286,                 /* T_UNION  */
    T_INTERSECT = 287,             /* T_INTERSECT  */
    T_MINUS = 288,                 /* T_MINUS  */
    T_AND_OP = 289,                /* T_AND_OP  */
    T_COMPOSE = 290,               /* T_COMPOSE  */
    T_DOLLAR = 291,                /* T_DOLLAR  */
    T_DOLLAR_MAPLE = 292,          /* T_DOLLAR_MAPLE  */
    T_INDEX_BEGIN = 293,           /* T_INDEX_BEGIN  */
    T_VECT_BEGIN = 294,            /* T_VECT_BEGIN  */
    T_VECT_DISPATCH = 295,         /* T_VECT_DISPATCH  */
    T_VECT_END = 296,              /* T_VECT_END  */
    T_SET_BEGIN = 297,             /* T_SET_BEGIN  */
    T_SET_END = 298,               /* T_SET_END  */
    T_SEMI = 299,                  /* T_SEMI  */
    T_DEUXPOINTS = 300,            /* T_DEUXPOINTS  */
    T_DOUBLE_DEUX_POINTS = 301,    /* T_DOUBLE_DEUX_POINTS  */
    T_IF = 302,                    /* T_IF  */
    T_RPN_IF = 303,                /* T_RPN_IF  */
    T_ELIF = 304,                  /* T_ELIF  */
    T_THEN = 305,                  /* T_THEN  */
    T_ELSE = 306,                  /* T_ELSE  */
    T_IFTE = 307,                  /* T_IFTE  */
    T_SWITCH = 308,                /* T_SWITCH  */
    T_CASE = 309,                  /* T_CASE  */
    T_DEFAULT = 310,               /* T_DEFAULT  */
    T_ENDCASE = 311,               /* T_ENDCASE  */
    T_FOR = 312,                   /* T_FOR  */
    T_FROM = 313,                  /* T_FROM  */
    T_TO = 314,                    /* T_TO  */
    T_DO = 315,                    /* T_DO  */
    T_BY = 316,                    /* T_BY  */
    T_WHILE = 317,                 /* T_WHILE  */
    T_MUPMAP_WHILE = 318,          /* T_MUPMAP_WHILE  */
    T_RPN_WHILE = 319,             /* T_RPN_WHILE  */
    T_REPEAT = 320,                /* T_REPEAT  */
    T_UNTIL = 321,                 /* T_UNTIL  */
    T_IN = 322,                    /* T_IN  */
    T_START = 323,                 /* T_START  */
    T_BREAK = 324,                 /* T_BREAK  */
    T_CONTINUE = 325,              /* T_CONTINUE  */
    T_TRY = 326,                   /* T_TRY  */
    T_CATCH = 327,                 /* T_CATCH  */
    T_TRY_CATCH = 328,             /* T_TRY_CATCH  */
    T_PROC = 329,                  /* T_PROC  */
    T_BLOC = 330,                  /* T_BLOC  */
    T_BLOC_BEGIN = 331,            /* T_BLOC_BEGIN  */
    T_BLOC_END = 332,              /* T_BLOC_END  */
    T_RETURN = 333,                /* T_RETURN  */
    T_LOCAL = 334,                 /* T_LOCAL  */
    T_LOCALBLOC = 335,             /* T_LOCALBLOC  */
    T_NAME = 336,                  /* T_NAME  */
    T_PROGRAM = 337,               /* T_PROGRAM  */
    T_NULL = 338,                  /* T_NULL  */
    T_ARGS = 339,                  /* T_ARGS  */
    T_FACTORIAL = 340,             /* T_FACTORIAL  */
    T_RPN_OP = 341,                /* T_RPN_OP  */
    T_RPN_BEGIN = 342,             /* T_RPN_BEGIN  */
    T_RPN_END = 343,               /* T_RPN_END  */
    T_STACK = 344,                 /* T_STACK  */
    T_GROUPE_BEGIN = 345,          /* T_GROUPE_BEGIN  */
    T_GROUPE_END = 346,            /* T_GROUPE_END  */
    T_LINE_BEGIN = 347,            /* T_LINE_BEGIN  */
    T_LINE_END = 348,              /* T_LINE_END  */
    T_VECTOR_BEGIN = 349,          /* T_VECTOR_BEGIN  */
    T_VECTOR_END = 350,            /* T_VECTOR_END  */
    T_CURVE_BEGIN = 351,           /* T_CURVE_BEGIN  */
    T_CURVE_END = 352,             /* T_CURVE_END  */
    T_ROOTOF_BEGIN = 353,          /* T_ROOTOF_BEGIN  */
    T_ROOTOF_END = 354,            /* T_ROOTOF_END  */
    T_SPOLY1_BEGIN = 355,          /* T_SPOLY1_BEGIN  */
    T_SPOLY1_END = 356,            /* T_SPOLY1_END  */
    T_POLY1_BEGIN = 357,           /* T_POLY1_BEGIN  */
    T_POLY1_END = 358,             /* T_POLY1_END  */
    T_MATRICE_BEGIN = 359,         /* T_MATRICE_BEGIN  */
    T_MATRICE_END = 360,           /* T_MATRICE_END  */
    T_ASSUME_BEGIN = 361,          /* T_ASSUME_BEGIN  */
    T_ASSUME_END = 362,            /* T_ASSUME_END  */
    T_HELP = 363,                  /* T_HELP  */
    TI_DEUXPOINTS = 364,           /* TI_DEUXPOINTS  */
    TI_LOCAL = 365,                /* TI_LOCAL  */
    TI_LOOP = 366,                 /* TI_LOOP  */
    TI_FOR = 367,                  /* TI_FOR  */
    TI_WHILE = 368,                /* TI_WHILE  */
    TI_STO = 369,                  /* TI_STO  */
    TI_TRY = 370,                  /* TI_TRY  */
    TI_DIALOG = 371,               /* TI_DIALOG  */
    T_PIPE = 372,                  /* T_PIPE  */
    TI_DEFINE = 373,               /* TI_DEFINE  */
    TI_PRGM = 374,                 /* TI_PRGM  */
    TI_SEMI = 375,                 /* TI_SEMI  */
    TI_HASH = 376,                 /* TI_HASH  */
    T_ACCENTGRAVE = 377,           /* T_ACCENTGRAVE  */
    T_MAPLELIB = 378,              /* T_MAPLELIB  */
    T_INTERROGATION = 379,         /* T_INTERROGATION  */
    T_UNIT = 380,                  /* T_UNIT  */
    T_BIDON = 381,                 /* T_BIDON  */
    T_LOGO = 382,                  /* T_LOGO  */
    T_SQ = 383,                    /* T_SQ  */
    T_CASE38 = 384,                /* T_CASE38  */
    T_IFERR = 385,                 /* T_IFERR  */
    T_MOINS38 = 386,               /* T_MOINS38  */
    T_NEG38 = 387,                 /* T_NEG38  */
    T_UNARY_OP_38 = 388,           /* T_UNARY_OP_38  */
    T_FUNCTION = 389,              /* T_FUNCTION  */
    T_IMPMULT = 390                /* T_IMPMULT  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
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
