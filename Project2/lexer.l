%option yylineno
%pointer

%{
#include <stdlib.h>
#include <string.h>
#include "parser.hpp"

void yyerror(const char *);
%}

/* 
  WRITE ME: put any definitions you need here.  Add your own regular expressions
            below for ALL tokens defined in the manual. You can choose not to use
	    definitions here.

*/
%token BOOL
%token ELSE
%token IF
%token INT
%token WHILE
%token VAR
%token FUNCTION
%token INTARRAY
%token RETURN

%token EQ
%token GT
%token GE
%token LT
%token LE
%token NE
%token AND
%token OR
%token NOT
%token PLUS
%token MINUS
%token MULT
%token DIVIDE

%token TRUE
%token FALSE
%token IDENTIFIER
%token INTEGER

%token RPAREN
%token LPAREN
%token ASSIGN
%token SEMICOLON
%token COMMA
%token LBRACE
%token RBRACE
%token LBRACKET
%token RBRACKET
/*
  The example below is very simple, but it shows almost everything you will need
  to do for this first part.

  The syntax is as follows:

  regular_expression      {  c code  }

  The lexer will try to match each regular expression.  When it does it will execute
  the c code.
*/

%%

bool        { printf("scanned a %s\n",yytext);  return BOOL;}
[\*\(\);\+\-\*\/<>=]                        { printf("scanned a %s\n",yytext);  return *yytext; }
[0-9]+           { return 'd'; }

[ \t\n]               ; /* skip whitespace */

.                     yyerror((char *) "invalid character");

%%


/* You shouldn't need to edit any part of this*/

int yywrap(void) {
	return 1;
}
