/// IFJ24
/// @brief Parser implementation for expression parsing and reduction in IFJ24.

#include "pars_expr.h"

extern symtable_t *symbol_table;
int bracket_count;
Stack main_stack;
Stack temp_stack;
bool is_main_stack = true;

// Precedence table for operators used in expressions
static int prec_table[PREC_TABLE_SIZE][PREC_TABLE_SIZE] = {
    // +    -    *    /    ==   !=   !    <    >    <=   >=   &&   ||   (    )    i    $
    { '>', '>', '<', '<', '>', '>', '<', '>', '>', '>', '>', '>', '>', '<', '>', '<', '>' }, // +
    { '>', '>', '<', '<', '>', '>', '<', '>', '>', '>', '>', '>', '>', '<', '>', '<', '>' }, // -
    { '>', '>', '>', '>', '>', '>', '<', '>', '>', '>', '>', '>', '>', '<', '>', '<', '>' }, // *
    { '>', '>', '>', '>', '>', '>', '<', '>', '>', '>', '>', '>', '>', '<', '>', '<', '>' }, // /
    { '<', '<', '<', '<', -1, -1, '<', -1, -1, -1, -1, '>', '>', '<', '>', '<', '>' },       // ==
    { '<', '<', '<', '<', -1, -1, '<', -1, -1, -1, -1, '>', '>', '<', '>', '<', '>' },       // !=
    { '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '>', '<', '>', '<', '>' }, // !
    { '<', '<', '<', '<', -1, -1, '<', -1, -1, -1, -1, '>', '>', '<', '>', '<', '>' },       // <
    { '<', '<', '<', '<', -1, -1, '<', -1, -1, -1, -1, '>', '>', '<', '>', '<', '>' },       // >
    { '<', '<', '<', '<', -1, -1, '<', -1, -1, -1, -1, '>', '>', '<', '>', '<', '>' },       // <=
    { '<', '<', '<', '<', -1, -1, '<', -1, -1, -1, -1, '>', '>', '<', '>', '<', '>' },       // >=
    { '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '>', '>', '<', '>', '<', '>' }, // &&
    { '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '>', '<', '>', '<', '>' }, // ||
    { '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '=', '<', -1 },  // (
    { '>', '>', '>', '>', '>', '>', -1, '>', '>', '>', '>', '>', '>', -1 , '>', -1, '>' },   // )
    { '>', '>', '>', '>', '>', '>', -1, '>', '>', '>', '>', '>', '>', -1 , '>', -1, '>' },   // i
    { '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', '<', -1, '<', -1  }   // $
};

/**
 * @brief Cleans up parsing stacks and resets the bracket counter.
 * @details Frees memory allocated for the main and temporary stacks, ensuring no memory leaks.
 */
void cleanup_stacks() {
    bracket_count = 0;
    if (!is_main_stack) {
        if (temp_stack.top != NULL) stack_destroy(&temp_stack);
        if (main_stack.top != NULL) stack_destroy(&main_stack);
    } else {
        if (main_stack.top != NULL) stack_destroy(&main_stack);
    }
}

/**
 * @brief Converts a token into a precedence table symbol.
 * @param data Pointer to a StackData structure to populate with token details.
 * @return Corresponding precedence symbol or -1 on error.
 */
int token_to_prec_symb(StackData *data) {
    data->type = void_type;      // Default type
    data->is_literal = false;
    data->token = token_copy(current_token);
    if (!data->token) {
        cleanup_stacks();
        set_error(ERROR_INTERNAL_COMPILER_ERROR, "Failed to copy token", 0, 0);
        exit(ERROR_INTERNAL_COMPILER_ERROR);
    }

    switch (current_token->type) {
        case TOKEN_ADD: data->symbol = ADD; return ADD;
        case TOKEN_SUB: data->symbol = SUB; return SUB;
        case TOKEN_MUL: data->symbol = MUL; return MUL;
        case TOKEN_DIV: data->symbol = DIV; return DIV;
        case TOKEN_EQ: data->symbol = EQ; return EQ;
        case TOKEN_NEQ: data->symbol = NEQ; return NEQ;
        case TOKEN_NOT: data->symbol = NOT; return NOT;
        case TOKEN_COMP_L: data->symbol = LT; return LT;
        case TOKEN_COMP_G: data->symbol = GT; return GT;
        case TOKEN_COMP_LE: data->symbol = LE; return LE;
        case TOKEN_COMP_GE: data->symbol = GE; return GE;
        case TOKEN_LOGICAL_AND: data->symbol = AND; return AND;
        case TOKEN_LOGICAL_OR: data->symbol = OR; return OR;
        case TOKEN_OPENING_PARENTHESES: data->symbol = LPAR; bracket_count++ ; return LPAR;
        case TOKEN_CLOSING_PARENTHESES: data->symbol = RPAR; bracket_count-- ; return RPAR;
        case TOKEN_STRING: 
            data->type = string_type; 
            data->symbol = ID; 
            data->is_literal = true;
            return ID;
        case TOKEN_INT: 
            data->type = int_type; 
            data->symbol = ID; 
            data->is_literal = true;
            return ID;
        case TOKEN_FLOAT: 
            data->type = float_type; 
            data->symbol = ID; 
            data->is_literal = true;
            return ID;
        case TOKEN_INT_NULL: data->type = null_int_type; data->symbol = ID;  return ID;
        case TOKEN_STRING_NULL: data->type = null_string_type; data->symbol = ID; return ID;
        case TOKEN_FLOAT_NULL: data->type = null_float_type; data->symbol = ID;  return ID;
        case KW_NULL: data->type = null_type; data->symbol = ID; data->is_literal = true; return ID;
        case TOKEN_ID: {
            symtable_data_t *entry = symtable_find(symbol_table, current_token->attribute.s);
            if (!entry) {
                cleanup_stacks();
                error_exit(ERROR_SEMANTIC_UNDEFINED_FUNCTION_OR_VARIABLE, "Undefined variable");
            }
            if(entry->type == var_t || entry->type == const_t){
                entry->varData->isUsed = true;
            }    
            if(entry->varData){
                data->type = entry->varData->type;
            } else if(entry->funcData){
                data_type func_return_type;
                int func_bracket_count = bracket_count;
                if (parse_function_call_expr(&func_return_type) != 0) {
                    cleanup_stacks();
                    error_exit(ERROR_SEMANTIC_INCORRECT_FUNCTION_PARAMETERS_OR_RETURN_VALUE, "Invalid function call in expression");
                } 
                bracket_count = func_bracket_count;
                data->type = func_return_type;
            }
            data->symbol = ID;
            return ID;
        }
        case TOKEN_OPENING_BRACKET:
        case TOKEN_SEMICOLON:
        case TOKEN_EOL:
        case TOKEN_COMMA:
            data->symbol = DOLLAR;
            return DOLLAR;
        default:
            cleanup_stacks();
            error_exit(ERROR_LEXICAL_ANALYSIS, "Invalid or unexpected token"); //Lexical
    }
    return -1;
}

/**
 * @brief Validates type compatibility for arithmetic operations.
 * @param lhs_type Type of the left-hand operand.
 * @param rhs_type Type of the right-hand operand.
 * @param result_type Pointer to store the resulting type.
 * @param lhs_is_literal Indicates if the left-hand operand is a literal.
 * @param rhs_is_literal Indicates if the right-hand operand is a literal.
 * @return True if the operation is compatible, false otherwise.
 */
bool check_arithmetic_compatibility(data_type lhs_type, data_type rhs_type, data_type *result_type, bool lhs_is_literal, bool rhs_is_literal) {
    if (lhs_type == int_type && rhs_type == int_type) {
        *result_type = int_type;
        return true;
    } else if (lhs_type == float_type && rhs_type == float_type) {
        *result_type = float_type;
        return true;
    } else if (lhs_type == int_type && rhs_type == float_type) {
        if (lhs_is_literal) {
            printf("POPS GF@temp\n");
            printf("INT2FLOATS\n"); 
            printf("PUSHS GF@temp\n");
        } else {
            cleanup_stacks();
            error_exit(ERROR_SEMANTIC_TYPE_INCOMPATIBILITY, "Incompatible types for arithmetic operation");
            return false;
        }
        *result_type = float_type;
        return true;
    } else if (lhs_type == float_type && rhs_type == int_type) {
        if (rhs_is_literal) {
            printf("INT2FLOATS\n"); 
        } else {
            cleanup_stacks();
            error_exit(ERROR_SEMANTIC_TYPE_INCOMPATIBILITY, "Incompatible types for arithmetic operation");
            return false;
        }
        *result_type = float_type;
        return true;
    }
    cleanup_stacks();
    error_exit(ERROR_SEMANTIC_TYPE_INCOMPATIBILITY, "Incompatible types for arithmetic operation");
    return false;
}

/**
 * @brief Validates type compatibility for equality operations (==, !=).
 * @param lhs_type Type of the left-hand operand.
 * @param rhs_type Type of the right-hand operand.
 * @param result_type Pointer to store the resulting type (boolean).
 * @param lhs_is_literal Indicates if the left-hand operand is a literal.
 * @param rhs_is_literal Indicates if the right-hand operand is a literal.
 * @return True if the operation is compatible, false otherwise.
 */
bool check_equality_compatibility(data_type lhs_type, data_type rhs_type, data_type *result_type, bool lhs_is_literal, bool rhs_is_literal) {
    if (lhs_type == rhs_type) {
        *result_type = bool_type;
        return true;
    } else if ((lhs_type == int_type && rhs_type == float_type) || (lhs_type == float_type && rhs_type == int_type)) {
        if (lhs_type == int_type && (lhs_is_literal || rhs_is_literal)) {
            printf("POPS GF@temp\n");
            printf("INT2FLOATS\n"); 
            printf("PUSHS GF@temp\n");
        } else if (rhs_type == int_type && (rhs_is_literal || lhs_is_literal)) {
            printf("INT2FLOATS\n"); 
        } else {
            cleanup_stacks();
            error_exit(ERROR_SEMANTIC_TYPE_INCOMPATIBILITY, "Incompatible types for relational operation");
            return false;
        }
        *result_type = bool_type;
        return true;
    } else if (lhs_type == null_type || rhs_type == null_type ||
        (lhs_type == null_int_type && rhs_type == int_type) ||
        (lhs_type == int_type && rhs_type == null_int_type) ||
        (lhs_type == null_float_type && rhs_type == float_type) ||
        (lhs_type == float_type && rhs_type == null_float_type)) {
        *result_type = bool_type;
        return true;
    }

    cleanup_stacks();
    error_exit(ERROR_SEMANTIC_TYPE_INCOMPATIBILITY, "Incompatible types for equality operation");
    return false;
}

/**
 * @brief Validates type compatibility for relational operations (<, >, <=, >=).
 * @param lhs_type Type of the left-hand operand.
 * @param rhs_type Type of the right-hand operand.
 * @param result_type Pointer to store the resulting type (boolean).
 * @param lhs_is_literal Indicates if the left-hand operand is a literal.
 * @param rhs_is_literal Indicates if the right-hand operand is a literal.
 * @return True if the operation is compatible, false otherwise.
 */
bool check_relational_compatibility(data_type lhs_type, data_type rhs_type, data_type *result_type, bool lhs_is_literal, bool rhs_is_literal) {
    if (lhs_type == null_type || rhs_type == null_type) {
        cleanup_stacks();
        set_error(ERROR_SEMANTIC_TYPE_INCOMPATIBILITY, "Null values not allowed in relational operation", 0, 0);
        return false;
    }

    if (lhs_type == rhs_type) {
        *result_type = bool_type;
        return true;
    } else if ((lhs_type == int_type && rhs_type == float_type) || (lhs_type == float_type && rhs_type == int_type)) {
        if (lhs_type == float_type && !lhs_is_literal) {
            printf("INT2FLOATS\n"); 
        } else if (rhs_type == float_type && !rhs_is_literal) {
            printf("POPS GF@temp\n");
            printf("INT2FLOATS\n"); 
            printf("PUSHS GF@temp\n");
        } else {
            cleanup_stacks();
            error_exit(ERROR_SEMANTIC_TYPE_INCOMPATIBILITY, "Incompatible types for relational operation");
            return false;
        }
        *result_type = bool_type;
        return true;
    }

    cleanup_stacks();
    error_exit(ERROR_SEMANTIC_TYPE_INCOMPATIBILITY, "Incompatible types for relational operation");
    return false;
}

/**
 * @brief Reduces the top of the stack according to grammar rules.
 * @param stack Pointer to the active parsing stack.
 * @param result_type Pointer to store the resulting type after reduction.
 * @return 0 on success, -1 on failure.
 */
int reduce(Stack* stack, data_type* result_type) {
    StackData *top = stack_top(stack);
    if (top == NULL) {
        cleanup_stacks();
        error_exit(ERROR_SYNTAX_ANALYSIS, "Stack underflow in reduce");
    }

     // Rule: E -> id (literal value)
    if (top->symbol == ID && top->is_literal) {
        *result_type = top->type;
        
        switch (top->type) {
            case int_type:
                printf("PUSHS int@%d\n", top->token->attribute.i);
                break;
            case float_type:
                printf("PUSHS float@%a\n", top->token->attribute.f);
                break;
            case string_type:
                printf("PUSHS string@%s\n", top->token->attribute.s->data);
                break;
            case null_type:
                printf("PUSHS nil@nil\n");
                break;
            default:
                cleanup_stacks();
                error_exit(ERROR_SEMANTIC_TYPE_INCOMPATIBILITY, "Unsupported literal type");
        }

        stack_pop(stack);

        StackData exp_data = {.symbol = EXP, .type = *result_type, .is_literal = true, .token = NULL};
        stack_push(stack, &exp_data);
        return 0;
    }

    // Rule: E -> id (identifier)
    if (top->symbol == ID) {
        symtable_data_t *entry = symtable_find(symbol_table, top->token->attribute.s);

        if (!entry) {
            cleanup_stacks();
            error_exit(ERROR_SEMANTIC_UNDEFINED_FUNCTION_OR_VARIABLE, "Undefined identifier");
        }

        if (entry->type == fn_t) {
            printf("PUSHS GF@return\n");
            *result_type = top->type;
        } else if (entry->type == var_t || entry->type == const_t) {
            if(top->token->attribute.s){
                printf("PUSHS LF@%s\n", top->token->attribute.s->data); 
            }
            *result_type = entry->varData->type;
        } else {
            cleanup_stacks();
            error_exit(ERROR_SEMANTIC_TYPE_INCOMPATIBILITY, "Unsupported identifier type");
        }

        stack_pop(stack);

        StackData exp_data = {.symbol = EXP, .type = *result_type, .is_literal = false, .token = NULL};
        stack_push(stack, &exp_data);
        return 0;
    }


    // Rule: E -> (E)
    StackData *rhs = stack_peek_n(stack, 0);     
    StackData *inner = stack_peek_n(stack, 1); 
    StackData *lhs = stack_peek_n(stack, 2);     


    if (rhs && inner && lhs &&
        rhs->symbol == RPAR && inner->symbol == EXP && lhs->symbol == LPAR) {
        *result_type = inner->type;
        stack_pop(stack); // Pop '('
        stack_pop(stack); // Pop expression E
        stack_pop(stack); // Pop ')'
        StackData exp_data = {.symbol = EXP, .type = *result_type, .is_literal = (rhs->is_literal && lhs->is_literal), .token = NULL};
        stack_push(stack, &exp_data); // Push reduced expression E
        return 0;
    }

    // Rule: E -> E op E
    StackData *op = stack_peek_n(stack, 1);

    if (lhs && op && rhs && lhs->symbol == EXP && rhs->symbol == EXP) {
        bool compatible = false;

        if (op->symbol == ADD || op->symbol == SUB || op->symbol == MUL) {
            compatible = check_arithmetic_compatibility(lhs->type, rhs->type, result_type, lhs->is_literal, rhs->is_literal);
        } else if (op->symbol == DIV) {
            compatible = check_arithmetic_compatibility(lhs->type, rhs->type, result_type, lhs->is_literal, rhs->is_literal);
        } else if (op->symbol == AND || op->symbol == OR) {
            compatible = true;
            *result_type = bool_type;
        } else if (op->symbol == EQ || op->symbol == NEQ){
            compatible = check_equality_compatibility(lhs->type, rhs->type, result_type, lhs->is_literal, rhs->is_literal);
        }else if(op->symbol == LT || op->symbol == GT || op->symbol == LE || op->symbol == GE) {
            compatible = check_relational_compatibility(lhs->type, rhs->type, result_type, lhs->is_literal, rhs->is_literal);
        }

        if (!compatible) {
            cleanup_stacks();
            error_exit(ERROR_SEMANTIC_TYPE_INCOMPATIBILITY, "Incompatible types for operation");
        }

        switch (op->symbol) {
            case ADD: printf("ADDS\n"); break;
            case SUB: printf("SUBS\n"); break;
            case MUL: printf("MULS\n"); break;
            case DIV: printf("DIVS\n"); break;
            case AND: printf("ANDS\n"); break;
            case OR:  printf("ORS\n"); break;
            case EQ:  printf("EQS\n"); break;
            case NEQ: printf("EQS\nNOTS\n"); break;
            case LT:  printf("LTS\n"); break;
            case GT:  printf("GTS\n"); break;
            case GE:  printf("LTS\nNOTS\n"); break;
            case LE:  printf("GTS\nNOTS\n"); break;
        }

        stack_pop(stack); // Pop rhs (E)
        stack_pop(stack); // Pop op
        stack_pop(stack); // Pop lhs (E)
        StackData exp_data = {.symbol = EXP, .type = *result_type, .is_literal = (rhs->is_literal && lhs->is_literal), .token = NULL};
        stack_push(stack, &exp_data); // Push reduced expression E
        return 0;
    }

    // Rule: E -> !E
    StackData *exp = stack_peek_n(stack, 0);
    StackData *not_op = stack_peek_n(stack, 1);

    if (not_op && exp && not_op->symbol == NOT && exp->symbol == EXP) {
        *result_type = bool_type;

        printf("NOTS\n");

        stack_pop(stack); // Pop E
        stack_pop(stack); // Pop NOT
        StackData exp_data = {.symbol = EXP, .type = *result_type, .is_literal = (rhs->is_literal && lhs->is_literal), .token = NULL};
        stack_push(stack, &exp_data); // Push reduced expression E
        return 0;
    }

    return -1;
}

/**
 * @brief Checks if the stack's top elements indicate the end of the expression.
 * @param stack Pointer to the stack.
 * @return 1 if the expression is correctly ended, 0 otherwise.
 */
int correct_exp_end(Stack *stack){
if(stack->top->data->symbol == EXP && stack->top->next->data->symbol == DOLLAR && stack->top->next->next == NULL){
    return 1;
}
return 0;
}

/**
 * @brief Retrieves the topmost terminal symbol from the stack.
 * @param stack Pointer to the stack.
 * @return Pointer to the top terminal StackData, or NULL if no terminal exists.
 */
StackData* stack_top_terminal(Stack *stack) {
    if (!stack || !stack->top) {
        return NULL;
    }

    StackElement *node = stack->top;
    while (node) {
        if (node->data->symbol != EXP ) {
            return node->data;
        }
        node = node->next;
    }
    
    return NULL; 
}

/**
 * @brief Parses an expression.
 * @param ret_type Pointer to store the resulting type of the parsed expression.
 * @return 0 on successful parsing, -1 on failure.
 */
int parse_expr(data_type *ret_type) {
    bracket_count = 0;
    Stack *active_stack = &main_stack;
    if (stack_is_empty(&main_stack)) {
        is_main_stack = true;
        stack_init(&main_stack);
    } else {
        is_main_stack = false;
        stack_init(&temp_stack);
        active_stack = &temp_stack;
    }                                         

    StackData dollar_data = { .symbol = DOLLAR, .type = void_type, .is_literal = false, .token = NULL};
    stack_push(active_stack, &dollar_data);

    
    StackData current_data;
    int current_symbol = token_to_prec_symb(&current_data);

    while (true) {
        StackData *top_terminal = stack_top_terminal(active_stack);
        if (!top_terminal) {
            cleanup_stacks();      
            error_exit(ERROR_SYNTAX_ANALYSIS, "Unexpected empty stack during parsing");
        }

        int top_symbol = top_terminal->symbol;
        int action = prec_table[top_symbol][current_symbol];
        //print_stack(active_stack);


        if (action == '<') {
            StackData new_data = {
                .symbol = current_data.symbol,
                .type = current_data.type,
                .is_literal = current_data.is_literal,
                .token = token_copy(current_data.token)  
            };
            stack_push(active_stack, &new_data);
            fetch_next_token();
            current_symbol = token_to_prec_symb(&current_data);
        } else if (action == '>') {
            if (reduce(active_stack, ret_type) != 0) {
                cleanup_stacks();
                *ret_type = null;
                return -1;
            }
        } else if (action == '=') {
            StackData new_data = {
                .symbol = current_data.symbol,
                .type = current_data.type,
                .is_literal = current_data.is_literal,
                .token = token_copy(current_data.token) 
            };
            stack_push(active_stack, &new_data);
            fetch_next_token();
            current_symbol = token_to_prec_symb(&current_data);
        } else {
            cleanup_stacks();
            *ret_type = null;
            error_exit(ERROR_SYNTAX_ANALYSIS, "Invalid precedence action");
        }
        //print_stack(active_stack);

        if (bracket_count < 0) {
            current_symbol = DOLLAR;
        }

        if (current_symbol == DOLLAR && correct_exp_end(active_stack)) {
            break;
        }
    }

    if (bracket_count > 0) {
        cleanup_stacks();
        *ret_type = null;
        error_exit(ERROR_SYNTAX_ANALYSIS, "Unmatched opening parenthesis");
    }
   

    data_type top = active_stack->top->data->type;
    *ret_type = top;
    if (!is_main_stack) {
        stack_destroy(&temp_stack);              
    } else {
        stack_destroy(&main_stack);                   
    }
    is_main_stack = true;
    return 0;
}