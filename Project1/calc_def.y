
%{
    #include <stdio.h>
    int yylex(void);
    void yyerror(char *);
%}


%%

List    : RelExpr ListP
        ;

ListP   : ';' {printf("parsed expresion\n");} ListPP
        ;

ListPP  :
        | List
        ;

RelExpr : ExprAS RelExpr2
        ;

RelExpr2: '<' ExprAS
        | '>' ExprAS
        | '=' ExprAS
        | // Empty
        ;

ExprAS  : ExprMD ExprASP
        ;

ExprASP : '+' ExprMD ExprASP
        | '-' ExprMD ExprASP
        | //Empty
        ;

ExprMD  : Vals ExprMDP
        ;

ExprMDP : '*' Vals ExprMDP
        | '/' Vals ExprMDP
        | //Empty
        ;

Vals    : 'n'
        | '(' ExprAS ')'
        ;

%%

void yyerror(char *s) {
    fprintf(stderr, "%s\n", s);
    return;
}

int main(void) {
    yyparse();
    return 0;
}
