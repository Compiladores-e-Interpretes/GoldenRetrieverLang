#include "symbols.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SCOPES 64
#define MAX_SYMBOLS_PER_SCOPE 256

typedef struct {
    char name[128];
    Value value;
    int offset;
} Symbol;

typedef struct {
    Symbol symbols[MAX_SYMBOLS_PER_SCOPE];
    int count;
    int offset_base;
} Scope;

static Scope scope_stack[MAX_SCOPES];
static int scope_top = -1;
static int next_offset = 0;

static char* xstrdup(const char* s) {
    size_t n = strlen(s) + 1;
    char* out = (char*)malloc(n);
    if (out == NULL) {
        fprintf(stderr, "Error: sin memoria\n");
        exit(1);
    }
    memcpy(out, s, n);
    return out;
}

static int find_in_scope(int scope_index, const char* name) {
    int i;
    if (scope_index < 0 || scope_index > scope_top) {
        return -1;
    }

    for (i = 0; i < scope_stack[scope_index].count; i++) {
        if (strcmp(scope_stack[scope_index].symbols[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

static int find_in_stack(const char* name, int* out_scope, int* out_index) {
    int s;
    for (s = scope_top; s >= 0; s--) {
        int idx = find_in_scope(s, name);
        if (idx != -1) {
            *out_scope = s;
            *out_index = idx;
            return 1;
        }
    }
    return 0;
}

Value make_int(int n) {
    Value v;
    v.type = TYPE_INT;
    v.int_value = n;
    v.str_value = NULL;
    return v;
}

Value make_string(char* s) {
    Value v;
    v.type = TYPE_STRING;
    v.int_value = 0;
    v.str_value = s;
    return v;
}

Value clone_value(Value v) {
    if (v.type == TYPE_INT) {
        return make_int(v.int_value);
    }
    return make_string(xstrdup(v.str_value == NULL ? "" : v.str_value));
}

void free_value(Value v) {
    if (v.type == TYPE_STRING && v.str_value != NULL) {
        free(v.str_value);
    }
}

int declare_symbol(const char* name, ValueType expected_type, Value value) {
    int idx;
    Scope* current;

    if (scope_top < 0) {
        return 0;
    }

    current = &scope_stack[scope_top];
    idx = find_in_scope(scope_top, name);
    if (idx != -1) {
        return 0;
    }

    if (value.type != expected_type) {
        return 0;
    }

    if (current->count >= MAX_SYMBOLS_PER_SCOPE) {
        return 0;
    }

    idx = current->count;
    strncpy(current->symbols[idx].name, name, sizeof(current->symbols[idx].name) - 1);
    current->symbols[idx].name[sizeof(current->symbols[idx].name) - 1] = '\0';
    current->symbols[idx].value = value;
    current->symbols[idx].offset = next_offset;
    current->count++;
    next_offset += 4;
    return 1;
}

int assign_symbol(const char* name, Value value) {
    int s;
    int idx;
    if (!find_in_stack(name, &s, &idx)) {
        return 0;
    }

    if (scope_stack[s].symbols[idx].value.type != value.type) {
        return 0;
    }

    free_value(scope_stack[s].symbols[idx].value);
    scope_stack[s].symbols[idx].value = value;
    return 1;
}

int get_symbol_value(const char* name, Value* out_value) {
    int s;
    int idx;
    if (!find_in_stack(name, &s, &idx)) {
        return 0;
    }

    *out_value = clone_value(scope_stack[s].symbols[idx].value);
    return 1;
}

int get_symbol_meta(const char* name, SymbolMeta* out_meta) {
    int s;
    int idx;
    if (!find_in_stack(name, &s, &idx)) {
        return 0;
    }

    out_meta->depth = s;
    out_meta->offset = scope_stack[s].symbols[idx].offset;
    return 1;
}

int init_symbol_stack(void) {
    scope_top = -1;
    next_offset = 0;
    return push_scope();
}

int push_scope(void) {
    if (scope_top + 1 >= MAX_SCOPES) {
        return 0;
    }

    scope_top++;
    scope_stack[scope_top].count = 0;
    scope_stack[scope_top].offset_base = next_offset;
    return 1;
}

int pop_scope(void) {
    int i;
    Scope* current;
    if (scope_top < 0) {
        return 0;
    }

    current = &scope_stack[scope_top];
    for (i = 0; i < current->count; i++) {
        free_value(current->symbols[i].value);
        current->symbols[i].name[0] = '\0';
    }

    next_offset = current->offset_base;
    current->count = 0;
    scope_top--;
    return 1;
}

int current_scope_depth(void) {
    return scope_top;
}

void cleanup_symbols(void) {
    while (scope_top >= 0) {
        pop_scope();
    }
}
