/**
 * IFJ24
 * @brief Header for implementation of a precedence parsing stack for the IFJ24 compiler.
 */

#ifndef PREC_STACK_H
#define PREC_STACK_H

#include <stdbool.h> 
#include "symtable.h"
#include "prec_sym_types.h"

#define STACK_SIZE 100


typedef struct StackData {
    int symbol;              
    data_type type;          
    bool is_literal;    
    token_t *token;            
} StackData;

typedef struct StackElement {
    StackData *data;                  
    struct StackElement *next;   
} StackElement;

typedef struct {
    StackElement *top;          
} Stack;


void stack_init(Stack* stack);
bool stack_is_empty(const Stack* stack);
bool stack_is_empty(const Stack* stack);
bool stack_push(Stack *stack, StackData *data);
void stack_pop(Stack* stack);
StackData *stack_top(const Stack *stack);
StackData* stack_peek_n(Stack *stack, int n);
void stack_destroy(Stack *stack);

#endif
