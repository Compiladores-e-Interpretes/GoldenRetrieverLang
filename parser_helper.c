#include "parser_helper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int yylineno;

void helper_fail(const char* msg) {
    fprintf(stderr, "Error en linea %d: %s\n", yylineno, msg);
    cleanup_symbols();
    exit(1);
}

void enter_scope_checked(void) {
    if (!push_scope()) {
        helper_fail("No se pudo abrir un nuevo ambito");
    }
}

void exit_scope_checked(void) {
    if (current_scope_depth() <= 0) {
        helper_fail("No se puede cerrar el ambito global");
    }
    if (!pop_scope()) {
        helper_fail("No se pudo cerrar el ambito actual");
    }
}

void declare_variable_checked(const char* name, ValueType expected_type, Value value) {
    if (!declare_symbol(name, expected_type, value)) {
        free_value(value);
        helper_fail("Declaracion invalida o variable repetida");
    }
}

void assign_variable_checked(const char* name, Value value) {
    if (!assign_symbol(name, value)) {
        free_value(value);
        helper_fail("Asignacion invalida");
    }
}

Value variable_value_checked(const char* name) {
    Value out;
    if (!get_symbol_value(name, &out)) {
        helper_fail("Variable no declarada");
    }
    return out;
}

Value add_values_checked(Value left, Value right) {
    if (left.type == TYPE_INT && right.type == TYPE_INT) {
        int n = left.int_value + right.int_value;
        free_value(left);
        free_value(right);
        return make_int(n);
    }

    if (left.type == TYPE_STRING && right.type == TYPE_STRING) {
        size_t ln = strlen(left.str_value);
        size_t rn = strlen(right.str_value);
        char* joined = (char*)malloc(ln + rn + 1);
        if (joined == NULL) {
            helper_fail("Sin memoria");
        }
        memcpy(joined, left.str_value, ln);
        memcpy(joined + ln, right.str_value, rn + 1);
        free_value(left);
        free_value(right);
        return make_string(joined);
    }

    free_value(left);
    free_value(right);
    helper_fail("Suma entre tipos incompatibles");
    return make_int(0);
}

Value sub_values_checked(Value left, Value right) {
    int n;
    if (left.type != TYPE_INT || right.type != TYPE_INT) {
        free_value(left);
        free_value(right);
        helper_fail("Resta solo para enteros");
    }

    n = left.int_value - right.int_value;
    free_value(left);
    free_value(right);
    return make_int(n);
}

Value mul_values_checked(Value left, Value right) {
    int n;
    if (left.type != TYPE_INT || right.type != TYPE_INT) {
        free_value(left);
        free_value(right);
        helper_fail("Multiplicacion solo para enteros");
    }

    n = left.int_value * right.int_value;
    free_value(left);
    free_value(right);
    return make_int(n);
}

Value div_values_checked(Value left, Value right) {
    int n;
    if (left.type != TYPE_INT || right.type != TYPE_INT) {
        free_value(left);
        free_value(right);
        helper_fail("Division solo para enteros");
    }
    if (right.int_value == 0) {
        free_value(left);
        free_value(right);
        helper_fail("Division entre cero");
    }

    n = left.int_value / right.int_value;
    free_value(left);
    free_value(right);
    return make_int(n);
}

void print_value_checked(Value value) {
    if (value.type == TYPE_INT) {
        printf("%d\n", value.int_value);
    } else {
        printf("%s\n", value.str_value);
    }
    free_value(value);
}
