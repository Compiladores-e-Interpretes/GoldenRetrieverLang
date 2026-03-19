#include "symbols.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SYMBOLS 256

typedef struct {
    char name[128];
    int used;
    Value value;
} Symbol;

static Symbol table[MAX_SYMBOLS];

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

static int find_index(const char* name) {
    int i;
    for (i = 0; i < MAX_SYMBOLS; i++) {
        if (table[i].used && strcmp(table[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

static int first_free_index(void) {
    int i;
    for (i = 0; i < MAX_SYMBOLS; i++) {
        if (!table[i].used) {
            return i;
        }
    }
    return -1;
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
    int idx = find_index(name);
    int free_idx;

    if (idx != -1) {
        return 0;
    }

    if (value.type != expected_type) {
        return 0;
    }

    free_idx = first_free_index();
    if (free_idx == -1) {
        return 0;
    }

    strncpy(table[free_idx].name, name, sizeof(table[free_idx].name) - 1);
    table[free_idx].name[sizeof(table[free_idx].name) - 1] = '\0';
    table[free_idx].used = 1;
    table[free_idx].value = value;
    return 1;
}

int assign_symbol(const char* name, Value value) {
    int idx = find_index(name);
    if (idx == -1) {
        return 0;
    }

    if (table[idx].value.type != value.type) {
        return 0;
    }

    free_value(table[idx].value);
    table[idx].value = value;
    return 1;
}

int get_symbol_value(const char* name, Value* out_value) {
    int idx = find_index(name);
    if (idx == -1) {
        return 0;
    }

    *out_value = clone_value(table[idx].value);
    return 1;
}

void cleanup_symbols(void) {
    int i;
    for (i = 0; i < MAX_SYMBOLS; i++) {
        if (table[i].used) {
            free_value(table[i].value);
            table[i].used = 0;
            table[i].name[0] = '\0';
        }
    }
}
