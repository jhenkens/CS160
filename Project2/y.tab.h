/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

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

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     T_BOOL = 258,
     T_ELSE = 259,
     T_IF = 260,
     T_INT = 261,
     T_WHILE = 262,
     T_VAR = 263,
     T_FUNCTION = 264,
     T_INTARRAY = 265,
     T_RETURN = 266,
     T_EQ = 267,
     T_GT = 268,
     T_GE = 269,
     T_LT = 270,
     T_LE = 271,
     T_NE = 272,
     T_AND = 273,
     T_OR = 274,
     T_NOT = 275,
     T_PLUS = 276,
     T_MINUS = 277,
     T_MULT = 278,
     T_DIVIDE = 279,
     T_TRUE = 280,
     T_FALSE = 281,
     T_IDENTIFIER = 282,
     T_INTEGER = 283,
     T_SEMICOLON = 284,
     T_COMMA = 285,
     T_BAR = 286,
     T_LBRACE = 287,
     T_RBRACE = 288,
     T_RPAREN = 289,
     T_LPAREN = 290,
     T_LBRACKET = 291,
     T_RBRACKET = 292,
     T_ASSIGN = 293,
     UMINUS = 294
   };
#endif
/* Tokens.  */
#define T_BOOL 258
#define T_ELSE 259
#define T_IF 260
#define T_INT 261
#define T_WHILE 262
#define T_VAR 263
#define T_FUNCTION 264
#define T_INTARRAY 265
#define T_RETURN 266
#define T_EQ 267
#define T_GT 268
#define T_GE 269
#define T_LT 270
#define T_LE 271
#define T_NE 272
#define T_AND 273
#define T_OR 274
#define T_NOT 275
#define T_PLUS 276
#define T_MINUS 277
#define T_MULT 278
#define T_DIVIDE 279
#define T_TRUE 280
#define T_FALSE 281
#define T_IDENTIFIER 282
#define T_INTEGER 283
#define T_SEMICOLON 284
#define T_COMMA 285
#define T_BAR 286
#define T_LBRACE 287
#define T_RBRACE 288
#define T_RPAREN 289
#define T_LPAREN 290
#define T_LBRACKET 291
#define T_RBRACKET 292
#define T_ASSIGN 293
#define UMINUS 294




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 19 "parser.ypp"
{
    int ival;
    char *sval;
}
/* Line 1529 of yacc.c.  */
#line 132 "y.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

