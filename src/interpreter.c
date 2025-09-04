#include <stdio.h>
#include <stdlib.h>
#include "interpreter.h"
#include "symbol.h"

extern Token getNextToken(const char **src);

// Forward declarations
double parseExpression(const char **src);
double parseTerm(const char **src);
double parseFactor(const char **src);
double parseComparison(const char **src);
void execStatement(const char **src);
void execBlock(const char **src);

// Parse a primary expression (number, variable, or parenthesized expression)
double parseFactor(const char **src)
{
    Token tk = getNextToken(src);

    if (tk.type == TOKEN_NUM)
    {
        return strtod(tk.text, NULL);
    }
    else if (tk.type == TOKEN_ID)
    {
        return getVar(tk.text);
    }
    else if (tk.type == TOKEN_LPAREN)
    {
        double val = parseComparison(src); // Allow comparisons in parentheses
        Token closing = getNextToken(src);
        if (closing.type != TOKEN_RPAREN)
        {
            printf("Syntax Error: Expected ')' after expression\n");
            return 0;
        }
        return val;
    }
    else if (tk.type == TOKEN_SUB)
    {
        return -parseFactor(src);
    }
    else if (tk.type == TOKEN_PLUS)
    {
        return parseFactor(src);
    }
    else
    {
        printf("Syntax Error: Expected number, variable, or '(' but got '%s'\n", tk.text);
        return 0;
    }
}

double parseTerm(const char **src)
{
    double val = parseFactor(src);

    while (1)
    {
        const char *save = *src;
        Token op = getNextToken(src);

        if (op.type == TOKEN_MUL)
        {
            val *= parseFactor(src);
        }
        else if (op.type == TOKEN_DIV)
        {
            double divisor = parseFactor(src);
            if (divisor == 0.0)
            {
                printf("Runtime Error: Division by zero\n");
                return 0;
            }
            val /= divisor;
        }
        else
        {
            *src = save;
            break;
        }
    }
    return val;
}

// Parse an expression with + and - operators (left associative)
double parseExpression(const char **src)
{
    double val = parseTerm(src);

    while (1)
    {
        const char *save = *src;
        Token op = getNextToken(src);

        if (op.type == TOKEN_PLUS)
        {
            val += parseTerm(src);
        }
        else if (op.type == TOKEN_SUB)
        {
            val -= parseTerm(src);
        }
        else
        {
            *src = save;
            break;
        }
    }

    return val;
}

// Parse comparison operators (==, !=, <, >, <=, >=)
double parseComparison(const char **src)
{
    double left = parseExpression(src);

    const char *save = *src;
    Token op = getNextToken(src);

    if(op.type == TOKEN_RPAREN){
        *src=save;
        return left;
    }

    if (op.type == TOKEN_EQ || op.type == TOKEN_NE ||
        op.type == TOKEN_LT || op.type == TOKEN_GT ||
        op.type == TOKEN_LE || op.type == TOKEN_GE)
    {
        double right = parseExpression(src);

        switch (op.type)
        {
        case TOKEN_EQ:
            return (left == right) ? 1.0 : 0.0;
        case TOKEN_NE:
            return (left != right) ? 1.0 : 0.0;
        case TOKEN_LT:
            return (left < right) ? 1.0 : 0.0;
        case TOKEN_GT:
            return (left > right) ? 1.0 : 0.0;
        case TOKEN_LE:
            return (left <= right) ? 1.0 : 0.0;
        case TOKEN_GE:
            return (left >= right) ? 1.0 : 0.0;
        default:
            return 0.0;
        }
    }
    else
    {
        *src = save; // Not a comparison, restore position
        return left;
    }
}

// Print a single expression (string or numeric)
void printExpression(const char **src)
{
    const char *save = *src;
    Token peek = getNextToken(src);
    *src = save; // Restore position

    if (peek.type == TOKEN_STR)
    {
        Token str = getNextToken(src);
        printf("%s", str.text);
    }
    else if (peek.type == TOKEN_NUM || peek.type == TOKEN_ID ||
             peek.type == TOKEN_LPAREN || peek.type == TOKEN_SUB ||
             peek.type == TOKEN_PLUS)
    {
        double val = parseComparison(src); // Allow comparisons in print
        printf("%g", val);
    }
    else
    {
        printf("Error: Invalid expression in print");
    }
}

// Execute a block of statements enclosed in { }
void execBlock(const char **src)
{
    Token lbrace = getNextToken(src);
    if (lbrace.type != TOKEN_LBRACE)
    {
        printf("Syntax Error: Expected '{' to start block\n");
        return;
    }

    while (1)
    {
        // Skip whitespace
        while (**src && (**src == ' ' || **src == '\t' || **src == '\n' || **src == '\r'))
            (*src)++;

        // Check for end of block
        const char *save = *src;
        Token peek = getNextToken(src);
        if (peek.type == TOKEN_RBRACE)
        {
            break; // End of block
        }
        else if (peek.type == TOKEN_EOF)
        {
            printf("Syntax Error: Expected '}' to close block\n");
            return;
        }

        *src = save; // Restore position
        execStatement(src);
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

        double val = parseComparison(src); // Allow comparisons in assignments
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
        int first = 1;
        while (1)
        {
            while (**src && (**src == ' ' || **src == '\t'))
                (*src)++;

            const char *save = *src;
            Token peek = getNextToken(src);

            if (peek.type == TOKEN_SEMI)
            {
                printf("\n");
                break;
            }
            else if (peek.type == TOKEN_STR || peek.type == TOKEN_NUM ||
                     peek.type == TOKEN_ID || peek.type == TOKEN_LPAREN ||
                     peek.type == TOKEN_SUB || peek.type == TOKEN_PLUS)
            {
                *src = save;
                if (!first)
                    printf(" ");
                printExpression(src);
                first = 0;
            }
            else if (peek.type == TOKEN_EOF)
            {
                printf("Syntax Error: Expected ';' to end print statement\n");
                return;
            }
            else
            {
                printf("Syntax Error: Invalid expression in print statement (got '%s')\n", peek.text);
                return;
            }
        }
    }
    else if (tk.type == TOKEN_IF)
    {
        Token lparen = getNextToken(src);
        if (lparen.type != TOKEN_LPAREN)
        {
            printf("Syntax Error: Expected '(' after 'if'\n");
            return;
        }

        double condition = parseComparison(src);

        Token rparen = getNextToken(src);
        if (rparen.type != TOKEN_RPAREN)
        {
            printf("Syntax Error: Expected ')' after if condition\n");
            return;
        }

        // Execute the block only if condition is true (non-zero)
        if (condition != 0.0)
        {
            execBlock(src);
        }
        else
        {
            // Skip the block
            int braceCount = 1;
            Token lbrace = getNextToken(src);
            if (lbrace.type != TOKEN_LBRACE)
            {
                printf("Syntax Error: Expected '{' after if condition\n");
                return;
            }

            while (braceCount > 0)
            {
                Token t = getNextToken(src);
                if (t.type == TOKEN_LBRACE)
                    braceCount++;
                else if (t.type == TOKEN_RBRACE)
                    braceCount--;
                else if (t.type == TOKEN_EOF)
                {
                    printf("Syntax Error: Unclosed if block\n");
                    return;
                }
            }
        }
    }
    else if (tk.type == TOKEN_EOF)
    {
        return;
    }
    else
    {
        printf("Syntax Error: Unexpected token '%s'\n", tk.text);
        return;
    }
}