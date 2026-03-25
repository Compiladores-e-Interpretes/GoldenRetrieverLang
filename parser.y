%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser_helper.h"

int yylex(void);
void yyerror(const char* s);
%}

%union {
    int ival;
    char* sval;
    Value value;
}

%token HUESO CORREA LADRA
%token <sval> ID STRING
%token <ival> NUMBER

%type <value> expr term factor

%left '+' '-'
%left '*' '/'

%%
program:
    statements
;

statements:
    /* empty */
    | statements statement
;

statement:
    HUESO ID '=' expr ';'
    {
        declare_variable_checked($2, TYPE_INT, $4);
        free($2);
    }
    | CORREA ID '=' expr ';'
    {
        declare_variable_checked($2, TYPE_STRING, $4);
        free($2);
    }
    | ID '=' expr ';'
    {
        assign_variable_checked($1, $3);
        free($1);
    }
    | LADRA '(' expr ')' ';'
    {
        print_value_checked($3);
    }
    | '{'
    {
        enter_scope_checked();
    }
    statements '}'
    {
        exit_scope_checked();
    }
;

expr:
    expr '+' term
    {
        $$ = add_values_checked($1, $3);
    }
    | expr '-' term
    {
        $$ = sub_values_checked($1, $3);
    }
    | term
    {
        $$ = $1;
    }
;

term:
    term '*' factor
    {
        $$ = mul_values_checked($1, $3);
    }
    | term '/' factor
    {
        $$ = div_values_checked($1, $3);
    }
    | factor
    {
        $$ = $1;
    }
;

factor:
    NUMBER
    {
        $$ = make_int($1);
    }
    | STRING
    {
        $$ = make_string($1);
    }
    | ID
    {
        $$ = variable_value_checked($1);
        free($1);
    }
    | '(' expr ')'
    {
        $$ = $2;
    }
;
%%

void yyerror(const char* s) {
    (void)s;
    helper_fail("Error de sintaxis");
}

int main(void) {
    if (!init_symbol_stack()) {
        fprintf(stderr, "Error: no se pudo crear el ambito global\n");
        return 1;
    }
    int result = yyparse();
    cleanup_symbols();
    return result;
}
