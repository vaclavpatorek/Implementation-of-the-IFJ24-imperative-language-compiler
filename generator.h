/**
 * IFJ24
 * @brief Header for generation module for the IFJ24 compiler.
 */

#ifndef GENERATOR_H
#define GENERATOR_H

#include "stack.h"
#include"dstring.h"



void generator_init();
void generator_cleanup();
void gen_header();
void gen_builtin_functions();
void gen_func_start(dstring_t *name);
void gen_func_end();
void gen_defvar(dstring_t *var_name);
void gen_if_start();
void gen_if_else();
void gen_if_end();
void gen_if_nullable_start(dstring_t *non_null_id);
void gen_if_nullable_else();
void gen_if_nullable_end();
void gen_while_start();
void gen_while_cond();
void gen_while_end();
void gen_while_nullable_cond(dstring_t *non_null_id);
void gen_while_nullable_end();
void gen_arithmetic(const char *operator, dstring_t *dest, dstring_t *op1, dstring_t *op2);
void gen_relational(const char *operator, dstring_t *dest, dstring_t *op1, dstring_t *op2);
void gen_logical(const char *operator, dstring_t *dest, dstring_t *op1, dstring_t *op2);
void gen_assignment(dstring_t *dest, dstring_t *source);
void gen_return();
void gen_push_operand(dstring_t *symbol);
void gen_pop_operand(dstring_t *var_name);
void gen_function_call(dstring_t *func_name);

#endif 
