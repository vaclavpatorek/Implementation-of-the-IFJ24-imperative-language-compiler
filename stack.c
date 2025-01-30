/**
 * IFJ24
 * @brief Implementation of a simple stack for generator.
 */

#include "stack.h"

/**
 * @brief Initializes a generic stack to an empty state.
 * @param stack Pointer to the stack to initialize.
 */
void gen_stack_init(genStack *stack) {
    if (!stack) {
        fprintf(stderr, "Error: genStack pointer is null.\n");
    }
    stack->top = 0; 
}

/**
 * @brief Checks if the stack is empty.
 * @param stack Pointer to the stack to check.
 * @return True if the stack is empty, false otherwise.
 */
bool gen_stack_is_empty(genStack *stack) {
    if (!stack) {
        fprintf(stderr, "Error: Stack pointer is null.\n");
        return true;
    }
    return stack->top == 0;
}

/**
 * @brief Checks if the stack is full.
 * @param stack Pointer to the stack to check.
 * @return True if the stack is full, false otherwise.
 */
bool gen_stack_is_full(genStack *stack) {
    if (!stack) {
        fprintf(stderr, "Error: Stack pointer is null.\n");
        return true; 
    }
    return stack->top == STACK_MAX - 1;
}

/**
 * @brief Pushes a value onto the stack.
 * @param stack Pointer to the stack.
 * @param value The value to push onto the stack.
 * @return True if the value is successfully pushed, false if the stack is full.
 */
bool gen_stack_push(genStack *stack, int value) {
    if (gen_stack_is_full(stack)) {
        fprintf(stderr, "Stack overflow\n");
        return false;
    }
    stack->data[++stack->top] = value;
    return true;
}

/**
 * @brief Pops the top value from the stack.
 * @param stack Pointer to the stack.
 * @return The value at the top of the stack.
 */
int gen_stack_pop(genStack *stack) {
    if (gen_stack_is_empty(stack)) {
        fprintf(stderr, "Stack underflow\n");
        exit(EXIT_FAILURE);
    }
    return stack->data[stack->top--]; 
}

/**
 * @brief Retrieves the top value of the stack without popping it.
 * @param stack Pointer to the stack.
 * @return The value at the top of the stack.
 */
int gen_stack_top(genStack *stack) {
    if (gen_stack_is_empty(stack)) {
        fprintf(stderr, "Stack is empty\n");
        exit(EXIT_FAILURE);
    }
    return stack->data[stack->top];
}
