
YACC    = bison -d -v
LEX     = flex
CC      = gcc
CPP     = g++ -g -Wno-deprecated
ASTBUILD = ./astbuilder.gawk

TARGET	= simple

OBJS += lexer.o parser.o main.o ast.o primitive.o ast2dot.o symtab.o typecheck.o constantfolding.o codegen.o
RMFILES = core.* lexer.cpp parser.cpp parser.hpp parser.output ast.hpp ast.cpp simple.s simple.o start start.o $(TARGET) $(OBJS)

# dependencies
$(TARGET): parser.cpp lexer.cpp parser.hpp $(OBJS)
	$(CPP) -o $(TARGET) $(OBJS) $(CPPFLAGS)

# rules
%.cpp: %.ypp
	$(YACC) -o $(@:%.o=%.d) $<

%.o: %.cpp
	$(CPP) -o $@ -c $< $(CPPFLAGS)

%.cpp: %.l
	$(LEX) -o$(@:%.o=%.d)  $<

ast.cpp: ast.cdef 
	$(ASTBUILD) -v outtype=cpp -v outfile=ast.cpp < ast.cdef 

ast.hpp: ast.cdef 
	$(ASTBUILD) -v outtype=hpp -v outfile=ast.hpp < ast.cdef

# source
lexer.o: lexer.cpp parser.hpp ast.hpp
lexer.cpp: lexer.l

parser.o: parser.cpp parser.hpp
parser.cpp: parser.ypp ast.hpp primitive.hpp symtab.hpp

main.o: parser.hpp ast.hpp symtab.hpp primitive.hpp constantfolding.cpp typecheck.cpp codegen.cpp
ast2dot.o: parser.hpp ast.hpp symtab.hpp primitive.hpp attribute.hpp

ast.o: ast.cpp ast.hpp primitive.hpp symtab.hpp attribute.hpp
ast.cpp: ast.cdef
ast.hpp: ast.cdef

primitive.o: primitive.hpp primitive.cpp ast.hpp

typecheck.o: typecheck.cpp ast.hpp symtab.hpp primitive.hpp attribute.hpp

constantfolding.o: constantfolding.cpp ast.hpp symtab.hpp primitive.hpp attribute.hpp

codegen.o: codegen.cpp ast.hpp symtab.hpp primitive.hpp

turnin: clean
	mkdir codegen
	cp ast2dot.cpp astbuilder.gawk ast.cdef attribute.hpp constantfolding.cpp lexer.l main.cpp Makefile parser.ypp primitive.cpp primitive.hpp README symtab.cpp symtab.hpp typecheck.cpp codegen.cpp codegen

clean:
	rm -f $(RMFILES)
	rm -rf codegen
