/**
 * IFJ24
 * @brief Definition of precedence symbols for expression parsing in IFJ24.
 */

#ifndef PREC_SYM_TYPES_H
#define PREC_SYM_TYPES_H

typedef enum prec_symbols {
    ADD,         // +
    SUB,         // -
    MUL,         // *
    DIV,         // /
    EQ,          // ==
    NEQ,         // !=
    NOT,         // !
    LT,          // <
    GT,          // >
    LE,          // <=
    GE,          // >=
    AND,         // &&
    OR,          // ||
    LPAR,        // (
    RPAR,        // )
    ID,          // i
    DOLLAR,      // $
    EXP,                // E
    UNDEFINED,
} prec_sym_t;

#endif 