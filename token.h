/// IFJ24
/// @brief Defines a structure for token representation in lexical analysis

#ifndef TOKEN_H
#define TOKEN_H

#include "dstring.h"
#include "error_codes.h"


typedef enum {
    //Keywords
    KW_CONST,
    KW_FN,
    KW_IF,
    KW_ELSE,
    KW_WHILE,
    KW_RETURN,
    KW_VAR,
    KW_VOID,
    KW_PUB,
    KW_NULL,
    KW_INT_TYPE,
    KW_FLOAT_TYPE,
    KW_CHAR_TYPE,
    KW_STRING_TYPE,

    TOKEN_INT_NULL,               // ?i32
    TOKEN_FLOAT_NULL,             // ?f64
    TOKEN_STRING_NULL,            //?[]u8

    TOKEN_NULL,                   
    TOKEN_ID,                     // Identifier
    TOKEN_EOF,                    // End of file
    TOKEN_EOL,                    // End of line
    TOKEN_UNDERSCORE,

    TOKEN_OPENING_PARENTHESES,    // (
    TOKEN_CLOSING_PARENTHESES,    // )
    TOKEN_OPENING_BRACKET,        // {
    TOKEN_CLOSING_BRACKET,        // }
    TOKEN_OPEN_BRACK_LEFT,        // [
    TOKEN_OPEN_BRACK_RIGHT,       // ]
    TOKEN_COMMA,                  // ,
    TOKEN_SEMICOLON,              // ;
    TOKEN_TWODOT,                 // :
    TOKEN_QUEST,                  // ?

    TOKEN_ASSIGN,                 // =
    TOKEN_EQ,                     // ==
    TOKEN_COMP_L,                 // <
    TOKEN_COMP_LE,                // <=
    TOKEN_COMP_G,                 // >
    TOKEN_COMP_GE,                // >=
    TOKEN_NOT,                    // Not !
    TOKEN_NEQ,                    // !=
    TOKEN_LOGICAL_AND,            // &&
    TOKEN_VERTICAL_BAR,
    TOKEN_LOGICAL_OR,             // ||

    TOKEN_ADD,                    // +
    TOKEN_SUB,                    // -
    TOKEN_MUL,                    // *
    TOKEN_DIV,                    // /

    TOKEN_STRING,                 // String
    TOKEN_INT,                    // Int
    TOKEN_FLOAT,                  // Double

    TOKEN_IMPORT,                     // @import
    TOKEN_DOT                  // .

} token_type;

typedef union {
    int i;
    float f;
    dstring_t *s;
} token_attribute;

typedef struct {
    token_type type;
    token_attribute attribute;
} token_t;



token_t *token_init(token_type type, token_attribute *attribute);
void token_free(token_t* token);
token_attribute token_get_attribute(token_t* token);
token_type token_get_type(token_t* token);
token_t *token_copy(const token_t *original);

#endif
