#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "lexer.h"

Token getNextToken(const char **src)
{
    // Skip whitespace
    while (**src && isspace(**src))
        (*src)++;

    if (**src == '\0')
        return (Token){TOKEN_EOF, ""};

    // Raw string: r"..."
    if (**src == 'r' && *(*src + 1) == '"')
    {
        (*src) += 2;
        Token tk = {TOKEN_STR, ""};
        size_t i = 0;
        while (**src && **src != '"' && i < sizeof(tk.text) - 1)
            tk.text[i++] = *(*src)++;
        tk.text[i] = '\0';
        if (**src == '"')
            (*src)++;
        return tk;
    }

    // Triple-quoted string: """..."""
    if (**src == '"' && *(*src + 1) == '"' && *(*src + 2) == '"')
    {
        (*src) += 3;
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
        if (**src)
            (*src) += 3;
        return tk;
    }

    // Normal string: "..."
    if (**src == '"')
    {
        (*src)++;
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
                case '"':
                    tk.text[i++] = '"';
                    break;
                case '\\':
                    tk.text[i++] = '\\';
                    break;
                default:
                    tk.text[i++] = **src;
                    break;
                }
            }
            else
                tk.text[i++] = **src;
            (*src)++;
        }
        tk.text[i] = '\0';
        if (**src == '"')
            (*src)++;
        return tk;
    }

    // Numbers
    if (isdigit(**src))
    {
        const char *start = *src;
        while (isdigit(**src))
            (*src)++;
        if (**src == '.')
        {
            (*src)++;
            while (isdigit(**src))
                (*src)++;
        }
        int len = *src - start;
        if (len >= 64)
            len = 63;
        Token tk;
        tk.type = TOKEN_NUM;
        strncpy(tk.text, start, len);
        tk.text[len] = '\0';
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
        // Keywords
        if (strcmp(tk.text, "print") == 0)
            tk.type = TOKEN_PRINT;
        else if (strcmp(tk.text, "let") == 0)
            tk.type = TOKEN_LET;
        else if (strcmp(tk.text, "if") == 0)
            tk.type = TOKEN_IF;
        else if (strcmp(tk.text, "else") == 0)
            tk.type = TOKEN_ELSE;
        else if (strcmp(tk.text, "elseif") == 0)
            tk.type = TOKEN_ELSEIF;
        else if (strcmp(tk.text, "for") == 0)
            tk.type = TOKEN_FOR;
        else if (strcmp(tk.text, "in") == 0)
            tk.type = TOKEN_IN;
        else if (strcmp(tk.text, "range") == 0)
            tk.type = TOKEN_RANGE;
        return tk;
    }

    // Multi-char operators
    if (**src == '=' && *(*src + 1) == '=')
    {
        (*src) += 2;
        return (Token){TOKEN_EQ, "=="};
    }
    if (**src == '!' && *(*src + 1) == '=')
    {
        (*src) += 2;
        return (Token){TOKEN_NE, "!="};
    }
    if (**src == '<' && *(*src + 1) == '=')
    {
        (*src) += 2;
        return (Token){TOKEN_LE, "<="};
    }
    if (**src == '>' && *(*src + 1) == '=')
    {
        (*src) += 2;
        return (Token){TOKEN_GE, ">="};
    }

    // Single-character tokens
    char ch = **src;
    (*src)++;
    Token tk = {TOKEN_EOF, ""};
    tk.text[0] = ch;
    tk.text[1] = '\0';

    switch (ch)
    {
    case '+':
        tk.type = TOKEN_PLUS;
        break;
    case '-':
        tk.type = TOKEN_SUB;
        break;
    case '*':
        tk.type = TOKEN_MUL;
        break;
    case '/':
        tk.type = TOKEN_DIV;
        break;
    case '(':
        tk.type = TOKEN_LPAREN;
        break;
    case ')':
        tk.type = TOKEN_RPAREN;
        break;
    case ';':
        tk.type = TOKEN_SEMI;
        break;
    case '=':
        tk.type = TOKEN_EQUAL;
        break;
    case '<':
        tk.type = TOKEN_LT;
        break;
    case '>':
        tk.type = TOKEN_GT;
        break;
    case '{':
        tk.type = TOKEN_LBRACE;
        break;
    case '}':
        tk.type = TOKEN_RBRACE;
        break;
    case ',':
        tk.type = TOKEN_COMMA;
        break;
    case '[':
        tk.type = TOKEN_LBRACKET;
        break;
    case ']':
        tk.type = TOKEN_RBRACKET;
        break;
    default:
        tk.type = TOKEN_EOF;
        break;
    }
    return tk;
}
