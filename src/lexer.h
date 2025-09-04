#ifndef LEXER_H
#define LEXER_H

typedef enum
{
    TOKEN_NUM,
    TOKEN_ID,
    TOKEN_PRINT,
    TOKEN_LET,
    TOKEN_IF,    // New: if keyword
    TOKEN_EQUAL, // = (assignment)
    TOKEN_EQ,    // New: == (comparison)
    TOKEN_NE,    // New: != (not equal)
    TOKEN_LT,    // New: < (less than)
    TOKEN_GT,    // New: > (greater than)
    TOKEN_LE,    // New: <= (less or equal)
    TOKEN_GE,    // New: >= (greater or equal)
    TOKEN_PLUS,
    TOKEN_SUB,
    TOKEN_SEMI,
    TOKEN_STR,
    TOKEN_MUL,
    TOKEN_DIV,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE, // New: {
    TOKEN_RBRACE, // New: }
    TOKEN_EOF
} TokenType;

typedef struct
{
    TokenType type;
    char text[64];
} Token;

Token getNextToken(const char **src);

#endif