/// IFJ24
/// @brief Implementation of the symbol table (hash table) for the IFJ24 compiler.

#include "symtable.h"
#include <string.h>

/**
 * @brief Hash function to compute an index for the given key.
 * @param key Pointer to the key.
 * @return The hash value .
 */
static size_t hash_function(const dstring_t *key) {
    size_t hash = 0;
    for (size_t i = 0; i < key->length; ++i) {
        hash = hash * 31 + key->data[i];
    }
    return hash % TABLE_SIZE; 
}

/**
 * @brief Adds built-in functions to the symbol table.
 * @param table Pointer to the symbol table.
 * @return 0 on success, -1 on failure.
 */
int built_in_func(symtable_t *table){
    func_data_t *readstr = malloc(sizeof(func_data_t));
    readstr->name = dstring_init();
    dstring_add_str(readstr->name, "ifj.readstr");
    readstr->isDefined = true;
    readstr->paramCount = 0;  
    readstr->params = NULL;  
    readstr->returnType = null_string_type; 
    symtable_insert_function(table, readstr->name, readstr, 0);

    func_data_t *readi32 = malloc(sizeof(func_data_t));
    readi32->name = dstring_init();
    dstring_add_str(readi32->name, "ifj.readi32");
    readi32->isDefined = true;
    readi32->paramCount = 0;  
    readi32->params = NULL;  
    readi32->returnType = null_int_type; 
    symtable_insert_function(table, readi32->name, readi32, 0);

    func_data_t *readf64 = malloc(sizeof(func_data_t));
    readf64->name = dstring_init();
    dstring_add_str(readf64->name, "ifj.readf64");
    readf64->isDefined = true;
    readf64->paramCount = 0;  
    readf64->params = NULL;  
    readf64->returnType = null_float_type; 
    symtable_insert_function(table, readf64->name, readf64, 0);

    func_data_t *write = malloc(sizeof(func_data_t));
    write->name = dstring_init();
    dstring_add_str(write->name, "ifj.write");
    write->isDefined = true;
    write->paramCount = 1;  
    write->params = malloc(write->paramCount * sizeof(data_type));
    if (!write->params) {
        dstring_free(write->name);
        free(write);
        return -1;
    };  
    write->params[0] = void_type; 
    write->returnType = void_type; 
    symtable_insert_function(table, write->name, write, 0);

    func_data_t *i2f = malloc(sizeof(func_data_t));
    i2f->name = dstring_init();
    dstring_add_str(i2f->name, "ifj.i2f");
    i2f->isDefined = true;
    i2f->paramCount = 1;  
    i2f->params = malloc(i2f->paramCount * sizeof(data_type));
    if (!i2f->params) {
        dstring_free(i2f->name);
        free(i2f);
        return -1;
    };  
    i2f->params[0] = int_type;  
    i2f->returnType = float_type; 
    symtable_insert_function(table, i2f->name, i2f, 0);

    func_data_t *f2i = malloc(sizeof(func_data_t));
    f2i->name = dstring_init(); 
    dstring_add_str(f2i->name, "ifj.f2i");
    f2i->isDefined = true;
    f2i->paramCount = 1;  
    f2i->params = malloc(f2i->paramCount * sizeof(data_type));
    if (!f2i->params) {
        dstring_free(f2i->name);
        free(f2i);
        return -1;
    };  
    f2i->params[0] = float_type;  
    f2i->returnType = int_type; 
    symtable_insert_function(table, f2i->name, f2i, 0);

    func_data_t *string = malloc(sizeof(func_data_t));
    string->name = dstring_init(); 
    dstring_add_str(string->name, "ifj.string");
    string->isDefined = true;
    string->paramCount = 1;  
    string->params = malloc(string->paramCount * sizeof(data_type));
    if (!string->params) {
        dstring_free(string->name);
        free(string);
        return -1;
    };  
    string->params[0] = void_type; 
    string->returnType = string_type; 
    symtable_insert_function(table, string->name, string, 0);

    func_data_t *length = malloc(sizeof(func_data_t));
    length->name = dstring_init();
    dstring_add_str(length->name, "ifj.length");
    length->isDefined = true;
    length->paramCount = 1;  
    length->params = malloc(length->paramCount * sizeof(data_type));
    if (!length->params) {
        dstring_free(length->name);
        free(length);
        return -1;
    };  
    length->params[0] = string_type;   
    length->returnType = int_type; 
    symtable_insert_function(table, length->name, length, 0);

    func_data_t *concat = malloc(sizeof(func_data_t));
    concat->name = dstring_init();
    dstring_add_str(concat->name, "ifj.concat");
    concat->isDefined = true;
    concat->paramCount = 2;  
    concat->params = malloc(concat->paramCount * sizeof(data_type));
    if (!concat->params) {
        dstring_free(concat->name);
        free(concat);
        return -1;
    };  
    concat->params[0] = string_type;  
    concat->params[1] = string_type;
    concat->returnType = string_type; 
    symtable_insert_function(table, concat->name, concat, 0);

    func_data_t *substring = malloc(sizeof(func_data_t));
    substring->name = dstring_init();
    dstring_add_str(substring->name, "ifj.substring");
    substring->isDefined = true;
    substring->paramCount = 3;  
    substring->params = malloc(substring->paramCount * sizeof(data_type));
    if (!substring->params) {
        dstring_free(substring->name);
        free(substring);
        return -1;
    };  
    substring->params[0] = string_type;
    substring->params[1] = int_type; 
    substring->params[2] = int_type;   
    substring->returnType = null_string_type; 
    symtable_insert_function(table, substring->name, substring, 0);

    func_data_t *strcmp = malloc(sizeof(func_data_t));
    strcmp->name = dstring_init(); 
    dstring_add_str(strcmp->name, "ifj.strcmp");
    strcmp->isDefined = true;
    strcmp->paramCount = 2;  
    strcmp->params = malloc(strcmp->paramCount * sizeof(data_type));
    if (!strcmp->params) {
        dstring_free(strcmp->name);
        free(strcmp);
        return -1;
    };  
    strcmp->params[0] = string_type;  
    strcmp->params[1] = string_type;
    strcmp->returnType = int_type; 
    symtable_insert_function(table, strcmp->name, strcmp, 0);

    func_data_t *ord = malloc(sizeof(func_data_t));
    ord->name = dstring_init(); 
    dstring_add_str(ord->name, "ifj.ord");
    ord->isDefined = true;
    ord->paramCount = 2;  
    ord->params = malloc(ord->paramCount * sizeof(data_type));
    if (!ord->params) {
        dstring_free(ord->name);
        free(ord);
        return -1;
    };  
    ord->params[0] = string_type;
    ord->params[1] = int_type;  
    ord->returnType = int_type; 
    symtable_insert_function(table, ord->name, ord, 0);

    func_data_t *chr = malloc(sizeof(func_data_t));
    chr->name = dstring_init(); 
    dstring_add_str(chr->name, "ifj.chr");
    chr->isDefined = true;
    chr->paramCount = 1;  
    chr->params = malloc(chr->paramCount * sizeof(data_type));
    if (!chr->params) {
        dstring_free(chr->name);
        free(chr);
        return -1;
    };  
    chr->params[0] = int_type;   
    chr->returnType = string_type; 
    symtable_insert_function(table, chr->name, chr, 0);

    return 0;
}

/**
 * @brief Creates a new symbol table.
 * @param size The number of buckets for the hash table.
 * @return Pointer to the newly created symbol table, or NULL on failure.
 */
symtable_t *symtable_create(size_t size) {
    symtable_t *table = malloc(sizeof(symtable_t));
    if (table == NULL) {
        return NULL;
    }
    table->items = calloc(size, sizeof(symtable_item_t*)); 
    if (table->items == NULL) {
        free(table);
        return NULL;
    }
    table->size = size;
    table->scope_level = 0;
    built_in_func(table);
    return table;
}

/**
 * @brief Inserts a function into the symbol table.
 * @param table Pointer to the symbol table.
 * @param name The function name.
 * @param func_data Pointer to the function data structure.
 * @param scope_level Scope level for the function.
 * @return 0 on success, -1 on failure.
 */
int symtable_insert_function(symtable_t *table, dstring_t *name, func_data_t *func_data, int scope_level) {
    size_t index = hash_function(name);
    symtable_item_t *new_item = (symtable_item_t *)malloc(sizeof(symtable_item_t));
    if (!new_item) return -1;

    new_item->key = dstring_copy(name);
    new_item->value = (symtable_data_t *)malloc(sizeof(symtable_data_t));
    if (!new_item->value) {
        free(new_item);
        return -1;
    }

    new_item->value->type = fn_t;
    new_item->value->funcData = func_data;
    new_item->value->varData = NULL;
    new_item->scope_level = scope_level;
    new_item->next = table->items[index];
    table->items[index] = new_item;

    return 0;
}

/**
 * @brief Adds a parameter to the given function's parameter list.
 * 
 * @param func_data Pointer to the function data.
 * @param paramType The data type of the parameter to add.
 * @return 0 on success, -1 on failure.
 */
int func_data_add_param(func_data_t *func_data, data_type paramType) {
    if (func_data->paramCount == func_data->paramCapacity) {
        func_data->paramCapacity *= 2;
        data_type *newArray = realloc(func_data->params, func_data->paramCapacity * sizeof(data_type));
        if (!newArray) return -1; 
        func_data->params = newArray;
    }

    func_data->params[func_data->paramCount++] = paramType;
    return 0;
}

/**
 * @brief Inserts a variable into the symbol table.
 * @param table Pointer to the symbol table.
 * @param name The variable name.
 * @param var_data Pointer to the variable data structure.
 * @param scope_level Scope level for the variable.
 * @param isConst True if the variable is a constant, false otherwise.
 * @return 0 on success, -1 on failure.
 */
int symtable_insert_variable(symtable_t *table, dstring_t *name, var_data_t *var_data, int scope_level, bool isConst) {
    size_t index = hash_function(name);
    symtable_item_t *new_item = (symtable_item_t *)malloc(sizeof(symtable_item_t));
    if (!new_item) return -1;

    new_item->key = dstring_copy(name);
    new_item->value = (symtable_data_t *)malloc(sizeof(symtable_data_t));
    if (!new_item->value) {
        free(new_item);
        return -1;
    }

    if(isConst){
        new_item->value->type = const_t;
    } else{
        new_item->value->type = var_t;
    }
    new_item->value->varData = var_data;
    new_item->value->funcData = NULL;
    new_item->scope_level = scope_level;
    new_item->next = table->items[index];
    table->items[index] = new_item;

    return 0;
}

/**
 * @brief Searches for a symbol in the table by its key.
 * @param table Pointer to the symbol table.
 * @param key The symbol's key.
 * @return Pointer to the symbol table data if found, NULL otherwise.
 */
symtable_data_t *symtable_find(symtable_t *table, dstring_t *key) {
    size_t index = hash_function(key);
    symtable_item_t *item = table->items[index];
    while (item) {
        if (dstring_compare(item->key, key) == 0) {
            return item->value;
        }
        item = item->next;
    }

    return NULL; 
}

/**
 * @brief Searches for a symbol in the table by its key and scope.
 * @param table Pointer to the symbol table.
 * @param key The symbol's key.
 * @param scope_level Scope level.
 * @return Pointer to the symbol table data if found, NULL otherwise.
 */
symtable_data_t *symtable_find_in_scope(symtable_t *table, dstring_t *key, int scope_level) {
    size_t index = hash_function(key);
    symtable_item_t *item = table->items[index];
    while (item) {
        if (dstring_compare(item->key, key) == 0 && item->scope_level == scope_level) {
            return item->value;
        }
        item = item->next;
    }
    return NULL; 
}

/**
 * @brief Enters a new scope by incrementing the scope level.
 * @param table Pointer to the symbol table.
 */
void symtable_enter_scope(symtable_t *table) {
    table->scope_level++;
}

/**
 * @brief Checks for unused variables in the current scope.
 * @param table Pointer to the symbol table.
 * @return ERROR_SEMANTIC_UNUSED_VARIABLE if an unused variable is found, 0 otherwise.
 */
int check_unused_variables_in_scope(symtable_t *table) {
    for (size_t i = 0; i < table->size; i++) {
        symtable_item_t *item = table->items[i];
        while (item) {
            if (item->scope_level == table->scope_level && 
                (item->value->type == var_t || item->value->type == const_t) && 
                item->value->varData && 
                !item->value->varData->isUsed) {
                return ERROR_SEMANTIC_UNUSED_VARIABLE;
            }
            item = item->next;
        }
    }
    return 0; 
}

/**
 * @brief Exits the current scope by decrementing the scope level and removing its symbols.
 * @param table Pointer to the symbol table.
 */
void symtable_exit_scope(symtable_t *table) {
    for (size_t i = 0; i < table->size; i++) {
        symtable_item_t *prev = NULL;
        symtable_item_t *item = table->items[i];

        while (item) {
            if (item->scope_level == table->scope_level) {
                if (prev) {
                    prev->next = item->next;
                } else {
                    table->items[i] = item->next;
                }

                symtable_item_t *temp = item;
                item = item->next;

                if (temp->key) {
                    dstring_free(temp->key);
                    temp->key = NULL;
                }

                if (temp->value) {
                    if (temp->value->funcData) {
                        if (temp->value->funcData->name) {
                            dstring_free(temp->value->funcData->name);
                            temp->value->funcData->name = NULL;
                        }
                        if (temp->value->funcData->params) {
                            free(temp->value->funcData->params);
                            temp->value->funcData->params = NULL;
                        }
                        free(temp->value->funcData);
                        temp->value->funcData = NULL;
                    }
                    if (temp->value->varData) {
                        free(temp->value->varData);
                        temp->value->varData = NULL;
                    }
                    free(temp->value);
                    temp->value = NULL;
                }
                free(temp);
                temp = NULL;
            } else {
                prev = item;
                item = item->next;
            }
        }
    }

    table->scope_level--;
}

/**
 * @brief Frees all memory associated with the symbol table.
 * @param table Pointer to the symbol table.
 */
void symtable_free(symtable_t *table) {
    if (!table) return;

    for (size_t i = 0; i < table->size; i++) {
        symtable_item_t *item = table->items[i];
        while (item) {
            symtable_item_t *temp = item;
            item = item->next;

            if (temp->key) {
                dstring_free(temp->key);
                temp->key = NULL;
            }

            if (temp->value) {
                if (temp->value->funcData) {
                    if (temp->value->funcData->name) {
                        dstring_free(temp->value->funcData->name);
                        temp->value->funcData->name = NULL;
                    }
                    if (temp->value->funcData->params) {
                        free(temp->value->funcData->params);
                        temp->value->funcData->params = NULL;
                    }
                    free(temp->value->funcData);
                    temp->value->funcData = NULL;
                }
                if (temp->value->varData) {
                    free(temp->value->varData);
                    temp->value->varData = NULL;
                }
                free(temp->value);
                temp->value = NULL;
            }
            free(temp);
            temp = NULL;
        }
        table->items[i] = NULL;
    }
    free(table->items);
    table->items = NULL;
    free(table);
}


const char *data_type_to_string(data_type type) {
    switch (type) {
        case int_type: return "int";
        case float_type: return "float";
        case string_type: return "string";
        case bool_type: return "bool";
        case null_type: return "null";
        case void_type: return "void";
        default: return "unknown";
    }
}

void debug_symtable(symtable_t *table) {
    if (!table) {
        printf("Symbol table is NULL.\n");
        return;
    }

    printf("----- Symbol Table Debug -----\n");
    printf("Number of buckets: %ld\n", table->size);
    printf("Current scope level: %d\n", table->scope_level);

    for (size_t i = 0; i < table->size; i++) {
        symtable_item_t *item = table->items[i];
        if (item) {
            printf("Bucket [%d]:\n", i);
        }
        while (item) {
            printf("  Key: %s\n", item->key->data);

            if (item->value->type == fn_t) {
                func_data_t *func_data = item->value->funcData;
                printf("    Function: %s\n", func_data->name->data);
                printf("    Is Defined: %s\n", func_data->isDefined ? "Yes" : "No");
                printf("    Parameter Count: %d\n", func_data->paramCount);
                printf("    Parameters: ");
                for (int j = 0; j < func_data->paramCount; j++) {
                    printf("%s ", data_type_to_string(func_data->params[j]));
                }
                printf("\n");
                printf("    Return Type: %s\n", data_type_to_string(func_data->returnType));
            } else if (item->value->type == var_t) {
                printf("    Variable\n");
            } else if (item->value->type == const_t) {
                printf("    Constant\n");
            }

            item = item->next;
        }
    }
    printf("----- End of Symbol Table -----\n");
}