/// IFJ24
/// @brief Header file for scanner.c

#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>
#include <stdlib.h>
#include "dstring.h"
#include "error_codes.h"
#include "token.h"

typedef struct {
    FILE *code_file;
    int line;
    int column;
    dstring_t *current_lexeme;
    int is_eof;
} stdin_code;

typedef enum lex_function_states {
    STATE_START = 0, // Starting state
    STATE_INVALID,  // State when FSM encounters invalid character

    STATE_DIV,
    STATE_COMMENT,

    STATE_INTEGER,
    STATE_DCM_START,
    STATE_FLOAT,
    STATE_EXP_START,
    STATE_EXP_SIGN,
    STATE_EXP_FLOAT,

    STATE_AT,

    // 
    STATE_STR_START,
    STATE_STR_ESC,
    STATE_STR_ESC_X1,
    STATE_STR_ESC_X2,
    STATE_STR_END,
    STATE_STR_ML_BACKSLASH,
    STATE_STR_ML_BODY,
    STATE_STR_ML_NEWLINE_CHECK,
    STATE_STR_ML_SKIP_WHITESPACE,

    STATE_UNDERSCORE,
    STATE_ID_KW,
    STATE_ID_IFJ_FUNC,

    STATE_WHITESPACE,
    STATE_SEPARATOR_WINDOWS, //
    STATE_SEPARATOR,

    STATE_OPENING_BRACKET,
    STATE_CLOSING_BRACKET,
    STATE_OPENING_PARENTHESES,
    STATE_CLOSING_PARENTHESES,
    STATE_OPEN_BRACK_LEFT,
    STATE_OPEN_BRACK_RIGHT,
    STATE_TWODOT,
    STATE_COMMA,
    STATE_SEMICOLON,
    STATE_MUL,
    STATE_ADD,
    STATE_SUB,
    STATE_QUESTION_MARK,
    STATE_NULL_TYPE,

    STATE_NOT,
    STATE_NEQ,
    STATE_ASSIGN,
    STATE_EQ,
    STATE_COMP_L,
    STATE_COMP_G,
    STATE_COMP_LE,
    STATE_COMP_GE,
    STATE_LOGIC_AND1,
    STATE_LOGIC_AND2,
    STATE_VERTICAL_BAR,
    STATE_LOGIC_OR2
    
    
} fsm_state_t;

int is_keyword(dstring_t *lexeme);
int get_next_token(FILE *source, token_t **current_token);

#endif
