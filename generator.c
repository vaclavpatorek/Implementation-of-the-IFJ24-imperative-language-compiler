/**
 * IFJ24
 * @brief Code generation module for the IFJ24 compiler.
 */

#include "generator.h"
#include <stdio.h>

genStack *if_stack;      
genStack *while_stack;   
int label_counter; 

/**
 * @brief Initializes the generator by setting up stacks and counters.
 * @details Allocates memory for the if and while stacks and initializes them.
 */
void generator_init() {
    if_stack = malloc(sizeof(genStack));
    if (if_stack == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for if_stack.\n");
        exit(EXIT_FAILURE);
    }
    gen_stack_init(if_stack);

    while_stack = malloc(sizeof(genStack));
    if (while_stack == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for while_stack.\n");
        exit(EXIT_FAILURE);
    }
    gen_stack_init(while_stack);

    label_counter = 0;
}

/**
 * @brief Cleans up the generator by freeing allocated stacks.
 * @details Ensures memory allocated for the if and while stacks is freed.
 */
void generator_cleanup() {
    if (if_stack) {
        free(if_stack);
        if_stack = NULL;
    }
    if (while_stack) {
        free(while_stack);
        while_stack = NULL;
    }
}

/**
 * @brief Generates the header for IFJcode24.
 * @details Outputs initial IFJcode24 setup, including definitions and the main label.
 */
void gen_header() {
    printf(".IFJcode24\n");
    printf("DEFVAR GF@return\n");
    printf("DEFVAR GF@_discard\n");
    printf("DEFVAR GF@temp\n");
    printf("JUMP $main\n");
    gen_builtin_functions();
}

/**
 * @brief Generates built-in function implementations.
 * @details Includes functions like readstr, write, concat, ord, and others.
 */
void gen_builtin_functions() {
    // Built-in: ifj.readstr
    printf("\nLABEL $ifj_readstr\n");
    printf("CREATEFRAME\n");
    printf("PUSHFRAME\n");
    printf("READ GF@return string\n");
    printf("POPFRAME\n");
    printf("RETURN\n");

    // Built-in: ifj.readi32
    printf("\nLABEL $ifj_readi32\n");
    printf("CREATEFRAME\n");
    printf("PUSHFRAME\n");
    printf("READ GF@return int\n");
    printf("POPFRAME\n");
    printf("RETURN\n");

    // Built-in: ifj.readf64
    printf("\nLABEL $ifj_readf64\n");
    printf("CREATEFRAME\n");
    printf("PUSHFRAME\n");
    printf("READ GF@return float\n");
    printf("POPFRAME\n");
    printf("RETURN\n");

    // Built-in: ifj.write
    printf("\nLABEL $ifj_write\n");
    printf("CREATEFRAME\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@param1\n");
    printf("POPS LF@param1\n");
    printf("WRITE LF@param1\n");
    printf("POPFRAME\n");
    printf("RETURN\n");

    // Built-in: ifj.i2f
    printf("\nLABEL $ifj_i2f\n");
    printf("CREATEFRAME\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@param1\n");;
    printf("POPS LF@param1\n");
    printf("INT2FLOAT GF@return LF@param1\n");
    printf("POPFRAME\n");
    printf("RETURN\n");

    // Built-in: ifj.f2i
    printf("\nLABEL $ifj_f2i\n");
    printf("CREATEFRAME\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@param1\n");
    printf("POPS LF@param1\n");
    printf("FLOAT2INT GF@return LF@param1\n");
    printf("POPFRAME\n");
    printf("RETURN\n");

    printf("\nLABEL $ifj_string\n");
    printf("CREATEFRAME\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@param1\n");  
    printf("POPS LF@param1\n");    
    printf("MOVE GF@return LF@param1\n"); 
    printf("POPFRAME\n");
    printf("RETURN\n");

    // Built-in: ifj.concat
    printf("\nLABEL $ifj_concat\n");
    printf("CREATEFRAME\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@param1\n");
    printf("DEFVAR LF@param2\n");
    printf("POPS LF@param1\n");
    printf("POPS LF@param2\n");
    printf("CONCAT GF@return LF@param1 LF@param2\n");
    printf("POPFRAME\n");
    printf("RETURN\n");

    // Built-in: ifj.length
    printf("\nLABEL $ifj_length\n");
    printf("CREATEFRAME\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@param1\n");
    printf("POPS LF@param1\n");
    printf("STRLEN GF@return LF@param1\n");
    printf("POPFRAME\n");
    printf("RETURN\n");

    // Built-in: ifj.chr
    printf("\nLABEL $ifj_chr\n");
    printf("CREATEFRAME\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@param1\n");
    printf("POPS LF@param1\n");
    printf("INT2CHAR GF@return LF@param1\n");
    printf("POPFRAME\n");
    printf("RETURN\n");

    // Built-in: ifj.ord
    printf("\nLABEL $ifj_ord\n");
    printf("CREATEFRAME\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@param1\n"); 
    printf("DEFVAR LF@param2\n"); 
    printf("DEFVAR LF@length\n"); 
    printf("DEFVAR LF@char\n");   
    printf("DEFVAR LF@result\n"); 
    printf("DEFVAR LF@type_check\n"); 
    printf("POPS LF@param1\n");
    printf("POPS LF@param2\n");
    printf("TYPE LF@type_check LF@param1\n");
    printf("JUMPIFNEQ $ord_error LF@type_check string@string\n");
    printf("STRLEN LF@length LF@param1\n");
    printf("LT GF@temp LF@param2 int@0\n");
    printf("JUMPIFEQ $ord_error GF@temp bool@true\n");
    printf("LT GF@temp LF@param2 LF@length\n");
    printf("JUMPIFEQ $ord_inbounds GF@temp bool@true\n");
    printf("LABEL $ord_error\n");
    printf("MOVE GF@return int@0\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("LABEL $ord_inbounds\n");
    printf("STRI2INT LF@result LF@param1 LF@param2\n");
    printf("MOVE GF@return LF@result\n");
    printf("POPFRAME\n");
    printf("RETURN\n");

    // Built-in: ifj.substring !!!!!
    printf("\nLABEL $ifj_substring\n");
    printf("CREATEFRAME\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@param1\n");
    printf("DEFVAR LF@param2\n");
    printf("DEFVAR LF@param3\n");
    printf("DEFVAR LF@result\n");
    printf("DEFVAR LF@char\n");
    printf("DEFVAR LF@index\n");
    printf("DEFVAR LF@end\n");
    printf("DEFVAR LF@type_check\n");
    printf("MOVE LF@result string@\n");
    printf("POPS LF@param1\n");
    printf("POPS LF@param2\n");
    printf("POPS LF@param3\n");
    printf("TYPE LF@type_check LF@param2\n");
    printf("JUMPIFNEQ $substr_error LF@type_check string@int\n");
    printf("TYPE LF@type_check LF@param3\n");
    printf("JUMPIFNEQ $substr_error LF@type_check string@int\n");
    printf("LT GF@temp LF@param2 int@0\n");
    printf("JUMPIFEQ $substr_error GF@temp bool@true\n");
    printf("LT GF@temp LF@param3 int@0\n");
    printf("JUMPIFEQ $substr_error GF@temp bool@true\n");
    printf("ADD LF@end LF@param2 LF@param3\n");
    printf("MOVE LF@index LF@param2\n");
    printf("LABEL $substr_loop\n");
    printf("LT GF@temp LF@index LF@end\n");
    printf("JUMPIFEQ $substr_end GF@temp bool@false\n");
    printf("STRLEN GF@temp LF@param1\n");
    printf("LT GF@temp LF@index GF@temp\n");
    printf("JUMPIFEQ $substr_error GF@temp bool@false\n");
    printf("STRI2INT LF@char LF@param1 LF@index\n");
    printf("INT2CHAR LF@char LF@char\n");
    printf("CONCAT LF@result LF@result LF@char\n");
    printf("ADD LF@index LF@index int@1\n");
    printf("JUMP $substr_loop\n");
    printf("LABEL $substr_end\n");
    printf("MOVE GF@return LF@result\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("LABEL $substr_error\n");
    printf("MOVE GF@return nil@nil\n");
    printf("POPFRAME\n");
    printf("RETURN\n");

    // Built-in: ifj.strcmp !!!!!
    printf("\nLABEL $ifj_strcmp\n");
    printf("CREATEFRAME\n");
    printf("PUSHFRAME\n");
    printf("DEFVAR LF@result\n");
    printf("DEFVAR LF@param1\n");
    printf("DEFVAR LF@param2\n");
    printf("POPS LF@param1\n"); 
    printf("POPS LF@param2\n");
    printf("GT LF@result LF@param1 LF@param2\n");
    printf("JUMPIFEQ $strcmp_greater GF@return bool@true\n");
    printf("LT GF@return LF@param1 LF@param2\n");
    printf("JUMPIFEQ $strcmp_less GF@return bool@true\n");
    printf("MOVE GF@return int@0\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("LABEL $strcmp_greater\n");
    printf("MOVE GF@return int@1\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("LABEL $strcmp_less\n");
    printf("MOVE GF@return int@-1\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
}

/**
 * @brief Generates arithmetic operations.
 * @param operator The arithmetic operator (+, -, *, /).
 * @param dest Destination variable.
 * @param op1 First operand.
 * @param op2 Second operand.
 */
void gen_arithmetic(const char *operator, dstring_t *dest, dstring_t *op1, dstring_t *op2) {
    if (strcmp(operator, "+") == 0) {
        printf("ADD LF@%s LF@%s LF@%s\n", dest->data, op1->data, op2->data);
    } else if (strcmp(operator, "-") == 0) {
        printf("SUB LF@%s LF@%s LF@%s\n", dest->data, op1->data, op2->data);
    } else if (strcmp(operator, "*") == 0) {
        printf("MUL LF@%s LF@%s LF@%s\n", dest->data, op1->data, op2->data);
    } else if (strcmp(operator, "/") == 0) {
        printf("DIV LF@%s LF@%s LF@%s\n", dest->data, op1->data, op2->data);
    } else {
        fprintf(stderr, "Unsupported arithmetic operator: %s\n", operator);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Generates relational operations.
 * @param operator The relational operator (==, !=, <, >, <=, >=).
 * @param dest Destination variable.
 * @param op1 First operand.
 * @param op2 Second operand.
 */
void gen_relational(const char *operator, dstring_t *dest, dstring_t *op1, dstring_t *op2) {
    if (strcmp(operator, "==") == 0) {
        printf("EQ LF@%s LF@%s LF@%s\n", dest->data, op1->data, op2->data);
    } else if (strcmp(operator, "!=") == 0) {
        printf("EQ LF@temp LF@%s LF@%s\n", op1->data, op2->data);
        printf("NOT LF@%s LF@temp\n", dest->data);
    } else if (strcmp(operator, "<") == 0) {
        printf("LT LF@%s LF@%s LF@%s\n", dest->data, op1->data, op2->data);
    } else if (strcmp(operator, ">") == 0) {
        printf("GT LF@%s LF@%s LF@%s\n", dest->data, op1->data, op2->data);
    } else if (strcmp(operator, "<=") == 0) {
        printf("GT LF@temp LF@%s LF@%s\n", op1->data, op2->data);
        printf("NOT LF@%s LF@temp\n", dest->data);
    } else if (strcmp(operator, ">=") == 0) {
        printf("LT LF@temp LF@%s LF@%s\n", op1->data, op2->data);
        printf("NOT LF@%s LF@temp\n", dest->data);
    } else {
        fprintf(stderr, "Unsupported relational operator: %s\n", operator);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Generates logical operations.
 * @param operator The logical operator (AND, OR, NOT).
 * @param dest Destination variable.
 * @param op1 First operand.
 * @param op2 Second operand.
 */
void gen_logical(const char *operator, dstring_t *dest, dstring_t *op1, dstring_t *op2) {
    if (strcmp(operator, "AND") == 0) {
        printf("AND LF@%s LF@%s LF@%s\n", dest->data, op1->data, op2->data);
    } else if (strcmp(operator, "OR") == 0) {
        printf("OR LF@%s LF@%s LF@%s\n", dest->data, op1->data, op2->data);
    } else if (strcmp(operator, "NOT") == 0) {
        printf("NOT LF@%s LF@%s\n", dest->data, op1->data);
    } else {
        fprintf(stderr, "Unsupported logical operator: %s\n", operator);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Generates an assignment operation.
 * @param dest Destination variable.
 * @param source Source variable or value.
 */
void gen_assignment(dstring_t *dest, dstring_t *source) {
    printf("MOVE LF@%s LF@%s\n", dest->data, source->data);
}

/**
 * @brief Starts an if block.
 * @details Sets up the condition check and jumps to the else block if false.
 */
void gen_if_start() {
    int label = label_counter++;
    printf("DEFVAR LF@if_cond_%d\n", label); 
    printf("POPS LF@if_cond_%d\n", label);  
    printf("JUMPIFEQ $if_else_%d LF@if_cond_%d bool@false\n", label, label);
    gen_stack_push(if_stack, label);  
}

/**
 * @brief Handles the else part of an if-else block.
 * @details Jumps to the end label and sets up the else block label.
 */
void gen_if_else() {
    if (gen_stack_is_empty(if_stack)) {
        fprintf(stderr, "Error: No active if block to handle else.\n");
        exit(EXIT_FAILURE);
    }
    int current_label = gen_stack_top(if_stack);
    printf("JUMP $if_end_%d\n", current_label);
    printf("LABEL $if_else_%d\n", current_label);
}

/**
 * @brief Ends an if or if-else block.
 * @details Outputs the end label for the if block.
 */
void gen_if_end() {
    if (gen_stack_is_empty(if_stack)) {
        fprintf(stderr, "Error: No active if block to end.\n");
        exit(EXIT_FAILURE);
    }
    int current_label = gen_stack_pop(if_stack);
    printf("LABEL $if_end_%d\n", current_label);
}

/**
 * @brief Starts an if block with nullable handling.
 * @param non_null_id Identifier for the non-null variable.
 */
void gen_if_nullable_start(dstring_t *non_null_id) {
    int current_label = label_counter++;
    printf("DEFVAR LF@nullable_check_%d\n", current_label); 
    printf("POPS LF@nullable_check_%d\n", current_label);  
    printf("JUMPIFEQ $if_nullable_else_%d LF@nullable_check_%d nil@nil\n", current_label, current_label);
    printf("DEFVAR LF@%s\n", non_null_id->data);
    printf("MOVE LF@%s LF@nullable_check_%d\n", dstring_get(non_null_id), current_label); 
    gen_stack_push(if_stack, current_label);
}

/**
 * @brief Handles the else part of a nullable if-else block.
 * @details Jumps to the nullable end label and sets up the else block label.
 */
void gen_if_nullable_else() {
    if (gen_stack_is_empty(if_stack)) {
        fprintf(stderr, "Error: No active nullable if block to handle else.\n");
        exit(EXIT_FAILURE);
    }
    int label = gen_stack_pop(if_stack);
    printf("JUMP $if_nullable_end_%d\n", label);
    printf("LABEL $if_nullable_else_%d\n", label);
    gen_stack_push(if_stack, label);
}

/**
 * @brief Ends a nullable if block.
 * @details Outputs the end label for the nullable if block.
 */
void gen_if_nullable_end() {
    if (gen_stack_is_empty(if_stack)) {
        fprintf(stderr, "Error: No active nullable if block to end.\n");
        exit(EXIT_FAILURE);
    }
    int label = gen_stack_pop(if_stack);
    printf("LABEL $if_nullable_end_%d\n", label);
}

/**
 * @brief Starts a while loop.
 * @details Outputs the start label for the loop.
 */
void gen_while_start() {
    int label = label_counter++;
    printf("DEFVAR LF@while_cond_%d\n", label);
    printf("LABEL $while_start_%d\n", label);
    gen_stack_push(while_stack, label);
}

/**
 * @brief Checks the while loop condition.
 * @details Jumps to the end label if the condition is false.
 */
void gen_while_cond() {
    if (gen_stack_is_empty(while_stack)) {
        fprintf(stderr, "Error: No active while loop to evaluate condition.\n");
        exit(EXIT_FAILURE);
    }
    int current_label = gen_stack_top(while_stack); 
    printf("POPS LF@while_cond_%d\n", current_label);  
    printf("JUMPIFEQ $while_end_%d LF@while_cond_%d bool@false\n", current_label, current_label);
}

/**
 * @brief Ends a while loop.
 * @details Outputs the end label and jumps back to the start label.
 */
void gen_while_end() {
    if (gen_stack_is_empty(while_stack)) {
        fprintf(stderr, "Error: No active while loop to end.\n");
        exit(EXIT_FAILURE);
    }
    int current_label = gen_stack_pop(while_stack);
    printf("JUMP $while_start_%d\n", current_label);
    printf("LABEL $while_end_%d\n", current_label);
}

/**
 * @brief Condition a nullable while loop.
 * @param non_null_id Identifier for the non-null variable.
 */
void gen_while_nullable_cond(dstring_t *non_null_id) {
    int current_label = gen_stack_pop(while_stack);
    // printf("DEFVAR LF@nullable_check\n"); 
    // printf("LABEL $while_nullable_start_%d\n", current_label);                     
    //printf("POPS LF@nullable_check\n");  
    printf("POPS LF@while_cond_%d\n", current_label);                     
    printf("JUMPIFEQ $while_nullable_end_%d LF@while_cond_%d nil@nil\n", current_label, current_label);
    printf("DEFVAR LF@%s\n", non_null_id->data);
    printf("MOVE LF@%s LF@while_cond_%d\n", dstring_get(non_null_id), current_label); 
    gen_stack_push(while_stack, current_label);
}

/**
 * @brief Ends a nullable while loop.
 * @details Outputs the end label and jumps back to the start label.
 */
void gen_while_nullable_end() {
    if (gen_stack_is_empty(while_stack)) {
        fprintf(stderr, "Error: No active nullable while loop to end.\n");
        exit(EXIT_FAILURE);
    }
    int label = gen_stack_pop(while_stack);
    printf("JUMP $while_start_%d\n", label);
    printf("LABEL $while_nullable_end_%d\n", label);
}

/**
 * @brief Starts a function definition.
 * @param func_name Name of the function.
 */
void gen_func_start(dstring_t *func_name) {
    printf("\nLABEL $%s\n", func_name->data); 
    printf("CREATEFRAME\n");
    printf("PUSHFRAME\n");            
}

/**
 * @brief Ends a function definition.
 * @details Outputs the return instruction and pops the frame.
 */

void gen_func_end() {
    printf("POPFRAME\n");              
    printf("RETURN\n");
}

/**
 * @brief Defines a local variable.
 * @param var_name Name of the variable to define.
 */
void gen_defvar(dstring_t *var_name) {
    printf("DEFVAR LF@%s\n", var_name->data);
}

/**
 * @brief Generates a function call.
 * @param func_name Name of the function to call.
 */
void gen_function_call(dstring_t *func_name) {
    if (dstring_compare_charstr(func_name, "ifj.string") == 0) {
        printf("CALL $ifj_string\n");
    } else if (dstring_compare_charstr(func_name, "ifj.write") == 0) {
        printf("CALL $ifj_write\n");
    } else if (dstring_compare_charstr(func_name, "ifj.readi32") == 0) {
        printf("CALL $ifj_readi32\n");
    } else if (dstring_compare_charstr(func_name, "ifj.readstr") == 0) {
        printf("CALL $ifj_readstr\n");
    } else if (dstring_compare_charstr(func_name, "ifj.readf64") == 0) {
        printf("CALL $ifj_readf64\n");
    } else if (dstring_compare_charstr(func_name, "ifj.i2f") == 0) {
        printf("CALL $ifj_i2f\n");
    } else if (dstring_compare_charstr(func_name, "ifj.f2i") == 0) {
        printf("CALL $ifj_f2i\n");
    } else if (dstring_compare_charstr(func_name, "ifj.concat") == 0) {
        printf("CALL $ifj_concat\n");
    } else if (dstring_compare_charstr(func_name, "ifj.length") == 0) {
        printf("CALL $ifj_length\n");
    } else if (dstring_compare_charstr(func_name, "ifj.substring") == 0) {
        printf("CALL $ifj_substring\n");
    } else if (dstring_compare_charstr(func_name, "ifj.strcmp") == 0) {
        printf("CALL $ifj_strcmp\n");
    } else if (dstring_compare_charstr(func_name, "ifj.ord") == 0) {
        printf("CALL $ifj_ord\n");
    } else if (dstring_compare_charstr(func_name, "ifj.chr") == 0) {
        printf("CALL $ifj_chr\n");
    } else {
        printf("CALL $%s\n", func_name->data);
    }
}

/**
 * @brief Generates an unconditional jump.
 * @param label_name Label to jump to.
 */
void gen_jump(const char *label_name) {
    printf("JUMP %s\n", label_name);
}

/**
 * @brief Generates a conditional jump if equal.
 * @param label_name Label to jump to.
 * @param symb1 First operand.
 * @param symb2 Second operand.
 */
void gen_jumpifeq(const char *label_name, dstring_t *symb1, dstring_t *symb2) {
    printf("JUMPIFEQ %s %s %s\n", label_name, symb1->data, symb2->data);
}

/**
 * @brief Generates a conditional jump if not equal.
 * @param label_name Label to jump to.
 * @param symb1 First operand.
 * @param symb2 Second operand.
 */
void gen_jumpifneq(const char *label_name, dstring_t *symb1, dstring_t *symb2) {
    printf("JUMPIFNEQ %s %s %s\n", label_name, symb1->data, symb2->data);
}

/**
 * @brief Pushes an operand onto the stack.
 * @param symbol Symbol to push.
 */
void gen_push_operand(dstring_t *symbol) {
    printf("PUSHS LF@%s\n", symbol->data);
}

/**
 * @brief Pops an operand from the stack.
 * @param var_name Variable to store the popped value. Null to discard.
 */
void gen_pop_operand(dstring_t *var_name) {
    if (var_name) {
        printf("POPS LF@%s\n", var_name->data);  
    } else {
        printf("POPS GF@_discard\n");  
    }
}

/**
 * @brief Generates a return statement.
 * @details Pops the frame and returns to the caller.
 */
void gen_return() {
    printf("POPS GF@return\n");
    printf("POPFRAME\n");
    printf("RETURN\n");
}