%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "interpreter.h"
#include "parser_helper.h"

int yylex(void);
void yyerror(const char* s);
extern FILE* yyin;

static ASTNode* g_root = NULL;
%}

%union {
    int ival;
    char* sval;
    ValueType vtype;
    void* ptr;
}

%token HUESO CORREA LADRA RETORNA
%token IF ELSE ELIF
%token OLFATEA_HUESO OLFATEA_CORREA
%token EQ NE LT LE GT GE
%token <sval> ID STRING
%token <ival> NUMBER

%type <vtype> var_type
%type <ptr> program statements statement block expr comp sum term factor
%type <ptr> if_stmt elif_chain
%type <ptr> call_args_opt call_args
%type <ptr> params_opt params param_decl

%left EQ NE LT LE GT GE
%left '+' '-'
%left '*' '/'

%%
program:
    statements
    {
        g_root = (ASTNode*)$1;
        $$ = $1;
    }
;

statements:
    /* empty */
    {
        $$ = ast_make_block();
    }
    | statements statement
    {
        ast_block_add((ASTNode*)$1, (ASTNode*)$2);
        $$ = $1;
    }
;

statement:
    var_type ID '=' expr ';'
    {
        $$ = ast_make_var_decl($1, $2, (ASTNode*)$4);
    }
    | ID '=' expr ';'
    {
        $$ = ast_make_assign($1, (ASTNode*)$3);
    }
    | LADRA '(' expr ')' ';'
    {
        $$ = ast_make_print((ASTNode*)$3);
    }
    | block
    {
        $$ = ast_make_scope((ASTNode*)$1);
    }
    | var_type ID '(' params_opt ')' block
    {
        $$ = ast_make_func_decl($1, $2, (ASTParamList*)$4, (ASTNode*)$6);
    }
    | RETORNA expr ';'
    {
        $$ = ast_make_return((ASTNode*)$2);
    }
    | if_stmt
    {
        $$ = $1;
    }
;

if_stmt:
    IF '(' expr ')' block elif_chain
    {
        $$ = ast_make_if((ASTNode*)$3, (ASTNode*)$5, (ASTNode*)$6);
    }
;

elif_chain:
    /* empty */
    {
        $$ = NULL;
    }
    | ELIF '(' expr ')' block elif_chain
    {
        $$ = ast_make_if((ASTNode*)$3, (ASTNode*)$5, (ASTNode*)$6);
    }
    | ELSE block
    {
        $$ = $2;
    }
    | ELSE IF '(' expr ')' block elif_chain
    {
        $$ = ast_make_if((ASTNode*)$4, (ASTNode*)$6, (ASTNode*)$7);
    }
;

block:
    '{' statements '}'
    {
        $$ = $2;
    }
;

params_opt:
    /* empty */
    {
        $$ = ast_make_param_list();
    }
    | params
    {
        $$ = $1;
    }
;

params:
    param_decl
    {
        $$ = $1;
    }
    | params ',' param_decl
    {
        int i;
        for (i = 0; i < ((ASTParamList*)$3)->count; i++) {
            ast_param_list_add((ASTParamList*)$1, ((ASTParamList*)$3)->types[i], ((ASTParamList*)$3)->names[i]);
        }
        ast_param_list_free_shallow((ASTParamList*)$3);
        $$ = $1;
    }
;

param_decl:
    var_type ID
    {
        ASTParamList* list = ast_make_param_list();
        ast_param_list_add(list, $1, $2);
        $$ = list;
    }
;

expr:
    comp
    {
        $$ = $1;
    }
;

comp:
    comp EQ sum
    {
        $$ = ast_make_binary(OP_EQ, (ASTNode*)$1, (ASTNode*)$3);
    }
    | comp NE sum
    {
        $$ = ast_make_binary(OP_NE, (ASTNode*)$1, (ASTNode*)$3);
    }
    | comp LT sum
    {
        $$ = ast_make_binary(OP_LT, (ASTNode*)$1, (ASTNode*)$3);
    }
    | comp LE sum
    {
        $$ = ast_make_binary(OP_LE, (ASTNode*)$1, (ASTNode*)$3);
    }
    | comp GT sum
    {
        $$ = ast_make_binary(OP_GT, (ASTNode*)$1, (ASTNode*)$3);
    }
    | comp GE sum
    {
        $$ = ast_make_binary(OP_GE, (ASTNode*)$1, (ASTNode*)$3);
    }
    | sum
    {
        $$ = $1;
    }
;

sum:
    sum '+' term
    {
        $$ = ast_make_binary(OP_ADD, (ASTNode*)$1, (ASTNode*)$3);
    }
    | sum '-' term
    {
        $$ = ast_make_binary(OP_SUB, (ASTNode*)$1, (ASTNode*)$3);
    }
    | term
    {
        $$ = $1;
    }
;

term:
    term '*' factor
    {
        $$ = ast_make_binary(OP_MUL, (ASTNode*)$1, (ASTNode*)$3);
    }
    | term '/' factor
    {
        $$ = ast_make_binary(OP_DIV, (ASTNode*)$1, (ASTNode*)$3);
    }
    | factor
    {
        $$ = $1;
    }
;

factor:
    NUMBER
    {
        $$ = ast_make_int($1);
    }
    | STRING
    {
        $$ = ast_make_string($1);
    }
    | OLFATEA_HUESO '(' ')'
    {
        $$ = ast_make_input(TYPE_INT);
    }
    | OLFATEA_CORREA '(' ')'
    {
        $$ = ast_make_input(TYPE_STRING);
    }
    | ID
    {
        $$ = ast_make_var($1);
    }
    | ID '(' call_args_opt ')'
    {
        $$ = ast_make_func_call($1, (ASTNodeArray*)$3);
    }
    | '(' expr ')'
    {
        $$ = $2;
    }
;

call_args_opt:
    /* empty */
    {
        $$ = ast_make_node_array();
    }
    | call_args
    {
        $$ = $1;
    }
;

call_args:
    expr
    {
        ASTNodeArray* list = ast_make_node_array();
        ast_node_array_add(list, (ASTNode*)$1);
        $$ = list;
    }
    | call_args ',' expr
    {
        ast_node_array_add((ASTNodeArray*)$1, (ASTNode*)$3);
        $$ = $1;
    }
;

var_type:
    HUESO
    {
        $$ = TYPE_INT;
    }
    | CORREA
    {
        $$ = TYPE_STRING;
    }
;
%%

void yyerror(const char* s) {
    (void)s;
    helper_fail("Error de sintaxis");
}

int main(int argc, char** argv) {
    int result;
    FILE* input = NULL;

    if (!init_symbol_stack()) {
        fprintf(stderr, "Error: no se pudo crear el ambito global\n");
        return 1;
    }

    if (argc > 1) {
        input = fopen(argv[1], "r");
        if (input == NULL) {
            fprintf(stderr, "Error: no se pudo abrir %s\n", argv[1]);
            cleanup_symbols();
            return 1;
        }
        yyin = input;
    }

    result = yyparse();
    if (result == 0) {
        execute_ast(g_root);
    }

    if (input != NULL) {
        fclose(input);
    }
    ast_free(g_root);
    cleanup_symbols();
    return result;
}
