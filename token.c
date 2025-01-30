/// IFJ24
/// @brief Implementation of token-related functions for lexical analysis

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"
#include "dstring.h"  

/**
 * @brief Initializes a token and sets its type and attribute.
 * @param type The type of the token.
 * @param attribute The attribute of the token
 * @return A pointer to the initialized token or NULL if memory allocation fails.
 */
token_t *token_init(token_type type, token_attribute *attribute) {
    token_t *token = malloc(sizeof(token_t));

    if (token == NULL) {
        set_error(ERROR_INTERNAL_COMPILER_ERROR, "Memory allocation failed for token", -1, -1);
        return NULL;
    }
    
    token->type = type;

    if (attribute != NULL) {
        token->attribute = *attribute; 
    } else {
        memset(&token->attribute, 0, sizeof(token_attribute));
    }

    return token;
}

/**
 * @brief Frees the memory allocated for the token.
 * @param token A pointer to the token to be freed.
 */
void token_free(token_t* token) {
    if (token == NULL) return;
    if (token->type == TOKEN_STRING && token->attribute.s != NULL) { 
        dstring_free(token->attribute.s);
        token->attribute.s = NULL;
    }

    free(token); 
}

/**
 * @brief Retrieves the attribute of the token.
 * @param token A pointer to the token from which to retrieve the attribute.
 * @return The token's attribute.
 */
token_attribute token_get_attribute(token_t* token) {
    return token->attribute;
}

/**
 * @brief Retrieves the type of the token.
 * @param token A pointer to the token from which to retrieve the type.
 * @return The type of the token.
 */
token_type token_get_type(token_t* token) {
    return token->type;
}

/**
 * @brief Creates a deep copy of a token.
 * @param original Pointer to the original token to copy.
 * @return Pointer to the copied token, or NULL on failure.
 */
token_t *token_copy(const token_t *original) {
    if (!original) return NULL;

    token_t *copy = malloc(sizeof(token_t));
    if (!copy) {
        set_error(ERROR_INTERNAL_COMPILER_ERROR, "Failed to allocate memory for token copy", 0, 0);
        return NULL;
    }

    copy->type = original->type;
    switch (original->type) {
        case TOKEN_STRING:
            if (original->attribute.s) {
                copy->attribute.s = dstring_copy(original->attribute.s);
                if (!copy->attribute.s) {
                    free(copy);
                    set_error(ERROR_INTERNAL_COMPILER_ERROR, "Failed to copy string attribute", 0, 0);
                    return NULL;
                }
            } else {
                copy->attribute.s = NULL; 
            }
            break;

        case TOKEN_INT:
            copy->attribute.i = original->attribute.i;
            break;

        case TOKEN_FLOAT:
            copy->attribute.f = original->attribute.f;
            break;

        default:
            copy->attribute = original->attribute;
            break;
    }

    return copy;
}
