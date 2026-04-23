#include "ast.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static ASTNode* ast_alloc(ASTNodeType type) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (node == NULL) {
        fprintf(stderr, "Error: sin memoria para AST\n");
        exit(1);
    }
    node->type = type;
    return node;
}

static void* grow_raw(void* old_ptr, size_t elem_size, int old_cap, int* out_cap) {
    int new_cap = old_cap == 0 ? 4 : old_cap * 2;
    void* ptr = realloc(old_ptr, elem_size * (size_t)new_cap);
    if (ptr == NULL) {
        fprintf(stderr, "Error: sin memoria\n");
        exit(1);
    }
    *out_cap = new_cap;
    return ptr;
}

static char* xstrdup_local(const char* s) {
    size_t n = strlen(s) + 1;
    char* out = (char*)malloc(n);
    if (out == NULL) {
        fprintf(stderr, "Error: sin memoria\n");
        exit(1);
    }
    memcpy(out, s, n);
    return out;
}

ASTNode* ast_make_block(void) {
    ASTNode* node = ast_alloc(AST_BLOCK);
    node->as.block.items = NULL;
    node->as.block.count = 0;
    node->as.block.capacity = 0;
    return node;
}

void ast_block_add(ASTNode* block, ASTNode* statement) {
    if (block == NULL || block->type != AST_BLOCK || statement == NULL) {
        return;
    }

    if (block->as.block.count >= block->as.block.capacity) {
        block->as.block.items = (ASTNode**)grow_raw(
            block->as.block.items,
            sizeof(ASTNode*),
            block->as.block.capacity,
            &block->as.block.capacity);
    }

    block->as.block.items[block->as.block.count++] = statement;
}

ASTNode* ast_make_scope(ASTNode* body) {
    ASTNode* node = ast_alloc(AST_SCOPE);
    node->as.scope_stmt.body = body;
    return node;
}

ASTNode* ast_make_var_decl(ValueType declared_type, char* name, ASTNode* expr) {
    ASTNode* node = ast_alloc(AST_VAR_DECL);
    node->as.var_decl.declared_type = declared_type;
    node->as.var_decl.name = name;
    node->as.var_decl.expr = expr;
    return node;
}

ASTNode* ast_make_assign(char* name, ASTNode* expr) {
    ASTNode* node = ast_alloc(AST_ASSIGN);
    node->as.assign.name = name;
    node->as.assign.expr = expr;
    return node;
}

ASTNode* ast_make_print(ASTNode* expr) {
    ASTNode* node = ast_alloc(AST_PRINT);
    node->as.print_stmt.expr = expr;
    return node;
}

ASTNode* ast_make_if(ASTNode* condition, ASTNode* then_branch, ASTNode* else_branch) {
    ASTNode* node = ast_alloc(AST_IF);
    node->as.if_stmt.condition = condition;
    node->as.if_stmt.then_branch = then_branch;
    node->as.if_stmt.else_branch = else_branch;
    return node;
}

ASTNode* ast_make_input(ValueType input_type) {
    ASTNode* node = ast_alloc(AST_INPUT);
    node->as.input.input_type = input_type;
    return node;
}

ASTNode* ast_make_func_decl(ValueType return_type, char* name, ASTParamList* params, ASTNode* body) {
    ASTNode* node = ast_alloc(AST_FUNC_DECL);
    int i;

    node->as.func_decl.return_type = return_type;
    node->as.func_decl.name = name;
    node->as.func_decl.body = body;

    if (params != NULL && params->count > 0) {
        node->as.func_decl.param_count = params->count;
        node->as.func_decl.param_names = (char**)malloc(sizeof(char*) * (size_t)params->count);
        node->as.func_decl.param_types = (ValueType*)malloc(sizeof(ValueType) * (size_t)params->count);
        if (node->as.func_decl.param_names == NULL || node->as.func_decl.param_types == NULL) {
            fprintf(stderr, "Error: sin memoria\n");
            exit(1);
        }
        for (i = 0; i < params->count; i++) {
            node->as.func_decl.param_names[i] = xstrdup_local(params->names[i]);
            node->as.func_decl.param_types[i] = params->types[i];
        }
    } else {
        node->as.func_decl.param_count = 0;
        node->as.func_decl.param_names = NULL;
        node->as.func_decl.param_types = NULL;
    }

    if (params != NULL) {
        ast_param_list_free_shallow(params);
    }

    return node;
}

ASTNode* ast_make_return(ASTNode* expr) {
    ASTNode* node = ast_alloc(AST_RETURN);
    node->as.return_stmt.expr = expr;
    return node;
}

ASTNode* ast_make_int(int value) {
    ASTNode* node = ast_alloc(AST_INT);
    node->as.int_lit.value = value;
    return node;
}

ASTNode* ast_make_string(char* value) {
    ASTNode* node = ast_alloc(AST_STRING);
    node->as.string_lit.value = value;
    return node;
}

ASTNode* ast_make_var(char* name) {
    ASTNode* node = ast_alloc(AST_VAR);
    node->as.var_ref.name = name;
    return node;
}

ASTNode* ast_make_binary(ASTOperator op, ASTNode* left, ASTNode* right) {
    ASTNode* node = ast_alloc(AST_BINOP);
    node->as.binary.op = op;
    node->as.binary.left = left;
    node->as.binary.right = right;
    return node;
}

ASTNode* ast_make_func_call(char* name, ASTNodeArray* args) {
    ASTNode* node = ast_alloc(AST_FUNC_CALL);
    node->as.func_call.name = name;
    if (args != NULL && args->count > 0) {
        node->as.func_call.arg_count = args->count;
        node->as.func_call.args = args->items;
    } else {
        node->as.func_call.arg_count = 0;
        node->as.func_call.args = NULL;
    }
    if (args != NULL) {
        free(args);
    }
    return node;
}

ASTNodeArray* ast_make_node_array(void) {
    ASTNodeArray* array = (ASTNodeArray*)malloc(sizeof(ASTNodeArray));
    if (array == NULL) {
        fprintf(stderr, "Error: sin memoria\n");
        exit(1);
    }
    array->items = NULL;
    array->count = 0;
    array->capacity = 0;
    return array;
}

void ast_node_array_add(ASTNodeArray* array, ASTNode* node) {
    if (array == NULL || node == NULL) {
        return;
    }
    if (array->count >= array->capacity) {
        array->items = (ASTNode**)grow_raw(
            array->items,
            sizeof(ASTNode*),
            array->capacity,
            &array->capacity);
    }
    array->items[array->count++] = node;
}

void ast_node_array_free_shallow(ASTNodeArray* array) {
    if (array == NULL) {
        return;
    }
    free(array->items);
    free(array);
}

ASTParamList* ast_make_param_list(void) {
    ASTParamList* params = (ASTParamList*)malloc(sizeof(ASTParamList));
    if (params == NULL) {
        fprintf(stderr, "Error: sin memoria\n");
        exit(1);
    }
    params->names = NULL;
    params->types = NULL;
    params->count = 0;
    params->capacity = 0;
    return params;
}

void ast_param_list_add(ASTParamList* params, ValueType type, char* name) {
    if (params == NULL || name == NULL) {
        return;
    }

    if (params->count >= params->capacity) {
        params->names = (char**)grow_raw(
            params->names,
            sizeof(char*),
            params->capacity,
            &params->capacity);
        params->types = (ValueType*)realloc(params->types, sizeof(ValueType) * (size_t)params->capacity);
        if (params->types == NULL) {
            fprintf(stderr, "Error: sin memoria\n");
            exit(1);
        }
    }

    params->names[params->count] = name;
    params->types[params->count] = type;
    params->count++;
}

void ast_param_list_free_shallow(ASTParamList* params) {
    if (params == NULL) {
        return;
    }
    free(params->names);
    free(params->types);
    free(params);
}

void ast_free(ASTNode* node) {
    int i;

    if (node == NULL) {
        return;
    }

    switch (node->type) {
        case AST_BLOCK:
            for (i = 0; i < node->as.block.count; i++) {
                ast_free(node->as.block.items[i]);
            }
            free(node->as.block.items);
            break;
        case AST_SCOPE:
            ast_free(node->as.scope_stmt.body);
            break;
        case AST_VAR_DECL:
            free(node->as.var_decl.name);
            ast_free(node->as.var_decl.expr);
            break;
        case AST_ASSIGN:
            free(node->as.assign.name);
            ast_free(node->as.assign.expr);
            break;
        case AST_PRINT:
            ast_free(node->as.print_stmt.expr);
            break;
        case AST_IF:
            ast_free(node->as.if_stmt.condition);
            ast_free(node->as.if_stmt.then_branch);
            ast_free(node->as.if_stmt.else_branch);
            break;
        case AST_INPUT:
            break;
        case AST_FUNC_DECL:
            free(node->as.func_decl.name);
            for (i = 0; i < node->as.func_decl.param_count; i++) {
                free(node->as.func_decl.param_names[i]);
            }
            free(node->as.func_decl.param_names);
            free(node->as.func_decl.param_types);
            ast_free(node->as.func_decl.body);
            break;
        case AST_RETURN:
            ast_free(node->as.return_stmt.expr);
            break;
        case AST_STRING:
            free(node->as.string_lit.value);
            break;
        case AST_VAR:
            free(node->as.var_ref.name);
            break;
        case AST_BINOP:
            ast_free(node->as.binary.left);
            ast_free(node->as.binary.right);
            break;
        case AST_FUNC_CALL:
            free(node->as.func_call.name);
            for (i = 0; i < node->as.func_call.arg_count; i++) {
                ast_free(node->as.func_call.args[i]);
            }
            free(node->as.func_call.args);
            break;
        case AST_INT:
            break;
    }

    free(node);
}
