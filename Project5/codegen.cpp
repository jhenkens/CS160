#include "ast.hpp"
#include "symtab.hpp"
#include "primitive.hpp"
#include "assert.h"
#include <typeinfo>
#include <stdio.h>

#pragma GCC diagnostic ignored "-Wwrite-strings"
#define FOLDING 0


#define TESTING 0
#define DUMPING 0
#define forall(iterator,listptr) \
    for(iterator = listptr->begin(); iterator != listptr->end(); iterator++) \

#define forallrev(iterator,listptr) \
    for(iterator = listptr->rbegin(); iterator != listptr->rend(); iterator++) \

#define tprint(...) if(TESTING) printf(__VA_ARGS__)
#define tdump(...) if(DUMPING) m_st->dump(stdout)

#define mpr(...) fprintf(m_outputfile,__VA_ARGS__)


class Codegen : public Visitor
{
    private:

        FILE * m_outputfile;
        SymTab *m_st;

        // basic size of a word (integers and booleans) in bytes
        static const int wordsize = 4;
        static const int fFBefore = 2*wordsize;
        //have 4 words here, because we save 3 words on the stack to save data
        //but we don't reach free memory till the fourth word
        static const int fFAfter = 4*wordsize;

        int label_count; //access with new_label

        // ********** Helper functions ********************************

        // this is used to get new unique labels (cleverly named label1, label2, ...)
        int new_label() { return label_count++; }

        // this mode is used for the code
        void set_text_mode() { fprintf( m_outputfile, ".text\n\n"); }

        // PART 1:
        // 1) get arithmetic expressions on integers working:
        //    you wont really be able to run your code,
        //    but you can visually inspect it to see that the correct
        //    chains of opcodes are being generated.
        // 2) get function calls working:
        //    if you want to see at least a very simple program compile
        //    and link successfully against gcc-produced code, you
        //    need to get at least this far
        // 3) get boolean operation working
        //    before we can implement any of the conditional control flow 
        //    stuff, we need to have booleans worked out.  
        // 4) control flow:
        //    we need a way to have if-elses and while loops in our language. 
        //
        // Hint: Symbols have an associated member variable called m_offset
        //    That offset can be used to figure out where in the activation 
        //    record you should look for a particuar variable

        ///////////////////////////////////////////////////////////////////////////////
        //
        //  function_prologue
        //  function_epilogue
        //
        //  Together these two functions implement the callee-side of the calling
        //  convention.  A stack frame has the following layout:
        //
        //                          <- SP (before pre-call / after post-ret)
        //  high -----------------
        //       | actual arg n  |
        //       | ...           |
        //       | actual arg 1  |  <- SP (just before call / just after ret)
        //       -----------------
        //       |  Return Addr  |  <- SP (just after call / just before ret)
        //       =================
        //       | previous %ebp |
        //       -----------------
        //       | temporary 1   |
        //       | ...           |
        //       | temporary n   |  <- SP (after prologue / before epilogue)
        //  low  -----------------
        //
        //
        //            ||        
        //            ||
        //           \  /
        //            \/
        //
        //
        //  The caller is responsible for placing the actual arguments
        //  and the return address on the stack. Actually, the return address
        //  is put automatically on the stack as part of the x86 call instruction.
        //
        //  On function entry, the callee
        //
        //  (1) allocates space for the callee's temporaries on the stack
        //  
        //  (2) saves callee-saved registers (see below) - including the previous activation record pointer (%ebp)
        //
        //  (3) makes the activation record pointer (frame pointer - %ebp) point to the start of the temporary region
        //
        //  (4) possibly copies the actual arguments into the temporary variables to allow easier access
        //
        //  On function exit, the callee:
        //
        //  (1) pops the callee's activation record (temporary area) off the stack
        //  
        //  (2) restores the callee-saved registers, including the activation record of the caller (%ebp)    
        //
        //  (3) jumps to the return address (using the x86 "ret" instruction, this automatically pops the 
        //    return address of the stack. After the ret, remove the arguments from the stack
        //
        //  For more info on this convention, see http://unixwiz.net/techtips/win32-callconv-asm.html
        //
        //  This convention is called __cdecl
        //
        //////////////////////////////////////////////////////////////////////////////

        void emit_prologue(SymName *name, unsigned int size_locals, unsigned int num_args)
        {
            tprint("// Function %s, with %d bytes of locals, and %d args\n",name->spelling(),size_locals,num_args);
            if(strcmp("Main",name->spelling())==0){
                mpr("_Main:\n");
            }
            mpr("%s:\n",name->spelling());
            tprint("// Store caller EBP\n");
            mpr("    push %%ebp\n");
            mpr("    mov %%esp, %%ebp\n");
            tprint("// Store other caller regs\n");
            mpr("    push %%ebx\n");
            mpr("    push %%esi\n");
            mpr("    push %%edi\n");
            if(num_args>0){
                tprint("// Copy function args to local space. Saved regs + return addr take up %d space before %%ebp\n",fFBefore);
                int offset = fFBefore;
                while(num_args>0){
                    tprint("// Copying arg to local: offset %d\n",offset);
                    mpr("    mov %d(%%ebp), %%eax\n",offset);
                    mpr("    push %%eax\n");
                    offset+=wordsize;
                    num_args--;
                }
                tprint("// Done copying function args\n");
            }
            if(size_locals>0){
                tprint("// Decrement the stack pointer to make space for locals\n");
                mpr("    sub $%d, %%esp\n",size_locals);
            }
        }

        void emit_epilogue()
        {
            tprint("// Starting function epilogue. Pop the three basic regs\n");
            tprint("// Then call leave, which does mov ebp esp, then pop ebp, then ret\n");
            mpr("    pop %%edi\n");
            mpr("    pop %%esi\n");
            mpr("    pop %%ebx\n");
            mpr("    leave\n");
            mpr("    ret\n");
            tprint("// Done with Epilogue\n");
        }

        // HERE: more functions to emit code
        //
        void emit_conditional(Expr* e1, Expr* e2, LatticeElem& lE, char* labelName, char* instr, int labelNum)
        {
            if(!FOLDING || lE == TOP){
                visit(e1);
                visit(e2);
                tprint("// %s. ebx==expr2, eax==expr1\n",labelName);
                mpr("    pop %%ebx\n");
                mpr("    pop %%eax\n");
                tprint("// %s:: Perform the cmparison, jump, otherwise set 0, and jump to done\n",labelName);
                mpr("    cmp %%ebx, %%eax\n");
                mpr("    %s %s%d\n",instr,labelName,labelNum);
                mpr("    mov $0, %%eax\n");
                mpr("    jmp %sDone%d\n",labelName,labelNum);
                mpr("%s%d:\n",labelName,labelNum);
                mpr("    mov $1, %%eax\n");
                mpr("%sDone%d:\n",labelName,labelNum);
                mpr("    push %%eax\n");
                tprint("// Done with %s\n",labelName);
            } else{
                tprint("// %s - FOLDED\n",labelName);
                mpr("    mov $%d, %%eax\n",lE.value);
                mpr("    push %%eax\n");
            }
        }

        ////////////////////////////////////////////////////////////////////////////////

    public:

        Codegen(FILE * outputfile, SymTab * st)
        {
            m_outputfile = outputfile;
            m_st = st;
            label_count = 0;
        }

        void visitProgram(Program * p)
        {
            mpr(".globl _Main\n");
            mpr(".globl Main\n");
            /*list<Func_ptr>::iterator listItr;
            mpr(".globl");
            forall(listItr,p->m_func_list){
                mpr(" ");
                lpr((*listItr)->m_symname->spelling());
            }
            mpr("\n");*/ // This isn't actually what you do!
            visit_children_of(p);

        }
        void visitFunc(Func * p)
        {
            int scopeSize = m_st->scopesize(p->m_function_block->m_attribute.m_scope);
            int numParams = p->m_param_list->size();
            emit_prologue(p->m_symname,scopeSize-(wordsize*numParams),numParams);
            visit(p->m_function_block);
            emit_epilogue();
        }
        void visitFunction_block(Function_block * p)
        {
            visit_children_of(p);
        }
        void visitNested_block(Nested_block * p)
        {
            visit_children_of(p);
        }
        void visitAssignment(Assignment * p)
        {
            Symbol* s = m_st->lookup(p->m_attribute.m_scope,p->m_symname->spelling());
            assert(s!=NULL);
            tprint("// Visiting assign %s: pop off stack, then save to loc with offset %d\n",p->m_symname->spelling(),s->get_offset());
            tprint("// There are %d bytes after ebp used for storing caller regs\n",fFAfter);
            if(!FOLDING || p->m_expr->m_attribute.m_lattice_elem == TOP){
                visit(p->m_expr);
                mpr("    pop %%eax\n");
                mpr("    mov %%eax, -%d(%%ebp)\n",s->get_offset()+fFAfter);
            } else{
                tprint("// Assign FOLDED!\n");
                mpr("    movl $%d, -%d(%%ebp)\n",p->m_expr->m_attribute.m_lattice_elem.value,s->get_offset()+fFAfter);
            }
        }
        void visitArrayAssignment(ArrayAssignment * p)
        {
            tdump();
            Symbol* s = m_st->lookup(p->m_attribute.m_scope,p->m_symname->spelling());
            assert(s!=NULL);
            int startOfArray = s->get_offset() - 1*wordsize + s->arr_length*wordsize;
            if(!FOLDING || (p->m_expr_1->m_attribute.m_lattice_elem == TOP && p->m_expr_1->m_attribute.m_lattice_elem == TOP)){
                visit(p->m_expr_1);
                visit(p->m_expr_2);
                tprint("// Visiting array assign to %s",p->m_symname->spelling());
                tprint("// Array has offset %d, but has %d elements. Thus, index of arr[0] is at offset: %d\n",s->get_offset(),s->arr_length,startOfArray);
                tprint("// This is because the offset is the value of the last element in the array.\n");
                tprint("// Second stack pop for for array index. a(b,c,d) == b+c*d+a\n");
                tprint("// There are %d bytes after ebp used for storing caller regs\n",fFAfter);
                mpr("    pop %%eax\n");
                mpr("    pop %%ebx\n");
                mpr("    mov %%eax, -%d(%%ebp,%%ebx,%d)\n",startOfArray+fFAfter,wordsize);
            } else if (p->m_expr_2->m_attribute.m_lattice_elem == TOP){
                visit(p->m_expr_2);
                tprint("// Visiting array assign to %s",p->m_symname->spelling());
                tprint("// Array has offset %d, but has %d elements. Thus, index of arr[0] is at offset: %d\n",s->get_offset(),s->arr_length,startOfArray);
                tprint("// Second stack pop for for array index. a(b,c,d) == b+c*d+a\n");
                tprint("// There are %d bytes after ebp used for storing caller regs\n",fFAfter);
                tprint("// Array Assign Expr1 folded!\n");
                mpr("    pop %%eax\n");
                mpr("    mov $%d, %%ebx\n",p->m_expr_1->m_attribute.m_lattice_elem.value);
                mpr("    mov %%eax, -%d(%%ebp)\n",startOfArray+fFAfter-(p->m_expr_1->m_attribute.m_lattice_elem.value*wordsize));
            } else if (p->m_expr_1->m_attribute.m_lattice_elem == TOP){
                visit(p->m_expr_1);
                tprint("// Visiting array assign to %s",p->m_symname->spelling());
                tprint("// Array has offset %d, but has %d elements. Thus, index of arr[0] is at offset: %d\n",s->get_offset(),s->arr_length,startOfArray);
                tprint("// Second stack pop for for array index. a(b,c,d) == b+c*d+a\n");
                tprint("// There are %d bytes after ebp used for storing caller regs\n",fFAfter);
                tprint("// Array Assign Expr2 folded!\n");
                mpr("    pop %%ebx\n");
                mpr("    mov $%d, %%eax\n",p->m_expr_2->m_attribute.m_lattice_elem.value);
                mpr("    mov %%eax, -%d(%%ebp,%%ebx,%d)\n",startOfArray+fFAfter,wordsize);
            } else{
                tprint("// Visiting array assign to %s",p->m_symname->spelling());
                tprint("// Array has offset %d, but has %d elements. Thus, index of arr[0] is at offset: %d\n",s->get_offset(),s->arr_length,startOfArray);
                tprint("// Second stack pop for for array index. a(b,c,d) == b+c*d+a\n");
                tprint("// There are %d bytes after ebp used for storing caller regs\n",fFAfter);
                tprint("// Array Assign completely folded!\n");
                mpr("    movl $%d, -%d(%%ebp)\n",p->m_expr_2->m_attribute.m_lattice_elem.value,startOfArray+fFAfter-(p->m_expr_1->m_attribute.m_lattice_elem.value*wordsize));
            }

        }
        void visitCall(Call * p)
        {
            Symbol* s1 = m_st->lookup(p->m_attribute.m_scope,p->m_symname_1->spelling());
            assert(s1!=NULL);
            list<Expr_ptr>::reverse_iterator exprItr;
            int dec = 0;
            tprint("// visitCall: %s\n",p->m_symname_2->spelling());
            tprint("// Visiting call arguments. Each will be pushed onto stack in reverse order.\n");
            tprint("// reverse order is x86 calling convention, as per http://www.delorie.com/djgpp/doc/ug/asm/calling.html\n");
            forallrev(exprItr,p->m_expr_list){
                visit((*exprItr));
                dec+=wordsize;
            }
            tprint("// Visiting call %s -- finished setting up arguments\n",p->m_symname_2->spelling());
            mpr("    call %s\n",p->m_symname_2->spelling());
            tprint("// After call, result is in eax\n");
            tprint("// Call assign: save eax to loc with offset %d\n",s1->get_offset());
            tprint("// There are %d bytes after ebp used for storing caller regs\n",fFAfter);
            mpr("    mov %%eax, -%d(%%ebp)\n",s1->get_offset()+fFAfter);
            tprint("// Now we have to clean up stack from our arguments. We pushed %d bytes on\n",dec);
            mpr("    add $%d, %%esp\n",dec);
        }
        void visitArrayCall(ArrayCall *p)
        {
            bool folded = FOLDING && (p->m_expr_1->m_attribute.m_lattice_elem != TOP);
            if(!folded){
                visit(p->m_expr_1);
            }

            Symbol* s1 = m_st->lookup(p->m_attribute.m_scope,p->m_symname_1->spelling());
            assert(s1!=NULL);
            int startOfArray = s1->get_offset() - 1*wordsize + s1->arr_length*wordsize;

            list<Expr_ptr>::reverse_iterator exprItr;
            int dec = 0;
            tprint("// visitArrayCallCall of function: %s\n",p->m_symname_2->spelling());
            tprint("// Assigning array call to %s",p->m_symname_1->spelling());
            tprint("// Array has offset %d, but has %d elements. Thus, index of arr[0] is at offset: %d\n",s1->get_offset(),s1->arr_length,startOfArray);
            tprint("// Visiting call arguments. Each will be pushed onto stack in reverse order.\n");
            tprint("// reverse order is x86 calling convention, as per http://www.delorie.com/djgpp/doc/ug/asm/calling.html\n");
            forallrev(exprItr,p->m_expr_list_2){
                visit((*exprItr));
                dec+=wordsize;
            }
            tprint("// Visiting array call %s -- finished setting up arguments\n",p->m_symname_2->spelling());
            mpr("    call %s\n",p->m_symname_2->spelling());
            tprint("// After call, result is in eax\n");
            tprint("// Call array assign: save eax to loc with offset, and index %d\n",s1->get_offset());
            tprint("// There are %d bytes after ebp used for storing caller regs\n",fFAfter);
            mpr("    pop %%eax\n");
            if(!folded){
                tprint("// Array index is in ebx. It was not folded\n");
                mpr("    pop %%ebx\n");
                mpr("    mov %%eax, -%d(%%ebp,%%ebx,%d)\n",startOfArray+fFAfter,wordsize);
            } else{
                tprint("// Array index folded in ArrayCall!\n");
                mpr("    movl %%eax, -%d(%%ebp)\n",startOfArray+fFAfter-(p->m_expr_1->m_attribute.m_lattice_elem.value*wordsize));
            }
            mpr("    add $%d, %%esp\n",dec);
        }
        void visitReturn(Return * p)
        {   tprint("// Starting return statement\n");
            if(!FOLDING || p->m_expr->m_attribute.m_lattice_elem == TOP){
                visit(p->m_expr);
                tprint("// Finished the expr of return, now pop to eax\n");
                mpr("    pop %%eax\n");
            } else{
                tprint("// Return FOLDED!\n");
                mpr("    movl $%d, %%eax\n",p->m_expr->m_attribute.m_lattice_elem.value);
            }
            tprint("// End of return statement\n");
        }

        // control flow
        void visitIfNoElse(IfNoElse * p)
        {
            if(!FOLDING || p->m_expr->m_attribute.m_lattice_elem == TOP){
                int label = new_label();
                visit(p->m_expr);
                tprint("// IfNoElse\n");
                mpr("    pop %%eax\n");
                mpr("    cmp $1, %%eax\n");
                tprint("// IfNoElse:: Compare with 1, but jump if not equal\n");
                mpr("    jne IfNoElseDone%d\n",label);
                visit(p->m_nested_block);
                mpr("IfNoElseDone%d:\n",label);
                tprint("// Done with IfNoElse\n");
            } else if(p->m_expr->m_attribute.m_lattice_elem.value == 1){
                tprint("// IfNoElse - FOLDED to TRUE\n");
                visit(p->m_nested_block);
            } else{
                tprint("// IfNoElse - FOLDED to FALSE. Eliminated all code.\n");
            }
        }
        void visitIfWithElse(IfWithElse * p)
        {
            if(!FOLDING || p->m_expr->m_attribute.m_lattice_elem == TOP){
                int label = new_label();
                int donelabel = new_label();
                visit(p->m_expr);
                tprint("// IfWithElse\n");
                mpr("    pop %%eax\n");
                mpr("    cmp $1, %%eax\n");
                tprint("// IfWithElse:: Compare with 1, but jump if not equal\n");
                mpr("    jne IfWithElse%d\n",label);
                tprint("// IfWithElse:: If block\n");
                visit(p->m_nested_block_1);
                mpr("    jmp IfWithElseDone%d\n",donelabel);
                tprint("// IfWithElse:: Else block\n");
                mpr("IfWithElse%d:\n",label);
                visit(p->m_nested_block_2);
                mpr("IfWithElseDone%d:\n",donelabel);
                tprint("// Done with IfWithElse\n");
            } else if(p->m_expr->m_attribute.m_lattice_elem.value == 1){
                tprint("// IfWithElse - FOLDED to TRUE\n");
                visit(p->m_nested_block_1);
            } else{
                tprint("// IfWithElse - FOLDED to FALSE\n");
                visit(p->m_nested_block_2);
            }
        }
        void visitWhileLoop(WhileLoop * p)
        {
            if(!FOLDING || p->m_expr->m_attribute.m_lattice_elem == TOP || p->m_expr->m_attribute.m_lattice_elem.value == 1){
                int label = new_label();
                int donelabel = new_label();
                tprint("// While\n");
                mpr("While%d:\n",label);
                visit(p->m_expr);
                mpr("    pop %%eax\n");
                mpr("    cmp $1, %%eax\n");
                tprint("// While:: Compare with 1, but jump if not equal\n");
                mpr("    jne WhileDone%d\n",donelabel);
                visit(p->m_nested_block);
                mpr("    jmp While%d\n",label);
                mpr("WhileDone%d:\n",donelabel);
                tprint("// Done with While\n");
            } else {
                tprint("// While - FOLDED to FALSE. Eliminated all code.\n");
            }
        }

        // variable declarations (no code generation needed)
        void visitDecl(Decl * p)
        {
            visit_children_of(p);
        }
        void visitParam(Param *p)
        {
            visit_children_of(p);
        }
        void visitTInt(TInt * p)
        {
            visit_children_of(p);
        }
        void visitTBool(TBool * p)
        {
            visit_children_of(p);
        }
        void visitTIntArray(TIntArray * p)
        {
            visit_children_of(p);
        }

        // comparison operations
        void visitCompare(Compare * p)
        {
            emit_conditional(p->m_expr_1,p->m_expr_2,p->m_attribute.m_lattice_elem,
                "Compare", "je", new_label());
        }
        void visitNoteq(Noteq * p)
        {
            emit_conditional(p->m_expr_1,p->m_expr_2,p->m_attribute.m_lattice_elem,
                "NotEq", "jne", new_label());
        }
        void visitGt(Gt * p)
        {
            emit_conditional(p->m_expr_1,p->m_expr_2,p->m_attribute.m_lattice_elem,
                "GreaterThan", "jg", new_label());
        }
        void visitGteq(Gteq * p)
        {
            emit_conditional(p->m_expr_1,p->m_expr_2,p->m_attribute.m_lattice_elem,
                "GreaterThanEq", "jge", new_label());
        }
        void visitLt(Lt * p)
        {
            emit_conditional(p->m_expr_1,p->m_expr_2,p->m_attribute.m_lattice_elem,
                "LessThan", "jl", new_label());
        }
        void visitLteq(Lteq * p)
        {
            emit_conditional(p->m_expr_1,p->m_expr_2,p->m_attribute.m_lattice_elem,
                "LessThanEqual", "jle", new_label());
        }

        // arithmetic and logic operations
        void visitAnd(And * p)
        {
            if(!FOLDING || p->m_attribute.m_lattice_elem == TOP){
                visit(p->m_expr_1);
                visit(p->m_expr_2);
                tprint("// And\n");
                mpr("    pop %%ebx\n");
                mpr("    pop %%eax\n");
                mpr("    and %%ebx, %%eax\n");
                mpr("    push %%eax\n");
            } else{
                tprint("// And - FOLDED\n");
                mpr("    mov $%d, %%eax\n",p->m_attribute.m_lattice_elem.value);
                mpr("    push %%eax\n");
            }
        }
        void visitOr(Or * p)
        {
            if(!FOLDING || p->m_attribute.m_lattice_elem == TOP){
                visit(p->m_expr_1);
                visit(p->m_expr_2);
                tprint("// Or\n");
                mpr("    pop %%ebx\n");
                mpr("    pop %%eax\n");
                mpr("    or %%ebx, %%eax\n");
                mpr("    push %%eax\n");
            } else{
                tprint("// Or - FOLDED\n");
                mpr("    mov $%d, %%eax\n",p->m_attribute.m_lattice_elem.value);
                mpr("    push %%eax\n");
            }
        }
        void visitMinus(Minus * p)
        {
            if(!FOLDING || p->m_attribute.m_lattice_elem == TOP){
                visit(p->m_expr_1);
                visit(p->m_expr_2);
                tprint("// Minus\n");
                mpr("    pop %%ebx\n"); // expr2
                mpr("    pop %%eax\n"); // expr1
                // sub src dest -> dest = dest - src
                mpr("    sub %%ebx, %%eax\n");
                mpr("    push %%eax\n");
            } else{
                tprint("// Minus - FOLDED\n");
                mpr("    mov $%d, %%eax\n",p->m_attribute.m_lattice_elem.value);
                mpr("    push %%eax\n");
            }
        }
        void visitPlus(Plus * p)
        {
            if(!FOLDING || p->m_attribute.m_lattice_elem == TOP){
                visit(p->m_expr_1);
                visit(p->m_expr_2);
                tprint("// Plus\n");
                mpr("    pop %%ebx\n");
                mpr("    pop %%eax\n");
                mpr("    add %%ebx, %%eax\n");
                mpr("    push %%eax\n");
            } else{
                tprint("// Plus - FOLDED\n");
                mpr("    mov $%d, %%eax\n",p->m_attribute.m_lattice_elem.value);
                mpr("    push %%eax\n");
            }
        }
        void visitTimes(Times * p)
        {
            if(!FOLDING || p->m_attribute.m_lattice_elem == TOP){
                visit(p->m_expr_1);
                visit(p->m_expr_2);
                tprint("// Times\n");
                mpr("    pop %%ebx\n");
                mpr("    pop %%eax\n");
                mpr("    imul %%ebx\n");
                mpr("    push %%eax\n");
            } else{
                tprint("// Times - FOLDED\n");
                mpr("    mov $%d, %%eax\n",p->m_attribute.m_lattice_elem.value);
                mpr("    push %%eax\n");
            }
        }
        void visitDiv(Div * p)
        {
            if(!FOLDING || p->m_attribute.m_lattice_elem == TOP){
                visit(p->m_expr_1);
                visit(p->m_expr_2);
                tprint("// Div\n");
                mpr("    pop %%ebx\n");
                mpr("    pop %%eax\n");
                mpr("    cdq\n");
                mpr("    idiv %%ebx\n");
                mpr("    push %%eax\n");
            } else{
                tprint("// Div - FOLDED\n");
                mpr("    mov $%d, %%eax\n",p->m_attribute.m_lattice_elem.value);
                mpr("    push %%eax\n");
            }
        }
        void visitNot(Not * p)
        {
            if(!FOLDING || p->m_attribute.m_lattice_elem == TOP){
                visit(p->m_expr);
                tprint("// Not\n");
                mpr("    pop %%eax\n");
                mpr("    not %%eax\n");
                mpr("    push %%eax\n");
            } else{
                tprint("// Not - FOLDED\n");
                mpr("    mov $%d, %%eax\n",p->m_attribute.m_lattice_elem.value);
                mpr("    push %%eax\n");
            }
        }
        void visitUminus(Uminus * p)
        {
            if(!FOLDING || p->m_attribute.m_lattice_elem == TOP){
                visit(p->m_expr);
                tprint("// Uminus\n");
                mpr("    pop %%eax\n");
                mpr("    neg %%eax\n");
                mpr("    push %%eax\n");
            } else{
                tprint("// Uminus - FOLDED\n");
                mpr("    mov $%d, %%eax\n",p->m_attribute.m_lattice_elem.value);
                mpr("    push %%eax\n");
            }
        }
        void visitMagnitude(Magnitude * p)
        {
            // From: http://stackoverflow.com/questions/2639173/x86-assembly-abs-implementation
            if(!FOLDING || p->m_attribute.m_lattice_elem == TOP){
                visit(p->m_expr);
                tprint("// Magnitude\n");
                mpr("    pop %%eax\n");
                mpr("    mov %%eax, %%ebx\n");
                mpr("    neg %%eax\n");
                mpr("    cmovl %%ebx, %%eax\n");
                mpr("    push %%eax\n");
            } else{
                tprint("// Magnitude - FOLDED\n");
                mpr("    mov $%d, %%eax\n",p->m_attribute.m_lattice_elem.value);
                mpr("    push %%eax\n");
            }
        }

        // variable and constant access
        void visitIdent(Ident * p)
        {
            if(!FOLDING || p->m_attribute.m_lattice_elem == TOP){
                Symbol* s = m_st->lookup(p->m_attribute.m_scope,p->m_symname->spelling());
                assert(s!=NULL);
                tprint("// Ident %s\n",p->m_symname->spelling());
                mpr("    mov -%d(%%ebp), %%eax\n",s->get_offset()+fFAfter);
                mpr("    push %%eax\n");
            } else{
                tprint("// Ident - FOLDED\n");
                mpr("    mov $%d, %%eax\n",p->m_attribute.m_lattice_elem.value);
                mpr("    push %%eax\n");
            }
        }
        void visitIntLit(IntLit * p)
        {
            tprint("// IntLit(%d)\n",p->m_primitive->m_data);
            mpr("    movl $%d, %%eax\n",p->m_primitive->m_data);
            mpr("    push %%eax\n");
        }
        void visitBoolLit(BoolLit * p)
        {
            tprint("// BoolLit(%d)\n",p->m_primitive->m_data);
            mpr("    movl $%d, %%eax\n",p->m_primitive->m_data);
            mpr("    push %%eax\n");
        }
        void visitArrayAccess(ArrayAccess * p)
        {
            Symbol* s = m_st->lookup(p->m_attribute.m_scope,p->m_symname->spelling());
            assert(s!=NULL);
            int startOfArray = s->get_offset() - 1*wordsize + s->arr_length*wordsize;
            tprint("// ArrayAccess %s\n", p->m_symname->spelling());
            tprint("// Array has offset %d, but has %d elements. Thus, index of arr[0] is at offset: %d\n",s->get_offset(),s->arr_length,startOfArray);
            if( !FOLDING || p->m_expr->m_attribute.m_lattice_elem == TOP){
                visit(p->m_expr);
                mpr("    pop %%ebx\n");
                mpr("    mov -%d(%%ebp,%%ebx,%d), %%eax\n",startOfArray+fFAfter,wordsize);
                mpr("    push %%eax\n");
            } else{
                tprint("// ArrayAccess was folded with index %d\n",p->m_expr->m_attribute.m_lattice_elem.value);
                mpr("    mov -%d(%%ebp), %%eax\n",startOfArray+fFAfter-(p->m_expr->m_attribute.m_lattice_elem.value*wordsize));
                mpr("    push %%eax\n");
            }
            tprint("// End Array Access\n");
        }

        // special cases
        void visitSymName(SymName * p)
        {
        }
        void visitPrimitive(Primitive * p)
        {
        }
};

