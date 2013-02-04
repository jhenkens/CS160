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

\[\$([^\$]|\$[^\]])*\$\] { if(TESTING) printf("removed a comment\n"); }
\[\$                yyerror((char *) "unclosed comment");
bool                { if(TESTING) printf("scanned a BOOL: %s\n",yytext);  return T_BOOL;}
else                { if(TESTING) printf("scanned a ELSE: %s\n",yytext);  return T_ELSE;}
if                  { if(TESTING) printf("scanned a IF: %s\n",yytext);  return T_IF;}
int                 { if(TESTING) printf("scanned a INT: %s\n",yytext);  return T_INT;}
while               { if(TESTING) printf("scanned a WHILE: %s\n",yytext);  return T_WHILE;}
var                 { if(TESTING) printf("scanned a VAR: %s\n",yytext);  return T_VAR;}
function            { if(TESTING) printf("scanned a FUNCTION: %s\n",yytext);  return T_FUNCTION;}
intarray            { if(TESTING) printf("scanned a INTARRAY: %s\n",yytext);  return T_INTARRAY;}
return              { if(TESTING) printf("scanned a RETURN: %s\n",yytext);  return T_RETURN;}
==                  { if(TESTING) printf("scanned a EQ: %s\n",yytext);  return T_EQ;}
>                   { if(TESTING) printf("scanned a GT: %s\n",yytext);  return T_GT;}
>=                  { if(TESTING) printf("scanned a GE: %s\n",yytext);  return T_GE;}
\<                  { if(TESTING) printf("scanned a LT: %s\n",yytext);  return T_LT;}
\<=                 { if(TESTING) printf("scanned a LE: %s\n",yytext);  return T_LE;}
!=                  { if(TESTING) printf("scanned a NE: %s\n",yytext);  return T_NE;}
&&                  { if(TESTING) printf("scanned a AND: %s\n",yytext);  return T_AND;}
\|\|                { if(TESTING) printf("scanned a OR: %s\n",yytext);  return T_OR;}
!                   { if(TESTING) printf("scanned a NOT: %s\n",yytext);  return T_NOT;}
\+                  { if(TESTING) printf("scanned a PLUS: %s\n",yytext);  return T_PLUS;}
-                   { if(TESTING) printf("scanned a MINUS: %s\n",yytext);  return T_MINUS;}
\*                  { if(TESTING) printf("scanned a MULT: %s\n",yytext);  return T_MULT;}
\/                  { if(TESTING) printf("scanned a DIVIDE: %s\n",yytext);  return T_DIVIDE;}
true                { if(TESTING) printf("scanned a TRUE: %s\n",yytext);  return T_TRUE;}
false               { if(TESTING) printf("scanned a FALSE: %s\n",yytext);  return T_FALSE;}
\;                  { if(TESTING) printf("scanned a SEMICOLON: %s\n",yytext);  return T_SEMICOLON;}
\,                  { if(TESTING) printf("scanned a COMMA: %s\n",yytext);  return T_COMMA;}
\|                  { if(TESTING) printf("scanned a BAR: %s\n",yytext);  return T_BAR;}
\{                  { if(TESTING) printf("scanned a LBRACE: %s\n",yytext);  return T_LBRACE;}
\}                  { if(TESTING) printf("scanned a RBRACE: %s\n",yytext);  return T_RBRACE;}
\(                  { if(TESTING) printf("scanned a LPAREN: %s\n",yytext);  return T_LPAREN;}
\)                  { if(TESTING) printf("scanned a RPAREN: %s\n",yytext);  return T_RPAREN;}
\[                  { if(TESTING) printf("scanned a LBRACKET: %s\n",yytext);  return T_LBRACKET;}
\]                  { if(TESTING) printf("scanned a RBRACKET: %s\n",yytext);  return T_RBRACKET;}
\=                  { if(TESTING) printf("scanned a ASSIGN: %s\n",yytext);  return T_ASSIGN;}
[a-zA-Z][a-zA-Z0-9]*  {yylval.sval = yytext; printf("scanned an identifier %s\n",yytext); return T_IDENTIFIER;}
[0-1]+b             { yylval.ival = strtol(yytext,NULL,2); if(TESTING) printf("scanned binary value %s as %d\n", yytext, yylval.ival); return T_INTEGER;}
(0x|0X)[0-9A-Fa-f]*    { yylval.ival = strtol(yytext,NULL,16); if(TESTING) printf("scanned hex value %s as %d\n", yytext, yylval.ival); return T_INTEGER;}
[1-9][0-9]*  { yylval.ival = strtol(yytext,NULL,10); if(TESTING) printf("scanned decimal value %s as %d\n", yytext, yylval.ival); return T_INTEGER;}
0[0-7]*             { yylval.ival = strtol(yytext,NULL,8); if(TESTING) printf("scanned octal value %s as %d\n", yytext, yylval.ival); return T_INTEGER;}
[ \t\n]               ; /* skip whitespace */
.                     yyerror((char *) "invalid character");

%%


/* You shouldn't need to edit any part of this*/

int yywrap(void) {
	return 1;
}
