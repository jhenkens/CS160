%option yylineno
%pointer

%{
#include <stdlib.h>
#include <string.h>
#include "ast.hpp"
#include "parser.hpp"

void yyerror(const char *);

%}

/* Insert your lexer here. Please don't just overwrite the file because of the enhanced header section. */

%%


%%

int yywrap(void) {
	return 1;
}

