#include "ast.hpp"
#include "primitive.hpp"
#include "attribute.hpp"
#include "symtab.hpp"
#include <iostream>

#define forall(iterator,listptr) \
    for(iterator = listptr->begin(); iterator != listptr->end(); iterator++) \

#define TESTING 0

#define tprint(...) if(TESTING) printf(__VA_ARGS__)

using namespace std;

/*
 * The only methods that are implemented for you are the While statement and a Not expression.
 * Use those two as a template. At the end of this analysis, each expression node should have a
 * valid LatticeElem associated with it.
 *
 * Overview of the code below:
 *
 *
 * LatticeElem:
 *   To see how to work with LatticeElem and LatticeElemMaps, open "attribute.hpp"
 *
 *   In short, LatticeElem is a class that stores a number, a value of a particular
 *   variable or stored in a node. You can just assign a number to it, or TOP or BOTTOM.
 *
 *   For example, to make an expression's value constant, you go:
 *     p -> m_attribute.m_lattice_elem = 5; -// or TOP or BOTTOM
 *
 *   LatticeElems can be compared to each other or to integers using the == operator. If you
 *   specifically need to access a LatticeElem's constant value, use it's public "value" field:
 *     int value = p -> m_attribute.m_lattice_elem.value;
 *
 *
 * LatticeElemMap:
 *   It is nothing but a map of type std::map<const char*, LatticeElem>
 *   http://en.cppreference.com/w/cpp/container/map
 *
 *   In short, you can just lookup a value using the [] operator with a char* as the
 *   argument representing the name of a variable. That will return the corresponding
 *   LatticeElem for a variable name; and if that variable was not stored in the map,
 *   its entry will be automatically added with value BOTTOM.
 *
 *   So, to lookup a LatticeElem in a LatticeElemMap *map, under a Symname "symname":
 *     LatticeElem &le = (*in)[symname->spelling()];
 *
 *   To store a value in the map you similarily go:
 *     (*in)[p->m_symname->spelling()] = TOP;
 *
 *   To clone a LatticeElemMap use a copy constructor. Look at the While statement for
 *   an example.
 *
 *   To JOIN one LatticeElemMap with another, call the join_lattice_maps() function. Example
 *   in the While statement implementation. Note that this function alters the 1st argument
 *   map to contain the join! 
 *
 * 
 * This visitor pattern is not as simple as the typecheck visitor.
 *
 * Each visit method receives an "in" LatticeElemMap, containing the information about variable states
 * from before the visit. You're supposed to edit this LatticeElemMap according to the rules of our constant
 * propagation and return it in the end.
 *
 * In addition, all Expression type nodes have a LatticeElem associated with them. This is the value of the
 * expression as determined by the analysis; by default, it's BOTTOM. You should set all these to either
 * constants, if the analysis can guarantee that an expression will be constant, or to TOP if the analysis 
 * cannot guarantee that an expression will be a constant. This LatticeElem can be accessed, for an expression node *p:
 *   p->m_attribute.m_lattice_elem
 *
 * To recursivelly do the analysis on all the children of a node, use "out = visit_children_of(node, in);"
 * Each child's "out" LatticeElemMap is propagated to the next child as it "in"; the last "out" is returned.
 *  
 * To visit a single child, use "out = visit(child_node, in);". To visit a child that is a list of nodes, use
 * "out = visit_list(list, in);" and the in/out sets are propagated between the individual elements.
 *
 *
 * The rules of our constant folding are:
 *   * All variables are uninitialized at the beginning of a function (TOP by default)
 *   * All assignments force the left hand side variable to the value of the right hand side
 *   * We are doing an intraprocedural analysis. This means that we do our analysis for each function
 *     separately, not minding other functions. As a consequence:
 *       + We assume each function always returns TOP. We will not assume anything more precise.
 *       + We don't assume anything about global variables; therefore they're (initially) TOP, and nothing one function
 *         does to a global affects the same global in other functions.
 *       + We assume that any function may alter any visible var's state. To simplify the analysis ALL variables should 
 *         be set to TOP after each call statement.
 *   * We are not keeping track of IntArrays. Any array element is assumed to be TOP at all times.
 * Also:
 *   * Anything multiplied by 0 is 0
 *   * Dividing 0 by anything is always 0, and dividing anything by 0 makes the result 0 for this assignment
 *   * false AND anything is false, true OR anything is true
 *
 * While it is possible to do a more detailed analysis, that is not allowed in order to keep the grading fair. If you have 
 * a good idea about doing a more precise analysis, great, talk to us about it, but for this assignment do exactly the 
 * analysis posted here; no more and no less.
 */

class ConstantFolding : public CFVisitor {
private:
  FILE* m_errorfile;
  SymTab* m_st;

public:
  LatticeElemMap* visitProgram(Program *p, LatticeElemMap *in)
  {
    in = visit_children_of(p, in);
    return in;
  }

  LatticeElemMap* visitFunc(Func *p, LatticeElemMap *in)
  {
    // Intraprocedural; so let's start a new analysis for each function with a blank LatticeElemMap
    LatticeElemMap* newMap = new LatticeElemMap();
    newMap = visit_children_of(p, newMap);
    delete newMap;
    return in;
  }

  LatticeElemMap* visitFunction_block(Function_block *p, LatticeElemMap *in)
  {
    in = visit_children_of(p, in);
    return in;
  }

  LatticeElemMap* visitNested_block(Nested_block *p, LatticeElemMap *in)
  {
    in = visit_children_of(p, in);
    return in;
  }

  LatticeElemMap* visitParam(Param *p, LatticeElemMap *in)
  {
    in = visit_children_of(p, in);
            //begin testing block:
            tprint("DEBUG::VISITPARAM: Found symname %s\n",(p->m_symname->spelling()));
            if(in->find(p->m_symname->spelling()) == in->end()){
                tprint("\tFound null in symname table\n");
                tprint("\tSanity check, value is found: %s\n",(in->find(p->m_symname->spelling()) != in->end())?"true":"false");

            } else {
                LatticeElem& e = (*in)[(p->m_symname->spelling())];
                tprint("\tFound non-null in symname table\n");
                if(e==TOP){
                    tprint("\t\tFound TOP\n");
                }
                else if(e==BOTTOM){
                    tprint("\t\tFound BOTTOM\n");
                }
                else{
                    tprint("\t\tFound %d\n",e.value);
                }
            }
            //end testing block
            tprint("\tAdding element with value top\n");
            (*in)[p->m_symname->spelling()]=TOP;
            tprint("\tSanity check, value is found: %s\n",(in->find(p->m_symname->spelling()) != in->end())?"true":"false");
    return in;

  }

  LatticeElemMap* visitDecl(Decl *p, LatticeElemMap *in)
  {
     in = visit_children_of(p, in);
            list<SymName_ptr>::iterator symname_iter;
            forall(symname_iter,p -> m_symname_list){
                //begin testing block:
                tprint("DEBUG::VISITDECL: Found symname %s\n",(*symname_iter)->spelling());
                if(in->find((*symname_iter)->spelling()) == in->end()){
                    tprint("\tFound null in symname table\n");
                    tprint("\tSanity check, value is found: %s\n",(in->find((*symname_iter)->spelling()) != in->end())?"true":"false");

                } else {
                    LatticeElem& e = (*in)[(*symname_iter)->spelling()];
                    tprint("\tFound non-null in symname table\n");
                    if(e==TOP){
                        tprint("\t\tFound TOP\n");
                    }
                    else if(e==BOTTOM){
                        tprint("\t\tFound BOTTOM\n");
                    }
                    else{
                        tprint("\t\tFound %d\n",e.value);
                    }
                }
                //end testing block
                tprint("\tAdding element with value top\n");
                (*in)[(*symname_iter)->spelling()]=TOP;
                tprint("\tSanity check, value is found: %s\n",(in->find(strdup((*symname_iter)->spelling())) != in->end())?"true":"false");
            }
     return in;
  }

  LatticeElemMap* visitReturn(Return *p, LatticeElemMap *in)
  {
    in = visit_children_of(p, in);
    return in;
  }

  LatticeElemMap* visitAssignment(Assignment *p, LatticeElemMap *in)
  {
    in = visit_children_of(p, in);
            (*in)[p->m_symname->spelling()]=p->m_expr->m_attribute.m_lattice_elem;
    return in;
  }

  LatticeElemMap* visitArrayAssignment(ArrayAssignment *p, LatticeElemMap *in)
  {
    in = visit_children_of(p, in);
    return in;
  }

  LatticeElemMap* visitCall(Call *p, LatticeElemMap *in)
  {
    in = visit_children_of(p, in);
            LatticeElemMap::iterator lem_iter;
            forall(lem_iter,in){
                lem_iter->second=TOP;
            }
    return in;
  }

  LatticeElemMap* visitArrayCall(ArrayCall *p, LatticeElemMap *in)
  {
    in = visit_children_of(p, in);
            LatticeElemMap::iterator lem_iter;
            forall(lem_iter,in){
                lem_iter->second=TOP;
            }
    return in;
  }

  LatticeElemMap* visitIfNoElse(IfNoElse *p, LatticeElemMap *in)
  {
            in = visit(p->m_expr, in);

            // Copy this lattice elem map into another
            LatticeElemMap* clone = new LatticeElemMap(*in);

            // Visit the block using this clone
            clone = visit(p->m_nested_block, clone);

            // Join the original "in" lattice_elem_map with the clone,
            // storing the result in the clone
            join_lattice_elem_maps(clone, in);

            // Make "in" point to the clone, deleting in
            delete in;
            in = clone;
    return in;
  }

  LatticeElemMap* visitIfWithElse(IfWithElse *p, LatticeElemMap *in)
  {
            in = visit(p->m_expr, in);

            // Copy this lattice elem map into another
            LatticeElemMap* clone = new LatticeElemMap(*in);

            // Visit the block using this clone
            clone = visit(p->m_nested_block_1, clone);
            in = visit(p->m_nested_block_2,in);
            // Join the original "in" lattice_elem_map with the clone,
            // storing the result in the clone
            join_lattice_elem_maps(clone, in);

            // Make "in" point to the clone, deleting in
            delete in;
            in = clone;
    return in;
  }

  LatticeElemMap* visitWhileLoop(WhileLoop *p, LatticeElemMap *in)
  {
    // NOTE: this here is implemented for you. Use it as a template
    
    // While loops happen in this order: the expression is evaluated at least
    // once, and then any number of times, the block is execuded followed by
    // one more expression evaluation.
    //
    // Therefore, our expression receives input from either the statement
    // before the while, or from the block itself! But the block also receives
    // information from the expression. It goes in circles.
    //
    // Therefore, we're going to loop our LatticeElemMap through the while block
    // as long as there are changes to the LatticeElemMap. Once the LatticeElemMap
    // stops being changed by iterating through the block, we've reached a fix-point
    // and only then can we continue.
    

    // So first, visit the expression.
    in = visit(p->m_expr, in);

    // And then, as many times as needed,
    while(true) {
	    // Copy this lattice elem map into another
    	LatticeElemMap* clone = new LatticeElemMap(*in);
	
	    // Visit the block using this clone
    	clone = visit(p->m_nested_block, clone);
    	
		// now visit the expression
		clone = visit(p->m_expr, clone);

	 	// Join the original "in" lattice_elem_map with the clone,
	 	// storing the result in the clone
	 	join_lattice_elem_maps(clone, in);

	    // Compare them
	    bool equal = lattice_maps_equal(in, clone);

		// Make "in" point to the clone, deleting in
		delete in;
		in = clone;

		// If the clone was the same as "in", meaning that we've reached a fix-point,
		// we're done!
		if (equal)
			return in;
    }
  }

  LatticeElemMap* visitTInt(TInt *p, LatticeElemMap *in)
  {
    in = visit_children_of(p, in);
    return in;
  }

  LatticeElemMap* visitTBool(TBool *p, LatticeElemMap *in)
  {
    in = visit_children_of(p, in);
    return in;
  }

  LatticeElemMap* visitTIntArray(TIntArray *p, LatticeElemMap *in)
  {
    in = visit_children_of(p, in);
    return in;
  }

  LatticeElemMap* visitAnd(And *p, LatticeElemMap *in)
  {
    in = visit_children_of(p, in);

            LatticeElem &e1 = p->m_expr_1->m_attribute.m_lattice_elem;
            LatticeElem &e2 = p->m_expr_2->m_attribute.m_lattice_elem;

            // If it's TOP, then we cannot know anything about this expression; it should be TOP as well
            if (!e1.value || !e2.value){
                p->m_attribute.m_lattice_elem = false;
            } else if (e1 == TOP || e2 == TOP){
                p->m_attribute.m_lattice_elem = TOP;
            } else {
                p->m_attribute.m_lattice_elem = e1.value&&e2.value;
            }

    return in;

  }
  
  LatticeElemMap* visitOr(Or *p, LatticeElemMap *in)
  {
    in = visit_children_of(p, in);

            LatticeElem &e1 = p->m_expr_1->m_attribute.m_lattice_elem;
            LatticeElem &e2 = p->m_expr_2->m_attribute.m_lattice_elem;

            // If it's TOP, then we cannot know anything about this expression; it should be TOP as well
            if (e1.value || e2.value){
                p->m_attribute.m_lattice_elem = true;
            } else if (e1 == TOP || e2 == TOP){
                p->m_attribute.m_lattice_elem = TOP;
            } else {
                p->m_attribute.m_lattice_elem = e1.value||e2.value;
            }

    return in;

  }

  LatticeElemMap* visitCompare(Compare *p, LatticeElemMap *in)
  {
    in = visit_children_of(p, in);

            LatticeElem &e1 = p->m_expr_1->m_attribute.m_lattice_elem;
            LatticeElem &e2 = p->m_expr_2->m_attribute.m_lattice_elem;

            // If it's TOP, then we cannot know anything about this expression; it should be TOP as well
            if (e1 == TOP || e2 == TOP){
                p->m_attribute.m_lattice_elem = TOP;
            } else {
                p->m_attribute.m_lattice_elem = e1.value==e2.value;
            }

    return in;

  }
  
  LatticeElemMap* visitNoteq(Noteq *p, LatticeElemMap *in)
  {
    in = visit_children_of(p, in);

            LatticeElem &e1 = p->m_expr_1->m_attribute.m_lattice_elem;
            LatticeElem &e2 = p->m_expr_2->m_attribute.m_lattice_elem;

            // If it's TOP, then we cannot know anything about this expression; it should be TOP as well
            if (e1 == TOP || e2 == TOP){
                p->m_attribute.m_lattice_elem = TOP;
            } else {
                p->m_attribute.m_lattice_elem = e1.value!=e2.value;
            }

    return in;

  }

  LatticeElemMap* visitGt(Gt *p, LatticeElemMap *in)
  {
    in = visit_children_of(p, in);

            LatticeElem &e1 = p->m_expr_1->m_attribute.m_lattice_elem;
            LatticeElem &e2 = p->m_expr_2->m_attribute.m_lattice_elem;

            // If it's TOP, then we cannot know anything about this expression; it should be TOP as well
            if (e1 == TOP || e2 == TOP){
                p->m_attribute.m_lattice_elem = TOP;
            } else {
                p->m_attribute.m_lattice_elem = e1.value>e2.value;
            }

    return in;

  }

  LatticeElemMap* visitGteq(Gteq *p, LatticeElemMap *in)
  {
    in = visit_children_of(p, in);

            LatticeElem &e1 = p->m_expr_1->m_attribute.m_lattice_elem;
            LatticeElem &e2 = p->m_expr_2->m_attribute.m_lattice_elem;

            // If it's TOP, then we cannot know anything about this expression; it should be TOP as well
            if (e1 == TOP || e2 == TOP){
                p->m_attribute.m_lattice_elem = TOP;
            } else {
                p->m_attribute.m_lattice_elem = e1.value>=e2.value;
            }

    return in;

  }

  LatticeElemMap* visitLt(Lt *p, LatticeElemMap *in)
  {
    in = visit_children_of(p, in);

            LatticeElem &e1 = p->m_expr_1->m_attribute.m_lattice_elem;
            LatticeElem &e2 = p->m_expr_2->m_attribute.m_lattice_elem;

            // If it's TOP, then we cannot know anything about this expression; it should be TOP as well
            if (e1 == TOP || e2 == TOP){
                p->m_attribute.m_lattice_elem = TOP;
            } else {
                p->m_attribute.m_lattice_elem = e1.value<e2.value;
            }

    return in;

  }

  LatticeElemMap* visitLteq(Lteq *p, LatticeElemMap *in)
  {
    in = visit_children_of(p, in);

            LatticeElem &e1 = p->m_expr_1->m_attribute.m_lattice_elem;
            LatticeElem &e2 = p->m_expr_2->m_attribute.m_lattice_elem;

            // If it's TOP, then we cannot know anything about this expression; it should be TOP as well
            if (e1 == TOP || e2 == TOP){
                p->m_attribute.m_lattice_elem = TOP;
            } else {
                p->m_attribute.m_lattice_elem = e1.value<=e2.value;
            }

    return in;
  }

  LatticeElemMap* visitUminus(Uminus *p, LatticeElemMap *in)
  {
    in = visit_children_of(p, in);
            LatticeElem &e = p->m_expr->m_attribute.m_lattice_elem;

            // If it's TOP, then we cannot know anything about this expression; it should be TOP as well
            if (e == TOP){
                p->m_attribute.m_lattice_elem = TOP;
            } else{
                p->m_attribute.m_lattice_elem = -1*(e.value);
            }
    return in;
  }
  
  LatticeElemMap* visitMagnitude(Magnitude *p, LatticeElemMap *in)
  {
    in = visit_children_of(p, in);
            // Read that lattice element
            LatticeElem &e = p->m_expr->m_attribute.m_lattice_elem;

            // If it's TOP, then we cannot know anything about this expression; it should be TOP as well
            if (e == TOP){
                p->m_attribute.m_lattice_elem = TOP;
            } else if( e.value <0){
                p->m_attribute.m_lattice_elem = -1*(e.value);
            } else{
                p->m_attribute.m_lattice_elem = (e.value);
            }

    return in;
  }

  LatticeElemMap* visitPlus(Plus *p, LatticeElemMap *in)
  {
    in = visit_children_of(p, in);

            LatticeElem &e1 = p->m_expr_1->m_attribute.m_lattice_elem;
            LatticeElem &e2 = p->m_expr_2->m_attribute.m_lattice_elem;

            // If it's TOP, then we cannot know anything about this expression; it should be TOP as well
            if (e1 == TOP || e2 == TOP){
                p->m_attribute.m_lattice_elem = TOP;
            } else {
                p->m_attribute.m_lattice_elem = e1.value+e2.value;
            }

    return in;
  }

  LatticeElemMap* visitMinus(Minus *p, LatticeElemMap *in)
  {
    in = visit_children_of(p, in);

            LatticeElem &e1 = p->m_expr_1->m_attribute.m_lattice_elem;
            LatticeElem &e2 = p->m_expr_2->m_attribute.m_lattice_elem;

            // If it's TOP, then we cannot know anything about this expression; it should be TOP as well
            if (e1 == TOP || e2 == TOP){
                p->m_attribute.m_lattice_elem = TOP;
            } else {
                p->m_attribute.m_lattice_elem = e1.value-e2.value;
            }

    return in;
  }

  LatticeElemMap* visitTimes(Times *p, LatticeElemMap *in)
  {
    in = visit_children_of(p, in);

            LatticeElem &e1 = p->m_expr_1->m_attribute.m_lattice_elem;
            LatticeElem &e2 = p->m_expr_2->m_attribute.m_lattice_elem;

            // If it's TOP, then we cannot know anything about this expression; it should be TOP as well
            if (e1.value == 0 || e2.value == 0){
                p->m_attribute.m_lattice_elem = 0;
            } else if (e1 == TOP || e2 == TOP){
                p->m_attribute.m_lattice_elem = TOP;
            } else {
                p->m_attribute.m_lattice_elem = e1.value*e2.value;
            }

    return in;
  }

  LatticeElemMap* visitDiv(Div *p, LatticeElemMap *in)
  {
    // now anything div by 0 will result in 0 instead of TOP ( x/0 = 0) update your code
    // accordingly
    in = visit_children_of(p, in);

            LatticeElem &e1 = p->m_expr_1->m_attribute.m_lattice_elem;
            LatticeElem &e2 = p->m_expr_2->m_attribute.m_lattice_elem;

            if (e2.value == 0){
                p->m_attribute.m_lattice_elem = 0;
            } else if (e1.value == 0){
                p->m_attribute.m_lattice_elem = 0;
            } else if (e1 == TOP || e2 == TOP){
                p->m_attribute.m_lattice_elem = TOP;
            } else {
                p->m_attribute.m_lattice_elem = e1.value/e2.value;
            }

    return in;
  }

  LatticeElemMap* visitNot(Not *p, LatticeElemMap *in)
  {
    // First visit the child expression. After this line of code, the child
    // expression should have a valid LatticeElem stored inside it
    in = visit_children_of(p, in);

    // Read that lattice element
    LatticeElem &e = p->m_expr->m_attribute.m_lattice_elem;

    // If it's TOP, then we cannot know anything about this expression; it should be TOP as well
    if (e == TOP)
	    p->m_attribute.m_lattice_elem = TOP;
    else
	    // Otherwise, it contains the boolean opposite of the child's LatticeElem
	    p->m_attribute.m_lattice_elem = !(e.value);

    // And now we return the LatticeElemMap. We didn't modify it, we didn't need to.
    return in;
  }

  LatticeElemMap* visitIdent(Ident *p, LatticeElemMap *in)
  {
    in = visit_children_of(p, in);
            if(in->find(p->m_symname->spelling()) == in->end()){
                p->m_attribute.m_lattice_elem = TOP;
            } else{
                p->m_attribute.m_lattice_elem = (*in)[p->m_symname->spelling()];
            }
    return in;
  }

  LatticeElemMap* visitArrayAccess(ArrayAccess *p, LatticeElemMap *in)
  {
    in = visit_children_of(p, in);
            p->m_attribute.m_lattice_elem = TOP;
    return in;
  }

  LatticeElemMap* visitIntLit(IntLit *p, LatticeElemMap *in)
  {
    // store the constant's value in this expression's LatticeElem
    p->m_attribute.m_lattice_elem = p -> m_primitive -> m_data;
    return in;
  }

  LatticeElemMap* visitBoolLit(BoolLit *p, LatticeElemMap *in)
  {
    // store the constant's value in this expression's LatticeElem
    p->m_attribute.m_lattice_elem = p -> m_primitive -> m_data;
    return in;
  }

  LatticeElemMap* visitSymName(SymName *p, LatticeElemMap *in)
  {
    in = visit_children_of(p, in);
    return in;
  }

  LatticeElemMap* visitPrimitive(Primitive *p, LatticeElemMap *in)
  {
    return in;
  }

  ConstantFolding(FILE* errorfile, SymTab* st) 
  {
	m_errorfile = errorfile;
	m_st = st; 
  }

  ~ConstantFolding() {}
};


