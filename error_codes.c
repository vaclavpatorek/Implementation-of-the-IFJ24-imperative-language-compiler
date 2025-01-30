/// IFJ24
/// @brief Implementation of error handling functions

#include <stdio.h>
#include "error_codes.h"

/**
 * @brief Global error variable, initialized to "no error" state
 */
static error_t current_error = {ERROR_OK, NULL, -1, -1};

/**
 * @brief Sets the current error if none has been set yet
 * 
 * @param code Error code
 * @param message Error text message
 * @param line number where the error occurred
 * @param column Number of the column where the error occurred
 */
void set_error(int code, const char* message, int line, int column) {
    if (current_error.code == ERROR_OK) {
        current_error.code = code;
        current_error.message = message;
        current_error.line = line;
        current_error.column = column;
        print_error();
    }
}

/**
 * @brief Returns the current error
 * 
 * @return Structure representing the current error
 */
error_t get_error(void) {
    return current_error;
}

/**
 * @brief Prints the current error to stderr if an error exists
 */
void print_error(void) {
    if (current_error.code != ERROR_OK) {
        fprintf(stderr, "Error %d: %s at line %d, column %d\n",
                current_error.code, current_error.message, 
                current_error.line, current_error.column);
    }
}

/**
 * @brief Returns a string description for a given error code
 * 
 * @param error_code Error code
 * @return Text description of the error
 */
const char* get_error_message(int error_code) {
    switch (error_code) {
        case ERROR_LEXICAL_ANALYSIS:
            return "Program error in lexical analysis - wrong structure of the current lexeme";
        case ERROR_SYNTAX_ANALYSIS:
            return "Error in the program within the syntactic analysis - incorrect syntax of the program, missing header, etc.";
        case ERROR_SEMANTIC_UNDEFINED_FUNCTION_OR_VARIABLE:
            return "Semantic error in the program - undefined function or variable";
        case ERROR_SEMANTIC_INCORRECT_FUNCTION_PARAMETERS_OR_RETURN_VALUE:
            return "Semantic error in the program - wrong number/type of parameters in a function call";
        case ERROR_SEMANTIC_REDEFINITION:
            return "Semantic error in the program - redefinition of variable or function; assignment to non-modifiable variable";
        case ERROR_SEMANTIC_MISSING_OR_ABSENT_EXPRESSION_IN_RETURN:
            return "Semantic error in the program - missing/absent expression in the return statement";
        case ERROR_SEMANTIC_TYPE_INCOMPATIBILITY:
            return "Semantic type compatibility error in arithmetic, string and relational expressions; incompatible expression type (e.g. in assignment)";
        case ERROR_SEMANTIC_TYPE_INFERENCE_FAILURE:
            return "Semantic type derivation error - the type of the variable is not specified and cannot be derived from the expression used";
        case ERROR_SEMANTIC_UNUSED_VARIABLE:
            return "Semantic error of unused variable in its scope of validity; modifiable variable without the possibility of modification after its initialization";
        case ERROR_SEMANTIC_OTHER_ERRORS:
            return "Other semantic errors";
        case ERROR_INTERNAL_COMPILER_ERROR:
            return "Internal compiler error, i.e., not affected by the input program (e.g., memory allocation error, etc.)";
        default:
            return "Unknown error";
    }
}
