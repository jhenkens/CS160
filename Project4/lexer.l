%option yylineno
%pointer

%{
#include <stdlib.h>
#include <string.h>
#include "ast.hpp"
#include "parser.hpp"

void yyerror(const char *);

int getIntValue(char* text, int type) {
	if(strlen(text) > 10) {
		yyerror("integer too long");
		exit(0);
	}
	int value = (int) strtol(yytext, 0, type);
	return value;
}

%}

ID        [a-zA-Z][a-zA-Z0-9]*


%%

&&      { return OP_AND; }
\|\|    { return OP_OR;  } 
!=      { return OP_NE;  }
==      { return OP_EQ;  }
>       { return OP_GT;  }
>=      { return OP_GE;  }
\<      { return OP_LT;  }
\<=     { return OP_LE;  }
!       { return OP_NOT; }
=       { return ASSIGN; }
\+      { return PLUS;   }
\-      { return MINUS;  }
\*      { return MULT;   }
\/      { return DIV;    }
;	{ return SEMI;   }
,	{ return COMMA;  }
\{	{ return CURLY_O;}
\}	{ return CURLY_C;}
\(	{ return PAREN_O;}
\)	{ return PAREN_C;}
\|      { return BAR;    }
\[	{ return ANGLE_O;}
\]	{ return ANGLE_C;}

bool   { return KEY_BOOL;   }
else      { return KEY_ELSE;   }
if        { return KEY_IF;     }
int   { return KEY_INT;    }
while       { return KEY_WHILE;    }
var       { return KEY_VAR;    }
function  { return KEY_FUNC;   }
intarray  { return KEY_INTARRAY; }
return    { return KEY_RET;    }
true      { return TRUE;       }
false     { return FALSE;      }

0|[1-9][0-9]*           { yylval.u_base_int = getIntValue(yytext, 10); return INT;   }
0[1-7][0-7]*           	{ yylval.u_base_int = getIntValue(yytext, 8); return OCT;   }
0[x|X][0-9A-F]+         { yylval.u_base_int = getIntValue(yytext, 16); return HEX;   }
[0-1]+b                 { yylval.u_base_int = getIntValue(yytext, 2); return BIN;   }

{ID}                    { 
						yylval.u_base_charptr = new char[strlen(yytext) + 1];
						strcpy(yylval.u_base_charptr, yytext); 
						yylval.u_base_charptr[strlen(yytext)] = '\0';
						return IDENT; 
			  			}



\[\$([^\$]*\$+[^\]])*[^\$]*\$+\] 	{ /*comment*/}

[ \n\t]   { /* skip whitespace */}

.	  { return ILLEGAL; }


%%


int yywrap(void) {
	return 1;
}

