#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <stddef.h>

typedef enum {
    TYPE_INT = 0,
    TYPE_STRING = 1
} ValueType;

typedef struct {
    ValueType type;
    int int_value;
    char* str_value;
} Value;

typedef struct {
    int depth;
    int offset;
} SymbolMeta;

Value make_int(int n);
Value make_string(char* s);
Value clone_value(Value v);
void free_value(Value v);

int declare_symbol(const char* name, ValueType expected_type, Value value);
int assign_symbol(const char* name, Value value);
int get_symbol_value(const char* name, Value* out_value);
int get_symbol_meta(const char* name, SymbolMeta* out_meta);

int init_symbol_stack(void);
int push_scope(void);
int pop_scope(void);
int current_scope_depth(void);
void cleanup_symbols(void);

#endif
