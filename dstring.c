/// IFJ24
/// @brief String for scanner implementation


#include "dstring.h"

/**
 * @brief Initializes a dynamic string.
 * @return A pointer to the initialized dynamic string, or NULL if memory allocation fails.
 */
dstring_t* dstring_init(void) {
    dstring_t* str = malloc(sizeof(dstring_t));
    
    if (str == NULL) {
        set_error(ERROR_INTERNAL_COMPILER_ERROR, "Memory allocation failed for dynamic string structure", -1, -1);
        return NULL;
    }

    str->data = malloc(sizeof(char) * STRING_INIT_SIZE);
    
    if (str->data == NULL) {
        free(str); 
        set_error(ERROR_INTERNAL_COMPILER_ERROR, "Memory allocation failed for dynamic string data", -1, -1);
        return NULL;
    }
    
    str->data[0] = '\0';
    str->capacity = STRING_INIT_SIZE;
    str->length = 0;
    return str;
}

/**
 * @brief Frees the dynamic string.
 * @param dstring Pointer to the dynamic string.
 */
void dstring_free(dstring_t *dstring) {
    if (dstring == NULL) return;
    if (dstring->data) {
        free(dstring->data); 
        dstring->data = NULL;
    }
    free(dstring);
}

/**
 * @brief Clears the content of the dynamic string.
 * @param dstring Pointer to the dynamic string.
 */
void dstring_clear(dstring_t *dstring) {
    dstring->length = 0;
    dstring->data[0] = '\0';
}

/**
 * @brief Extends the capacity of the string.
 * @param dstring Pointer to the dynamic string.
 * @return 0 on success, -1 on failure.
 */
int dstring_extend(dstring_t *dstring, size_t required_size) {
    size_t new_capacity = dstring->capacity;
    while (new_capacity < required_size) {
        new_capacity *= 2;
    }

    char *new_data = realloc(dstring->data, new_capacity + 1); 
    if (!new_data) {
        return -1; 
    }

    dstring->data = new_data;
    dstring->capacity = new_capacity;
    return 0;
}


/**
 * @brief Returns a pointer to the character array of the dynamic string.
 * @param dstring Pointer to the dynamic string.
 * @return A pointer to the character array (string data).
 */
char* dstring_get(dstring_t *dstring) {
    return dstring->data;
}

/**
 * @brief Returns the length of the dynamic string.
 * @param dstring Pointer to the dynamic string.
 * @return The length of the string.
 */
int dstring_length(dstring_t *dstring) {
    return dstring->length;
}

/**
 * @brief Compares two dynamic strings.
 * @param string1 First dynamic string to compare.
 * @param string2 Second dynamic string to compare.
 * @return 0 if the strings are equal, non-zero otherwise.
 */
int dstring_compare(dstring_t *string1, dstring_t *string2) {
    int tmp = strcmp(string1->data, string2->data);
    return tmp;
}

/**
 * @brief Compares the content of a dynamic string with a character array.
 * @param string1 Pointer to the dynamic string.
 * @param string2 Pointer to the character array.
 * @return 0 if the strings are equal, non-zero otherwise.
 */
int dstring_compare_charstr(dstring_t *string1, char *string2) {
    return strcmp(string1->data, string2);
}

/**
 * @brief Adds a character to the dynamic string.
 * @param string Pointer to the dynamic string.
 * @param c Character to add.
 * @return 0 on success, -1 on failure.
 */
int dstring_add_char(dstring_t *string, char c) {
    if (string->length + 1 >= string->capacity) {
        if (dstring_extend(string, string->length + 1) != 0) {
            return -1; 
        }
    }
    string->data[string->length] = c;          
    string->data[++string->length] = '\0';    
    return 0;
}


/**
 * @brief Appends one dynamic string to another.
 * @param dest Destination dynamic string.
 * @param src Source dynamic string.
 * @return 0 on success, -1 on failure.
 */
int dstring_add_dstring(dstring_t *dest, dstring_t *src) {
    if (dest->length + src->length >= dest->capacity) {
        if (dstring_extend(dest, dest->length + src->length) != 0) {
            return -1; 
        }
    }

    memcpy(dest->data + dest->length, src->data, src->length);
    dest->length += src->length;             
    dest->data[dest->length] = '\0';        
    return 0;
}


/**
 * @brief Appends a character array to the end of a dynamic string.
 * @param dest Pointer to the destination dynamic string.
 * @param src Pointer to the character array.
 * @return 0 on success, -1 on failure.
 */
int dstring_add_str(dstring_t *dest, char *src) {
    size_t src_length = strlen(src);
    if (dest->length + src_length >= dest->capacity) {
        if (dstring_extend(dest, dest->length + src_length) != 0) {
            return -1; 
        }
    }
    memcpy(dest->data + dest->length, src, src_length + 1);
    dest->length += src_length;
    return 0;
}

/**
 * @brief Assigns the content of one dynamic string to another.
 * @param dest Destination dynamic string.
 * @param src Source dynamic string.
 * @return 0 on success, -1 on failure.
 */
int dstring_assign(dstring_t *dest, dstring_t *src) {
    if (dest->capacity < src->length) {
        if (dstring_extend(dest, src->length) != 0) {
            return -1; 
        }
    }
    strcpy(dest->data, src->data); 
    dest->length = src->length;
    return 0;
}

/**
 * @brief Creates a copy of a dynamic string.
 * @param src Source dynamic string.
 * @return A pointer to the new copied dynamic string, or NULL on failure.
 */
dstring_t *dstring_copy(dstring_t *src) {
    if (!src) return NULL; 

    dstring_t *dest = dstring_init();
    if (!dest) return NULL; 

    if (dstring_assign(dest, src) != 0) {
        dstring_free(dest);
        return NULL;
    }
    return dest;
}

/**
 * @brief Finds the first occurrence of a character in the dynamic string.
 * @param string Dynamic string to search.
 * @param c Character to search for.
 * @return Index of the character if found, -1 if not found.
 */
int dstring_get_char_index(dstring_t *string, char c) {
    int count = 0;
    for (size_t i = 0; i < string->length; i++) {
        if (string->data[i] == c) {
            count++;
        }
    }
    return -1;
}
