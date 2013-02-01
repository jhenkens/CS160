%option yylineno
%pointer

%{
#include <stdlib.h>
#include <string.h>
#include "y.tab.h"
#include "parser.hpp"
#define TESTING 1

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

\[\$(.|\n|\r)*?\$\] { if(TESTING) printf("removed a comment\n"); }
bool                { if(TESTING) printf("scanned a BOOL: %s\n",yytext);  return BOOL;}
else                { if(TESTING) printf("scanned a ELSE: %s\n",yytext);  return ELSE;}
if                  { if(TESTING) printf("scanned a IF: %s\n",yytext);  return IF;}
int                 { if(TESTING) printf("scanned a INT: %s\n",yytext);  return INT;}
while               { if(TESTING) printf("scanned a WHILE: %s\n",yytext);  return WHILE;}
var                 { if(TESTING) printf("scanned a VAR: %s\n",yytext);  return VAR;}
function            { if(TESTING) printf("scanned a FUNCTION: %s\n",yytext);  return FUNCTION;}
intarray            { if(TESTING) printf("scanned a INTARRAY: %s\n",yytext);  return INTARRAY;}
return              { if(TESTING) printf("scanned a RETURN: %s\n",yytext);  return RETURN;}
==                  { if(TESTING) printf("scanned a EQ: %s\n",yytext);  return EQ;}
>                   { if(TESTING) printf("scanned a GT: %s\n",yytext);  return GT;}
>=                  { if(TESTING) printf("scanned a GE: %s\n",yytext);  return GE;}
\<                  { if(TESTING) printf("scanned a LT: %s\n",yytext);  return LT;}
\<=                 { if(TESTING) printf("scanned a LE: %s\n",yytext);  return LE;}
!=                  { if(TESTING) printf("scanned a NE: %s\n",yytext);  return NE;}
&&                  { if(TESTING) printf("scanned a AND: %s\n",yytext);  return AND;}
\|\|                { if(TESTING) printf("scanned a OR: %s\n",yytext);  return OR;}
!                   { if(TESTING) printf("scanned a NOT: %s\n",yytext);  return NOT;}
\+                  { if(TESTING) printf("scanned a PLUS: %s\n",yytext);  return PLUS;}
-                   { if(TESTING) printf("scanned a MINUS: %s\n",yytext);  return MINUS;}
\*                  { if(TESTING) printf("scanned a MULT: %s\n",yytext);  return MULT;}
\/                  { if(TESTING) printf("scanned a DIVIDE: %s\n",yytext);  return DIVIDE;}
true                { if(TESTING) printf("scanned a TRUE: %s\n",yytext);  return TRUE;}
false               { if(TESTING) printf("scanned a FALSE: %s\n",yytext);  return FALSE;}
\;                  { if(TESTING) printf("scanned a SEMICOLON: %s\n",yytext);  return SEMICOLON;}
\,                  { if(TESTING) printf("scanned a COMMA: %s\n",yytext);  return COMMA;}
\|                  { if(TESTING) printf("scanned a BAR: %s\n",yytext);  return BAR;}
\{                  { if(TESTING) printf("scanned a LBRACE: %s\n",yytext);  return LBRACE;}
\}                  { if(TESTING) printf("scanned a RBRACE: %s\n",yytext);  return RBRACE;}
\(                  { if(TESTING) printf("scanned a LPAREN: %s\n",yytext);  return LPAREN;}
\)                  { if(TESTING) printf("scanned a RPAREN: %s\n",yytext);  return RPAREN;}
\[                  { if(TESTING) printf("scanned a LBRACKET: %s\n",yytext);  return LBRACKET;}
\]                  { if(TESTING) printf("scanned a RBRACKET: %s\n",yytext);  return RBRACKET;}
\=                  { if(TESTING) printf("scanned a ASSIGN: %s\n",yytext);  return ASSIGN;}
[a-zA-Z][a-zA-Z0-9]* {yylval.sval = yytext; printf("scanned an identifier %s\n",yytext); return IDENTIFIER;}
0x[0-9A-F]+         { yylval.ival = strtol(yytext,NULL,16); if(TESTING) printf("scanned hex value %s as %d\n", yytext, yylval.ival); return INTEGER;}
[1-9][0-9]*         { yylval.ival = strtol(yytext,NULL,10); if(TESTING) printf("scanned decimal value %s as %d\n", yytext, yylval.ival); return INTEGER;}
0[0-7]+             { yylval.ival = strtol(yytext,NULL,8); if(TESTING) printf("scanned octal value %s as %d\n", yytext, yylval.ival); return INTEGER;}
[0-1]+b             { yylval.ival = strtol(yytext,NULL,2); if(TESTING) printf("scanned binary value %s as %d\n", yytext, yylval.ival); return INTEGER;}
[ \t\n]               ; /* skip whitespace */
.                     yyerror((char *) "invalid character");

%%


/* You shouldn't need to edit any part of this*/

int yywrap(void) {
	return 1;
}
