/**
 * IFJ24
 * @brief Implementation of a precedence parsing stack for the IFJ24 compiler.
 */

#include "prec_stack.h"
#include <stdio.h>  

/**
 * @brief Initializes the stack to an empty state.
 * @param stack Pointer to the stack to initialize.
 */
void stack_init(Stack *stack) {
    stack->top = NULL;
}

/**
 * @brief Checks if the stack is empty.
 * @param stack Pointer to the stack to check.
 * @return True if the stack is empty, false otherwise.
 */
bool stack_is_empty(const Stack *stack) {
    return stack->top == NULL;
}

/**
 * @brief Pushes a new element onto the stack.
 * @param stack Pointer to the stack.
 * @param data Pointer to the data to be pushed.
 * @return True on success, false on memory allocation failure.
 */
bool stack_push(Stack *stack, StackData *data) {
    StackElement *new_element = malloc(sizeof(StackElement));
    if (!new_element) {
        set_error(ERROR_INTERNAL_COMPILER_ERROR, "Memory allocation failed", -1, -1);
        return false;
    }

    new_element->data = malloc(sizeof(StackData));
    if (!new_element->data) {
        free(new_element);
        set_error(ERROR_INTERNAL_COMPILER_ERROR, "Memory allocation failed", -1, -1);
        return false;
    }

    *(new_element->data) = *data;
    if (data->token) {
        new_element->data->token = malloc(sizeof(token_t));
        if (!new_element->data->token) {
            free(new_element->data);
            free(new_element);
            set_error(ERROR_INTERNAL_COMPILER_ERROR, "Failed to allocate memory for token copy", 0, 0);
            return false;
        }

        *(new_element->data->token) = *(data->token); 

        if (data->token->type == TOKEN_STRING && data->token->attribute.s) {
            new_element->data->token->attribute.s = dstring_copy(data->token->attribute.s);
            if (!new_element->data->token->attribute.s) {
                free(new_element->data->token);
                free(new_element->data);
                free(new_element);
                set_error(ERROR_INTERNAL_COMPILER_ERROR, "Failed to copy string attribute in token", 0, 0);
                return false;
            }
        }
    } else {
        new_element->data->token = NULL; 
    }

    new_element->next = stack->top;
    stack->top = new_element;

    return true;
}


/**
 * @brief Pops the top element from the stack and frees its memory.
 * @param stack Pointer to the stack.
 */
void stack_pop(Stack *stack) {
    if (stack_is_empty(stack)) {
        return;
    }

    StackElement *element_to_pop = stack->top;
    StackData *data = element_to_pop->data;

    if (data->token) {
        token_free(data->token);
    }

    free(data);
    stack->top = element_to_pop->next;

    free(element_to_pop);
}

/**
 * @brief Retrieves the top element from the stack without removing it.
 * @param stack Pointer to the stack.
 * @return Pointer to the top StackData, or NULL if the stack is empty.
 */
StackData *stack_top(const Stack *stack) {
    if (stack_is_empty(stack)) {
        printf("Error: Stack is empty in stack_peek.\n");
        return NULL;
    }
    return stack->top->data;
}

/**
 * @brief Retrieves the nth element from the top of the stack.
 * @param stack Pointer to the stack.
 * @param n Index of the element to peek.
 * @return Pointer to the nth StackData, or NULL if out of bounds.
 */
StackData* stack_peek_n(Stack *stack, int n) {
    StackElement *element = stack->top;
    for (int i = 0; i < n && element != NULL; i++) {
        element = element->next;
    }
    return (element != NULL) ? element->data : NULL;
}

/**
 * @brief Destroys the stack and frees all its allocated memory.
 * @param stack Pointer to the stack.
 */
void stack_destroy(Stack *stack) {
    while (!stack_is_empty(stack)) {
        stack_pop(stack); 
    }
}

/**
 * @brief Prints the contents of the stack for debugging purposes.
 * @param stack Pointer to the stack.
 */
void print_stack(Stack *stack) {
    StackElement *current = stack->top;
    printf("Stack: ");
    while (current) {
        printf("[%d (%d)] ", current->data->symbol, current->data->type);
        current = current->next;
    }
    printf("\n");
}