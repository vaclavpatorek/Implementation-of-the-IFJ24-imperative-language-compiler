/// IFJ24
/// @brief Header file for pars_expr.c

#ifndef PARS_EXPR_H
#define PARS_EXPR_H

#define PREC_TABLE_SIZE 17

#include <stdio.h>
#include <stdlib.h>
#include "dstring.h"
#include "error_codes.h"
#include "scanner.h"
#include "token.h"
#include "symtable.h"
#include "prec_stack.h"
#include "parser.h"

int parse_expr(data_type *ret_type);

#endif