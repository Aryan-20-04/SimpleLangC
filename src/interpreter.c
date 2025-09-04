#include <stdio.h>
#include <string.h>
#include "interpreter.h"
#include "symbol.h"
#include "ast.h"

#define OUTPUT_BUFFER_SIZE (1024 * 1024)
static char outputBuffer[OUTPUT_BUFFER_SIZE];
static int outputPos = 0;

void flushOutput()
{
    if (outputPos > 0)
    {
        fwrite(outputBuffer, 1, outputPos, stdout);
        outputPos = 0;
    }
}

// ------------------- AST EVALUATION -------------------

double evalExpr(ASTNode *node)
{
    if (!node)
        return 0.0;

    switch (node->type)
    {
    case NODE_NUM:
        return node->number;

    case NODE_VAR:
        // hash table lookup (O(1) average)
        return getVar(node->varName);

    case NODE_BINOP:
    {
        double l = evalExpr(node->binop.left);
        double r = evalExpr(node->binop.right);

        switch (node->binop.op)
        {
        case OP_ADD:
            return l + r;
        case OP_SUB:
            return l - r;
        case OP_MUL:
            return l * r;
        case OP_DIV:
            return r == 0.0 ? (printf("Runtime Error: Division by zero\n"), 0.0) : l / r;
        case OP_EQ:
            return l == r ? 1.0 : 0.0;
        case OP_NE:
            return l != r ? 1.0 : 0.0;
        case OP_LT:
            return l < r ? 1.0 : 0.0;
        case OP_LE:
            return l <= r ? 1.0 : 0.0;
        case OP_GT:
            return l > r ? 1.0 : 0.0;
        case OP_GE:
            return l >= r ? 1.0 : 0.0;
        default:
            return 0.0;
        }
    }

    case NODE_STR:
        return 0.0; // numeric value of string is 0

    default:
        return 0.0;
    }
}

// ------------------- AST EXECUTION -------------------

void execAST(ASTNode *node)
{
    if (!node)
        return;

    switch (node->type)
    {
    case NODE_BLOCK:
        for (int i = 0; i < node->block.count; ++i)
            execAST(node->block.items[i]);
        break;

    case NODE_ASSIGN:
        setVar(node->assign.varName, evalExpr(node->assign.value));
        break;

    case NODE_PRINT:
    {
        for (int i = 0; i < node->print.count; ++i)
        {
            ASTNode *expr = node->print.exprs[i];
            if (!expr)
                continue;

            if (expr->type == NODE_STR)
                fputs(expr->string, stdout);
            else
                printf("%g", evalExpr(expr));

            if (i < node->print.count - 1)
                putchar(' '); // separate multiple values
        }
        putchar('\n'); // newline after print
        break;
    }

    case NODE_IF:
    {
        double cond = evalExpr(node->ifstmt.cond);
        execAST(cond != 0.0 ? node->ifstmt.thenBlock : node->ifstmt.elseBlock);
        break;
    }

    case NODE_FOR:
    {
        if (node->forstmt.init)
            execAST(node->forstmt.init);

        ASTNode *condNode = node->forstmt.cond;
        ASTNode *incrNode = node->forstmt.incr;
        ASTNode *bodyNode = node->forstmt.body;

        while (!condNode || evalExpr(condNode) != 0.0)
        {
            if (bodyNode)
                execAST(bodyNode);
            if (incrNode)
                execAST(incrNode);
        }
        break;
    }

    default:
        break;
    }
}
