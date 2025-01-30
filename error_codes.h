/// IFJ24
/// @brief Error code definitions

#ifndef ERROR_CODES_H
#define ERROR_CODES_H

/**
 * @brief Successful compilation and execution
 */
#define ERROR_OK 0


/**
 * @brief Program error in lexical analysis - wrong structure of the current lexeme
 */
#define ERROR_LEXICAL_ANALYSIS 1


/**
 * @brief Error in the program within the syntactic analysis - incorrect syntax of the program, missing header, etc.
 */
#define ERROR_SYNTAX_ANALYSIS 2


/**
 * @brief Semantic error in the program - undefined function or variable
 */
#define ERROR_SEMANTIC_UNDEFINED_FUNCTION_OR_VARIABLE 3

/**
 * @brief Semantic error in the program - wrong number/type of parameters in a function call
 */
#define ERROR_SEMANTIC_INCORRECT_FUNCTION_PARAMETERS_OR_RETURN_VALUE 4

/**
 * @brief Semantic error in the program - redefinition of variable or function; assignment to non-modifiable variable
 */
#define ERROR_SEMANTIC_REDEFINITION 5

/**
 * @brief Semantic error in the program - missing/absent expression in the return statement
 */
#define ERROR_SEMANTIC_MISSING_OR_ABSENT_EXPRESSION_IN_RETURN 6

/**
 * @brief Semantic type compatibility error in arithmetic, string and relational expressions; incompatible expression type (e.g. in assignment)
 */
#define ERROR_SEMANTIC_TYPE_INCOMPATIBILITY 7

/**
 * @brief Semantic type derivation error - the type of the variable is not specified and cannot be derived from the expression used
 */
#define ERROR_SEMANTIC_TYPE_INFERENCE_FAILURE 8

/**
 * @brief Semantic error of unused variable in its scope of validity; modifiable variable without the possibility of modification after its initialization
 */
#define ERROR_SEMANTIC_UNUSED_VARIABLE 9

/**
 * @brief Other semantic errors
 */
#define ERROR_SEMANTIC_OTHER_ERRORS 10

/**
 * @brief Internal compiler error, i.e., not affected by the input program (e.g., memory allocation error, etc.)
 */
#define ERROR_INTERNAL_COMPILER_ERROR 99

/**
 * @brief Structure for error representation
 */
typedef struct {
    int code;               // Error code
    const char* message;    // Error message
    int line;               // Line number where the error occurred
    int column;             // Number of the column where the error occurred
} error_t;

/**
 * @brief Sets the current error if no error has been set before
 * 
 * @param code Error code
 * @param message Text message of the error
 * @param line Line number where the error occurred
 * @param column Number of the column where the error occurred
 */
void set_error(int code, const char* message, int line, int column);

/**
 * @brief Returns the current error
 * 
 * @return Structure representing the current error
 */
error_t get_error(void);

/**
 * @brief Prints the current error to stderr if an error exists
 */
void print_error(void);

#endif
