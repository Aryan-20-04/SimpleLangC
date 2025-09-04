#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

enum Tokenizer
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
};

typedef struct
{
    enum Tokenizer type;
    char text[64];
} Token;
Token getNextToken(const char **src)
{
    static int call_count = 0;
    call_count++;

    while (**src && isspace(**src))
        (*src)++;

    if (**src == '\0')
    {

        return (Token){TOKEN_EOF, ""};
    }

    // Raw string: r"..." - CHECK THIS FIRST before identifiers
    if (**src == 'r' && *(*src + 1) == '"')
    {

        (*src) += 2; // Skip 'r"'
        Token tk = {TOKEN_STR, ""};
        size_t i = 0;
        while (**src && **src != '"' && i < sizeof(tk.text) - 1)
            tk.text[i++] = *(*src)++;
        tk.text[i] = '\0';
        if (**src == '"') // Skip closing quote
            (*src)++;
        return tk;
    }

    // Triple-quoted string: """...""" - CHECK BEFORE normal strings
    if (**src == '"' && *(*src + 1) == '"' && *(*src + 2) == '"')
    {

        (*src) += 3; // Skip opening """
        Token tk = {TOKEN_STR, ""};
        size_t i = 0;
        while (**src && !(**src == '"' && *(*src + 1) == '"' && *(*src + 2) == '"'))
        {
            if (i < sizeof(tk.text) - 1)
                tk.text[i++] = *(*src)++;
            else
                (*src)++;
        }
        tk.text[i] = '\0';
        if (**src) // Skip closing """
            (*src) += 3;
        return tk;
    }

    // Normal string: "..."
    if (**src == '"')
    {

        (*src)++; // Skip opening quote
        Token tk = {TOKEN_STR, ""};
        size_t i = 0;
        while (**src && **src != '"' && i < sizeof(tk.text) - 1)
        {
            if (**src == '\\')
            {
                (*src)++;
                switch (**src)
                {
                case 'n':
                    tk.text[i++] = '\n';
                    break;
                case 't':
                    tk.text[i++] = '\t';
                    break;
                case '\\':
                    tk.text[i++] = '\\';
                    break;
                case '"':
                    tk.text[i++] = '"';
                    break;
                default:
                    tk.text[i++] = **src;
                    break;
                }
            }
            else
            {
                tk.text[i++] = **src;
            }
            (*src)++;
        }
        tk.text[i] = '\0';
        if (**src == '"') // Skip closing quote
            (*src)++;
        return tk;
    }

    // Numbers
    if (isdigit(**src))
    {

        Token tk = {TOKEN_NUM, ""};
        size_t i = 0;
        while (isdigit(**src) && i < sizeof(tk.text) - 1)
            tk.text[i++] = *(*src)++;
        tk.text[i] = '\0';

        return tk;
    }

    // Identifiers / keywords
    if (isalpha(**src))
    {

        Token tk = {TOKEN_ID, ""};
        size_t i = 0;
        while (isalnum(**src) && i < sizeof(tk.text) - 1)
            tk.text[i++] = *(*src)++;
        tk.text[i] = '\0';

        if (strcmp(tk.text, "print") == 0)
            tk.type = TOKEN_PRINT;
        else if (strcmp(tk.text, "let") == 0)
            tk.type = TOKEN_LET;
        return tk;
    }

    // Symbols

    if (**src == '+')
    {

        (*src)++;

        return (Token){TOKEN_PLUS, "+"};
    }
    else if (**src == '-')
    {

        (*src)++;

        return (Token){TOKEN_SUB, "-"};
    }
    else if (**src == '=')
    {

        (*src)++;
        return (Token){TOKEN_EQUAL, "="};
    }
    else if (**src == ';')
    {

        (*src)++;
        return (Token){TOKEN_SEMI, ";"};
    }

    // Unknown character, skip

    (*src)++;
    return (Token){TOKEN_EOF, ""};
}