%option yylineno
%pointer

%{
#include <stdlib.h>
#include <string.h>
#include "y.tab.h"
#include "parser.hpp"

void yyerror(const char *);
%}

/* 
  WRITE ME: put any definitions you need here.  Add your own regular expressions
            below for ALL tokens defined in the manual. You can choose not to use
	    definitions here.

*/
/*
  The example below is very simple, but it shows almost everything you will need
  to do for this first part.

  The syntax is as follows:

  regular_expression      {  c code  }

  The lexer will try to match each regular expression.  When it does it will execute
  the c code.
*/

%%

\[\$[^\$\]]*\$\]    { printf("removed a comment\n"); }
bool                { printf("scanned a BOOL: %s\n",yytext);  return BOOL;}
else                { printf("scanned a ELSE: %s\n",yytext);  return ELSE;}
if                  { printf("scanned a IF: %s\n",yytext);  return IF;}
int                 { printf("scanned a INT: %s\n",yytext);  return INT;}
while               { printf("scanned a WHILE: %s\n",yytext);  return WHILE;}
var                 { printf("scanned a VAR: %s\n",yytext);  return VAR;}
function            { printf("scanned a FUNCTION: %s\n",yytext);  return FUNCTION;}
intarray            { printf("scanned a INTARRAY: %s\n",yytext);  return INTARRAY;}
return              { printf("scanned a RETURN: %s\n",yytext);  return RETURN;}
==                  { printf("scanned a EQ: %s\n",yytext);  return EQ;}
>                   { printf("scanned a GT: %s\n",yytext);  return GT;}
>=                  { printf("scanned a GE: %s\n",yytext);  return GE;}
\<                  { printf("scanned a LT: %s\n",yytext);  return LT;}
\<=                 { printf("scanned a LE: %s\n",yytext);  return LE;}
!=                  { printf("scanned a NE: %s\n",yytext);  return NE;}
&&                  { printf("scanned a AND: %s\n",yytext);  return AND;}
\|\|                { printf("scanned a OR: %s\n",yytext);  return OR;}
!                   { printf("scanned a NOT: %s\n",yytext);  return NOT;}
\+                  { printf("scanned a PLUS: %s\n",yytext);  return PLUS;}
-                   { printf("scanned a MINUS: %s\n",yytext);  return MINUS;}
\*                  { printf("scanned a MULT: %s\n",yytext);  return MULT;}
\/                  { printf("scanned a DIVIDE: %s\n",yytext);  return DIVIDE;}
true                { printf("scanned a TRUE: %s\n",yytext);  return TRUE;}
false               { printf("scanned a FALSE: %s\n",yytext);  return FALSE;}
\;                  { printf("scanned a SEMICOLON: %s\n",yytext);  return SEMICOLON;}
\,                  { printf("scanned a COMMA: %s\n",yytext);  return COMMA;}
\|                  { printf("scanned a BAR: %s\n",yytext);  return BAR;}
\{                  { printf("scanned a LBRACE: %s\n",yytext);  return LBRACE;}
\}                  { printf("scanned a RBRACE: %s\n",yytext);  return RBRACE;}
\(                  { printf("scanned a LPAREN: %s\n",yytext);  return LPAREN;}
\)                  { printf("scanned a RPAREN: %s\n",yytext);  return RPAREN;}
\[                  { printf("scanned a LBRACKET: %s\n",yytext);  return LBRACKET;}
\]                  { printf("scanned a RBRACKET: %s\n",yytext);  return RBRACKET;}
\=                  { printf("scanned a ASSIGN: %s\n",yytext);  return ASSIGN;}
[a-zA-Z][a-zA-Z0-9]* {yylval.sval = yytext; printf("scanned an identifier %s\n",yytext); return IDENTIFIER;}
0x[0-9A-F]+         { yylval.ival = strtol(yytext,NULL,16); printf("scanned hex value %s as %d\n", yytext, yylval.ival); return INTEGER;}
[1-9][0-9]*         { yylval.ival = strtol(yytext,NULL,10); printf("scanned decimal value %s as %d\n", yytext, yylval.ival); return INTEGER;}
0[0-7]+             { yylval.ival = strtol(yytext,NULL,8); printf("scanned octal value %s as %d\n", yytext, yylval.ival); return INTEGER;}
[0-1]+b             { yylval.ival = strtol(yytext,NULL,2); printf("scanned binary value %s as %d\n", yytext, yylval.ival); return INTEGER;}

.                     yyerror((char *) "invalid character");

%%


/* You shouldn't need to edit any part of this*/

int yywrap(void) {
	return 1;
}
