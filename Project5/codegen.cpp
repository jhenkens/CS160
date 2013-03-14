#include "ast.hpp"
#include "symtab.hpp"
#include "primitive.hpp"
#include "assert.h"
#include <typeinfo>
#include <stdio.h>

#pragma GCC diagnostic ignored "-Wwrite-strings"
#ifdef NOFOLDING
#define FOLDING 0
#else
#define FOLDING 1
#endif

#ifdef TESTING
#define TESTING 1
#else
#define TESTING 0
#endif

#ifdef DUMPING
#define DUMPING 1
#else
#define DUMPING 0
#endif

#define forall(iterator,listptr) \
    for(iterator = listptr->begin(); iterator != listptr->end(); iterator++) \

#define forallrev(iterator,listptr) \
    for(iterator = listptr->rbegin(); iterator != listptr->rend(); iterator++) \

#ifndef tprint
#define tprint(...) if(TESTING) printf(__VA_ARGS__)
#endif
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
        // Used inside binary expr to just handle the conditional part!
        void emit_conditional(char* labelName, char* instr, int labelNum)
        {
            tprint("// %s:: Perform the cmparison, jump, otherwise set 0, and jump to done\n",labelName);
            mpr("    cmp %%ebx, %%eax\n");
            mpr("    %s %s%d\n",instr,labelName,labelNum);
            mpr("    mov $0, %%eax\n");
            mpr("    jmp %sDone%d\n",labelName,labelNum);
            mpr("%s%d:\n",labelName,labelNum);
            mpr("    mov $1, %%eax\n");
            mpr("%sDone%d:\n",labelName,labelNum);
        }

        void emit_binary_expr(Expr* e1, Expr* e2, LatticeElem& lE, char op, char* desc)
        {
            if(!FOLDING || lE == TOP){
                LatticeElem& lEL = e1->m_attribute.m_lattice_elem;
                LatticeElem& lER = e2->m_attribute.m_lattice_elem;
                //lC = leftIsConstant, rC = rightIsConstant
                bool lC = lEL != TOP;
                bool rC = lER != TOP;

                if(!FOLDING || !lC){
                    visit(e1);
                }
                if(!FOLDING || !rC){
                    visit(e2);
                }
                tprint("// Visit %s\n",desc);
                //If not folding, or if the right is not constant
                //Then pop. We do right first because if we aren't folding, or if
                //both are non-constant, then we need to pop expr2 first.
                if(!FOLDING || !rC ){
                    mpr("    pop %%ebx\n");
                } else{
                    tprint("// Visit %s not folded, but right hand side did fold\n",desc);
                    mpr("    mov $%d, %%ebx\n",lER.value);
                }
                if(!FOLDING || !lC){
                    mpr("    pop %%eax\n");
                } else{
                    tprint("// Visit %s not folded, but left hand side did fold\n",desc);
                    mpr("    mov $%d, %%eax\n",lEL.value);
                }
                switch(op){
                    case '*':
                        mpr("    imul %%ebx\n");
                        break;
                    case '/':
                        mpr("    cdq\n");
                        mpr("    idiv %%ebx\n");
                        break;
                    case '+':
                        mpr("    add %%ebx, %%eax\n");
                        break;
                    case '-':
                        mpr("    sub %%ebx, %%eax\n");
                        break;
                    case '&':
                        mpr("    and %%ebx, %%eax\n");
                        break;
                    case '|':
                        mpr("    or %%ebx, %%eax\n");
                        break;
                    case '<':
                        emit_conditional(desc,"jl", new_label());
                        break;
                    case '>':
                        emit_conditional(desc,"jg", new_label());
                        break;
                    // This is <=
                    case '(':
                        emit_conditional(desc,"jle", new_label());
                        break;
                    // This is >=
                    case ')':
                        emit_conditional(desc,"jge", new_label());
                        break;
                    case '=':
                        emit_conditional(desc,"je", new_label());
                        break;
                    case '!':
                        emit_conditional(desc,"jne", new_label());
                        break;
                    default:
                        throw "Op not defined in emit_binary_expr!";
                }
                mpr("    push %%eax\n");
            } else{
                tprint("// %s - FOLDED\n",desc);
                mpr("    mov $%d, %%eax\n",lE.value);
                mpr("    push %%eax\n");
            }
        }

        void emit_non_folded_mov(int offset){
            mpr("    mov %%eax, -%d(%%ebp)\n",offset+fFAfter);
        }

        void emit_folded_mov(int val, int offset){
            mpr("    movl $%d, -%d(%%ebp)\n",val,offset+fFAfter);
        }

        int get_folded_array_offset(const char* str, Symbol* s, int index){
            int result = s->get_offset() + (s->arr_length - 1 - index) * wordsize + fFAfter;
            tprint("//Array index expr was folded!\n");
            tprint("//Generated folded array offset (%s[%d] for %s is %d\n",str,index,str,result);
            tprint("//Values used, offset: %d, wordsize: %d, arr_length: %d, index: %d\n",
                s->get_offset(),wordsize,s->arr_length,index);
            return result;
        }

        int get_non_folded_array_offset(const char* str, Symbol* s){
            int result = s->get_offset() + (s->arr_length - 1) * wordsize + fFAfter;
            tprint("//Generated array offset (%s[0]) for %s is %d\n",str,str,result);
            tprint("//Values used, offset: %d, wordsize: %d, arr_length: %d\n",
                s->get_offset(),wordsize,s->arr_length,index);
            return result;
        }

        void emit_generic_array_assign_epilogue(const char* arr_name, bool arr_c,
            LatticeElem& arr_le, Symbol* arr_s, Expr* arr_expr, bool val_c,
            LatticeElem& val_le,bool isCall = false){
            if(!FOLDING || (!arr_c && !val_c)){
                if(!isCall) mpr("    pop %%eax\n");
                mpr("    pop %%ebx\n");
                mpr("    mov %%eax, -%d(%%ebp,%%ebx,%d)\n",
                    get_non_folded_array_offset(arr_name,arr_s),wordsize);
            } else if (arr_c && !val_c){
                tprint("// Visit array%s assign index folded\n",isCall?"Call ":"");
                if(!isCall) mpr("    pop %%eax\n");
                mpr("    mov %%eax, -%d(%%ebp)\n",
                    get_folded_array_offset(arr_name,arr_s,arr_le.value));
            } else if (!arr_c && val_c){
                tprint("// Visit array%s assign value folded\n",isCall?"Call ":"");
                mpr("    pop %%ebx\n");
                mpr("    movl $%d, -%d(%%ebp,%%ebx,%d)\n", val_le.value,
                    get_non_folded_array_offset(arr_name,arr_s),wordsize);
            } else{
                tprint("// Visit array%s assign completely folded\n",isCall?"Call ":"");
                mpr("    movl $%d, -%d(%%ebp)\n", val_le.value,
                    get_folded_array_offset(arr_name,arr_s,arr_le.value));
            }
        }

        int emit_call(char* callType, Symbol* f, const char* f_name, list<Expr_ptr>* expr_list){
            int dec = 0;
            int count = 0;
            list<Expr_ptr>::reverse_iterator exprItr;

            tprint("// %s: %s\n",callType,f_name);
            tprint("// Visiting args. Each will be pushed onto stack in reverse order.\n");
            tprint("// reverse order is x86 calling convention, as per http://www.delorie.com/djgpp/doc/ug/asm/calling.html\n");
            forallrev(exprItr,expr_list){
                LatticeElem& lE = (*exprItr)->m_attribute.m_lattice_elem;
                if(lE!=TOP){
                    tprint("// %s: folding parameter %d\n",callType,count);
                    mpr("    mov $%d, %%eax\n", lE.value);
                } else{
                    visit((*exprItr));
                }
                count++;
                dec+=wordsize;
            }
            tprint("// %s: %s -- finished setting up arguments\n",callType,f_name);
            mpr("    call %s\n",f_name);
            tprint("// After call, result is in eax\n");
            return dec;
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
            tprint("// There are %d bytes after ebp used for storing caller regs\n",fFAfter);
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
            const char* name = p->m_symname->spelling();
            Symbol* s = m_st->lookup(p->m_attribute.m_scope,name);
            assert(s!=NULL);
            Expr* expr = p->m_expr;
            LatticeElem& lE = expr->m_attribute.m_lattice_elem;

            tprint("// Visiting assign %s\n",name);
            tprint("// Pop off stack, then save to loc with offset %d\n",s->get_offset());
            if(!FOLDING || lE == TOP){
                visit(p->m_expr);
                mpr("    pop %%eax\n");
                emit_non_folded_mov(s->get_offset());
            } else{
                tprint("// Assign FOLDED!\n");
                emit_folded_mov(lE.value,s->get_offset());
            }
            tprint("// Done with visit assign\n");
        }
        void visitArrayAssignment(ArrayAssignment * p)
        {
            tdump();

            const char* arr_name = p->m_symname->spelling();
            Symbol* arr_s = m_st->lookup(p->m_attribute.m_scope,arr_name);
            assert(arr_s!=NULL);
            Expr* arr_index_expr = p->m_expr_1;
            LatticeElem& arr_index_lE = arr_index_expr->m_attribute.m_lattice_elem;
            bool arr_index_const = arr_index_lE != TOP;

            Expr* val_expr = p->m_expr_2;
            LatticeElem& val_lE = val_expr->m_attribute.m_lattice_elem;
            bool val_const = val_lE != TOP;

            if(!FOLDING || !arr_index_const){
                visit(arr_index_expr);
            }
            if(!FOLDING || !val_const){
                visit(val_expr);
            }
            tprint("// Visiting array assign to %s\n", arr_name);
            emit_generic_array_assign_epilogue(arr_name,arr_index_const,
                arr_index_lE, arr_s, arr_index_expr, val_const,
                val_lE);
            tprint("// Done with visit array assign\n");
        }
        void visitCall(Call * p)
        {
            const char* var_name = p->m_symname_1->spelling();
            Symbol* var_s = m_st->lookup(p->m_attribute.m_scope,var_name);
            assert(var_s!=NULL);

            const char* f_name = p->m_symname_2->spelling();
            Symbol* f = m_st->lookup(p->m_attribute.m_scope,f_name);

            int argsBytes = emit_call("visitCall",f,f_name,p->m_expr_list);

            tprint("// Call assign: save eax to loc with offset %d\n",var_s->get_offset());
            emit_non_folded_mov(var_s->get_offset());
            tprint("// Now we have to clean up stack from our arguments. We pushed %d bytes on\n",argsBytes);
            mpr("    add $%d, %%esp\n",argsBytes);
        }
        void visitArrayCall(ArrayCall *p)
        {
            const char* arr_name = p->m_symname_1->spelling();
            Symbol* arr_s = m_st->lookup(p->m_attribute.m_scope,arr_name);
            assert(arr_s!=NULL);
            Expr* arr_index_expr = p->m_expr_1;
            LatticeElem& arr_index_lE = arr_index_expr->m_attribute.m_lattice_elem;
            bool arr_index_const = arr_index_lE != TOP;

            const char* f_name = p->m_symname_2->spelling();
            Symbol* f = m_st->lookup(p->m_attribute.m_scope,f_name);

            if(!FOLDING || !arr_index_const){
                tprint("// visitArrayCall, evaling index expr\n");
                visit(arr_index_expr);
            }

            int argsBytes = emit_call("visitArrayCall",f,f_name,p->m_expr_list_2);
            tprint("// visitArrayCall: now we need to save eax to the array\n");
            tprint("// Now we have to clean up stack from our arguments.\n");
            tprint("// We do this before the assign so we can find ebx\n");
            tprint("// We pushed %d bytes on\n",argsBytes);
            mpr("    add $%d, %%esp\n",argsBytes);
            emit_generic_array_assign_epilogue(arr_name,arr_index_const,
                arr_index_lE, arr_s, arr_index_expr, false,
                arr_index_lE,true);
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
                tprint("// While\n");
                mpr("While%d:\n",label);
                visit(p->m_expr);
                mpr("    pop %%eax\n");
                mpr("    cmp $1, %%eax\n");
                tprint("// While:: Compare with 1, but jump if not equal\n");
                mpr("    jne WhileDone%d\n",label);
                visit(p->m_nested_block);
                mpr("    jmp While%d\n",label);
                mpr("WhileDone%d:\n",label);
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
            emit_binary_expr(p->m_expr_1,p->m_expr_2,p->m_attribute.m_lattice_elem,
                '=',"Compare");
        }
        void visitNoteq(Noteq * p)
        {
            emit_binary_expr(p->m_expr_1,p->m_expr_2,p->m_attribute.m_lattice_elem,
                '!',"NotEq");
        }
        void visitGt(Gt * p)
        {
            emit_binary_expr(p->m_expr_1,p->m_expr_2,p->m_attribute.m_lattice_elem,
                '>',"GreaterThan");
        }
        void visitGteq(Gteq * p)
        {
            emit_binary_expr(p->m_expr_1,p->m_expr_2,p->m_attribute.m_lattice_elem,
                ')',"GreaterThanEq");
        }
        void visitLt(Lt * p)
        {
            emit_binary_expr(p->m_expr_1,p->m_expr_2,p->m_attribute.m_lattice_elem,
                '<',"LessThan");
        }
        void visitLteq(Lteq * p)
        {
            emit_binary_expr(p->m_expr_1,p->m_expr_2,p->m_attribute.m_lattice_elem,
                '(',"LessThanEqual");
        }

        // arithmetic and logic operations
        void visitAnd(And * p)
        {
            emit_binary_expr(p->m_expr_1,p->m_expr_2,p->m_attribute.m_lattice_elem,
                '&',"And");
        }
        void visitOr(Or * p)
        {
            emit_binary_expr(p->m_expr_1,p->m_expr_2,p->m_attribute.m_lattice_elem,
                '|',"Or");
        }
        void visitMinus(Minus * p)
        {
            emit_binary_expr(p->m_expr_1,p->m_expr_2,p->m_attribute.m_lattice_elem,
                '-',"Minus");
        }
        void visitPlus(Plus * p)
        {
            emit_binary_expr(p->m_expr_1,p->m_expr_2,p->m_attribute.m_lattice_elem,
                '+',"Plus");
        }
        void visitTimes(Times * p)
        {
            emit_binary_expr(p->m_expr_1,p->m_expr_2,p->m_attribute.m_lattice_elem,
                '*',"Times");
        }
        void visitDiv(Div * p)
        {
            emit_binary_expr(p->m_expr_1,p->m_expr_2,p->m_attribute.m_lattice_elem,
                '/',"Div");
        }
        void visitNot(Not * p)
        {
            if(!FOLDING || p->m_attribute.m_lattice_elem == TOP){
                visit(p->m_expr);
                // Sourced from http://www.pagetable.com/?p=13
                tprint("// Not\n");
                mpr("    pop %%eax\n");
                // get the negative version of what is in eax
                // This also sets carry flag to 0 if eax is 0, 1 otherwise
                mpr("    neg %%eax\n");
                // Perform (eax) - (eax+carryFlag). This gives us the value (- carry flag) inside eax.
                mpr("    sbb %%eax, %%eax\n");
                // Since carry flag is 0 if eax is 0, and 1 otherwise, we now have 0 -> 0. and 1 -> -1. So lets increment
                mpr("    inc %%eax\n");
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
            const char* arr_name = p->m_symname->spelling();
            Symbol* arr_s = m_st->lookup(p->m_attribute.m_scope,arr_name);
            assert(arr_s!=NULL);
            Expr* arr_index_expr = p->m_expr;
            LatticeElem& arr_index_lE = arr_index_expr->m_attribute.m_lattice_elem;

            tprint("// ArrayAccess %s\n", arr_name);
            if( !FOLDING || arr_index_lE == TOP){
                visit(arr_index_expr);
                mpr("    pop %%ebx\n");
                mpr("    mov -%d(%%ebp,%%ebx,%d), %%eax\n",
                    get_non_folded_array_offset(arr_name,arr_s),wordsize);
                mpr("    push %%eax\n");
            } else{
                mpr("    mov -%d(%%ebp), %%eax\n",
                    get_folded_array_offset(arr_name,arr_s,arr_index_lE.value));
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

