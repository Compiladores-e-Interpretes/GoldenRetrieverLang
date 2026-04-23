#include "interpreter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "parser_helper.h"
#include "symbols.h"

typedef struct FunctionDef {
    char* name;
    ValueType return_type;
    char** param_names;
    ValueType* param_types;
    int param_count;
    ASTNode* body;
    struct FunctionDef* next;
} FunctionDef;

typedef struct {
    int has_return;
    Value value;
} ExecResult;

static FunctionDef* g_functions = NULL;

static char* xstrdup_local(const char* s) {
    size_t n = strlen(s) + 1;
    char* out = (char*)malloc(n);
    if (out == NULL) {
        helper_fail("Sin memoria");
    }
    memcpy(out, s, n);
    return out;
}

static Value default_value_for_type(ValueType type) {
    if (type == TYPE_STRING) {
        return make_string(xstrdup_local(""));
    }
    return make_int(0);
}

static FunctionDef* find_function(const char* name) {
    FunctionDef* current = g_functions;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

static void register_function(ASTNode* node) {
    FunctionDef* fn;
    int i;

    if (node == NULL || node->type != AST_FUNC_DECL) {
        return;
    }

    if (find_function(node->as.func_decl.name) != NULL) {
        helper_fail("Funcion ya declarada");
    }

    fn = (FunctionDef*)malloc(sizeof(FunctionDef));
    if (fn == NULL) {
        helper_fail("Sin memoria");
    }

    fn->name = xstrdup_local(node->as.func_decl.name);
    fn->return_type = node->as.func_decl.return_type;
    fn->param_count = node->as.func_decl.param_count;
    fn->body = node->as.func_decl.body;
    fn->next = g_functions;

    if (fn->param_count > 0) {
        fn->param_names = (char**)malloc(sizeof(char*) * (size_t)fn->param_count);
        fn->param_types = (ValueType*)malloc(sizeof(ValueType) * (size_t)fn->param_count);
        if (fn->param_names == NULL || fn->param_types == NULL) {
            helper_fail("Sin memoria");
        }
        for (i = 0; i < fn->param_count; i++) {
            fn->param_names[i] = xstrdup_local(node->as.func_decl.param_names[i]);
            fn->param_types[i] = node->as.func_decl.param_types[i];
        }
    } else {
        fn->param_names = NULL;
        fn->param_types = NULL;
    }

    g_functions = fn;
}

static void free_functions(void) {
    FunctionDef* current = g_functions;
    while (current != NULL) {
        FunctionDef* next = current->next;
        int i;
        free(current->name);
        for (i = 0; i < current->param_count; i++) {
            free(current->param_names[i]);
        }
        free(current->param_names);
        free(current->param_types);
        free(current);
        current = next;
    }
    g_functions = NULL;
}

static Value eval_expr(ASTNode* node);
static ExecResult exec_statement(ASTNode* node, int in_function);
static ExecResult exec_block(ASTNode* block, int in_function);
static ExecResult exec_if(ASTNode* node, int in_function);
static Value eval_input(ValueType input_type);
static Value eval_compare(ASTOperator op, Value left, Value right);

static Value eval_call(ASTNode* call_node) {
    FunctionDef* fn;
    int i;

    fn = find_function(call_node->as.func_call.name);
    if (fn == NULL) {
        helper_fail("Funcion no declarada");
    }

    if (call_node->as.func_call.arg_count != fn->param_count) {
        helper_fail("Cantidad de argumentos invalida");
    }

    enter_scope_checked();

    for (i = 0; i < fn->param_count; i++) {
        Value arg = eval_expr(call_node->as.func_call.args[i]);

        if (arg.type != fn->param_types[i]) {
            free_value(arg);
            exit_scope_checked();
            helper_fail("Tipo de argumento invalido");
        }

        declare_variable_checked(fn->param_names[i], fn->param_types[i], arg);
    }

    {
        ExecResult result = exec_block(fn->body, 1);
        Value ret_value;

        if (result.has_return) {
            ret_value = result.value;
        } else {
            ret_value = default_value_for_type(fn->return_type);
        }

        exit_scope_checked();
        return ret_value;
    }
}

static Value eval_expr(ASTNode* node) {
    if (node == NULL) {
        helper_fail("Expresion invalida");
    }

    switch (node->type) {
        case AST_INT:
            return make_int(node->as.int_lit.value);
        case AST_STRING:
            return make_string(xstrdup_local(node->as.string_lit.value));
        case AST_VAR:
            return variable_value_checked(node->as.var_ref.name);
        case AST_INPUT:
            return eval_input(node->as.input.input_type);
        case AST_FUNC_CALL:
            return eval_call(node);
        case AST_BINOP: {
            Value left = eval_expr(node->as.binary.left);
            Value right = eval_expr(node->as.binary.right);

            switch (node->as.binary.op) {
                case OP_ADD:
                    return add_values_checked(left, right);
                case OP_SUB:
                    return sub_values_checked(left, right);
                case OP_MUL:
                    return mul_values_checked(left, right);
                case OP_DIV:
                    return div_values_checked(left, right);
                case OP_EQ:
                case OP_NE:
                case OP_LT:
                case OP_LE:
                case OP_GT:
                case OP_GE:
                    return eval_compare(node->as.binary.op, left, right);
            }
            break;
        }
        default:
            break;
    }

    helper_fail("Expresion no soportada");
    return make_int(0);
}

static Value eval_compare(ASTOperator op, Value left, Value right) {
    int result = 0;

    if (left.type != TYPE_INT || right.type != TYPE_INT) {
        free_value(left);
        free_value(right);
        helper_fail("Comparacion solo para enteros");
    }

    switch (op) {
        case OP_EQ:
            result = (left.int_value == right.int_value);
            break;
        case OP_NE:
            result = (left.int_value != right.int_value);
            break;
        case OP_LT:
            result = (left.int_value < right.int_value);
            break;
        case OP_LE:
            result = (left.int_value <= right.int_value);
            break;
        case OP_GT:
            result = (left.int_value > right.int_value);
            break;
        case OP_GE:
            result = (left.int_value >= right.int_value);
            break;
        default:
            break;
    }

    free_value(left);
    free_value(right);
    return make_int(result);
}

static char* read_input_line(void) {
    char buffer[1024];
    size_t len;

    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        helper_fail("No se pudo leer entrada");
    }

    len = strlen(buffer);
    while (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r')) {
        buffer[len - 1] = '\0';
        len--;
    }

    return xstrdup_local(buffer);
}

static Value eval_input(ValueType input_type) {
    char* line = read_input_line();

    if (input_type == TYPE_STRING) {
        return make_string(line);
    }

    {
        char* start = line;
        char* end = NULL;
        long value;

        while (*start != '\0' && isspace((unsigned char)*start)) {
            start++;
        }

        value = strtol(start, &end, 10);
        if (end == start) {
            free(line);
            helper_fail("Entrada HUESO invalida");
        }

        while (*end != '\0' && isspace((unsigned char)*end)) {
            end++;
        }

        if (*end != '\0') {
            free(line);
            helper_fail("Entrada HUESO invalida");
        }

        free(line);
        return make_int((int)value);
    }
}

static ExecResult make_exec_result(int has_return, Value value) {
    ExecResult result;
    result.has_return = has_return;
    result.value = value;
    return result;
}

static ExecResult exec_if(ASTNode* node, int in_function) {
    Value cond = eval_expr(node->as.if_stmt.condition);
    int truthy;

    if (cond.type != TYPE_INT) {
        free_value(cond);
        helper_fail("Condicion debe ser HUESO");
    }

    truthy = cond.int_value != 0;
    free_value(cond);

    if (truthy) {
        ExecResult result;
        enter_scope_checked();
        result = exec_block(node->as.if_stmt.then_branch, in_function);
        exit_scope_checked();
        return result;
    }

    if (node->as.if_stmt.else_branch == NULL) {
        return make_exec_result(0, make_int(0));
    }

    if (node->as.if_stmt.else_branch->type == AST_BLOCK) {
        ExecResult result;
        enter_scope_checked();
        result = exec_block(node->as.if_stmt.else_branch, in_function);
        exit_scope_checked();
        return result;
    }

    if (node->as.if_stmt.else_branch->type == AST_IF) {
        return exec_if(node->as.if_stmt.else_branch, in_function);
    }

    helper_fail("Else invalido");
    return make_exec_result(0, make_int(0));
}

static ExecResult exec_block(ASTNode* block, int in_function) {
    ExecResult result = make_exec_result(0, make_int(0));
    int i;

    if (block == NULL || block->type != AST_BLOCK) {
        return result;
    }

    for (i = 0; i < block->as.block.count; i++) {
        ExecResult step = exec_statement(block->as.block.items[i], in_function);

        if (result.value.type == TYPE_STRING) {
            free_value(result.value);
        }
        result = step;

        if (result.has_return) {
            return result;
        }
    }

    return result;
}

static ExecResult exec_statement(ASTNode* node, int in_function) {
    ExecResult result = make_exec_result(0, make_int(0));

    if (node == NULL) {
        return result;
    }

    switch (node->type) {
        case AST_VAR_DECL: {
            Value value = eval_expr(node->as.var_decl.expr);
            declare_variable_checked(node->as.var_decl.name, node->as.var_decl.declared_type, value);
            break;
        }
        case AST_ASSIGN: {
            Value value = eval_expr(node->as.assign.expr);
            assign_variable_checked(node->as.assign.name, value);
            break;
        }
        case AST_PRINT: {
            Value value = eval_expr(node->as.print_stmt.expr);
            print_value_checked(value);
            break;
        }
        case AST_IF:
            return exec_if(node, in_function);
        case AST_SCOPE: {
            ExecResult nested;
            enter_scope_checked();
            nested = exec_block(node->as.scope_stmt.body, in_function);
            exit_scope_checked();
            return nested;
        }
        case AST_FUNC_DECL:
            break;
        case AST_RETURN:
            if (!in_function) {
                helper_fail("RETORNA fuera de una funcion");
            }
            if (node->as.return_stmt.expr != NULL) {
                result.value = eval_expr(node->as.return_stmt.expr);
            } else {
                result.value = make_int(0);
            }
            result.has_return = 1;
            break;
        default:
            helper_fail("Sentencia no soportada");
    }

    return result;
}

static void register_all_functions(ASTNode* node) {
    int i;

    if (node == NULL) {
        return;
    }

    switch (node->type) {
        case AST_BLOCK:
            for (i = 0; i < node->as.block.count; i++) {
                register_all_functions(node->as.block.items[i]);
            }
            break;
        case AST_SCOPE:
            register_all_functions(node->as.scope_stmt.body);
            break;
        case AST_FUNC_DECL:
            register_function(node);
            break;
        default:
            break;
    }
}

int execute_ast(ASTNode* root) {
    ExecResult result;
    int i;

    if (root == NULL || root->type != AST_BLOCK) {
        helper_fail("Programa AST invalido");
    }

    register_all_functions(root);

    result = make_exec_result(0, make_int(0));
    for (i = 0; i < root->as.block.count; i++) {
        ASTNode* stmt = root->as.block.items[i];
        if (stmt != NULL && stmt->type == AST_FUNC_DECL) {
            continue;
        }

        if (result.value.type == TYPE_STRING) {
            free_value(result.value);
        }
        result = exec_statement(stmt, 0);

        if (result.has_return) {
            free_value(result.value);
            free_functions();
            helper_fail("RETORNA fuera de una funcion");
        }
    }

    free_value(result.value);
    free_functions();
    return 0;
}
