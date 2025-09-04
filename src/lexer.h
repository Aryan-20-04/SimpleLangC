#ifndef LEXER_H
#define LEXER_H

typedef enum
{
    TOKEN_NUM,
    TOKEN_ID,
    TOKEN_PRINT,
    TOKEN_LET,
    TOKEN_EQUAL,
    TOKEN_PLUS,
    TOKEN_SUB,
    TOKEN_SEMI,
    TOKEN_STR,
    TOKEN_EOF

}TokenType;

typedef struct
{
    TokenType type;
    char text[64];
}Token;

Token getNextToken(const char **src);

#endif