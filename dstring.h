/// IFJ24
/// @brief Header file for dstring.c

#ifndef DSTRING_H
#define DSTRING_H

#define STRING_INIT_SIZE 8

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error_codes.h"

typedef struct {
    char *data;
    size_t length;
    size_t capacity;
} dstring_t;

dstring_t* dstring_init(void);
void dstring_free(dstring_t *dstring);
void dstring_clear(dstring_t *dstring);
int dstring_extend(dstring_t *dstring, size_t required_size);
char* dstring_get(dstring_t *dstring);
int dstring_length(dstring_t *dstring);
int dstring_compare(dstring_t *dstring, dstring_t *dstring2);
int dstring_compare_charstr(dstring_t *string1, char *string2);
int dstring_add_char(dstring_t *string, char c);
int dstring_add_dstring(dstring_t *dest, dstring_t *src);
int dstring_add_str(dstring_t *dest, char *src);
int dstring_assign(dstring_t *dest, dstring_t *src);
dstring_t *dstring_copy(dstring_t *src);
int dstring_get_char_index(dstring_t *string, char c);

#endif
