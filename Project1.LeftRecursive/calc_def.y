
%{
    #include <stdio.h>
    int yylex(void);
    void yyerror(char *);
%}


%%

List    : RelExpr ListP
        ;

ListP   : ';' ListPP
        ;

ListPP   :
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

ExprASP : '+' ExprAS
        | '-' ExprAS
        | //Empty
        ;

ExprMD  : Vals ExprMDP
        ;

ExprMDP : '*' ExprMD
        | '/' ExprMD
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
