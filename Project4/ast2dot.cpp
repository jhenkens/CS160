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
	add_node( count, n );		// name the this node
	add_edge( s.top(), count ); 	// from parent to this 
	s.push(count);			// now this node is the parent
	if (p != 0)
		p->visit_children(this);	
	s.pop();			// now restore old parent
 }
 
 void draw_expr(const char* n, Visitable* p) {
	char buffer[12];
	p->m_attribute.m_lattice_elem.to_string(buffer);

	count++; 			// each node gets a unique number
	fprintf( m_out, "\"%d\" [label=\"%s\\n<%s>\"]\n" , count, n, buffer );
	add_edge( s.top(), count ); 	// from parent to this 
	s.push(count);			// now this node is the parent
	if (p != 0)
		p->visit_children(this);
	s.pop();			// now restore old parent
 }

 void draw_symname(const char* n, SymName* p) {
	count++; 			// each node gets a unique number
	// print symname strings
	if (p != 0)
		fprintf( m_out, "\"%d\" [label=\"%s\\n\\\"%s\\\"\"]\n" , count, n, p->spelling() );
	else
		fprintf( m_out, "\"%d\" [label=\"%s\\n\\\"null\\\"\"]\n", count, n );
	add_edge( s.top(), count ); 	// from parent to this 
 }

 void draw_primitive(const char* n, Primitive* p) {
	count++; 			// each node gets a unique number
	if (p != 0)
		fprintf( m_out, "\"%d\" [label=\"%s\\n%d\"]\n" , count, n, p->m_data );
	else
		fprintf( m_out, "\"%d\" [label=\"%s\\nnull\"]\n" , count, n );
	add_edge( s.top(), count ); 	// from parent to this
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

 void visitAnd(And *p) { draw_expr("And", p); }
 void visitDiv(Div *p) { draw_expr("Div", p); }
 void visitCompare(Compare *p) { draw_expr("Compare", p); }
 void visitGt(Gt *p) { draw_expr("Gt", p); }
 void visitGteq(Gteq *p) { draw_expr("Gteq", p); }
 void visitLt(Lt *p) { draw_expr("Lt", p); }
 void visitLteq(Lteq *p) { draw_expr("Lteq", p); }
 void visitMinus(Minus *p) { draw_expr("Minus", p); }
 void visitNoteq(Noteq *p) { draw_expr("Noteq", p); }
 void visitOr(Or *p) { draw_expr("Or", p); }
 void visitPlus(Plus *p) { draw_expr("Plus", p); }
 void visitTimes(Times *p) { draw_expr("Times", p); }
 void visitNot(Not *p) { draw_expr("Not", p); }
 void visitUminus(Uminus *p) { draw_expr("Uminus", p); }
 void visitMagnitude(Magnitude *p) { draw_expr("Magnitude", p); }
 void visitIdent(Ident *p) { draw_expr("Ident", p); }
 void visitArrayAccess(ArrayAccess *p) { draw_expr("ArrayAccess", p); }
 void visitIntLit(IntLit *p) { draw_expr("IntLit", p); }
 void visitBoolLit(BoolLit *p) { draw_expr("BoolLit", p); }

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
