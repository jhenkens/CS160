#include "ast.hpp"
#include "symtab.hpp"
#include "primitive.hpp"
#include <string.h>
#include <stdio.h>

#include <stack>

class Ast2dot : public Visitor {
 private:
 FILE *m_out; //file for writting output
 int count; //used to give each node a uniq id
 stack<int> s; //stack for tracking parent/child pairs

 public:

 Ast2dot( FILE* out ) { 
	count = 0;
	s.push(0);
	m_out = out;
	fprintf( m_out, "digraph G { \n" );
 }

 void finish() { 
	fprintf( m_out, "}; \n" );	
 }

 void add_edge( int p, int c )
 {
	fprintf( m_out, "\"%d\" -> \"%d\"\n", p, c );
 }

 void add_node( int c, const char* n )
 {
	fprintf( m_out, "\"%d\" [label=\"%s\"]\n" , c, n );
 }

 void draw(const char* n, Visitable* p) {
	count++; 			// each node gets a unique number
	add_edge( s.top(), count ); 	// from parent to this 
	add_node( count, n );		// name the this node
	s.push(count);			// now this node is the parent
	if (p != 0)
		p->visit_children(this);	
	s.pop();			// now restore old parent
 }

 void draw_symname(const char* n, SymName* p) {
	count++; 			// each node gets a unique number
	add_edge( s.top(), count ); 	// from parent to this 
	// print symname strings
	if (p != 0)
		fprintf( m_out, "\"%d\" [label=\"%s\\n\\\"%s\\\"\"]\n" , count, n, p->spelling() );
	else
		fprintf( m_out, "\"%d\" [label=\"%s\\n\\\"null\\\"\"]\n", count, n );
 }

 void draw_primitive(const char* n, Primitive* p) {
	count++; 			// each node gets a unique number
	add_edge( s.top(), count ); 	// from parent to this
	if (p != 0)
		fprintf( m_out, "\"%d\" [label=\"%s\\n%d\"]\n" , count, n, p->m_data );
	else
		fprintf( m_out, "\"%d\" [label=\"%s\\nnull\"]\n" , count, n );
 }


 void visitProgram(Program *p) { draw("Program", p); }
 void visitFunc(Func *p) { draw("Func", p); }
 void visitFunction_block(Function_block *p) { draw("FunctionBlock", p); }
 void visitNested_block(Nested_block *p) { draw("NestedBlock", p); } 
 void visitParam(Param *p) { draw("Param", p); }
 void visitDecl(Decl *p) { draw("Decl", p); }
 void visitAssignment(Assignment *p) { draw("Assignment", p); }
 void visitArrayAssignment(ArrayAssignment *p) { draw("ArrayAssignment", p); }

 void visitCall(Call *p) { draw("Call", p); }
 void visitArrayCall(ArrayCall *p) { draw("ArrayCall", p); }
 void visitReturn(Return *p) { draw("Return", p); }
 void visitIfNoElse(IfNoElse *p) { draw("IfNoElse", p); }
 void visitIfWithElse(IfWithElse *p) { draw("IfWithElse", p); }
 void visitWhileLoop(WhileLoop *p) { draw("WhileLoop", p); }
 void visitTInt(TInt *p) { draw("TInt", p); }
 void visitTBool(TBool *p) { draw("TBool", p); }
 void visitTIntArray(TIntArray *p) { draw("TIntArray", p); }

 void visitAnd(And *p) { draw("And", p); }
 void visitDiv(Div *p) { draw("Div", p); }
 void visitCompare(Compare *p) { draw("Compare", p); }
 void visitGt(Gt *p) { draw("Gt", p); }
 void visitGteq(Gteq *p) { draw("Gteq", p); }
 void visitLt(Lt *p) { draw("Lt", p); }
 void visitLteq(Lteq *p) { draw("Lteq", p); }
 void visitMinus(Minus *p) { draw("Minus", p); }
 void visitNoteq(Noteq *p) { draw("Noteq", p); }
 void visitOr(Or *p) { draw("Or", p); }
 void visitPlus(Plus *p) { draw("Plus", p); }
 void visitTimes(Times *p) { draw("Times", p); }
 void visitNot(Not *p) { draw("Not", p); }
 void visitUminus(Uminus *p) { draw("Uminus", p); }
 void visitMagnitude(Magnitude *p) { draw("Magnitude", p); }
 void visitIdent(Ident *p) { draw("Ident", p); }
 void visitArrayAccess(ArrayAccess *p) { draw("ArrayAccess", p); }
 void visitIntLit(IntLit *p) { draw("IntLit", p); }
 void visitBoolLit(BoolLit *p) { draw("BoolLit", p); }

 //special cases
 void visitSymName(SymName *p) { draw_symname("SymName",p); }
 void visitPrimitive(Primitive *p) { draw_primitive("Primitive",p); }
};

void dopass_ast2dot(Program_ptr ast) {
        Ast2dot* ast2dot = new Ast2dot(stdout); //create the visitor
        ast->accept(ast2dot); //walk the tree with the visitor above
	ast2dot->finish(); // finalize the printout
	delete ast2dot;
}
