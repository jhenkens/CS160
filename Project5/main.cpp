#include "ast.hpp"
#include "parser.hpp"
#include "symtab.hpp"
#include "primitive.hpp"
#include "typecheck.cpp"
#include "constantfolding.cpp"
#include "codegen.cpp"
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

void dopass_codegen(Program_ptr ast, SymTab * st)
{
	Codegen *codegen = new Codegen(stdout, st);	// create the visitor
	ast->accept(codegen);				// walk the tree with the visitor above
	delete codegen;
}

Program_ptr ast; /* make sure to set this to the final syntax tree in parser.ypp*/

int main(void) {

	SymTab st; //symbol table

	// set this to 1 if you would like to print a trace 
	// of the entire parsing process (it prints to stdout)
        yydebug = 0; 

	// after parsing, the global "ast" should be set to the
	// syntax tree that we have built up during the parse
        yyparse();  

	if (ast) {
		dopass_typecheck( ast, &st );
		dopass_constantfolding(ast, &st);

		// do codegen!
		dopass_codegen( ast, &st );
	}
    return 0;
}

