
YACC    = bison -d -v
LEX     = flex
CC      = gcc
CPP     = g++ -g -Wno-deprecated
ASTBUILD = ./astbuilder.gawk

TARGET	= simple

OBJS += lexer.o y.tab.o main.o primitive.o ast2dot.o symtab.o typecheck.o constantfolding.o codegen.o
RMFILES = core.* lexer.cpp y.tab.c y.tab.h y.output ast.h ast.cpp simple.s simple.o start start.o $(TARGET) $(OBJS)

# dependencies
$(TARGET): y.tab.c lexer.cpp y.tab.h $(OBJS)
	$(CPP) -o $(TARGET) $(OBJS) $(CPPFLAGS)

# rules
y.tab.c: parser.ypp
	$(YACC) -o y.tab.c $<

%.o: %.cpp
	$(CPP) -o $@ -c $< $(CPPFLAGS)

%.o: %.c
	$(CPP) -o $@ -c $< $(CPPFLAGS)

%.cpp: %.lpp
	$(LEX) -o$(@:%.o=%.d)  $<

ast.cpp: ast.cdef 
	$(ASTBUILD) -v outtype=cpp -v outfile=ast.cpp < ast.cdef 

ast.h: ast.cdef 
	$(ASTBUILD) -v outtype=h -v outfile=ast.h < ast.cdef

# source
lexer.o: lexer.cpp y.tab.h ast.h
lexer.cpp: lexer.lpp

y.tab.o: y.tab.c y.tab.h
y.tab.c: parser.ypp ast.h primitive.h symtab.h

main.o: y.tab.h ast.h ast.cpp symtab.h primitive.h constantfolding.cpp typecheck.cpp codegen.cpp
ast2dot.o: y.tab.h ast.h symtab.h primitive.h attribute.h

ast.cpp: ast.cdef
ast.h: ast.cdef

primitive.o: primitive.h primitive.cpp ast.h

typecheck.o: typecheck.cpp ast.h symtab.h primitive.h attribute.h

constantfolding.o: constantfolding.cpp ast.h symtab.h primitive.h attribute.h

codegen.o: codegen.cpp ast.h symtab.h primitive.h

clean:
	rm -f $(RMFILES)


