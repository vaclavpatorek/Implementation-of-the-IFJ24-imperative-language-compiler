/// IFJ24
/// @brief Header for implementation of the symbol table (hash table) for the IFJ24 compiler.

#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "dstring.h"  
#include "token.h"   
#include <stdbool.h>
#include <stdlib.h>

#define TABLE_SIZE 1024

typedef enum {
    var_t,
    const_t,
    fn_t,
} symbol_type;

typedef enum {
    int_type,
    float_type,
    string_type,
    bool_type,
    null_int_type,
    null_float_type,
    null_string_type,
    null_type,
    void_type,
    null,
} data_type;

typedef struct {
    dstring_t *name;
    bool isDefined;
    int paramCount;
    int paramCapacity;
    data_type *params;  
    data_type returnType;
} func_data_t;

typedef struct {
    dstring_t *name;
    data_type type;
    bool isUsed;
} var_data_t;

typedef struct {
    symbol_type type;
    func_data_t *funcData;
    var_data_t *varData;
} symtable_data_t;

typedef struct symtable_item {
    dstring_t *key;            
    symtable_data_t *value; 
    int scope_level;            
    struct symtable_item *next; 
} symtable_item_t;

typedef struct symtable {
    symtable_item_t **items;   
    size_t size;                
    int scope_level;  
} symtable_t;

symtable_t *symtable_create(size_t size);
int symtable_insert_function(symtable_t *table, dstring_t *name, func_data_t *func_data, int scope_level);
int func_data_add_param(func_data_t *func_data, data_type paramType);
int symtable_insert_variable(symtable_t *table, dstring_t *name, var_data_t *var_data, int scope_level, bool isConst);
symtable_data_t *symtable_find(symtable_t *table, dstring_t *key);
symtable_data_t *symtable_find_in_scope(symtable_t *table, dstring_t *key, int scope_level);
void symtable_enter_scope(symtable_t *table);
int check_unused_variables_in_scope(symtable_t *table);
void symtable_exit_scope(symtable_t *table);
void symtable_free(symtable_t *table);
void debug_symtable(symtable_t *table);

#endif 
