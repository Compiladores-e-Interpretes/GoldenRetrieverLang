#ifndef AST_H
#define AST_H

#include "symbols.h"

typedef enum {
    AST_BLOCK = 0,
    AST_SCOPE,
    AST_VAR_DECL,
    AST_ASSIGN,
    AST_PRINT,
    AST_IF,
    AST_INPUT,
    AST_FUNC_DECL,
    AST_RETURN,
    AST_INT,
    AST_STRING,
    AST_VAR,
    AST_BINOP,
    AST_FUNC_CALL
} ASTNodeType;

typedef enum {
    OP_ADD = 0,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_EQ,
    OP_NE,
    OP_LT,
    OP_LE,
    OP_GT,
    OP_GE
} ASTOperator;

typedef struct ASTNode ASTNode;

typedef struct {
    ASTNode** items;
    int count;
    int capacity;
} ASTNodeArray;

typedef struct {
    char** names;
    ValueType* types;
    int count;
    int capacity;
} ASTParamList;

struct ASTNode {
    ASTNodeType type;
    union {
        ASTNodeArray block;
        struct {
            ASTNode* body;
        } scope_stmt;
        struct {
            ValueType declared_type;
            char* name;
            ASTNode* expr;
        } var_decl;
        struct {
            char* name;
            ASTNode* expr;
        } assign;
        struct {
            ASTNode* expr;
        } print_stmt;
        struct {
            ASTNode* condition;
            ASTNode* then_branch;
            ASTNode* else_branch;
        } if_stmt;
        struct {
            ValueType input_type;
        } input;
        struct {
            ValueType return_type;
            char* name;
            char** param_names;
            ValueType* param_types;
            int param_count;
            ASTNode* body;
        } func_decl;
        struct {
            ASTNode* expr;
        } return_stmt;
        struct {
            int value;
        } int_lit;
        struct {
            char* value;
        } string_lit;
        struct {
            char* name;
        } var_ref;
        struct {
            ASTOperator op;
            ASTNode* left;
            ASTNode* right;
        } binary;
        struct {
            char* name;
            ASTNode** args;
            int arg_count;
        } func_call;
    } as;
};

ASTNode* ast_make_block(void);
void ast_block_add(ASTNode* block, ASTNode* statement);
ASTNode* ast_make_scope(ASTNode* body);
ASTNode* ast_make_var_decl(ValueType declared_type, char* name, ASTNode* expr);
ASTNode* ast_make_assign(char* name, ASTNode* expr);
ASTNode* ast_make_print(ASTNode* expr);
ASTNode* ast_make_if(ASTNode* condition, ASTNode* then_branch, ASTNode* else_branch);
ASTNode* ast_make_input(ValueType input_type);
ASTNode* ast_make_func_decl(ValueType return_type, char* name, ASTParamList* params, ASTNode* body);
ASTNode* ast_make_return(ASTNode* expr);
ASTNode* ast_make_int(int value);
ASTNode* ast_make_string(char* value);
ASTNode* ast_make_var(char* name);
ASTNode* ast_make_binary(ASTOperator op, ASTNode* left, ASTNode* right);
ASTNode* ast_make_func_call(char* name, ASTNodeArray* args);

ASTNodeArray* ast_make_node_array(void);
void ast_node_array_add(ASTNodeArray* array, ASTNode* node);
void ast_node_array_free_shallow(ASTNodeArray* array);

ASTParamList* ast_make_param_list(void);
void ast_param_list_add(ASTParamList* params, ValueType type, char* name);
void ast_param_list_free_shallow(ASTParamList* params);

void ast_free(ASTNode* node);

#endif
