#include <stdio.h>
#include <stdlib.h>
#include "interpreter.h"
#include "symbol.h"

extern Token getNextToken(const char **src);

// Forward declaration
int parseExpression(const char **src);

// Parse a primary expression (number, variable, or parenthesized expression)
int parsePrimary(const char **src)
{
    Token tk = getNextToken(src);

    if (tk.type == TOKEN_NUM)
    {
        return atoi(tk.text);
    }
    else if (tk.type == TOKEN_ID)
    {
        return getVar(tk.text);
    }
    else
    {
        printf("Syntax Error: Expected number or variable, got '%s'\n", tk.text);
        return 0;
    }
}

// Parse an expression with + and - operators (left associative)
int parseExpression(const char **src)
{
    int result = parsePrimary(src);

    while (1)
    {
        const char *save = *src;
        Token op = getNextToken(src);

        if (op.type == TOKEN_PLUS)
        {
            int right = parsePrimary(src);
            result += right;
        }
        else if (op.type == TOKEN_SUB)
        {
            int right = parsePrimary(src);
            result -= right;
        }
        else
        {
            // Not an operator, restore position
            *src = save;
            break;
        }
    }

    return result;
}

// Print a single expression (string or numeric)
void printExpression(const char **src)
{
    const char *save = *src;
    Token peek = getNextToken(src);
    *src = save; // Restore position

    if (peek.type == TOKEN_STR)
    {
        // String literal
        Token str = getNextToken(src);
        printf("%s", str.text); // No newline here
    }
    else if (peek.type == TOKEN_NUM || peek.type == TOKEN_ID)
    {
        // Numeric expression
        int val = parseExpression(src);
        printf("%d", val); // No newline here
    }
    else
    {
        printf("Error: Invalid expression in print");
    }
}

void execStatement(const char **src)
{
    Token tk = getNextToken(src);

    if (tk.type == TOKEN_LET)
    {
        Token name = getNextToken(src);
        if (name.type != TOKEN_ID)
        {
            printf("Syntax Error: Expected variable name after 'let'\n");
            return;
        }

        Token eq = getNextToken(src);
        if (eq.type != TOKEN_EQUAL)
        {
            printf("Syntax Error: Expected '=' after variable name\n");
            return;
        }

        int val = parseExpression(src);
        setVar(name.text, val);

        Token semi = getNextToken(src);
        if (semi.type != TOKEN_SEMI)
        {
            printf("Syntax Error: Expected ';' after assignment\n");
            return;
        }
    }
    else if (tk.type == TOKEN_PRINT)
    {
        // Handle multiple expressions separated by spaces
        while (1)
        {
            // Check what's next
            const char *save = *src;
            Token peek = getNextToken(src);

            if (peek.type == TOKEN_SEMI)
            {
                // End of print statement
                printf("\n"); // Add newline at the end
                break;
            }
            else if (peek.type == TOKEN_STR || peek.type == TOKEN_NUM || peek.type == TOKEN_ID)
            {
                // Valid expression, restore and print it
                *src = save;
                printExpression(src);
            }
            else
            {
                printf("Syntax Error: Invalid expression in print statement\n");
                return;
            }
        }
        fflush(stdout);
    }
    else if (tk.type == TOKEN_EOF)
    {
        // End of input, do nothing
        return;
    }
    else
    {
        printf("Syntax Error: Unexpected token '%s'\n", tk.text);
        return;
    }
}