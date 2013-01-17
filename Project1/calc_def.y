
%{
    #include <stdio.h>
    int yylex(void);
    void yyerror(char *);
%}


%%

List    : RelExpr List1
        ;

List1   : ';' List2
        ;

List2   :
        | List
        ;

RelExpr : ExprAS RelExpr2
        ;

RelExpr2: '<' ExprAS
        | '>' ExprAS
        | '=' ExprAS
        | // Empty
        ;

ExprAS  : ExprMD ExprAS2
        ;

ExprAS2 : '+' ExprAS
        | '-' ExprAS
        | //Empty
        ;

ExprMD  : Vals ExprMD2
        ;

ExprMD2 : '*' ExprMD
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
