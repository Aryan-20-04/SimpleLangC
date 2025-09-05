#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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
static void evalArrayLiteral(struct ASTNode *arrNode, double **outData, int *outLen)
{
    *outData = NULL;
    *outLen = 0;
    if (!arrNode || arrNode->type != NODE_ARRAY)
        return;

    int n = arrNode->ArrayNode.count;
    if (n <= 0)
        return;

    double *buf = (double *)malloc(sizeof(double) * n);
    if (!buf)
    {
        printf("Runtime Error: out of memory\n");
        return;
    }

    for (int i = 0; i < n; ++i)
    {
        struct ASTNode *elem = arrNode->ArrayNode.elements[i];
        // for now: numeric-only arrays
        double v = 0.0;
        if (elem->type == NODE_STR)
        {
            printf("Type Error: string in numeric array literal\n");
            v = 0.0;
        }
        else
        {
            v = evalExpr(elem);
        }
        buf[i] = v;
    }
    *outData = buf;
    *outLen = n;
}

double evalExpr(struct ASTNode *node)
{
    if (!node)
        return 0.0;

    switch (node->type)
    {
    case NODE_NUM:
        return node->number;

    case NODE_VAR:
        // hash table lookup (O(1) average)
        if (isArray(node->varName))
            return getArrayLen(node->varName);
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
    case NODE_ARRAY:
        // arrays have no numeric value; return 0 when used in numeric context
        return 0.0;

    case NODE_ARR_ACCESS:
    {
        int idx = (int)evalExpr(node->ArrAccessNode.index);
        double val = 0.0;
        if (!getArrayElem(node->ArrAccessNode.varName, idx, &val))
            return 0.0;
        return val;
    }
    case NODE_FUNC_CALL:
    {
        if (strcmp(node->funcCall.funcName, "length") == 0)
        {
            if (node->funcCall.argCount != 1)
            {
                printf("Runtime Error: length() takes exactly 1 argument\n");
                return 0.0;
            }
            struct ASTNode *arg = node->funcCall.args[0];
            if (node->funcCall.argCount == 1)
            {
                struct ASTNode *arg = node->funcCall.args[0];
                if (arg->type == NODE_VAR)
                    return (double)getArrayLen(arg->varName);
                else if (arg->type == NODE_ARRAY)
                    return (double)arg->ArrayNode.count;
                else
                {
                    printf("Runtime Error: length() argument must be array or variable\n");
                    return 0.0;
                }
            }

            if (arg->type == NODE_VAR)
            {
                return (double)getArrayLen(arg->varName);
            }
            else
            {
                printf("Runtime Error: length() argument must be an array variable\n");
                return 0.0;
            }
        }
        printf("Runtime Error: unknown function '%s'\n", node->funcCall.funcName);
        return 0.0;
    }

    default:
        return 0.0;
    }
}

// ------------------- AST EXECUTION -------------------

void execAST(struct ASTNode *node)
{
    if (!node)
        return;

    switch (node->type)
    {
    case NODE_BLOCK:
        for (int i = 0; i < node->block.count; ++i)
            execAST(node->block.items[i]);
        break;
    case NODE_PRINT:
    {
        for (int i = 0; i < node->print.count; ++i)
        {
            struct ASTNode *expr = node->print.exprs[i];
            if (!expr)
                continue;

            if (expr->type == NODE_STR)
                fputs(expr->string, stdout);
            else if (expr->type == NODE_VAR && isArray(expr->varName))
            {
                int len = getArrayLen(expr->varName);
                printf("[");
                for (int j = 0; j < len; j++)
                {
                    double val;
                    if (getArrayElem(expr->varName, j, &val))
                        printf("%g", val);
                    else
                        printf("?");
                    if (j < len - 1)
                        printf(", ");
                }
                printf("]");
            }
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

        struct ASTNode *condNode = node->forstmt.cond;
        struct ASTNode *incrNode = node->forstmt.incr;
        struct ASTNode *bodyNode = node->forstmt.body;

        while (!condNode || evalExpr(condNode) != 0.0)
        {
            if (bodyNode)
                execAST(bodyNode);
            if (incrNode)
                execAST(incrNode);
        }
        break;
    }
    case NODE_ASSIGN:
    {
        struct ASTNode *rhs = node->assign.value;
        if (rhs && rhs->type == NODE_ARRAY)
        {
            double *data = NULL;
            int len = 0;
            evalArrayLiteral(rhs, &data, &len);
            setArray(node->assign.varName, data, len); // copies
            if (data)
                free(data); // free temp buffer
        }
        else
        {
            setVar(node->assign.varName, evalExpr(rhs));
        }
        break;
    }
    case NODE_ARR_ASSIGN:
    {
        int idx = (int)evalExpr(node->arrAssign.index);
        double val = evalExpr(node->arrAssign.value);
        if (!setArrayAt(node->arrAssign.varName, idx, val))
        {
            printf("Runtime Error: invalid array assignment %s[%d]\n",
                   node->arrAssign.varName, idx);
        }
        break;
    }

    default:
        break;
    }
}
