#include "parser.hpp"
#include <assert.h>

extern int yydebug; // set this to 1 if you want yyparse to dump a trace
extern int yyparse(); // this actually the parser which then calls the scanner

int main(void) {

	// set this to 1 if you would like to print a trace 
	// of the entire parsing process (it prints to stdout)
        yydebug = 0; 

        yyparse();  

        return 0;

}

