#include "ast.hpp"
#include "symtab.hpp"
#include "primitive.hpp"
#include "assert.h"
#include <typeinfo>
#include <stdio.h>

#define TESTING 0

#define tprint(...) if(TESTING) printf(__VA_ARGS__)

#define mpr(...) fprintf(m_outputfile,__VA_ARGS__)


class Codegen : public Visitor
{
    private:

        FILE * m_outputfile;
        SymTab *m_st;

        // basic size of a word (integers and booleans) in bytes
        static const int wordsize = 4;

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
        }

        void emit_epilogue()
        {
        }

        // HERE: more functions to emit code

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
        }
        void visitFunc(Func * p)
        {
        }
        void visitFunction_block(Function_block * p)
        {
        }
        void visitNested_block(Nested_block * p)
        {
        }
        void visitAssignment(Assignment * p)
        {
        }
        void visitArrayAssignment(ArrayAssignment * p)
        {
        }
        void visitCall(Call * p)
        {
        }
        void visitArrayCall(ArrayCall *p)
        {
        }
        void visitReturn(Return * p)
        {
        }

        // control flow
        void visitIfNoElse(IfNoElse * p)
        {
        }
        void visitIfWithElse(IfWithElse * p)
        {
        }
        void visitWhileLoop(WhileLoop * p)
        {
        }

        // variable declarations (no code generation needed)
        void visitDecl(Decl * p)
        {
        }
        void visitParam(Param *p)
        {
        }
        void visitTInt(TInt * p)
        {
        }
        void visitTBool(TBool * p)
        {
        }
        void visitTIntArray(TIntArray * p)
        {
        }

        // comparison operations
        void visitCompare(Compare * p)
        {
        }
        void visitNoteq(Noteq * p)
        {
        }
        void visitGt(Gt * p)
        {
        }
        void visitGteq(Gteq * p)
        {
        }
        void visitLt(Lt * p)
        {
        }
        void visitLteq(Lteq * p)
        {
        }

        // arithmetic and logic operations
        void visitAnd(And * p)
        {
            visit(p->m_expr_1);
            visit(p->m_expr_2);
            mpr("pop %%ebx\n");
            mpr("pop %%eax\n");
            mpr("and %%ebx %%eax\n");
            mpr("push %%eax\n");
        }
        void visitOr(Or * p)
        {
            visit(p->m_expr_1);
            visit(p->m_expr_2);
            mpr("pop %%ebx\n");
            mpr("pop %%eax\n");
            mpr("or %%ebx %%eax\n");
            mpr("push %%eax\n");
        }
        void visitMinus(Minus * p)
        {
            visit(p->m_expr_1);
            visit(p->m_expr_2);
            mpr("pop %%ebx\n"); // expr2
            mpr("pop %%eax\n"); // expr1
            // sub src dest -> dest = dest - src
            mpr("sub %%ebx %%eax\n");
            mpr("push %%eax\n");
        }
        void visitPlus(Plus * p)
        {
            visit(p->m_expr_1);
            visit(p->m_expr_2);
            mpr("pop %%ebx\n");
            mpr("pop %%eax\n");
            mpr("add %%ebx %%eax\n");
            mpr("push %%eax\n");
        }
        void visitTimes(Times * p)
        {
            visit(p->m_expr_1);
            visit(p->m_expr_2);
            mpr("pop %%ebx\n");
            mpr("pop %%eax\n");
            mpr("mul %%ebx\n");
            mpr("push %%eax\n");
        }
        void visitDiv(Div * p)
        {
            visit(p->m_expr_1);
            visit(p->m_expr_2);
            mpr("pop %%ebx\n");
            mpr("pop %%eax\n");
            mpr("cdq\n");
            mpr("div %%ebx\n");
            mpr("push %%eax\n");
        }
        void visitNot(Not * p)
        {
            visit(p->m_expr);
            mpr("pop %%eax\n");
            mpr("not %%eax\n");
            mpr("push %%eax\n");
        }
        void visitUminus(Uminus * p)
        {
            visit(p->m_expr);
            mpr("pop %%eax\n");
            mpr("neg %%eax\n");
            mpr("push %%eax\n");
        }
        void visitMagnitude(Magnitude * p)
        {
            // From: http://stackoverflow.com/questions/2639173/x86-assembly-abs-implementation
            visit(p->m_expr);
            mpr("pop %%eax\n");
            mpr("mov %%eax %%ebx\n");
            mpr("neg %%eax\n");
            mpr("cmovl %%ebx %%eax\n");
            mpr("push %%eax\n");
        }

        // variable and constant access
        void visitIdent(Ident * p)
        {
        }
        void visitIntLit(IntLit * p)
        {
            mpr("push %d\n",p->m_primitive->m_data);
        }
        void visitBoolLit(BoolLit * p)
        {
        }
        void visitArrayAccess(ArrayAccess * p)
        {
        }

        // special cases
        void visitSymName(SymName * p)
        {
        }
        void visitPrimitive(Primitive * p)
        {
        }
};

