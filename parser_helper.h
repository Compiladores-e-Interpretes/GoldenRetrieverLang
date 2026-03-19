#ifndef PARSER_HELPER_H
#define PARSER_HELPER_H

#include "symbols.h"

void helper_fail(const char* msg);
void declare_variable_checked(const char* name, ValueType expected_type, Value value);
void assign_variable_checked(const char* name, Value value);
Value variable_value_checked(const char* name);
Value add_values_checked(Value left, Value right);
Value sub_values_checked(Value left, Value right);
Value mul_values_checked(Value left, Value right);
Value div_values_checked(Value left, Value right);
void print_value_checked(Value value);

#endif
