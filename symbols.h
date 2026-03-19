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

Value make_int(int n);
Value make_string(char* s);
Value clone_value(Value v);
void free_value(Value v);

int declare_symbol(const char* name, ValueType expected_type, Value value);
int assign_symbol(const char* name, Value value);
int get_symbol_value(const char* name, Value* out_value);
void cleanup_symbols(void);

#endif
