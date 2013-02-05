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
     BOOL = 258,
     ELSE = 259,
     IF = 260,
     INT = 261,
     WHILE = 262,
     VAR = 263,
     FUNCTION = 264,
     INTARRAY = 265,
     RETURN = 266,
     EQ = 267,
     GT = 268,
     GE = 269,
     LT = 270,
     LE = 271,
     NE = 272,
     AND = 273,
     OR = 274,
     NOT = 275,
     PLUS = 276,
     MINUS = 277,
     MULT = 278,
     DIVIDE = 279,
     TRUE = 280,
     FALSE = 281,
     IDENTIFIER = 282,
     INTEGER = 283,
     SEMICOLON = 284,
     COMMA = 285,
     BAR = 286,
     LBRACE = 287,
     RBRACE = 288,
     RPAREN = 289,
     LPAREN = 290,
     LBRACKET = 291,
     RBRACKET = 292,
     ASSIGN = 293
   };
#endif
/* Tokens.  */
#define BOOL 258
#define ELSE 259
#define IF 260
#define INT 261
#define WHILE 262
#define VAR 263
#define FUNCTION 264
#define INTARRAY 265
#define RETURN 266
#define EQ 267
#define GT 268
#define GE 269
#define LT 270
#define LE 271
#define NE 272
#define AND 273
#define OR 274
#define NOT 275
#define PLUS 276
#define MINUS 277
#define MULT 278
#define DIVIDE 279
#define TRUE 280
#define FALSE 281
#define IDENTIFIER 282
#define INTEGER 283
#define SEMICOLON 284
#define COMMA 285
#define BAR 286
#define LBRACE 287
#define RBRACE 288
#define RPAREN 289
#define LPAREN 290
#define LBRACKET 291
#define RBRACKET 292
#define ASSIGN 293




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 58 "parser.ypp"
{
    int ival;
    char *sval;
}
/* Line 1529 of yacc.c.  */
#line 130 "y.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

