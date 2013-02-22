/*
	This file is provided for you to run your parser.  You should not have
	to edit anything if you did the yacc and lex files correctly.  All this
	file does is calls yyparse() and uses the visitor class ast2dot to print
	the graph
*/
#include "ast.hpp"
#include "parser.hpp"
#include "typecheck.cpp"
#include "constantfolding.cpp"
#include <assert.h>

extern int yydebug; // set this to 1 if you want yyparse to dump a trace
extern int yyparse(); // this actually the parser which then calls the scanner

void dopass_ast2dot(Program_ptr ast); /*this is defined in ast2dot.cpp*/

void dopass_typecheck(Program_ptr ast, SymTab* st) {
        Typecheck* typecheck = new Typecheck(stderr, st); //create the visitor
        ast->accept(typecheck); //walk the tree with the visitor above
	delete typecheck;

}

void dopass_constantfolding(Program_ptr ast, SymTab* st) {
        ConstantFolding* constant_folding = new ConstantFolding(stderr, st); //create the visitor
	LatticeElemMap *map = new LatticeElemMap();
        map = ast->accept(constant_folding, map); //walk the tree with the visitor above
	delete map;
	delete constant_folding;

}

Program_ptr ast; /* make sure to set this to the final syntax tree in parser.ypp*/

int main(void) {
	SymTab st; //symbol table

	// set this to 1 if you would like to print a trace 
	// of the entire parsing process (it prints to stdout)
        yydebug = 0; 

        yyparse(); 

	dopass_typecheck(ast, &st);
	dopass_constantfolding(ast, &st);

        // walk over the ast and print it out as a dot file
        if (ast) dopass_ast2dot( ast );
        return 0;

}

