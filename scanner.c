/// IFJ24
/// @brief Scanner implementation

#include "scanner.h"
#include <stdbool.h>
#include <ctype.h>

/**
 * @brief Checks if the provided lexeme matches any known keyword.
 * @param lexeme The lexeme to check.
 * @return The corresponding keyword token type or -1 if the lexeme is not a keyword.
 */
int is_keyword(dstring_t *lexeme)
{
    if (dstring_compare_charstr(lexeme, "f64") == 0)
        return KW_FLOAT_TYPE;
    if (dstring_compare_charstr(lexeme, "else") == 0)
        return KW_ELSE;
    if (dstring_compare_charstr(lexeme, "fn") == 0)
        return KW_FN;
    if (dstring_compare_charstr(lexeme, "if") == 0)
        return KW_IF;
    if (dstring_compare_charstr(lexeme, "i32") == 0)
        return KW_INT_TYPE;
    if (dstring_compare_charstr(lexeme, "pub") == 0)
        return KW_PUB;
    if (dstring_compare_charstr(lexeme, "return") == 0)
        return KW_RETURN;
    if (dstring_compare_charstr(lexeme, "u8") == 0)
        return KW_CHAR_TYPE;
    if (dstring_compare_charstr(lexeme, "[]u8") == 0)
        return KW_STRING_TYPE;
    if (dstring_compare_charstr(lexeme, "var") == 0)
        return KW_VAR;
    if (dstring_compare_charstr(lexeme, "while") == 0)
        return KW_WHILE;
    if (dstring_compare_charstr(lexeme, "null") == 0)
        return KW_NULL;
    if (dstring_compare_charstr(lexeme, "void") == 0)
        return KW_VOID;
    if (dstring_compare_charstr(lexeme, "const") == 0)
        return KW_CONST;
    return -1;
}

char *builtin_functions[] = {
    "ifj.readstr",
    "ifj.readi32",
    "ifj.readf64",
    "ifj.write",
    "ifj.i2f",
    "ifj.f2i",
    "ifj.length",
    "ifj.string",
    "ifj.concat",
    "ifj.substring",
    "ifj.strcmp",
    "ifj.ord",
    "ifj.chr"};

/**
 * @brief Checks if a given lexeme matches any built-in function name.
 * @param lexeme Pointer to the lexeme to check.
 * @return 0 if the lexeme matches a built-in function name, -1 otherwise.
 */
int is_built_in(dstring_t *lexeme)
{
    for (size_t i = 0; i < 13; ++i)
    {
        if (dstring_compare_charstr(lexeme, builtin_functions[i]) == 0)
        {
            return 0;
        }
    }
    return -1;
}

/**
 * @brief Processes a string literal by handling escape sequences and removing quotes.
 * @param lexeme Tthe string literal.
 * @return The processed string.
 */
dstring_t *process_str(dstring_t *lexeme)
{
    dstring_t *new_lex = dstring_init();
    if (new_lex == NULL)
    {
        set_error(ERROR_INTERNAL_COMPILER_ERROR, "Memory allocation failed", -1, -1);
        return NULL;
    }

    // Determine if it's a multiline string or a regular string
    bool is_multiline = (lexeme->length > 1 && lexeme->data[0] == '\\' && lexeme->data[1] == '\\');
    size_t start = 0;
    size_t end = lexeme->length;

    if (!is_multiline && lexeme->data[0] == '"' && lexeme->data[lexeme->length - 1] == '"')
    {
        start = 1;
        end = lexeme->length - 1;
    }

    for (size_t i = start; i < end; i++)
    {
        char current = lexeme->data[i];

        if (current == '\\')
        {
            i++;
            if (i >= lexeme->length)
            {
                set_error(ERROR_LEXICAL_ANALYSIS, "Invalid escape sequence", -1, -1);
                dstring_free(new_lex);
                return NULL;
            }
            char next = lexeme->data[i];
            switch (next)
            {
            case 'n':
                dstring_add_str(new_lex, "\\010");
                break;
            case 't':
                dstring_add_str(new_lex, "\\009");
                break;
            case 'r':
                dstring_add_str(new_lex, "\\013");
                break;
            case '\\':
                if(!is_multiline){
                    dstring_add_str(new_lex, "\\092");
                }    
                break;
            case '"':
                dstring_add_str(new_lex, "\"");
                break;
            case 'x':
            {
                if (i + 2 >= lexeme->length || !isxdigit(lexeme->data[i + 1]) || !isxdigit(lexeme->data[i + 2]))
                {
                    set_error(ERROR_LEXICAL_ANALYSIS, "Invalid \\x escape sequence: must be followed by exactly two hexadecimal digits", -1, -1);
                    dstring_free(new_lex);
                    return NULL;
                }

                char hex[3] = {lexeme->data[i + 1], lexeme->data[i + 2], '\0'};
                i += 2;

                int ascii_value = (int)strtol(hex, NULL, 16);

                if (ascii_value > 255)
                {
                    set_error(ERROR_LEXICAL_ANALYSIS, "Invalid \\x escape sequence: value out of range", -1, -1);
                    dstring_free(new_lex);
                    return NULL;
                }

                char encoded[5];
                snprintf(encoded, sizeof(encoded), "\\%03d", ascii_value);
                dstring_add_str(new_lex, encoded);

                break;
            }

            default:
                set_error(ERROR_LEXICAL_ANALYSIS, "Invalid escape sequence", -1, -1);
                dstring_free(new_lex);
                return NULL;
            }
        }
        else if (current <= 32 || current == 35)
        {
            char encoded[5];
            snprintf(encoded, sizeof(encoded), "\\%03d", current);
            dstring_add_str(new_lex, encoded);
        }
        else
        {
            dstring_add_char(new_lex, current);
        }
    }

    return new_lex;
}

/**
 * @brief Retrieves the next token from the input stream.
 * @return A pointer to the next token or NULL if an error occurred.
 */
int get_next_token(FILE *source, token_t **current_token)
{
    if (*current_token != NULL)
    {
        token_free(*current_token);
        *current_token = NULL;
    }
    fsm_state_t state = STATE_START;
    dstring_t *lexeme = dstring_init();
    token_attribute attribute;

    if (lexeme == NULL)
    {
        set_error(ERROR_INTERNAL_COMPILER_ERROR, "Memory allocation failed for lexeme", -1, -1);
        return ERROR_INTERNAL_COMPILER_ERROR;
    }

    char c;

    while (1)
    {
        c = getc(source);
        switch (state)
        {
        case STATE_START:
            switch (c)
            {
            case '"':
                state = STATE_STR_START;
                dstring_add_char(lexeme, c);
                break;
            case '0' ... '9':
                state = STATE_INTEGER;
                dstring_add_char(lexeme, c);
                break;
            case '\\':
                state = STATE_STR_ML_BACKSLASH;
                dstring_add_char(lexeme, c);
                break;
            case '+':
                *current_token = token_init(TOKEN_ADD, NULL);
                dstring_free(lexeme);
                return 0;
            case '*':
                *current_token = token_init(TOKEN_MUL, NULL);
                dstring_free(lexeme);
                return 0;
            case '-':
                *current_token = token_init(TOKEN_SUB, NULL);
                dstring_free(lexeme);
                return 0;
            case '/':
                state = STATE_DIV;
                break;
            case '!':
                state = STATE_NOT;
                break;
            case '?':
                state = STATE_QUESTION_MARK;
                break;
            case '>':
                state = STATE_COMP_G;
                break;
            case '<':
                state = STATE_COMP_L;
                break;
            case '=':
                state = STATE_ASSIGN;
                break;
            case '&':
                state = STATE_LOGIC_AND1;
                break;
            case '|':
                state = STATE_VERTICAL_BAR;
                break;
            case '{':
                *current_token = token_init(TOKEN_OPENING_BRACKET, NULL);
                dstring_free(lexeme);
                return 0;
            case '}':
                *current_token = token_init(TOKEN_CLOSING_BRACKET, NULL);
                dstring_free(lexeme);
                return 0;
            case '(':
                *current_token = token_init(TOKEN_OPENING_PARENTHESES, NULL);
                dstring_free(lexeme);
                return 0;
            case ')':
                *current_token = token_init(TOKEN_CLOSING_PARENTHESES, NULL);
                dstring_free(lexeme);
                return 0;
            case '[':
                state = STATE_OPEN_BRACK_LEFT;
                dstring_add_char(lexeme, c);
                break;
            case ']':
                state = STATE_OPEN_BRACK_RIGHT;
                break;
            case ':':
                *current_token = token_init(TOKEN_TWODOT, NULL);
                dstring_free(lexeme);
                return 0;
            case ',':
                *current_token = token_init(TOKEN_COMMA, NULL);
                dstring_free(lexeme);
                return 0;
            case ';':
                *current_token = token_init(TOKEN_SEMICOLON, NULL);
                dstring_free(lexeme);
                return 0;
            case '_':
                state = STATE_UNDERSCORE;
                break;
            case '@':
                state = STATE_AT;
                dstring_add_char(lexeme, c);
                break;
            case 'a' ... 'z':
            case 'A' ... 'Z':
                state = STATE_ID_KW;
                dstring_add_char(lexeme, c);
                break;
            case ' ':
            case '\t':
            case '\n':
                state = STATE_START;
                break;
            case EOF:
                *current_token = token_init(TOKEN_EOF, NULL);
                dstring_free(lexeme);
                return 0;
            default:
                set_error(ERROR_LEXICAL_ANALYSIS, "Invalid character", -1, -1);
                dstring_free(lexeme);
                return ERROR_LEXICAL_ANALYSIS;
            }
            break;
        case STATE_AT:
            switch (c)
            {
            case 'a' ... 'z':
            case 'A' ... 'Z':
                dstring_add_char(lexeme, c);
                break;
            default:
                ungetc(c, source);
                if (dstring_compare_charstr(lexeme, "@import") != 0)
                {
                    set_error(ERROR_LEXICAL_ANALYSIS, "Invalid character", -1, -1);
                    dstring_free(lexeme);
                    return ERROR_LEXICAL_ANALYSIS;
                }
                *current_token = token_init(TOKEN_IMPORT, NULL);
                dstring_free(lexeme);
                return 0;
            }
            break;

        case STATE_DIV:
            if (c == '/')
            {
                state = STATE_COMMENT;
            }
            else
            {
                ungetc(c, source);
                *current_token = token_init(TOKEN_DIV, NULL);
                dstring_free(lexeme);
                return 0;
            }
            break;
        case STATE_OPEN_BRACK_LEFT:
            if (c == ']')
            {
                dstring_add_char(lexeme, c);
                state = STATE_OPEN_BRACK_RIGHT;
            }
            else
            {
                ungetc(c, source);
                *current_token = token_init(TOKEN_OPEN_BRACK_LEFT, NULL);
                dstring_free(lexeme);
                return 0;
            }
            break;
        case STATE_OPEN_BRACK_RIGHT:
            switch (c)
            {
            case '_':
            case 'a' ... 'z':
            case 'A' ... 'Z':
            case '0' ... '9':
                dstring_add_char(lexeme, c);
                state = STATE_ID_KW;
                break;
            default:
                ungetc(c, source);
                *current_token = token_init(TOKEN_OPEN_BRACK_RIGHT, NULL);
                dstring_free(lexeme);
                return 0;
            }
            break;
        case STATE_COMMENT:
            if (c == '\n')
            {
                state = STATE_START;
            }
            break;
        case STATE_QUESTION_MARK:
            switch (c)
            {
            case 'a' ... 'z':
            case 'A' ... 'Z':
            case '[':
                state = STATE_NULL_TYPE;
                dstring_add_char(lexeme, c);
                break;
            default:
                ungetc(c, source);
                *current_token = token_init(TOKEN_QUEST, NULL);
                dstring_free(lexeme);
                return 0;
            }
            break;
        case STATE_NULL_TYPE:
            if (c == ']' || c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
            {
                dstring_add_char(lexeme, c);
            }
            else
            {
                ungetc(c, source);
                token_type keyword = is_keyword(lexeme);
                if (keyword == -1)
                {
                    set_error(ERROR_LEXICAL_ANALYSIS, "Invalid keyword", -1, -1);
                    dstring_free(lexeme);
                    return ERROR_LEXICAL_ANALYSIS;
                }
                switch (keyword)
                {
                case KW_INT_TYPE:
                    *current_token = token_init(TOKEN_INT_NULL, NULL);
                    dstring_free(lexeme);
                    return 0;
                case KW_FLOAT_TYPE:
                    *current_token = token_init(TOKEN_FLOAT_NULL, NULL);
                    dstring_free(lexeme);
                    return 0;
                case KW_STRING_TYPE:
                    *current_token = token_init(TOKEN_STRING_NULL, NULL);
                    dstring_free(lexeme);
                    return 0;
                default:
                    set_error(ERROR_LEXICAL_ANALYSIS, "Invalid keyword", -1, -1);
                    dstring_free(lexeme);
                    return ERROR_LEXICAL_ANALYSIS;
                }
            }
            break;
        case STATE_NOT:
            if (c == '=')
            {
                state = STATE_NEQ;
                break;
            }
            else
            {
                ungetc(c, source);
                *current_token = token_init(TOKEN_NOT, NULL);
                dstring_free(lexeme);
                return 0;
            }
            break;
        case STATE_NEQ:
            ungetc(c, source);
            *current_token = token_init(TOKEN_NEQ, NULL);
            dstring_free(lexeme);
            return 0;
        case STATE_ASSIGN:
            if (c == '=')
            {
                state = STATE_EQ;
                break;
            }
            else
            {
                ungetc(c, source);
                *current_token = token_init(TOKEN_ASSIGN, NULL);
                dstring_free(lexeme);
                return 0;
            }
            break;
        case STATE_EQ:
            ungetc(c, source);
            *current_token = token_init(TOKEN_EQ, NULL);
            dstring_free(lexeme);
            return 0;
        case STATE_COMP_L:
            if (c == '=')
            {
                state = STATE_COMP_LE;
                break;
            }
            else
            {
                ungetc(c, source);
                *current_token = token_init(TOKEN_COMP_L, NULL);
                dstring_free(lexeme);
                return 0;
            }
            break;
        case STATE_COMP_LE:
            ungetc(c, source);
            *current_token = token_init(TOKEN_COMP_LE, NULL);
            dstring_free(lexeme);
            return 0;
        case STATE_COMP_G:
            if (c == '=')
            {
                state = STATE_COMP_GE;
                break;
            }
            else
            {
                ungetc(c, source);
                *current_token = token_init(TOKEN_COMP_G, NULL);
                dstring_free(lexeme);
                return 0;
            }
            break;
        case STATE_COMP_GE:
            ungetc(c, source);
            *current_token = token_init(TOKEN_COMP_GE, NULL);
            dstring_free(lexeme);
            return 0;
        case STATE_LOGIC_AND1:
            if (c == '&')
            {
                state = STATE_LOGIC_AND2;
                break;
            }
            else
            {
                set_error(ERROR_LEXICAL_ANALYSIS, "Invalid character", -1, -1);
                dstring_free(lexeme);
                return ERROR_LEXICAL_ANALYSIS;
            }
            break;
        case STATE_LOGIC_AND2:
            ungetc(c, source);
            *current_token = token_init(TOKEN_LOGICAL_AND, NULL);
            dstring_free(lexeme);
            return 0;
        case STATE_VERTICAL_BAR:
            if (c == '|')
            {
                state = STATE_LOGIC_OR2;
                break;
            }
            else
            {
                ungetc(c, source);
                *current_token = token_init(TOKEN_VERTICAL_BAR, NULL);
                dstring_free(lexeme);
                return 0;
            }
            break;
        case STATE_LOGIC_OR2:
            ungetc(c, source);
            *current_token = token_init(TOKEN_LOGICAL_OR, NULL);
            dstring_free(lexeme);
            return 0;
        case STATE_UNDERSCORE:
            switch (c)
            {
            case 'a' ... 'z':
            case 'A' ... 'Z':
            case '0' ... '9':
                state = STATE_ID_KW;
                dstring_add_char(lexeme, '_');
                dstring_add_char(lexeme, c);
                break;
            default:
                ungetc(c, source);
                *current_token = token_init(TOKEN_UNDERSCORE, NULL);
                dstring_free(lexeme);
                return 0;
            }
        case STATE_ID_KW:
            switch (c)
            {
            case '_':
            case 'a' ... 'z':
            case 'A' ... 'Z':
            case '0' ... '9':
                dstring_add_char(lexeme, c);
                break;
            case ' ':
            case '\t':
                state = STATE_WHITESPACE;
                break;
            case '.':
                dstring_add_char(lexeme, c);
                state = STATE_ID_IFJ_FUNC;
                break;
            default:
                ungetc(c, source);
                token_type keyword = is_keyword(lexeme);
                if (keyword == -1)
                {
                    attribute.s = dstring_copy(lexeme);
                    *current_token = token_init(TOKEN_ID, &attribute);
                }
                else
                {
                    *current_token = token_init(keyword, NULL);
                }
                dstring_free(lexeme);
                return 0;
            }
            break;
        case STATE_WHITESPACE:
            switch (c)
            {
            case ' ':
            case '\t':
                break;
            case '.':
                dstring_add_char(lexeme, c);
                state = STATE_ID_IFJ_FUNC;
                break;
            default:
                ungetc(c, source);
                token_type keyword = is_keyword(lexeme);
                if (keyword == -1)
                {
                    attribute.s = dstring_copy(lexeme);
                    *current_token = token_init(TOKEN_ID, &attribute);
                }
                else
                {
                    *current_token = token_init(keyword, NULL);
                }
                dstring_free(lexeme);
                return 0;
            }
            break;
        case STATE_ID_IFJ_FUNC:
            switch (c)
            {
            case 'a' ... 'z':
            case 'A' ... 'Z':
            case '0' ... '9':
                dstring_add_char(lexeme, c);
                break;
            case ' ':
            case '\t':
            case '\n':
                break;
            default:
                ungetc(c, source);
                int res = is_built_in(lexeme);
                if (res == 0)
                {
                    attribute.s = dstring_copy(lexeme);
                    *current_token = token_init(TOKEN_ID, &attribute);
                }
                else
                {
                    set_error(ERROR_LEXICAL_ANALYSIS, "Invalid built in ifj function", -1, -1);
                    dstring_free(lexeme);

                    return ERROR_LEXICAL_ANALYSIS;
                }
                dstring_free(lexeme);
                return 0;
            }
            break;
        case STATE_INTEGER:
            switch (c)
            {
            case '0' ... '9':
                if (lexeme->data[0] == '0')
                {
                    set_error(ERROR_LEXICAL_ANALYSIS, "Number cannot start with 0 unless it is 0 or a decimal", -1, -1);
                    dstring_free(lexeme);
                    return ERROR_LEXICAL_ANALYSIS;
                }
                dstring_add_char(lexeme, c);
                break;
            case '.':
                state = STATE_DCM_START;
                dstring_add_char(lexeme, c);
                break;
            case 'e':
            case 'E':
                state = STATE_EXP_START;
                dstring_add_char(lexeme, c);
                break;
            default:
                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
                {
                    set_error(ERROR_LEXICAL_ANALYSIS, "Invalid character in numeric literal", -1, -1);
                    dstring_free(lexeme);
                    return ERROR_LEXICAL_ANALYSIS;
                }
                ungetc(c, source);
                attribute.i = (int)strtol(lexeme->data, NULL, 10);
                *current_token = token_init(TOKEN_INT, &attribute);
                dstring_free(lexeme);
                return 0;
            }
            break;
        case STATE_DCM_START:
            if (c >= '0' && c <= '9')
            {
                state = STATE_FLOAT;
                dstring_add_char(lexeme, c);
            }
            else
            {
                set_error(ERROR_LEXICAL_ANALYSIS, "Invalid decimal number format", -1, -1);
                dstring_free(lexeme);

                return ERROR_LEXICAL_ANALYSIS;
            }
            break;
        case STATE_FLOAT:
            if (c >= '0' && c <= '9')
            {
                dstring_add_char(lexeme, c);
                break;
            }
            else if (c == 'e' || c == 'E')
            {
                state = STATE_EXP_START;
                dstring_add_char(lexeme, c);
                break;
            }
            else
            {
                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
                {
                    set_error(ERROR_LEXICAL_ANALYSIS, "Invalid character in float", -1, -1);
                    dstring_free(lexeme);
                    return ERROR_LEXICAL_ANALYSIS;
                }
                ungetc(c, source);
                attribute.f = atof(lexeme->data);
                *current_token = token_init(TOKEN_FLOAT, &attribute);
                dstring_free(lexeme);
                return 0;
            }
            break;
        case STATE_EXP_START:
            if (c >= '0' && c <= '9')
            {
                state = STATE_EXP_FLOAT;
                dstring_add_char(lexeme, c);
                break;
            }
            else if (c == '-' || c == '+')
            {
                state = STATE_EXP_SIGN;
                dstring_add_char(lexeme, c);
                break;
            }
            else
            {
                set_error(ERROR_LEXICAL_ANALYSIS, "Invalid exponent format", -1, -1);
                dstring_free(lexeme);

                return ERROR_LEXICAL_ANALYSIS;
            }
            break;
        case STATE_EXP_SIGN:
            if (c >= '0' && c <= '9')
            {
                state = STATE_EXP_FLOAT;
                dstring_add_char(lexeme, c);
                break;
            }
            else
            {
                set_error(ERROR_LEXICAL_ANALYSIS, "Expected a digit after exponent sign", -1, -1);
                dstring_free(lexeme);

                return ERROR_LEXICAL_ANALYSIS;
            }
            break;
        case STATE_EXP_FLOAT:
            if (c >= '0' && c <= '9')
            {
                dstring_add_char(lexeme, c);
                break;
            }
            else
            {
                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
                {
                    set_error(ERROR_LEXICAL_ANALYSIS, "Invalid character in float", -1, -1);
                    dstring_free(lexeme);
                    return ERROR_LEXICAL_ANALYSIS;
                }
                ungetc(c, source);
                attribute.f = atof(lexeme->data);
                *current_token = token_init(TOKEN_FLOAT, &attribute);
                dstring_free(lexeme);
                return 0;
            }
            break;
        case STATE_STR_START:
            if (c == '"')
            {
                state = STATE_STR_END;
                dstring_add_char(lexeme, c);
                break;
            }
            else if (c == '\\')
            {
                state = STATE_STR_ESC;
                dstring_add_char(lexeme, c);
                break;
            }
            else if (c == '\n' || c == '\0' || c == EOF || c == '\t')
            {
                set_error(ERROR_LEXICAL_ANALYSIS, "Unterminated string literal", -1, -1);
                dstring_free(lexeme);

                return ERROR_LEXICAL_ANALYSIS;
            }
            else
            {
                dstring_add_char(lexeme, c);
                break;
            }
            break;
        case STATE_STR_END:
            ungetc(c, source);
            attribute.s = process_str(lexeme);
            if (attribute.s == NULL)
            {
                set_error(ERROR_LEXICAL_ANALYSIS, "Invalid string", -1, -1);

                return ERROR_LEXICAL_ANALYSIS;
            }
            *current_token = token_init(TOKEN_STRING, &attribute);
            dstring_free(lexeme);
            return 0;
        case STATE_STR_ESC:
            switch (c)
            {
            case '"':
            case 'n':
            case 'r':
            case 't':
            case '\\':
                dstring_add_char(lexeme, c);
                state = STATE_STR_START;
                break;
            case 'x':
                dstring_add_char(lexeme, c);
                state = STATE_STR_ESC_X1;
                break;
            default:
                set_error(ERROR_LEXICAL_ANALYSIS, "Invalid escape sequence", -1, -1);
                dstring_free(lexeme);

                return ERROR_LEXICAL_ANALYSIS;
            }
            break;
        case STATE_STR_ESC_X1:
            switch (c)
            {
            case '0' ... '9':
            case 'a' ... 'f':
            case 'A' ... 'F':
                dstring_add_char(lexeme, c);
                state = STATE_STR_ESC_X2;
                break;
            default:
                set_error(ERROR_LEXICAL_ANALYSIS, "Invalid Unicode escape sequence", -1, -1);
                dstring_free(lexeme);

                return ERROR_LEXICAL_ANALYSIS;
            }
            break;
        case STATE_STR_ESC_X2:
            switch (c)
            {
            case '0' ... '9':
            case 'a' ... 'f':
            case 'A' ... 'F':
                dstring_add_char(lexeme, c);
                state = STATE_STR_START;
                break;
            default:
                set_error(ERROR_LEXICAL_ANALYSIS, "Invalid Unicode escape sequence", -1, -1);
                dstring_free(lexeme);

                return ERROR_LEXICAL_ANALYSIS;
            }
            break;
        case STATE_STR_ML_BACKSLASH:
            if (c == '\\')
            {
                state = STATE_STR_ML_BODY;
                dstring_add_char(lexeme, c);
            }
            else
            {
                set_error(ERROR_LEXICAL_ANALYSIS, "Unexpected character after backslash", -1, -1);
                dstring_free(lexeme);

                return ERROR_LEXICAL_ANALYSIS;
            }
            break;
        case STATE_STR_ML_BODY:
            if (c == '\n')
            {
                state = STATE_STR_ML_NEWLINE_CHECK;
            }
            else if (c == EOF)
            {
                attribute.s = process_str(lexeme);
                if (attribute.s == NULL)
                {
                    set_error(ERROR_LEXICAL_ANALYSIS, "Invalid string", -1, -1);
                    dstring_free(lexeme);

                    return ERROR_LEXICAL_ANALYSIS;
                }
                *current_token = token_init(TOKEN_STRING, &attribute);
                dstring_free(lexeme);
                return 0;
            }
            else
            {
                dstring_add_char(lexeme, c);
            }
            break;
        case STATE_STR_ML_NEWLINE_CHECK:
            if (c == '\t' || c == ' ')
            {
                state = STATE_STR_ML_SKIP_WHITESPACE;
            }
            else if (c == '\\')
            {
                dstring_add_char(lexeme, '\n');
                state = STATE_STR_ML_BACKSLASH;
                dstring_add_char(lexeme, c);
            }
            else
            {
                ungetc(c, source);
                attribute.s = process_str(lexeme);
                if (attribute.s == NULL)
                {
                    set_error(ERROR_LEXICAL_ANALYSIS, "Invalid string", -1, -1);
                    dstring_free(lexeme);

                    return ERROR_LEXICAL_ANALYSIS;
                }
                *current_token = token_init(TOKEN_STRING, &attribute);
                dstring_free(lexeme);
                return 0;
            }
            break;
        case STATE_STR_ML_SKIP_WHITESPACE:
            if (c == '\t' || c == ' ')
            {
                // continue skipping
            }
            else if (c == '\\')
            {
                dstring_add_char(lexeme, '\n');
                state = STATE_STR_ML_BACKSLASH;
                dstring_add_char(lexeme, c);
            }
            else
            {
                ungetc(c, source);
                attribute.s = process_str(lexeme);
                if (attribute.s == NULL)
                {
                    set_error(ERROR_LEXICAL_ANALYSIS, "Invalid string", -1, -1);
                    dstring_free(lexeme);

                    return ERROR_LEXICAL_ANALYSIS;
                }
                *current_token = token_init(TOKEN_STRING, &attribute);
                dstring_free(lexeme);
                return 0;
            }
            break;
        default:
            set_error(ERROR_LEXICAL_ANALYSIS, "Invalid state", -1, -1);
            dstring_free(lexeme);

            return ERROR_LEXICAL_ANALYSIS;
        }
    }

    return 0;
}
