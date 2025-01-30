/// IFJ24
/// @brief Header file for parser.c

#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include "dstring.h"
#include "error_codes.h"
#include "scanner.h"
#include "token.h"
#include "symtable.h"
#include "prec_stack.h"
#include "pars_expr.h"
#include "file.h"
#include "generator.h"

extern symtable_t *symbol_table;
extern token_t *current_token;

int fetch_next_token();
void parser_init(FILE *source);
void error_exit(int exit_code, char *message);
void parser_cleanup();
int parse_program();
int parse_prologue();
int parse_function_header();
int parse_function_definition();
int parse_main_function();
int parse_parameters(func_data_t *func_data);
int parse_parameter(func_data_t *func_data);
int parse_return_type(func_data_t *func_data);
int parse_statements();
int parse_statement();
int fetch_next_token();
int parse_var_decl();
int parse_const_decl(); 
int parse_assignment();
int parse_if_statement();
int parse_if_nullable_statement(data_type cond_type);
int parse_while_statement();
int parse_while_nullable_statement(data_type cond_type);
int parse_return_statement();
int parse_type();
int parse_expression(data_type *ret_type);
int parse_function_call_expr();
int parse_function_call();
int parse_arguments();

#endif
