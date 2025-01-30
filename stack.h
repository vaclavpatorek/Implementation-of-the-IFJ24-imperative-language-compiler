/**
 * IFJ24
 * @brief Header for implementation of a simple stack for generator.
 */

#ifndef STACK_H
#define STACK_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define STACK_MAX 256

typedef struct {
    int data[STACK_MAX]; 
    int top;             
} genStack;


void gen_stack_init(genStack *stack);
bool gen_stack_is_empty(genStack *stack);
bool gen_stack_is_full(genStack *stack);
bool gen_stack_push(genStack *stack, int value);
int gen_stack_pop(genStack *stack);
int gen_stack_top(genStack *stack);
void gen_stack_clear(genStack *stack);

#endif 
