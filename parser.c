/// IFJ24
/// @brief Parser implementation

#include "parser.h"

symtable_t *symbol_table;
token_t *current_token;
func_data_t *current_function;
FILE *file;
bool first_control = true;
bool has_return = false;

/**
 * @brief Fetches the next token from the input source.
 * @return 0 on success, non-zero error code on failure.
 */
int fetch_next_token() {
    int result = get_next_token(file, &current_token);
    if (result != 0) {
        error_exit(result, "Failed to fetch the next token");
    }
    return result;
}

/**
 * @brief Handles critical errors and performs cleanup before exiting the program.
 * @param exit_code The error code to return on program exit.
 * @param message The error message to display or log.
 */
void error_exit(int exit_code, char *message) {
    if (symbol_table != NULL) {
        symtable_free(symbol_table);
        symbol_table = NULL;
    }
    if (current_token != NULL) {
        token_free(current_token);
        current_token = NULL;
    }

    generator_cleanup();

    set_error(exit_code, message, 0, 0);

    fclose(file);
    cleanup_temp_file();

    exit(exit_code);
}

/**
 * @brief Checks if two data types are compatible.
 * @param first_type The first data type to compare.
 * @param second_type The second data type to compare.
 * @return True if the types are compatible, false otherwise.
 */
bool is_type_compatible(data_type first_type, data_type second_type) {
    if (first_type == second_type) {
        return true;
    }

    if ((first_type == null_int_type && second_type == int_type) ||
        (first_type == null_float_type && second_type == float_type) ||
        (first_type == null_string_type && second_type == string_type)) {
        return true;
    }

    if ((first_type == null_int_type && second_type == null_type) ||
        (first_type == null_float_type && second_type == null_type) ||
        (first_type == null_string_type && second_type == null_type)) {
        return true;
    }

    return false;
}

/**
 * @brief Initializes the parser.
 * @param source Pointer to the input file to parse.
 */
void parser_init(FILE *source) {
    symbol_table = symtable_create(TABLE_SIZE);
    if (!symbol_table) {
        error_exit(ERROR_INTERNAL_COMPILER_ERROR, "Failed to create symbol table");
    } 

    file = source;
    fetch_next_token();
}

/**
 * @brief Parses the entire program
 * @return 0 on successful parsing, -1 on failure.
 */
int parse_program() {
    generator_init();
    bool hasMain = false;
    int result = parse_prologue();
    if (result < 0) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Error in prologue"); 
    }

    if (current_token->type != KW_PUB) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected keyword 'pub'");
    }

    while (current_token->type == KW_PUB) {
        if (fetch_next_token() != 0) return -1; 
        if (current_token->type != KW_FN) {
            error_exit(ERROR_SYNTAX_ANALYSIS, "Expected 'fn' keyword");
        }

        if (fetch_next_token() != 0) return -1;  
            if(current_token->attribute.s){
                if (dstring_compare_charstr(current_token->attribute.s, "main") == 0) {
                hasMain = true;
            }
        } else {
           error_exit(ERROR_SYNTAX_ANALYSIS, "Expected 'id' ot 'main' keyword"); 
        }
        

        if (parse_function_header() != 0) {
            return -1;
        }

        if (current_token->type == TOKEN_EOF) {
            break;
        }
    }

    if(!hasMain){
        error_exit(ERROR_SEMANTIC_UNDEFINED_FUNCTION_OR_VARIABLE, "Program should has main function"); 
    }

    //debug_temp_file();
    //debug_symtable(symbol_table);
    rewind_file(file); 
    first_control = false;
    fetch_next_token();

    parse_prologue();
    gen_header();

    while (current_token->type == KW_PUB) {
        if (fetch_next_token() != 0) return -1; 
        if (current_token->type != KW_FN) {
            error_exit(ERROR_SYNTAX_ANALYSIS, "Expected 'fn' keyword");
        }

        if (fetch_next_token() != 0) return -1; 

        if (parse_function_definition() != 0) {
            return -1;
        }

        if(current_token->type == TOKEN_EOF){
            break;
        }
    }

    parser_cleanup();
    return 0;
}

/// @brief Parses the prologue section at the beginning of the program
/// @return Returns 1 if successful, otherwise returns ERROR_SYNTAX_ANALYSIS
int parse_prologue() {
    if (current_token->type != KW_CONST) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected keyword 'const'");
    }
    if (fetch_next_token() != 0) return -1;

    if (current_token->type != TOKEN_ID || dstring_compare_charstr(current_token->attribute.s, "ifj") != 0) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected identifier 'ifj'");
    }
    if (fetch_next_token() != 0) return -1;

    if (current_token->type != TOKEN_ASSIGN) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected '='");
    }
    if (fetch_next_token() != 0) return -1;

    if (current_token->type != TOKEN_IMPORT) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected '@import'");
    }

    if (fetch_next_token() != 0) return -1;

    if (current_token->type != TOKEN_OPENING_PARENTHESES) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected '('");
    }
    if (fetch_next_token() != 0) return -1;

    if (current_token->type != TOKEN_STRING || dstring_compare_charstr(current_token->attribute.s, "ifj24.zig") != 0) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected 'ifj24.zig'");
    }

    if (fetch_next_token() != 0) return -1;

    if (current_token->type != TOKEN_CLOSING_PARENTHESES) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected ')'");
    }
    if (fetch_next_token() != 0) return -1;

    if (current_token->type != TOKEN_SEMICOLON) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected semicolon");
    }
    if (fetch_next_token() != 0) return -1;

    return 0; 
}

/**
 * @brief Parses the header of a function, including its name, parameters, and return type.
 * @return 0 on successful parsing, -1 on failure.
 */
int parse_function_header() {
    if (current_token->type != TOKEN_ID) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected identifier at the start of function definition");
    }

    dstring_t* func_name = dstring_copy(current_token->attribute.s);
    if (symtable_find(symbol_table, func_name)) {
        dstring_free(func_name);
        error_exit(ERROR_SEMANTIC_REDEFINITION, "Function redefinition is not allowed");
    }

    bool is_main = (dstring_compare_charstr(func_name, "main") == 0);

    func_data_t *func_data = malloc(sizeof(func_data_t));
    if (!func_data) {
        dstring_free(func_name);
        error_exit(ERROR_INTERNAL_COMPILER_ERROR, "Memory allocation failed");
    }
    func_data->name = func_name;
    func_data->isDefined = true;
    func_data->paramCount = 0;  
    func_data->paramCapacity = 4;
    func_data->params = malloc(func_data->paramCapacity * sizeof(data_type));
    if (!func_data->params) {
        free(func_data);
        error_exit(ERROR_INTERNAL_COMPILER_ERROR, "Memory allocation failed for parameters");
    }  
    func_data->returnType = void_type; 

    if (symtable_insert_function(symbol_table, func_name, func_data, symbol_table->scope_level) != 0) {
        dstring_free(func_name);
        free(func_data->params);
        free(func_data);
        return -1;
    } 

    if (fetch_next_token() != 0) return -1;  

    if (current_token->type != TOKEN_OPENING_PARENTHESES) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected '(' after function identifier");
    }

    if (fetch_next_token() != 0) return -1; 

    if (parse_parameters(func_data) != 0) {
        error_exit(ERROR_SEMANTIC_INCORRECT_FUNCTION_PARAMETERS_OR_RETURN_VALUE, "Incorrect function parameters");
    }

    if (current_token->type != TOKEN_CLOSING_PARENTHESES) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected ')' after parameters");
    }

    if (is_main && func_data->paramCount > 0) {
        error_exit(ERROR_SEMANTIC_INCORRECT_FUNCTION_PARAMETERS_OR_RETURN_VALUE, "Main function cannot have parameters");
    }

    if (fetch_next_token() != 0) return -1;

    if(parse_return_type(func_data) != 0) {
        error_exit(ERROR_SEMANTIC_INCORRECT_FUNCTION_PARAMETERS_OR_RETURN_VALUE, "Incorrect return type");
    }

    if (is_main && func_data->returnType != void_type) {
        error_exit(ERROR_SEMANTIC_INCORRECT_FUNCTION_PARAMETERS_OR_RETURN_VALUE, "Main function must return void");
    }

    if (current_token->type != TOKEN_OPENING_BRACKET) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected '{' to start function body");
    }

    while(current_token->type != KW_PUB && current_token->type != TOKEN_EOF){
        if (fetch_next_token() != 0) return -1;
    } 

    return 0;
}

/// @brief Parses a single function definition
/// @return Returns 1 if successful, otherwise returns error
int parse_function_definition() {
    if (current_token->type != TOKEN_ID) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected function identifier");
    }

    dstring_t *func_name = dstring_copy(current_token->attribute.s);
    symtable_data_t *entry = symtable_find(symbol_table, func_name);
    if (!entry || entry->type != fn_t) {
        dstring_free(func_name);
        error_exit(ERROR_SEMANTIC_UNDEFINED_FUNCTION_OR_VARIABLE, "Function not declared");
    }
    dstring_free(func_name);

    gen_func_start(entry->funcData->name);

    symtable_enter_scope(symbol_table);

    if (fetch_next_token() != 0) return -1;  
    if (current_token->type != TOKEN_OPENING_PARENTHESES) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected '(' after function identifier");
    }

    if (fetch_next_token() != 0) return -1; 

    if (parse_parameters(entry->funcData) != 0) {
        error_exit(ERROR_SEMANTIC_INCORRECT_FUNCTION_PARAMETERS_OR_RETURN_VALUE, "Incorrect function parameters");
    }

    if (current_token->type != TOKEN_CLOSING_PARENTHESES) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected ')' after parameters");
    }

    while(current_token->type != TOKEN_OPENING_BRACKET){
        if (fetch_next_token() != 0) return -1;
    } 

    if (current_token->type != TOKEN_OPENING_BRACKET) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected '{' to start function body");
    }

    if (fetch_next_token() != 0) { 
        return -1;
    }

    current_function = entry->funcData;
    has_return = false;

    if (parse_statements() != 0) {
        return -1;
    }

    if (current_token->type != TOKEN_CLOSING_BRACKET) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected '}' to start function body");
    }

    if(entry->funcData->returnType != void_type){
        if(!has_return){
            error_exit(ERROR_SEMANTIC_MISSING_OR_ABSENT_EXPRESSION_IN_RETURN, "Function should has return statement");
        }
    } else if (entry->funcData->returnType == void_type) {
        if(!has_return){
            printf("POPFRAME\n");
        }
    }

    if(dstring_compare_charstr(entry->funcData->name, "main") == 0){
        printf("EXIT int@0\n");
    }

    if (check_unused_variables_in_scope(symbol_table) != 0){
        error_exit(ERROR_SEMANTIC_UNUSED_VARIABLE, " Variable declared at scope level was not used.\n");
    }
    symtable_exit_scope(symbol_table);

    if (fetch_next_token() != 0) return -1;

    return 0;
}

// Function to parse the parameters of a function
/// @brief Parses function parameters
/// @return Returns 1 if successful, otherwise returns ERROR_SYNTAX
int parse_parameters(func_data_t *func_data) {
    if (first_control) {
        func_data->paramCount = 0;
    }

    if (current_token->type == TOKEN_CLOSING_PARENTHESES) {
        return 0;
    }

    do {
        if (parse_parameter(func_data) != 0) {
            return -1; 
        }

        if (current_token->type == TOKEN_COMMA) {
            if (fetch_next_token() != 0) {
                error_exit(ERROR_SYNTAX_ANALYSIS, "Error fetching token after ','");
            }
            if (current_token->type != TOKEN_ID) {
                error_exit(ERROR_SYNTAX_ANALYSIS, "Wrong parameters");
            }
        } else if (current_token->type != TOKEN_CLOSING_PARENTHESES) {
            error_exit(ERROR_SYNTAX_ANALYSIS, "Expected ',' or closing parenthesis in parameter list");
        }
    } while (current_token->type != TOKEN_CLOSING_PARENTHESES);

    return 0;
}

// Function to parse a single parameter 
/// @brief Parses a single parameter declaration with a type
/// @return Returns 1 if successful, otherwise returns error
int parse_parameter(func_data_t *func_data) {
    if (current_token->type != TOKEN_ID) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected parameter identifier");
    }

    dstring_t *param_name = dstring_copy(current_token->attribute.s);

    if (!first_control) {
        if (symtable_find_in_scope(symbol_table, param_name, symbol_table->scope_level)) {
            dstring_free(param_name);
            error_exit(ERROR_SEMANTIC_REDEFINITION, "Parameter redefinition in the same scope");
        }
    }

    if (fetch_next_token() != 0) {
        dstring_free(param_name);
        error_exit(ERROR_SYNTAX_ANALYSIS, "Error fetching token after parameter identifier");
    }

    if (current_token->type != TOKEN_TWODOT) {
        dstring_free(param_name);
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected ':' after parameter identifier");
    }

    if (fetch_next_token() != 0) {
        dstring_free(param_name);
        return -1;
    }

    data_type param_type;
    if (parse_type() == 0) {

        switch (current_token->type) {
            case KW_INT_TYPE:
                param_type = int_type;
                break;
            case KW_FLOAT_TYPE:
                param_type = float_type;
                break;
            case KW_STRING_TYPE:
                param_type = string_type;
                break;
            case TOKEN_INT_NULL:
                param_type = null_int_type;
                break;
            case TOKEN_FLOAT_NULL:
                param_type = null_float_type;
                break;
            case TOKEN_STRING_NULL:
                param_type = null_string_type;
                break;
            default:
                dstring_free(param_name);
                error_exit(ERROR_SYNTAX_ANALYSIS, "Invalid type for parameter");
        }

        if (first_control) {
            if (func_data_add_param(func_data, param_type) != 0) {
                dstring_free(param_name);
                error_exit(ERROR_INTERNAL_COMPILER_ERROR, "Failed to add parameter type");
            }
        } else {
            var_data_t *param_data = (var_data_t *)malloc(sizeof(var_data_t));
            if (!param_data) {
                dstring_free(param_name);
                error_exit(ERROR_INTERNAL_COMPILER_ERROR, "Memory allocation failed for parameter");
            }

            param_data->name = param_name;
            param_data->type = param_type;
            param_data->isUsed = false;

            if (symtable_insert_variable(symbol_table, param_name, param_data, symbol_table->scope_level, true) != 0) {
                dstring_free(param_name);
                free(param_data);
                error_exit(ERROR_INTERNAL_COMPILER_ERROR, "Failed to insert parameter into symbol table");
            }
            
            gen_defvar(param_name);  
            gen_pop_operand(param_name);
        }

        if (fetch_next_token() != 0) {
            return -1;
        }

        return 0;
    }

    error_exit(ERROR_SYNTAX_ANALYSIS, "Expected a valid type for parameter");
    dstring_free(param_name);
    return -1;
}

/// @brief Parses the return type of a function
/// @return Returns 1 if successful, otherwise returns error
int parse_return_type(func_data_t *func_data) {
    if (current_token->type == KW_VOID) {
        func_data->returnType = void_type; 
        if (fetch_next_token() != 0) return -1;
        return 0;
    }

    if (!parse_type()) {
        switch (current_token->type) {
            case KW_INT_TYPE:
                func_data->returnType = int_type;
                break;
            case KW_FLOAT_TYPE:
                func_data->returnType = float_type;
                break;
            case KW_STRING_TYPE:
                func_data->returnType = string_type;
                break;
            case TOKEN_INT_NULL:
                func_data->returnType = null_int_type;
                break;
            case TOKEN_FLOAT_NULL:
                func_data->returnType = null_float_type;
                break;
            case TOKEN_STRING_NULL:
                func_data->returnType = null_string_type;
                break;
            default:
                error_exit(ERROR_SYNTAX_ANALYSIS, "Unexpected type after parsing");
        }
        if (fetch_next_token() != 0) return -1;
        return 0;
    }
    error_exit(ERROR_SYNTAX_ANALYSIS, "Expected a valid type");
    return -1;
}

/// @brief Parses statements in the body of a function
/// @return Returns 1 if successful, otherwise returns error
int parse_statements() {
    while (current_token->type != TOKEN_CLOSING_BRACKET) {
        if (parse_statement() != 0) {
            error_exit(ERROR_SYNTAX_ANALYSIS, "Error in satement");
        }  
    }

    return 0;
}

/// @brief Parses a single statement
/// @return Returns 1 if successful, otherwise returns error
int parse_statement() {
    switch (current_token->type) {
        case KW_VAR: 
            if (parse_var_decl() != 0) return -1;
            break;
        case KW_CONST: 
            return parse_const_decl();
        case TOKEN_ID: { 
            dstring_t *id_name = dstring_copy(current_token->attribute.s); 
            if(id_name == NULL){
                 error_exit(ERROR_INTERNAL_COMPILER_ERROR, "Fail copy string");
            }

            if (fetch_next_token() != 0) {
                dstring_free(id_name);
                return -1;
            }

            if (current_token->type == TOKEN_ASSIGN) {
                if (parse_assignment(id_name) != 0) { 
                    dstring_free(id_name);
                    return -1;
                }
                dstring_free(id_name);
                return 0;
            } else if (current_token->type == TOKEN_OPENING_PARENTHESES) {
                if (parse_function_call(id_name) != 0) { 
                    dstring_free(id_name);
                    return -1;
                }
                dstring_free(id_name);
                return 0;
            } else {
                dstring_free(id_name);
                error_exit(ERROR_SYNTAX_ANALYSIS, "Expected assignment or function call");
            }
        }
        case TOKEN_UNDERSCORE:
            if (fetch_next_token() != 0) return -1;
            if( parse_assignment(NULL) != 0){
                return -1;
            }
            return 0; 
        case KW_IF: 
            return parse_if_statement();
        case KW_WHILE: 
            return parse_while_statement();
        case KW_RETURN: 
            return parse_return_statement();
        case TOKEN_CLOSING_BRACKET:
            return 0;
        default:
            error_exit(ERROR_SYNTAX_ANALYSIS, "Unrecognized statement");
    }
    return 0;
}

/**
 * @brief Parses a variable declaration.
 * @return 0 on successful parsing, -1 on failure.
 */
int parse_var_decl() {
    if (current_token->type != KW_VAR) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected 'var' keyword");
    }

    if (fetch_next_token() != 0) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Error fetching token after 'var'");
    }
    if (current_token->type != TOKEN_ID) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected id");
    }
    dstring_t *var_name = dstring_copy(current_token->attribute.s);
    if (!var_name) return -1;

    if (symtable_find(symbol_table, var_name)) {
        dstring_free(var_name);
        error_exit(ERROR_SEMANTIC_REDEFINITION, "Variable redefinition in the same block or sub-block");
    }

    if (fetch_next_token() != 0) {
        dstring_free(var_name);
        return -1;
    }

    data_type var_type = null_type;
    bool type_specified = false;

    if (current_token->type == TOKEN_TWODOT) {
        type_specified = true;
        if (fetch_next_token() != 0) {
            dstring_free(var_name);
            return -1;
        }

        if (parse_type() == 0) {
            switch (current_token->type) {
                case KW_INT_TYPE:
                    var_type = int_type;
                    break;
                case KW_FLOAT_TYPE:
                    var_type = float_type;
                    break;
                case KW_STRING_TYPE:
                    var_type = string_type;
                    break;
                    case TOKEN_INT_NULL:
                    var_type = null_int_type;
                    break;
                case TOKEN_FLOAT_NULL:
                    var_type = null_float_type;
                    break;
                case TOKEN_STRING_NULL:
                    var_type = null_string_type;
                    break;
                default:
                    dstring_free(var_name);
                    error_exit(ERROR_SYNTAX_ANALYSIS, "Invalid type for variable declaration");
            }

            if (fetch_next_token() != 0) {
                dstring_free(var_name);
                return -1;
            }
        } else {
            dstring_free(var_name);
            return -1;
        }
    }

    if (current_token->type != TOKEN_ASSIGN) {
        dstring_free(var_name);
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected '=' in variable declaration");
    }

    if (fetch_next_token() != 0) {
        dstring_free(var_name);
        return -1;
    }

    gen_defvar(var_name);

    data_type expr_type;
    if (parse_expression(&expr_type) != 0) {
        dstring_free(var_name);
        error_exit(ERROR_SYNTAX_ANALYSIS, "Syntax error in expression");
    }

    if (expr_type == null) {
        dstring_free(var_name);
        error_exit(ERROR_SEMANTIC_TYPE_INCOMPATIBILITY, "Invalid type in expression");
    }

    if ((expr_type == null || expr_type == null_type) && !type_specified) {
        dstring_free(var_name);
        error_exit(ERROR_SEMANTIC_TYPE_INFERENCE_FAILURE, "Cannot infer type "); 
    }

    if (!type_specified) {
        var_type = expr_type;
    } else if (!is_type_compatible(var_type, expr_type)) {
        dstring_free(var_name);
        error_exit(ERROR_SEMANTIC_TYPE_INCOMPATIBILITY, "Type mismatch in variable declaration");
    }

    gen_pop_operand(var_name);

    if (current_token->type != TOKEN_SEMICOLON) {
        dstring_free(var_name);  
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected ';' at the end of variable declaration");
    }

    var_data_t *var_data = (var_data_t *)malloc(sizeof(var_data_t));
    if (!var_data) {
        dstring_free(var_name);  
        error_exit(ERROR_INTERNAL_COMPILER_ERROR, "Memory allocation failed for variable");
    }
    var_data->name = var_name;
    var_data->type = var_type;
    var_data->isUsed = false;

    if (symtable_insert_variable(symbol_table, var_name, var_data, symbol_table->scope_level, false) != 0) {
        dstring_free(var_name);
        free(var_data);
        return -1;
    }

    if (fetch_next_token() != 0) return -1;

    return 0;
}

/**
 * @brief Parses a constant declaration.
 * @return 0 on successful parsing, -1 on failure.
 */
int parse_const_decl() {
    if (current_token->type != KW_CONST) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected 'const' keyword");
    }

    if (fetch_next_token() != 0) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Error fetching token after 'const'");
    }
    if (current_token->type != TOKEN_ID) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected id");
    }
    dstring_t *const_name = dstring_copy(current_token->attribute.s);

    if (symtable_find(symbol_table, const_name)) {
        dstring_free(const_name);
        error_exit(ERROR_SEMANTIC_REDEFINITION, "Constant redefinition in the same block or sub-block");
    }

    if (fetch_next_token() != 0) {
        dstring_free(const_name);
        return -1;
    }

    data_type const_type = null_type;
    bool type_specified = false;

    if (current_token->type == TOKEN_TWODOT) {
        type_specified = true;
        if (fetch_next_token() != 0) {
            dstring_free(const_name);
            return -1;
        }

        if (parse_type() == 0) {
            switch (current_token->type) {
                case KW_INT_TYPE:
                    const_type = int_type;
                    break;
                case KW_FLOAT_TYPE:
                    const_type = float_type;
                    break;
                case KW_STRING_TYPE:
                    const_type = string_type;
                    break;
                case TOKEN_INT_NULL:
                    const_type = null_int_type;
                    break;
                case TOKEN_FLOAT_NULL:
                    const_type = null_float_type;
                    break;
                case TOKEN_STRING_NULL:
                    const_type = null_string_type;
                    break;
                default:
                    dstring_free(const_name);
                    error_exit(ERROR_SYNTAX_ANALYSIS, "Invalid type for constant declaration");
            }

            if (fetch_next_token() != 0) {
                dstring_free(const_name);
                return -1;
            }
        } else {
            dstring_free(const_name);
            return -1;
        }
    }

    if (current_token->type != TOKEN_ASSIGN) {
        dstring_free(const_name);
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected '=' in constant declaration");
    }

    if (fetch_next_token() != 0) {
        dstring_free(const_name);
        return -1;
    }

    gen_defvar(const_name);

    data_type expr_type;
    if (parse_expression(&expr_type) != 0) {
        dstring_free(const_name);
        error_exit(ERROR_SYNTAX_ANALYSIS, "Syntax error in expression");
    }

    if (expr_type == null) {
        dstring_free(const_name);
        error_exit(ERROR_SEMANTIC_TYPE_INCOMPATIBILITY, "Invalid type in expression");
    }

    if ((expr_type == null || expr_type == null_type) && !type_specified) {
        dstring_free(const_name);
        error_exit(ERROR_SEMANTIC_TYPE_INFERENCE_FAILURE, "Cannot infer type "); 
    }

    if (!type_specified) {
        const_type = expr_type;
    } else if (!is_type_compatible(const_type, expr_type)) {
        dstring_free(const_name);
        error_exit(ERROR_SEMANTIC_TYPE_INCOMPATIBILITY, "Type mismatch in constant declaration");
    }

    gen_pop_operand(const_name);

    if (current_token->type != TOKEN_SEMICOLON) {
        dstring_free(const_name);
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected ';' at the end of constant declaration");
    }

    var_data_t *const_data = (var_data_t *)malloc(sizeof(var_data_t));
    if (!const_data) {
        dstring_free(const_name);
        error_exit(ERROR_INTERNAL_COMPILER_ERROR, "Memory allocation failed for constant");
    }
    const_data->name = const_name;
    const_data->type = const_type;
    const_data->isUsed = false;

    if (symtable_insert_variable(symbol_table, const_name, const_data, symbol_table->scope_level, true) != 0) {
        dstring_free(const_name);
        free(const_data);
        return -1;
    }

    if (fetch_next_token() != 0) return -1;

    return 0;
}

/**
 * @brief Parses an assignment operation.
 * @param id_name Identifier of the variable being assigned.
 * @return 0 on successful parsing, -1 on failure.
 */
int parse_assignment(dstring_t *id_name) {
    dstring_t *var_name = NULL;
    if(id_name != NULL){
        var_name = dstring_copy(id_name);
    } 

    if (var_name != NULL) {
        symtable_data_t *entry = symtable_find(symbol_table, var_name);
        if (!entry) {
            dstring_free(var_name);
            error_exit(ERROR_SEMANTIC_UNDEFINED_FUNCTION_OR_VARIABLE, "Variable not defined before assignment");
        }
        dstring_free(var_name);

        entry->varData->isUsed = true;

        if (entry->type == const_t) {  
            error_exit(ERROR_SEMANTIC_REDEFINITION, "Cannot reassign a constant variable");
        }

        if (current_token->type != TOKEN_ASSIGN) {
            error_exit(ERROR_SYNTAX_ANALYSIS, "Expected '=' in assignment");
        }

        if (fetch_next_token() != 0) return -1;

        data_type expr_type;
        if (parse_expression(&expr_type) != 0) {
            return -1;
        }

        if (current_token->type != TOKEN_SEMICOLON) {
            error_exit(ERROR_SYNTAX_ANALYSIS, "Expected ';' at the end of assignment");
        }

        if (fetch_next_token() != 0) return -1;

        if (!is_type_compatible(entry->varData->type, expr_type)) {
            error_exit(ERROR_SEMANTIC_TYPE_INCOMPATIBILITY, "Type mismatch in assignment");
        }

        gen_pop_operand(entry->varData->name); ////
    } else {
        if (current_token->type != TOKEN_ASSIGN) {
            error_exit(ERROR_SYNTAX_ANALYSIS, "Expected '=' in assignment");
        }

        if (fetch_next_token() != 0) return -1;

        data_type expr_type;
        if (parse_expression(&expr_type) != 0) {
            return -1;
        }

        gen_pop_operand(NULL); ////

        if (current_token->type != TOKEN_SEMICOLON) {
            error_exit(ERROR_SYNTAX_ANALYSIS, "Expected ';' at the end of assignment");
        }

        if (fetch_next_token() != 0) return -1;
    }

    if(!var_name){
        dstring_free(var_name);
    }
    return 0;
}

/**
 * @brief Parses an "if" statement.
 * @return 0 on successful parsing, -1 on failure.
 */
int parse_if_statement() {
    
    if (fetch_next_token() != 0) return -1;

    if (current_token->type != TOKEN_OPENING_PARENTHESES) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected '(' after 'if'");
    }

    if (fetch_next_token() != 0) return -1;

    data_type cond_type;
    if (parse_expression(&cond_type) != 0) {
        return -1;
    }

    if (current_token->type != TOKEN_CLOSING_PARENTHESES) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected ')' after the condition in 'if' statement");
    }

    if (fetch_next_token() != 0) return -1;

    if(current_token->type == TOKEN_VERTICAL_BAR){
        if(parse_if_nullable_statement(cond_type) != 0){
            return -1;
        } else {
            return 0;
        }
    }

    if (cond_type != bool_type) {
        error_exit(ERROR_SEMANTIC_TYPE_INCOMPATIBILITY, "Condition expression must be of boolean type");
    }

    gen_if_start();

    if (current_token->type != TOKEN_OPENING_BRACKET) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected '{' to start the 'if' block");
    }

    symtable_enter_scope(symbol_table);

    if (fetch_next_token() != 0) {
        symtable_exit_scope(symbol_table);
        return -1;
    }

    if (parse_statements() != 0) {
        symtable_exit_scope(symbol_table);
        return -1;
    }

    if (current_token->type != TOKEN_CLOSING_BRACKET) {
        symtable_exit_scope(symbol_table);
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected '}' at the end of the 'if' block");
    }

    if (fetch_next_token() != 0) {
        symtable_exit_scope(symbol_table);
        return -1;
    }

    if (check_unused_variables_in_scope(symbol_table) != 0){
        error_exit(ERROR_SEMANTIC_UNUSED_VARIABLE, " Variable declared at scope level was not used.\n");
    }
    gen_if_else();
    symtable_exit_scope(symbol_table);

    if (current_token->type != KW_ELSE) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected 'else' after 'if' block");
    }

    if (fetch_next_token() != 0) {
        return -1;
    }

    if (current_token->type != TOKEN_OPENING_BRACKET) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected '{' to start the 'else' block");
    }

    symtable_enter_scope(symbol_table);

    if (fetch_next_token() != 0) {
        symtable_exit_scope(symbol_table);
        return -1;
    }

    if (parse_statements() != 0) {
        symtable_exit_scope(symbol_table);
        return -1;
    }

    if (current_token->type != TOKEN_CLOSING_BRACKET) {
        symtable_exit_scope(symbol_table);
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected '}' at the end of the 'else' block");
    }

    if (fetch_next_token() != 0) {
        symtable_exit_scope(symbol_table);
        return -1;
    }

    if (check_unused_variables_in_scope(symbol_table) != 0){
        error_exit(ERROR_SEMANTIC_UNUSED_VARIABLE, " Variable declared at scope level was not used.\n");
    }
    symtable_exit_scope(symbol_table);
    gen_if_end();

    return 0;
}

/**
 * @brief Parses a nullable "if" statement.
 * @param cond_type The type of the nullable condition.
 * @return 0 on success, -1 on failure.
 */
int parse_if_nullable_statement(data_type cond_type){
    if (current_token->type != TOKEN_VERTICAL_BAR) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected '|' after nullable expression");
    }
    if (fetch_next_token() != 0) return -1;

    // Parse <id_without_null>
    if (current_token->type != TOKEN_ID) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected identifier after '|'");
    }

    dstring_t *id_name = dstring_copy(current_token->attribute.s);

    gen_if_nullable_start(id_name);

    if (fetch_next_token() != 0) return -1;

    if (current_token->type != TOKEN_VERTICAL_BAR) {
        dstring_free(id_name);
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected '|' after identifier");
    }

    if (fetch_next_token() != 0) return -1;

    if (current_token->type != TOKEN_OPENING_BRACKET) {
        dstring_free(id_name);
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected '{' to start 'if' block");
    }

    symtable_enter_scope(symbol_table);

    if (symtable_find_in_scope(symbol_table, id_name, symbol_table->scope_level)) {
        dstring_free(id_name);
        error_exit(ERROR_SEMANTIC_REDEFINITION, "Variable redefinition in the same block or sub-block"); 
    }


    if (cond_type != null_type && cond_type != null){
        var_data_t *var_data = malloc(sizeof(var_data_t));
        if (!var_data) {
            dstring_free(id_name);
            error_exit(ERROR_INTERNAL_COMPILER_ERROR, "Memory allocation failed");
        }

        var_data->name = id_name;
        var_data->type = (cond_type == null_int_type) ? int_type :
                        (cond_type == null_float_type) ? float_type :
                        (cond_type == null_string_type) ? string_type : cond_type;
        var_data->isUsed = false;

        if (symtable_insert_variable(symbol_table, id_name, var_data, symbol_table->scope_level, false) != 0) {
            dstring_free(id_name);
            free(var_data);
            error_exit(ERROR_INTERNAL_COMPILER_ERROR, "Failed to insert id_without_null into symbol table");
        }
    } else {
        dstring_free(id_name);
    }

    if (fetch_next_token() != 0) {
        symtable_exit_scope(symbol_table);
        return -1;
    }

    if (parse_statements() != 0) {
        symtable_exit_scope(symbol_table);
        return -1;
    }

    if (current_token->type != TOKEN_CLOSING_BRACKET) {
        symtable_exit_scope(symbol_table);
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected '}' at the end of the 'if' block");
    }

    if (fetch_next_token() != 0) {
        symtable_exit_scope(symbol_table);
        return -1;
    }

    if (check_unused_variables_in_scope(symbol_table) != 0){
        error_exit(ERROR_SEMANTIC_UNUSED_VARIABLE, " Variable declared at scope level was not used.\n");
    }
    symtable_exit_scope(symbol_table);

    if (current_token->type != KW_ELSE) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected 'else' after 'if' block");
    }

    gen_if_nullable_else();

    if (fetch_next_token() != 0) {
        return -1;
    }

    if (current_token->type != TOKEN_OPENING_BRACKET) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected '{' to start the 'else' block");
    }

    symtable_enter_scope(symbol_table);

    if (fetch_next_token() != 0) {
        symtable_exit_scope(symbol_table);
        return -1;
    }

    if (parse_statements() != 0) {
        symtable_exit_scope(symbol_table);
        return -1;
    }

    if (current_token->type != TOKEN_CLOSING_BRACKET) {
        symtable_exit_scope(symbol_table);
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected '}' at the end of the 'else' block");
    }

    if (fetch_next_token() != 0) {
        symtable_exit_scope(symbol_table);
        return -1;
    }

    if (check_unused_variables_in_scope(symbol_table) != 0){
        error_exit(ERROR_SEMANTIC_UNUSED_VARIABLE, " Variable declared at scope level was not used.\n");
    }
    symtable_exit_scope(symbol_table);

    gen_if_nullable_end();

    return 0;
}

/**
 * @brief Parses a "while" statement.
 * @return 0 on successful parsing, -1 on failure.
 */
int parse_while_statement() {
    if (fetch_next_token() != 0) return -1;

    if (current_token->type != TOKEN_OPENING_PARENTHESES) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected '(' after 'while'");
    }

    gen_while_start();

    if (fetch_next_token() != 0) return -1;

    data_type cond_type;
    if (parse_expression(&cond_type) != 0) {
        return -1;
    }

    if (current_token->type != TOKEN_CLOSING_PARENTHESES) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected ')' after the condition in 'while' statement");
    }

    if (fetch_next_token() != 0) return -1;

    if (current_token->type == TOKEN_VERTICAL_BAR){
        if(parse_while_nullable_statement(cond_type) != 0){
            return -1;
        } else {
            return 0;
        }
    }
    
    if (cond_type != bool_type) {
        error_exit(ERROR_SEMANTIC_TYPE_INCOMPATIBILITY, "Condition expression must be of boolean type");
    }

    if (current_token->type != TOKEN_OPENING_BRACKET) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected '{' to start the 'while' block");
    }

    gen_while_cond();

    symtable_enter_scope(symbol_table);

    if (fetch_next_token() != 0) {
        symtable_exit_scope(symbol_table);
        return -1;
    }

    if (parse_statements() != 0) {
        symtable_exit_scope(symbol_table);
        return -1;
    }

    if (current_token->type != TOKEN_CLOSING_BRACKET) {
        symtable_exit_scope(symbol_table);
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected '}' at the end of the 'while' block");
    }

    if (check_unused_variables_in_scope(symbol_table) != 0){
        error_exit(ERROR_SEMANTIC_UNUSED_VARIABLE, " Variable declared at scope level was not used.\n");
    }
    symtable_exit_scope(symbol_table);
    gen_while_end();

    if (fetch_next_token() != 0) return -1;

    return 0;
}

/**
 * @brief Parses a nullable "while" statement.
 * @param cond_type The data type of the nullable condition.
 * @return 0 on successful parsing, -1 on failure.
 */
int parse_while_nullable_statement(data_type cond_type){
    if (current_token->type != TOKEN_VERTICAL_BAR) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected '|' after nullable expression");
    }
    if (fetch_next_token() != 0) return -1;

    if (current_token->type != TOKEN_ID) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected identifier after '|'");
    }

    dstring_t *id_name = dstring_copy(current_token->attribute.s);

    gen_while_nullable_cond(id_name);

    if (fetch_next_token() != 0) return -1;

    if (current_token->type != TOKEN_VERTICAL_BAR) {
        dstring_free(id_name);
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected '|' after identifier");
    }

    if (fetch_next_token() != 0) return -1;

    if (current_token->type != TOKEN_OPENING_BRACKET) {
        dstring_free(id_name);
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected '{' to start 'while' block");
    }

    if (fetch_next_token() != 0) return -1;

    symtable_enter_scope(symbol_table);

    if (symtable_find_in_scope(symbol_table, id_name, symbol_table->scope_level)) {
        dstring_free(id_name);
        error_exit(ERROR_SEMANTIC_REDEFINITION, "Variable redefinition in the same block or sub-block");
    }

    if (cond_type != null_type && cond_type != null){
        var_data_t *var_data = malloc(sizeof(var_data_t));
        if (!var_data) {
            dstring_free(id_name);
            error_exit(ERROR_INTERNAL_COMPILER_ERROR, "Memory allocation failed");
        }

        var_data->name = id_name;
        var_data->type = (cond_type == null_int_type) ? int_type :
                        (cond_type == null_float_type) ? float_type :
                        (cond_type == null_string_type) ? string_type : cond_type;
        var_data->isUsed = false;

        if (symtable_insert_variable(symbol_table, id_name, var_data, symbol_table->scope_level, false) != 0) {
            dstring_free(id_name);
            free(var_data);
            error_exit(ERROR_INTERNAL_COMPILER_ERROR, "Failed to insert id_without_null into symbol table");
        }
    } else {
        dstring_free(id_name);
    }

    if (parse_statements() != 0) {
        symtable_exit_scope(symbol_table);
        return -1;
    }

    if (current_token->type != TOKEN_CLOSING_BRACKET) {
        symtable_exit_scope(symbol_table);
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected '}' at the end of the 'while' block");
    }

    if (fetch_next_token() != 0) {
        symtable_exit_scope(symbol_table);
        return -1;
    }

    if (check_unused_variables_in_scope(symbol_table) != 0){
        error_exit(ERROR_SEMANTIC_UNUSED_VARIABLE, " Variable declared at scope level was not used.\n");
    }

    gen_while_nullable_end();
    symtable_exit_scope(symbol_table);

    return 0;
}

/**
 * @brief Parses a return statement.
 * @return 0 on successful parsing, -1 on failure.
 */
int parse_return_statement() {
    if (current_token->type != KW_RETURN) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected 'return' keyword");
    }

    if (fetch_next_token() != 0) return -1;

    if (current_token->type == TOKEN_SEMICOLON) {
        if (current_function->returnType != void_type) {
            error_exit(ERROR_SEMANTIC_MISSING_OR_ABSENT_EXPRESSION_IN_RETURN, "Expected an expression in return statement for non-void function");
        }
        
        if (fetch_next_token() != 0) return -1;

        gen_func_end(); ////

        has_return = true;

        return 0;
    } else {
        data_type expr_type;
        if (parse_expression(&expr_type) != 0) {
            error_exit(ERROR_SEMANTIC_MISSING_OR_ABSENT_EXPRESSION_IN_RETURN, "Error parsing expression in return statement");
        }

        if (current_token->type != TOKEN_SEMICOLON) {
            error_exit(ERROR_SYNTAX_ANALYSIS, "Expected ';' at the end of return statement");
        }

        if (current_function->returnType == void_type) {
            error_exit(ERROR_SEMANTIC_MISSING_OR_ABSENT_EXPRESSION_IN_RETURN, "Void function should not return an expression");
        } else if (current_function->returnType != expr_type) {
            error_exit(ERROR_SEMANTIC_INCORRECT_FUNCTION_PARAMETERS_OR_RETURN_VALUE, "Return type does not match function definition");
        }

        if (fetch_next_token() != 0) return -1;
        has_return = true;

        gen_return();

        return 0;
    }
}

/**
 * @brief Parses a data type.
 * @return 0 if the token matches a valid data type, -1 otherwise.
 */
int parse_type() {
    if (current_token->type == KW_INT_TYPE || current_token->type == KW_FLOAT_TYPE || current_token->type == KW_STRING_TYPE 
    || current_token->type == TOKEN_INT_NULL || current_token->type == TOKEN_FLOAT_NULL || current_token->type == TOKEN_STRING_NULL) {
        return 0;
    }
    return -1;
}

/**
 * @brief Parses a function call expression and retrieves its return type.
 * @param ret_type Pointer to store the return type of the function.
 * @return 0 on successful parsing, -1 on failure.
 */
int parse_function_call_expr(data_type *ret_type) {
    dstring_t *func_name = dstring_copy(current_token->attribute.s);
    symtable_data_t *func_entry = symtable_find(symbol_table, func_name);
    if (!func_entry || func_entry->type != fn_t) {
        dstring_free(func_name);
        error_exit(ERROR_SEMANTIC_UNDEFINED_FUNCTION_OR_VARIABLE, "Undefined function or not a valid function type"); 
    }

    func_data_t *func_data = func_entry->funcData;

    if (fetch_next_token() != 0) {
        dstring_free(func_name);
        return -1;
    }

    if (current_token->type != TOKEN_OPENING_PARENTHESES) {
        dstring_free(func_name);
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected '(' after function identifier");
    }

    if (fetch_next_token() != 0) {
        dstring_free(func_name);
        return -1;
    }

    if (parse_arguments(func_data) != 0) {
        dstring_free(func_name);
        error_exit(ERROR_SEMANTIC_INCORRECT_FUNCTION_PARAMETERS_OR_RETURN_VALUE, "Incorrect arguments for function call");
    }

    if (current_token->type != TOKEN_CLOSING_PARENTHESES) {
        dstring_free(func_name);
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected ')' after function arguments");
    }

    gen_function_call(func_name);

    *ret_type = func_data->returnType;

    dstring_free(func_name);

    return 0; 
}

/**
 * @brief Parses a full function call.
 * @param id_name The name of the function being called.
 * @return 0 on successful parsing, -1 on failure.
 */
int parse_function_call(dstring_t *id_name) {

    if (current_token->type != TOKEN_OPENING_PARENTHESES) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected '(' after function identifier");
    }

    symtable_data_t *func_entry = symtable_find(symbol_table, id_name);
    if (!func_entry || func_entry->type != fn_t) {
        error_exit(ERROR_SEMANTIC_UNDEFINED_FUNCTION_OR_VARIABLE, "Undefined function or not a valid function type"); 
    }

    func_data_t *func_data = func_entry->funcData;

    if (fetch_next_token() != 0) {
        return -1;
    }

    if (parse_arguments(func_data) != 0) {
        error_exit(ERROR_SEMANTIC_INCORRECT_FUNCTION_PARAMETERS_OR_RETURN_VALUE, "Incorrect arguments for function call");
    }

    if (current_token->type != TOKEN_CLOSING_PARENTHESES) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected ')' after function arguments");
    }

    if (fetch_next_token() != 0) return -1;

    if (current_token->type != TOKEN_SEMICOLON) {
        error_exit(ERROR_SYNTAX_ANALYSIS, "Expected ';' at the end of function call");
    }

    if(func_data->returnType != void_type){
        error_exit(ERROR_SEMANTIC_INCORRECT_FUNCTION_PARAMETERS_OR_RETURN_VALUE, "Return should be assign to variable");
    }

    gen_function_call(id_name);

    if (fetch_next_token() != 0) return -1;

    return 0;
}

/**
 * @brief Parses an expression and retrieves its return type.
 * @param ret_type Pointer to store the resulting type of the expression.
 * @return 0 on successful parsing, -1 on failure.
 */
int parse_expression(data_type *ret_type){
    if (current_token->type == TOKEN_ID) {
        symtable_data_t *entry = symtable_find(symbol_table, current_token->attribute.s);
        if (entry && entry->type == fn_t) {
            //if(parse_function_call_expr(ret_type) != 0) return -1;
            if (parse_expr(ret_type) != 0) return -1;
        } else if(entry && (entry->type == var_t ||entry->type == const_t)){
            if (parse_expr(ret_type) != 0) return -1;
        } else {
            free(entry);
            error_exit(ERROR_SEMANTIC_UNDEFINED_FUNCTION_OR_VARIABLE, "Undefined function or not a valid function type");
        }
    } else{
        if (parse_expr(ret_type) != 0) return -1;
    }
    return 0;
}

/**
 * @brief Parses the arguments of a function call.
 * @param func_data Pointer to the function's data, including parameter information.
 * @return 0 on successful parsing, -1 on failure.
 */
int parse_arguments(func_data_t *func_data) {
    int argIndex = 0;

    while (current_token->type != TOKEN_CLOSING_PARENTHESES) {
        if (argIndex >= func_data->paramCount) {
            error_exit(ERROR_SEMANTIC_INCORRECT_FUNCTION_PARAMETERS_OR_RETURN_VALUE, 
                      "Too many arguments for function call");
        }

        data_type argType;
        if (parse_expression(&argType) != 0) return -1;

        if((dstring_compare_charstr(func_data->name, "ifj.write") == 0) || (dstring_compare_charstr(func_data->name, "ifj.string") == 0)){
                if (argType == null) {
                error_exit(ERROR_SEMANTIC_INCORRECT_FUNCTION_PARAMETERS_OR_RETURN_VALUE, "Argument type mismatch in function call");
            }
        } else {
            if (argType != func_data->params[argIndex]) {
                error_exit(ERROR_SEMANTIC_INCORRECT_FUNCTION_PARAMETERS_OR_RETURN_VALUE, "Argument type mismatch in function call");
            }    
        }

        argIndex++;
        if (current_token->type == TOKEN_COMMA) {
            fetch_next_token();
        }
    }

    if (argIndex != func_data->paramCount) {
        error_exit(ERROR_SEMANTIC_INCORRECT_FUNCTION_PARAMETERS_OR_RETURN_VALUE, "Too few arguments for function call");
    }

    return 0;
}

/**
 * @brief Cleans up resources used by the parser.
 */
void parser_cleanup() {
    if (symbol_table != NULL) {
        symtable_free(symbol_table);
        symbol_table = NULL;
    }
    if (current_token) {
        token_free(current_token);
        current_token = NULL;
    }

    generator_cleanup();
}